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

#ifndef httpResponseStream_h
#define httpResponseStream_h

#include "sconex/Stream.h"
#include "http/http.h"
#include "http/MessageStream.h"

namespace http {

//=========================================================================
class HTTP_API ResponseStream : public scx::Stream {

public:

  ResponseStream(const std::string& stream_name);
  
  ~ResponseStream();

  static std::string html_esc(std::string str);
  // Escape a string for html
  
protected:

  virtual scx::Condition event(scx::Stream::Event e);

  virtual scx::Condition send(http::MessageStream& msg) =0;

  bool is_opt(const std::string& name) const;
  std::string get_opt(const std::string& name) const;
  
private:

  scx::Condition decode_opts(http::MessageStream& msg);
  bool decode_opts_string(const char* cstr,int cstr_len);
  
  std::map<std::string,std::string> m_opts;

};

};
#endif
