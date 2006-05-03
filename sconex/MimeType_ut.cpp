/* SconeServer (http://www.sconemad.com)

UNIT TESTS for MIME Type

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

#include "sconex/MimeType.h"
#include "sconex/UnitTester.h"
using namespace scx;
  
void MimeType_ut()
{
  UTSEC("construction");
  
  UTMSG("default");
  UTCOD(MimeType m1);
  UTEST(m1.get_type() == "");
  UTEST(m1.get_subtype() == "");

  UTMSG("individual args");
  UTCOD(MimeType m2("dog","fish","pigeon=chipmunk;donkey=ass"));
  UTEST(m2.get_type() == "dog");
  UTEST(m2.get_subtype() == "fish");
  UTEST(m2.get_param("pigeon") == "chipmunk");
  UTEST(m2.get_param("donkey") == "ass");

  UTMSG("string parsed");
  UTCOD(MimeType m3("chicken/gorilla"));
  UTEST(m3.get_type() == "chicken");
  UTEST(m3.get_subtype() == "gorilla");

  UTCOD(MimeType m4("ostrich/sausages;yes=please"));
  UTEST(m4.get_type() == "ostrich");
  UTEST(m4.get_subtype() == "sausages");
  UTEST(m4.get_param("yes") == "please");
  UTEST(m4.get_param("undefined") == "");


  UTSEC("operators");

  UTMSG("equality");
  UTEST(MimeType("text/plain") == MimeType("text/plain"));
  UTEST(m4 == MimeType("ostrich","sausages","yes=please"));

  UTMSG("inequality");
  UTEST(MimeType("text/plain") != MimeType("image/jpeg"));
  UTEST(MimeType("text/plain") != MimeType("text/html"));
  UTEST(MimeType("text/plain") != MimeType("video/plain"));
  UTEST(m3 != m4);

  UTMSG("assignment");
  UTCOD(m3 = m4);
  UTEST(m3 == m4);
  UTEST(m3 == MimeType("ostrich/sausages;yes=please"));
}
