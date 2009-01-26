/* SconeServer (http://www.sconemad.com)

Lettuce command stream

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

#ifndef lettuceCommandStream_h
#define lettuceCommandStream_h

#include "sconex/Module.h"
#include "sconex/Stream.h"


//=========================================================================
class LettuceBuffer {

public:
  enum Type {
    LettuceBufferEmpty = 0,
    LettuceBufferBool,
    LettuceBufferUInt,
    LettuceBufferString,
    LettuceBufferBinary,
    LettuceBufferIPAddr
  };

  LettuceBuffer();
  ~LettuceBuffer();
  
  scx::Condition read(scx::Stream& stream);
  scx::Condition write(scx::Stream& stream);

  void set_type(Type type);
  Type get_type();

  scx::Arg* new_arg();
  
private:

  Type m_type;
  int m_size;
  char m_buffer[128];
  
};


//=========================================================================
class LettuceCommandStream : public scx::Stream {

public:

  LettuceCommandStream(scx::Module& module);
  ~LettuceCommandStream();
  
protected:

  virtual scx::Condition event(scx::Stream::Event e);

private:

  scx::Module& m_module;

};
#endif
