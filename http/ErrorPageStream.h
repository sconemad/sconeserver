/* SconeServer (http://www.sconemad.com)

HTTP error page stream

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpErrorPageStream_h
#define httpErrorPageStream_h

#include <http/HTTPModule.h>
#include <sconex/Stream.h>
namespace http {

//=========================================================================
class ErrorPageStream : public scx::Stream {
public:

  ErrorPageStream(HTTPModule* module) 
  : Stream("errorpage"),
    m_module(module),
    m_file_mode(false)
  { };

  ~ErrorPageStream() { };

protected:

  void log(const std::string message,
	   scx::Logger::Level level = scx::Logger::Info);

  virtual scx::Condition event(scx::Stream::Event e);

  void send_basic_page();
  
private:
    
  scx::ScriptRefTo<HTTPModule> m_module;

  bool m_file_mode;
  
};

};
#endif
