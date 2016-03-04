/* SconeServer (http://www.sconemad.com)

Simple streams

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef SimpleStreams_h
#define SimpleStreams_h

#include <sconex/Stream.h>
#include <sconex/Module.h>
#include <sconex/Buffer.h>

//=========================================================================
// EchoStream - Echo protocol (RFC862)
//
class EchoStream : public scx::Stream {
public:

  EchoStream(scx::Module* module, int buffer_size=1);
  ~EchoStream();

protected:

  virtual scx::Condition event(scx::Stream::Event e);
  virtual std::string stream_status() const;

private:
  
  scx::Module::Ref m_module;
  scx::Buffer m_buffer;
  int m_read;

};

//=========================================================================
// DiscardStream - Discard protocol (RFC863)
//
class DiscardStream : public scx::Stream {
public:
  
  DiscardStream(scx::Module* module);
  ~DiscardStream();

protected:

  virtual scx::Condition event(scx::Stream::Event e);
  
private:
  
  scx::Module::Ref m_module;

};

//=========================================================================
// ChargenStream - Character generator protocol (RFC864)
//
class ChargenStream : public scx::Stream {
public:
  
  ChargenStream(scx::Module* module);
  ~ChargenStream();

protected:

  virtual scx::Condition event(scx::Stream::Event e);

private:
  
  scx::Module::Ref m_module;

  int m_x;
  int m_s;

  static std::string s_chargen_str;
  
};

#endif
