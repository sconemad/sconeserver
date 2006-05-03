/* SconeServer (http://www.sconemad.com)

Generic byte buffer

For example, here's a 16 byte buffer containing 5 bytes of data:

 |---------------------size----------------------|   size=16
                                                     
  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15    head=3
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   tail=8
 |  |  |  |AA|BB|CC|DD|EE|  |  |  |  |  |  |  |  |   
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   wasted=head=3
           ^-head         ^-tail                     used=tail-head=5
                                                     free=size-tail=8
 |-wasted-|----used------|---------free----------|   
                                                     

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

#ifndef scxBuffer_h
#define scxBuffer_h

#include "sconex/sconex.h"
namespace scx {

//=============================================================================
class SCONEX_API Buffer {

public:

  Buffer(int buffer_size);
  ~Buffer();

  const void* head() const;
  // Don't copy more than used() from head()
  
  void* tail();
  // Don't copy more than free() to tail

  void push(int n);
  void pop(int n);

  int pop_to(void* a, int n);
  int push_from(const void* a, int n);
  int push_string(const std::string& s);
  
  int size() const;
  int free() const;
  int used() const;
  int wasted() const;

  void compact();
  // Rearrange buffer to remove any wasted space.
  
protected:

private:

  char* m_buffer;
  int m_size;
  int m_head;
  int m_tail;
  
};

};
#endif
