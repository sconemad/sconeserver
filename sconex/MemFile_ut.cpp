/* SconeServer (http://www.sconemad.com)

UNIT TESTS for MemFile

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

#include <sconex/MemFile.h>
#include <sconex/UnitTester.h>
#include <sconex/LineBuffer.h>
using namespace scx;

void MemFile_ut()
{
  UTSEC("general");

  MemFileBuffer mfb(1000);

  char text[] = "hello world\n";
    
  int len = strlen(text) + 1;
  mfb.get_buffer()->push_from(text,len);

  MemFile mf(&mfb);

  LineBuffer* lb = new LineBuffer("line");
  mf.add_stream(lb);

  std::string s;
  while (scx::Ok == lb->tokenize(s)) {
    std::cout << " tok: " << s << "\n";
  }

}
