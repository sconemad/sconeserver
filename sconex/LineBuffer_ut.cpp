/* SconeServer (http://www.sconemad.com)

UNIT TESTS for LineBuffer

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

#include "sconex/MemFile.h"
#include "sconex/UnitTester.h"
#include "sconex/LineBuffer.h"
using namespace scx;

void LineBuffer_ut()
{
  UTSEC("general");

  MemFileBuffer mfb(1000);

  char text[] = "this is a test - nl\n"
                "another one - cr\r"
                "and another - cr nl\r\n"
                "yet another - nl cr\n\r"
                "what sillyness - nl nl\n\n"
                "more sillyness - cr nl nl\r\n\n"
                "even more sillyness - cr nl cr nl\r\n\r\n";
    
  int len = strlen(text) + 1;
  mfb.get_buffer()->push_from(text,len);

  MemFile mf(&mfb);

  LineBuffer* lb = new LineBuffer("line");
  mf.add_stream(lb);

  std::string tok;
  int seq=0;
  while (scx::Ok == lb->tokenize(tok)) {
    std::cout << " tok: " << tok << "\n";
    switch (++seq) {
      case 1:  UTEST(tok == "this is a test - nl"); break;
      case 2:  UTEST(tok == "another one - cr"); break;
      case 3:  UTEST(tok == "and another - cr nl"); break;
      case 4:  UTEST(tok == "yet another - nl cr"); break;
      case 5:  UTEST(tok == ""); break;
      case 6:  UTEST(tok == "what sillyness - nl nl"); break;
      case 7:  UTEST(tok == ""); break;
      case 8:  UTEST(tok == "more sillyness - cr nl nl"); break;
      case 9:  UTEST(tok == ""); break;
      case 10: UTEST(tok == "even more sillyness - cr nl cr nl"); break;
      case 11: UTEST(tok == ""); break;
    }
  }
  UTEST(seq == 11);
}
