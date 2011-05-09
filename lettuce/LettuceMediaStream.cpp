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

#include "LettuceMediaStream.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/File.h>
#include <sconex/Stream.h>
#include <sconex/StreamTransfer.h>
#include <sconex/LineBuffer.h>
#include <sconex/Kernel.h>

#define PLS_PATH "/mnt/data/music/lettuce.m3u"


//=========================================================================
LettuceMediaStream::LettuceMediaStream(
  scx::Module& module
) : Stream("lettuce-media"),
    m_module(module),
    m_pls_file(0),
    m_pls_buffer(0)
{

}

//=========================================================================
LettuceMediaStream::~LettuceMediaStream()
{
  delete m_pls_file;
}
  
//=========================================================================
scx::Condition LettuceMediaStream::next_track()
{
  if (!m_pls_buffer) {
    return scx::Error;
  }
  
  scx::Condition c = scx::Ok;
  
  do {
    std::string line;
    c = m_pls_buffer->tokenize(line);
    
    if (line.size()) {
      scx::File* file; 
      file = new scx::File();
      scx::FilePath path("/mnt/data/music");
      path += line;
      if (file->open(path,scx::File::Read) == scx::Ok) {
        m_module.log("--START '" + path.path() + "'"); 
        
        scx::StreamTransfer* xfer =
          new scx::StreamTransfer(file,1024);
        xfer->set_close_when_finished(true);
        endpoint().add_stream(xfer);
        
        // Add file to kernel
        scx::Kernel::get()->connect(file,0);
        return scx::Ok;
      }

      m_module.log("--ERROR opening '" + path.path() + "'"); 
      delete file;
    }
  } while (c == scx::Ok);
  
  return c;
}

//=========================================================================
scx::Condition LettuceMediaStream::event(scx::Stream::Event e) 
{
  if (e == scx::Stream::Opening) {

    endpoint().set_timeout(scx::Time(60));

    m_pls_file = new scx::File();
    if (m_pls_file->open(PLS_PATH,scx::File::Read) != scx::Ok) {
      m_module.log("Cannot open playlist '" PLS_PATH "'"); 
      return scx::Close;
    }
    m_module.log("-START playlist '" PLS_PATH "'"); 
    
    m_pls_buffer = new scx::LineBuffer("pls");
    m_pls_file->add_stream(m_pls_buffer);
    
    return next_track();
  }      
  
  if (e == scx::Stream::Closing) {
    if (next_track() == scx::Ok) {
      return scx::End;
    }
  }
  
  m_module.log("-END playlist '" PLS_PATH "'"); 
  return scx::Ok;
}
