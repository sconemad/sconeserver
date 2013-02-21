/* SconeServer (http://www.sconemad.com)

HTTP Directory index stream

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

#ifndef httpDirIndexStream_h
#define httpDirIndexStream_h

#include <http/ResponseStream.h>
#include <http/HTTPModule.h>
#include <sconex/Stream.h>
namespace http {

//=========================================================================
class DirIndexStream : public http::ResponseStream {
public:

  DirIndexStream(
    HTTPModule* module
  ) : http::ResponseStream("dirindex"),
      m_module(module)
  { };

  ~DirIndexStream() { };
  
protected:

  void log(const std::string message,
	   scx::Logger::Level level = scx::Logger::Info);

  virtual scx::Condition send_response();

private:
    
  scx::ScriptRefTo<HTTPModule> m_module;

};

};
#endif
