/* SconeServer (http://www.sconemad.com)

HTTP Response stream

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

namespace http {

//=========================================================================
ResponseStream::ResponseStream(
  const std::string& stream_name
) : scx::Stream(stream_name)
{
  enable_event(scx::Stream::Writeable,true);
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
  if (e == scx::Stream::Writeable) {
    scx::Condition c = decode_opts();
    if (c == scx::Ok) {
      c = send();
    }
    return c;
  }

  return scx::Ok;
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
scx::Condition ResponseStream::decode_opts()
{
  http::MessageStream* msg = 
    dynamic_cast<http::MessageStream*>(find_stream("http:message"));
  if (!msg) {
    return scx::Close;
  }
  const http::Request& req = msg->get_request();
  const scx::Uri& uri = req.get_uri();
  
  if (req.get_method() != "GET" &&
      req.get_method() != "HEAD" &&
      req.get_method() != "POST") {
    // Don't understand the method
    msg->set_status(http::Status::NotImplemented);
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

};
