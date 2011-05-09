/* SconeServer (http://www.sconemad.com)

Lettuce media stream

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

#ifndef lettuceMediaStream_h
#define lettuceMediaStream_h

#include <sconex/Module.h>
#include <sconex/File.h>
#include <sconex/Stream.h>
#include <sconex/StreamTransfer.h>
#include <sconex/LineBuffer.h>
#include <sconex/Kernel.h>


//=========================================================================
class LettuceMediaStream : public scx::Stream {

public:

  LettuceMediaStream(scx::Module& module);
  ~LettuceMediaStream();
  
protected:

  scx::Condition next_track();
  
  virtual scx::Condition event(scx::Stream::Event e);

private:

  scx::Module& m_module;

  scx::File* m_pls_file;
  scx::LineBuffer* m_pls_buffer;
  
};
#endif
