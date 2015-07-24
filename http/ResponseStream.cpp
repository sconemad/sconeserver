/* SconeServer (http://www.sconemad.com)

HTTP Response stream

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (see the file COPYING); if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA */


#include <http/ResponseStream.h>
#include <http/Request.h>
#include <http/MessageStream.h>
#include <http/Status.h>

#include <sconex/Date.h>
#include <sconex/StreamTransfer.h>
#include <sconex/Kernel.h>
#include <sconex/File.h>

namespace http {

// Uncomment to enable debug logging
//#define RESPONSE_DEBUG_LOG(m) STREAM_DEBUG_LOG(m)

#ifndef RESPONSE_DEBUG_LOG
#  define RESPONSE_DEBUG_LOG(m)
#endif

//=========================================================================
class MimeHeaderStream : public scx::LineBuffer {

public:

  MimeHeaderStream(ResponseStream& resp)
    : scx::LineBuffer("http:mimeheader"),
      m_resp(resp),
      m_done(false)
  {
    enable_event(scx::Stream::Readable,true);
  };

  virtual scx::Condition event(scx::Stream::Event e)
  {
    if (!m_done && e == scx::Stream::Readable) {
      scx::Condition c;
      std::string line;
    
      while ( (c=tokenize(line)) == scx::Ok ) {
        if (line.empty()) {
          m_done = true;
          enable_event(scx::Stream::Readable,false);
          m_resp.mimeheader_end();
          return scx::Ok;

        } else {
          
          m_resp.mimeheader_line(line);
        }
      }
    }
    
    return scx::Ok;
  };

protected:
  
  ResponseStream& m_resp;
  bool m_done;
  
};

 
//=========================================================================
ResponseStream::ResponseStream(const std::string& stream_name)
  : scx::Stream(stream_name),
    m_resp_seq(resp_Start),
    m_buffer(1024),
    m_mime_boundary_pos(-1),
    m_mime_boundary_num(0),
    m_section_headers(),
    m_source_cond(scx::Ok)
{

}

//=========================================================================
ResponseStream::~ResponseStream()
{
  
}

//=========================================================================
std::string ResponseStream::stream_status() const
{
  std::ostringstream oss;
  oss << "seq:";
  switch (m_resp_seq) {
    case resp_Start: oss << "S"; break;
    case resp_ReadSingle: oss << "RS"; break;
    case resp_ReadMultiStart: oss << "RMS"; break;
    case resp_ReadMultiBoundary: oss << "RMBnd"; break;
    case resp_ReadMultiHeader: oss << "RMH"; break;
    case resp_ReadMultiBody: oss << "RMB"; break;
    case resp_ReadEnd: oss << "RE"; break;
    case resp_Write: oss << "W"; break;
    case resp_WriteWait: oss << "WW"; break;
    case resp_End: oss << "E"; break;
  }
  return oss.str();
}

//=========================================================================
scx::Condition ResponseStream::event(scx::Stream::Event e) 
{
  MessageStream* msg = GET_HTTP_MESSAGE();
  Request& req = const_cast<Request&>(msg->get_request());
    
  if (e == scx::Stream::Opening) {
    
    decode_param_string(req.get_uri().get_query(),req);
    
    if (req.get_method() == "POST") {
      // Need to read message body
      std::string type;
      std::string content_type = req.get_header("Content-Type");
      std::string::size_type ic = content_type.find_first_of(";");
      if (ic != std::string::npos) {
        type = content_type.substr(0,ic);
        ic = content_type.find("boundary=");
        if (ic != std::string::npos) {
          m_mime_boundary = std::string("--") + content_type.substr(ic+9);
	  m_mime_boundary_len = m_mime_boundary.size();
        }
      }
    
      if (type == "multipart/form-data") {
        m_resp_seq = resp_ReadMultiStart;
        
      } else {
        m_resp_seq = resp_ReadSingle;
      }
      enable_event(scx::Stream::Readable,true);

    } else if (req.get_method() == "PUT") {
      // Need to read message body (single entity)
      m_resp_seq = resp_ReadSingle;
      enable_event(scx::Stream::Readable,true);
      
    } else {
      // Go straight to write response
      m_resp_seq = resp_Write;
      enable_event(scx::Stream::Writeable,true);
    }
  }

  if (e == scx::Stream::Readable) {
    switch (m_resp_seq) {
      case resp_ReadSingle: {
	char buffer[1024];
	int na = 0;
	while (true) {
	  scx::Condition c = read(buffer,1024,na);
          RESPONSE_DEBUG_LOG("ReadSingle na=" << na << " c=" << c);
	  if (c == scx::Ok) {
	    m_param_string += std::string(buffer,na);

	  } else if (c == scx::Wait) {
	    break;

	  } else if (c == scx::End) {
	    decode_param_string(m_param_string,req);
	    m_resp_seq = resp_Write;
	    enable_event(scx::Stream::Readable,false);
	    enable_event(scx::Stream::Writeable,true);
	    break;

	  } else {
	    return scx::Error;
	  }
	}
      } break;

      case resp_ReadMultiStart: {
        char buffer[100];
        int na;
        while (m_resp_seq == resp_ReadMultiStart) {
          scx::Condition c = read(buffer,100,na);
          RESPONSE_DEBUG_LOG("ReadMultiStart na=" << na << " c=" << c);
          if (c == scx::End) {
            enable_event(scx::Stream::Readable,false);
            return scx::Close;
          } else if (c != scx::Ok) {
            return c;
          }
        }
      } break;

      default:
        std::cerr << "READABLE in state " << m_resp_seq << "\n";
        break;
    }
  }

  if (e == scx::Stream::Writeable) {
    if (m_resp_seq == resp_Write) {
      scx::Condition c = send_response();
      RESPONSE_DEBUG_LOG("Write c=" << c);
      switch (c) {
        case scx::Wait:
          m_resp_seq = resp_WriteWait;
          enable_event(scx::Stream::Writeable,false);
          return scx::Close;
        case scx::Ok:
          break;
        default:
          m_resp_seq = resp_End;
          return c;
      }
    }
  }
  
  if (e == scx::Stream::Closing) {
    RESPONSE_DEBUG_LOG("Response closing seq=" << m_resp_seq);
    if (m_resp_seq == resp_ReadMultiBoundary) {
      m_resp_seq = resp_ReadMultiHeader;
      m_section_headers = scx::MimeHeaderTable();
      MimeHeaderStream* mh = new MimeHeaderStream(*this);
      endpoint().add_stream(mh);
      return scx::End;

    } else if (m_resp_seq == resp_ReadEnd) {
      m_resp_seq = resp_Write;
      enable_event(scx::Stream::Readable,false);
      enable_event(scx::Stream::Writeable,true);
      return scx::End;

    } else if (m_resp_seq == resp_WriteWait) {
      m_resp_seq = resp_Write;
      enable_event(scx::Stream::Writeable,true);
      return scx::End;
      
    } else if (m_resp_seq != resp_End) {
      // Don't let the stream close until send_response has returned close.
      return scx::End;
    }
  }
  
  return scx::Ok;
}

//=========================================================================
scx::Condition ResponseStream::read(void* buffer,int n,int& na)
{
  if (m_resp_seq == resp_ReadSingle) {
    return scx::Stream::read(buffer,n,na);
  }

  na = 0;
  RESPONSE_DEBUG_LOG("read n=" << n <<
                     " buf=" << m_buffer.used() <<
                     " bpos=" << m_mime_boundary_pos);

  if (m_resp_seq == resp_ReadMultiBoundary ||
      m_resp_seq == resp_ReadEnd) {
    RESPONSE_DEBUG_LOG("boundary already reached");
    return scx::End;
  }
  
  if (m_buffer.free()) {
    // Try to fill the buffer from the source if there is some room
    int nr = 0;
    m_source_cond = scx::Stream::read(m_buffer.tail(),m_buffer.free(),nr);
    RESPONSE_DEBUG_LOG("source read " << nr << " c=" << m_source_cond);
    if (nr > 0) {
      m_buffer.push(nr);
      find_mime_boundary();
    }
  }

  if (m_mime_boundary_pos == 0) {
    // A mime boundary has been reached -
    // pop the boundary and return end

    int sz = m_mime_boundary_len + 2;

    switch (m_mime_boundary_type) {

      case bound_Initial:
        RESPONSE_DEBUG_LOG("boundary (initial)");
        m_resp_seq = resp_ReadMultiBoundary;
        break;

      case bound_Intermediate:
        RESPONSE_DEBUG_LOG("boundary");
	//	sz += 2;
        m_resp_seq = resp_ReadMultiBoundary;
        break;

      case bound_Final:
        RESPONSE_DEBUG_LOG("boundary (final)");
	//	sz += 4;
	sz += 2;
        m_resp_seq = resp_ReadEnd;
        break;
    }
    RESPONSE_DEBUG_LOG("popping boundary (" << sz << " bytes)");
    m_buffer.pop(sz);
    find_mime_boundary();
    enable_event(Stream::SendReadable,true);
    return scx::End;
  }
  
  // Calculate how much of the buffer can be read
  int allow_read = 0;
  if (m_mime_boundary_pos > 0) {
    // Read up to a detected mime boundary
    allow_read = m_mime_boundary_pos;
    RESPONSE_DEBUG_LOG("Read up to detected mime boundary (max=" << allow_read << ")");

  } else if (m_source_cond == scx::End) {
    // If the end of the buffer has been reached, then allow all to be read
    allow_read = m_buffer.used();
    RESPONSE_DEBUG_LOG("Read remaining buffer (max=" << allow_read << ")");

  } else {
    // Allow all that has been scanned to be read
    allow_read = m_buffer.used() - (m_mime_boundary_len+4) + 1;
    RESPONSE_DEBUG_LOG("Read scanned data (max=" << allow_read << ")");
  }

  if (allow_read <= 0) {
    // Cannot return any data right now
    RESPONSE_DEBUG_LOG("No data, allow_read=" << allow_read);
    //    enable_event(Stream::SendReadable,false); 
    return scx::Wait;
  }

  // Reduce if the caller wanted less data than allowed
  n = std::min(n,allow_read);
  RESPONSE_DEBUG_LOG("n=" << n << " buffer used=" << m_buffer.used());

  // Pop the required amount out of the buffer
  na += m_buffer.pop_to(buffer,n);
  m_buffer.compact();    
  RESPONSE_DEBUG_LOG("pop " << na << " / " << n << " existing");

  if (m_mime_boundary_pos > 0) {
    // Update position of detected mime boundary
    m_mime_boundary_pos -= na;
  }
  
  enable_event(Stream::SendReadable,m_buffer.used()); 
  return scx::Ok;
}

//=========================================================================
scx::Condition ResponseStream::start_section(const scx::MimeHeaderTable& headers)
{
  return scx::Ok;
}

//=========================================================================
scx::Condition ResponseStream::send_response()
{
  return scx::Close;
}

//=========================================================================
bool ResponseStream::send_file(const scx::FilePath& path)
{
  scx::File* file = new scx::File();

  if (file->open(path.path(),scx::File::Read) != scx::Ok) {
    delete file;
    return false;
  }

  // Setup transfer object into this stream
  scx::StreamTransfer* xfer = new scx::StreamTransfer(file,1024);
  xfer->set_close_when_finished(true);
  endpoint().add_stream(xfer);

  // Add file to kernel table
  scx::Kernel::get()->connect(file);
  
  return true;
}

//=========================================================================
bool ResponseStream::find_mime_boundary()
{
  m_mime_boundary_pos = -1;
  int bmax = m_mime_boundary.length();
  m_mime_boundary_len = bmax;
  int max = m_buffer.used() - bmax - 2;

  for (int i=0; i<max; ++i) {
    char* cp = (char*)m_buffer.head()+i;
    if (*cp == m_mime_boundary[0]) {
      std::string bcmp(cp,bmax);
      if (bcmp == m_mime_boundary) {
        m_mime_boundary_pos = i;
        if (m_mime_boundary_num == 0) {
          m_mime_boundary_type = bound_Initial;
	  m_mime_boundary = "\r\n" + m_mime_boundary;

        } else if (*(cp+bmax) == '-') {
          m_mime_boundary_type = bound_Final;

        } else {
          m_mime_boundary_type = bound_Intermediate;
        }
        RESPONSE_DEBUG_LOG("found boundary at " << m_mime_boundary_pos);
        ++m_mime_boundary_num;
        return true;
      }
    }
  }
  return false;
}

//=========================================================================
bool ResponseStream::decode_param_string(const std::string& str,Request& request)
{
  if (str.empty()) return true;

  std::string::size_type start = 0;
  std::string::size_type end;
  do { 
    end = str.find_first_of("&",start);
    std::string param;
    if (end == std::string::npos) {
      param = str.substr(start);
    } else {
      param = str.substr(start,end-start);
    }

    std::string name;
    std::string value;
    std::string::size_type ieq = param.find_first_of("=");
    if (ieq == std::string::npos) {
      name = scx::Uri::decode(param);
    } else {
      name = scx::Uri::decode(param.substr(0,ieq));
      value = scx::Uri::decode(param.substr(ieq+1));
    }
    RESPONSE_DEBUG_LOG("param '" << name << "'='" << value << "'");
    request.set_param(name, value);

    start = end+1;    
  } while (end != std::string::npos);

  return true;
}

//=========================================================================
void ResponseStream::mimeheader_line(const std::string& line)
{
  if (m_resp_seq == resp_ReadMultiHeader) {
    RESPONSE_DEBUG_LOG("mimeheader_line '" << line << "'");    
    m_section_headers.parse_line(line);
  }
}

//=========================================================================
void ResponseStream::mimeheader_end()
{
  if (m_resp_seq == resp_ReadMultiHeader) {
    RESPONSE_DEBUG_LOG("mimeheader_end");
    start_section(m_section_headers);
    m_resp_seq = resp_ReadMultiBody;
  }
}

};
