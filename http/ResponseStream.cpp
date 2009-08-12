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


#include "http/ResponseStream.h"
#include "http/Request.h"
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/Date.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Kernel.h"

namespace http {

// Uncomment to enable debug logging
#define RESPONSE_DEBUG_LOG(m) STREAM_DEBUG_LOG(m)

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
    m_section_header(""),
    m_prev_cond(scx::Ok)
{

}

//=========================================================================
ResponseStream::~ResponseStream()
{
  
}

//=========================================================================
std::string ResponseStream::html_esc(
  std::string str
)
{
  std::string::size_type i = 0;
  while ((i = str.find_first_of("&<>'\"",i)) != std::string::npos) {
    char c = str[i];
    str.erase(i,1);
    switch (c) {
      case '&': str.insert(i,"&amp;"); break;
      case '>': str.insert(i,"&gt;"); break;
      case '<': str.insert(i,"&lt;"); break;
      case '\'': str.insert(i,"&#39;"); break;
      case '"': str.insert(i,"&quot;"); break;
    }
    ++i;
  }
  return str;
}

//=========================================================================
scx::Condition ResponseStream::event(scx::Stream::Event e) 
{
  MessageStream* msg = GET_HTTP_MESSAGE();
  
  if (e == scx::Stream::Opening) {

    const Request& req = msg->get_request();
    
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
        }
      }
    
      if (type == "multipart/form-data") {
        m_resp_seq = resp_ReadMultiStart;
        
      } else {
        m_resp_seq = resp_ReadSingle;
      }
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
        scx::Condition c = decode_opts(*msg);
        if (c == scx::Ok) {
          m_resp_seq = resp_Write;
          enable_event(scx::Stream::Readable,false);
          enable_event(scx::Stream::Writeable,true);
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
      switch (c) {
        case scx::Wait:
          m_resp_seq = resp_WriteWait;
          enable_event(scx::Stream::Writeable,false);
          break;
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
      m_section_header = Request("");
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
  na = 0;
  RESPONSE_DEBUG_LOG("read n=" << n <<
                     " buf=" << m_buffer.used() <<
                     " bpos=" << m_mime_boundary_pos);

  if (m_resp_seq == resp_ReadMultiBoundary ||
      m_resp_seq == resp_ReadEnd) {
    RESPONSE_DEBUG_LOG("boundary already reached");
    return scx::End;
  }
  
  if (m_mime_boundary_pos == 0) {
    int sz = m_mime_boundary.size() + 2;
    switch (m_mime_boundary_type) {

      case bound_Initial:
        RESPONSE_DEBUG_LOG("boundary (initial)");
        m_resp_seq = resp_ReadMultiBoundary;
        break;

      case bound_Intermediate:
        RESPONSE_DEBUG_LOG("boundary");
        sz += 2;
        m_resp_seq = resp_ReadMultiBoundary;
        break;

      case bound_Final:
        RESPONSE_DEBUG_LOG("boundary (final)");
        sz += 4;
        m_resp_seq = resp_ReadEnd;
        break;
    }
    m_buffer.pop(sz);
    find_mime_boundary();
    enable_event(Stream::SendReadable,true);
    return scx::End;
  }
  
  if (m_mime_boundary_pos > 0) {
    if (n >= m_mime_boundary_pos) {
      // Read right upto boundary
      n = m_mime_boundary_pos;
    }
    na += m_buffer.pop_to(buffer,n);
    m_mime_boundary_pos -= na;
    enable_event(Stream::SendReadable,m_buffer.used()); 
    return scx::Ok;
  }

  const int bs = m_mime_boundary.size() + 4; // account for "--\r\n"
  const int nmax = m_buffer.size() - (2*bs) + 1;
  n = std::min(n,nmax);
  
  if (m_buffer.used()) {
    na += m_buffer.pop_to(buffer,n);
    RESPONSE_DEBUG_LOG("pop " << na << " existing");
    if (na == n) {
      enable_event(Stream::SendReadable,m_buffer.used()); 
      return scx::Ok;
    }
  }
  
  int left = n-na;
  if (left < m_buffer.free()) {
    int nr = 0;
    scx::Condition c = scx::Stream::read(m_buffer.tail(),m_buffer.free(),nr);
    m_prev_cond = c;
    RESPONSE_DEBUG_LOG("source read " << nr << " c=" << c);
    if (nr > 0) {
      m_buffer.push(nr);
      
      if (find_mime_boundary()) {
        enable_event(Stream::SendReadable,true); 
        return scx::Ok;
      }
      
      na += m_buffer.pop_to((char*)buffer+na,left);
      RESPONSE_DEBUG_LOG("pop " << na);
      
    } else if (c == scx::End) {
      m_resp_seq = resp_ReadEnd;
      RESPONSE_DEBUG_LOG("end");
    }
    
    if (na == 0) {
      enable_event(Stream::SendReadable,m_buffer.used() || c == scx::End);
      return c;
    }
  }
  
  enable_event(Stream::SendReadable,m_buffer.used()); 
  return scx::Ok;
}

//=========================================================================
scx::Condition ResponseStream::start_section(const Request& request)
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
  scx::Kernel::get()->connect(file,0);
  
  return true;
}

//=========================================================================
bool ResponseStream::find_mime_boundary()
{
  m_mime_boundary_pos = -1;
  int bmax = m_mime_boundary.length();
  int max = m_buffer.used() - bmax;
  
  for (int i=0; i<max; ++i) {
    char* cp = (char*)m_buffer.head()+i;
    if (*cp == m_mime_boundary[0]) {
      std::string bcmp(cp,bmax);
      if (bcmp == m_mime_boundary) {
        m_mime_boundary_pos = i;
        if (m_mime_boundary_num == 0) {
          m_mime_boundary_type = bound_Initial;

        } else if (*(cp+bmax) == '-') {
          m_mime_boundary_type = bound_Final;
          m_mime_boundary_pos -= 2;

        } else {
          m_mime_boundary_type = bound_Intermediate;
          m_mime_boundary_pos -= 2;
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
bool ResponseStream::is_opt(const std::string& name) const
{
  return (m_opts.find(name) != m_opts.end());
}

//=========================================================================
std::string ResponseStream::get_opt(const std::string& name) const
{
  std::map<std::string,std::string>::const_iterator it =
    m_opts.find(name);
  if (it != m_opts.end()) {
    return (*it).second;
  }
  return "";
}

//=========================================================================
scx::Condition ResponseStream::decode_opts(http::MessageStream& msg)
{
  const http::Request& req = msg.get_request();
  const scx::Uri& uri = req.get_uri();
  
  if (req.get_method() != "GET" &&
      req.get_method() != "HEAD" &&
      req.get_method() != "POST") {
    // Don't understand the method
    msg.get_response().set_status(http::Status::NotImplemented);
    return scx::Close;
  }
    
  if (req.get_method() == "POST") {
    const std::string& clength = req.get_header("Content-Length");
    if (!clength.empty()) {
      int post_str_len = atoi(clength.c_str());
      char* post_str = new char[post_str_len+1];
      int t = 0;
      while (t < post_str_len) {
        int nr;
        scx::Condition c = read(post_str+t,post_str_len-t,nr);
        if (c == scx::Error || c == scx::End) {
          delete[] post_str;
          return c;
        }
        if (nr > 0) {
          t += nr;
        }
      }
      post_str[post_str_len] = '\0';
      bool ret = decode_opts_string(post_str,post_str_len);
      delete[] post_str;
      if (!ret) {
        return scx::Error;
      }
    }
  }

  const std::string& get_str = uri.get_query();
  bool ret = decode_opts_string(get_str.c_str(),get_str.size());
  if (!ret) {
    return scx::Error;
  }
  return scx::Ok;
}

//=========================================================================
bool ResponseStream::decode_opts_string(
  const char* cstr,
  int cstr_len
)
{
  const int buflen = 1024;
  char cur_name[buflen];
  cur_name[0] = '\0';
  char cur_value[buflen];
  cur_value[0] = '\0';
  char* cur = cur_name;
  int j = 0;

  for (int i=0; i<cstr_len; ++i) {
    char a = cstr[i];
    
    if (a == '&') {
      if (cur_name[0]) {
        cur_value[j] = '\0';
        m_opts[cur_name] = cur_value;
      }
      cur_name[0] = '\0';
      cur_value[0] = '\0';
      cur = cur_name;
      j = 0;
      
    } else if (a == '=') {
      cur_name[j] = '\0';
      cur = cur_value;
      j = 0;
      
    } else if (a == '%' && i+2 < cstr_len) {
      char c = 0;
      
      a = cstr[i+1];
      if (a >= '0' && a <= '9') c += (a - '0');
      else if (a >= 'A' && a <= 'F') c += (10 + a - 'A');
      else if (a >= 'a' && a <= 'f') c += (10 + a - 'a');
      c = (c << 4);
      
      a = cstr[i+2];
      if (a >= '0' && a <= '9') c += (a - '0');
      else if (a >= 'A' && a <= 'F') c += (10 + a - 'A');
      else if (a >= 'a' && a <= 'f') c += (10 + a - 'a');
        
      cur[j++] = c;
      i += 2;
        
    } else if (a == '+') {
      cur[j++] = ' ';
      
    } else {
      cur[j++] = a;
    }
    
    if (j >= buflen-1) {
      DEBUG_LOG("Overflow");
      // Overflow
      break;
    }
    
  }

  if (cur_name[0]) {
    cur_value[j] = '\0';
    m_opts[cur_name] = cur_value;
  }

  return true;
}

//=========================================================================
void ResponseStream::mimeheader_line(const std::string& line)
{
  if (m_resp_seq == resp_ReadMultiHeader) {
    RESPONSE_DEBUG_LOG("mimeheader_line '" << line << "'");    
    m_section_header.parse_header(line);
  }
}

//=========================================================================
void ResponseStream::mimeheader_end()
{
  if (m_resp_seq == resp_ReadMultiHeader) {
    RESPONSE_DEBUG_LOG("mimeheader_end");
    start_section(m_section_header);
    m_resp_seq = resp_ReadMultiBody;
  }
}

};
