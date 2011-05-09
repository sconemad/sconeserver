/* SconeServer (http://www.sconemad.com)

Simple response stream

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxResponse_h
#define scxResponse_h

#include <sconex/sconex.h>
#include <sconex/StreamBuffer.h>
namespace scx {

//=============================================================================
class SCONEX_API Response : public StreamBuffer {

public:

  Response(const std::string& text);
  Response(const void* buffer,int n);
  virtual ~Response();

  virtual Condition event(Stream::Event e);

  virtual std::string stream_status() const;
  
protected:

private:

};

};
#endif
