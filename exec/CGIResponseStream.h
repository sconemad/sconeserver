/* SconeServer (http://www.sconemad.com)

CGI Response stream

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

#ifndef execCGIResponseStream_h
#define execCGIResponseStream_h

#include "ExecModule.h"
#include <sconex/LineBuffer.h>

namespace http { class MessageStream; };

//=========================================================================
class CGIResponseStream : public scx::LineBuffer {

public:

  CGIResponseStream(
    http::MessageStream* http_msg
  );

  ~CGIResponseStream();
  
protected:

  virtual scx::Condition event(scx::Stream::Event e);

private:

  http::MessageStream* m_http_msg;
  bool m_done_headers;
  
};

#endif
