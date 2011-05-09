/* SconeServer (http://www.sconemad.com)

Test Builder Control Stream

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

#ifndef testbuilderControlStream_h
#define testbuilderControlStream_h

#include "TestBuilderModule.h"
#include <sconex/Stream.h>
#include "http/ResponseStream.h"

//=========================================================================
class TestBuilderControlStream : public http::ResponseStream {

public:

  TestBuilderControlStream(TestBuilderModule& module);
  
  ~TestBuilderControlStream();
  
protected:

  virtual scx::Condition send(http::MessageStream& msg);

private:
  
  TestBuilderModule::Ref m_module;

};

#endif
