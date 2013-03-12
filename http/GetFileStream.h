/* SconeServer (http://www.sconemad.com)

HTTP Get file stream

Stream to send a disk file via HTTP.

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

#ifndef httpGetFileStream_h
#define httpGetFileStream_h

#include <http/HTTPModule.h>
#include <sconex/Stream.h>
namespace http {

//=========================================================================
class HTTP_API GetFileStream : public scx::Stream {
public:

  GetFileStream(
    HTTPModule* module
  ) : Stream("getfile"),
      m_module(module)
  { };

  ~GetFileStream() { };
  
protected:

  virtual scx::Condition event(scx::Stream::Event e);

private:

  scx::ScriptRefTo<HTTPModule> m_module;

};

};
#endif
