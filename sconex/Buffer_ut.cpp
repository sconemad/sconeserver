/* SconeServer (http://www.sconemad.com)

UNIT TESTS for Buffer

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

#include <sconex/Buffer.h>
#include <sconex/UnitTester.h>
using namespace scx;

char test_data(int i)
{
  return (i % 256);
}

int write_seq=0;
void Buffer_write(Buffer& b,int n)
{
  std::cout << " writing " << n << " (" << b.free() << ")\n";
  n = std::min(n,b.free());
  for (int i=0; i<n; ++i) {
    *(((char*)b.tail())+i) = test_data(write_seq++);
  }
  b.push(n);
}

int read_seq=0;
void Buffer_read(Buffer& b,int n)
{
  std::cout << " reading " << n << " (" << b.used() << ")\n";
  n = std::min(n,b.used());
  bool check=true;
  for (int i=0; i<n; ++i) {
    if (*(((char*)b.head())+i) != test_data(read_seq++)) {
      if (check) std::cout << " data corruption at ";
      std::cout << i << " ";
      check = false;
    }
  }
  if (!check) std::cout << "\n";
  UTEST(check);
  b.pop(n);
}

void Buffer_ut()
{
  UTSEC("general");
  int i;
  
  for (i=1; i<100000; i = (int)((double)i*2.8)) {
    std::cout << " testing with buffer size = " << i << "\n";
    UTCOD(Buffer b(i));
    UTEST(b.size() == i);
    UTEST(b.free() == i);
    UTEST(b.used() == 0);
    UTEST(b.wasted() == 0);
    read_seq = write_seq;

    for (int j=1; j<10; ++j) {
      for (int k=1; k<2*i; k = (int)((double)k*2.8)) {
        if ((j+k) % 3 == 0) {
          Buffer_write(b,k);
        }
        if ((j+k) % 4 == 0) {
          Buffer_read(b,k);
        }
      }
    }
  }
}
