/* SconeServer (http://www.sconemad.com)

HTTP Partial Response Stream

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#include <http/PartialResponseStream.h>
#include <http/MessageStream.h>
#include <http/Request.h>

#include <sconex/sconex.h>

namespace http {
  
// Uncomment to enable debug logging
//#define PARTIALSTREAM_DEBUG_LOG(m) STREAM_DEBUG_LOG(m)

#ifndef PARTIALSTREAM_DEBUG_LOG
#  define PARTIALSTREAM_DEBUG_LOG(m)
#endif

//=============================================================================
PartialResponseStream::PartialResponseStream(
  HTTPModule& module
)
  : scx::Stream("http:partialresponse"),
    m_module(module),
    m_range_start(-1),
    m_range_length(0),
    m_position(0),
    m_content_length(-1),
    m_error(false)
{

}

//=============================================================================
PartialResponseStream::~PartialResponseStream()
{

}

//=============================================================================
scx::Condition PartialResponseStream::write(const void* buffer,int n,int& na)
{
  na = 0;

  if (m_position == 0) {
    http::MessageStream* msg = GET_HTTP_MESSAGE();
    const http::Request& req = msg->get_request();
    http::Response& resp = msg->get_response();

    std::string range = req.get_header("Range");
    if (range.empty()) {
      // Not a partial request, so we don't need to go any further
      PARTIALSTREAM_DEBUG_LOG("No range header - not a partial response");
      m_position = -1;
      return scx::End;

    } else {
      std::string content_len_str = resp.get_header("Content-Length");
      if (!content_len_str.empty()) {
	m_content_length = atoi(content_len_str.c_str());
      }
      
      const std::string bytes = "bytes=";
      std::string::size_type ib = range.find(bytes);
      if (ib == std::string::npos) {
	// Cannot find bytes header
	DEBUG_LOG("Invalid range header - ignoring");
	m_position = -1;
	return scx::End;

      } else {
	range = range.substr(ib+bytes.length());
	std::string::size_type ih = range.find_first_of("-");
	if (ih == std::string::npos) {
	  DEBUG_LOG("Invalid range header - ignoring");
	  m_position = -1;
	  return scx::End;

	} else {
	  std::string before_str = range.substr(0,ih);
	  std::string after_str;
	  if (ih < range.length()) after_str = range.substr(ih+1);
	  
	  if (!before_str.empty()) {
	    m_range_start = atoi(before_str.c_str());
	  }
	  if (!after_str.empty()) {
	    int after = atoi(after_str.c_str());
	    if (before_str.empty()) {
	      if (m_content_length < 0) {
		DEBUG_LOG("Cannot calculate range as content length is unknown");
		resp.set_status(Status::RequestedRangeNotSatisfiable);
		m_error = true;
	      }
	      m_range_length = after;
	      m_range_start = m_content_length - m_range_length;
	    } else {
	      m_range_length = after + 1 - m_range_start;
	    }
	  } else {
	    if (m_content_length < 0) {
	      DEBUG_LOG("Cannot calculate range as content length is unknown");
	      resp.set_status(Status::RequestedRangeNotSatisfiable);
	      m_error = true;
	    }
	    m_range_length = m_content_length - m_range_start;
	  }
	  PARTIALSTREAM_DEBUG_LOG("New range request: start:" << m_range_start 
				  << ", length:" << m_range_length 
				  << ", total: " << m_content_length);

	  if (m_range_start < 0 || m_range_length < 0) {
	    DEBUG_LOG("Invalid range specification");
	    resp.set_status(Status::RequestedRangeNotSatisfiable);
	    m_error = true;

	  } else if ((m_content_length >= 0) && (m_range_start + m_range_length > m_content_length)) {
	    DEBUG_LOG("Invalid range specification - exceeds content length");
	    resp.set_status(Status::RequestedRangeNotSatisfiable);
	    m_error = true;

	  } else {

	    // Set the status to indicate a partial response
	    resp.set_status(Status::PartialContent);
	    
	    // Update the content length to what we are actually sending
	    std::ostringstream oss;
	    oss << m_range_length;
	    resp.set_header("Content-Length",oss.str());
	    
	    // Add the content-range header to confirm the range
	    oss.str("");
	    oss << "bytes " << m_range_start << "-" << (m_range_start + m_range_length - 1);
	    if (m_content_length >= 0) oss << "/" << m_content_length;
	    resp.set_header("Content-Range",oss.str());
	  }
	}
      }
    }    
  }

  if (m_error) {
    // A range error occurred, so don't send the message body
    if (m_position == 0) {
      http::MessageStream* msg = GET_HTTP_MESSAGE();
      const http::Request& req = msg->get_request();
      http::Response& resp = msg->get_response();

      resp.remove_header("Content-Length");
      m_position = -1;
      Stream::write("\n");
    }
    na = n;
    return scx::Ok;
  }
  
  if (m_position < 0) {
    // Not a partial response
    return Stream::write(buffer,n,na);
  }

  PARTIALSTREAM_DEBUG_LOG("Write pos: " << m_position << ", size: " << n << " bytes");

  if (m_position + n < m_range_start) {
    // Discard (before start of range)
    na += n;
    m_position += n;
    PARTIALSTREAM_DEBUG_LOG("IGNORE (before): all " << n << " bytes");

  } else if (m_position > m_range_start + m_range_length) {
    // Discard (past end of range)
    na += n;
    m_position += n;
    PARTIALSTREAM_DEBUG_LOG("IGNORE (after): all " << n << " bytes");

  } else {
    
    int start_pos = 0;
    if (m_position < m_range_start) {
      // Start overlap
      start_pos = m_range_start - m_position;
      PARTIALSTREAM_DEBUG_LOG("IGNORE (before): " << start_pos << " bytes");
    }

    int length = n - start_pos;
    int postfix = 0;
    if ((m_position + n) > (m_range_start + m_range_length)) {
      length = m_range_start + m_range_length - m_position - start_pos;
      postfix = m_position + n - (m_range_start + m_range_length);
    }

    na += start_pos;
    m_position += start_pos;
    
    int nw = 0;
    if (length > 0) {
      Stream::write((char*)buffer+start_pos,length,nw);
      PARTIALSTREAM_DEBUG_LOG("PASSED: " << nw << " / " << length << " bytes");
    }
    
    if (postfix && length == nw) {
      PARTIALSTREAM_DEBUG_LOG("IGNORE (after): " << postfix << " bytes");
      na += postfix;
      m_position += postfix;
    }

    na += nw;
    m_position += nw;

  }
  
  return scx::Ok;
}

//=============================================================================
std::string PartialResponseStream::stream_status() const
{
  std::ostringstream oss;
  oss << m_position;
  oss << " " << m_range_start << "-" << (m_range_start + m_range_length);
  if (m_content_length >= 0) oss << "/" << m_content_length;
  return oss.str();
}

};
