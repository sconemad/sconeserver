/* SconeServer (http://www.sconemad.com)

Sconex memory file

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxMemFile_h
#define scxMemFile_h

#include "sconex/sconex.h"
#include "sconex/Descriptor.h"
#include "sconex/Buffer.h"

namespace scx {

class MemFileBuffer;
class File;
  
//=============================================================================
class SCONEX_API MemFile : public Descriptor {

public:

  MemFile(const MemFileBuffer* buffer);
  virtual ~MemFile();

  virtual void close();

  long tell() const;
  // Get the current file position

  long size() const;
  // Get the file size

protected:

  virtual int event_create();

  virtual int fd();

  virtual Condition endpoint_read(void* buffer,int n,int& na);
  virtual Condition endpoint_write(const void* buffer,int n,int& na);

  const MemFileBuffer* m_buffer;
  int m_pos;

private:

};

//=============================================================================
class SCONEX_API MemFileBuffer {

public:

  MemFileBuffer(int buffer_size);
  MemFileBuffer(File& file);
    
  ~MemFileBuffer();
  
  const Buffer* get_buffer() const;
  Buffer* get_buffer();

private:

  Buffer* m_buffer;
  
};
  

};
#endif
