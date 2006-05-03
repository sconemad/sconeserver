/* SconeServer (http://www.sconemad.com)

UNIT TESTS for Version tag

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

#include "sconex/VersionTag.h"
#include "sconex/UnitTester.h"
using namespace scx;
  
void VersionTag_ut()
{
  UTSEC("construction");
  
  UTMSG("default");
  UTCOD(VersionTag vp1);
  UTEST(vp1.get_major() < 0);
  UTEST(vp1.get_minor() < 0);
  UTEST(vp1.get_sub() < 0);

  UTMSG("with args");
  UTCOD(VersionTag vp2(101,202,303));
  UTEST(vp2.get_major() == 101);
  UTEST(vp2.get_minor() == 202);
  UTEST(vp2.get_sub() == 303);


  UTSEC("operators");

  UTMSG("equality");
  UTEST(VersionTag(1,0) == VersionTag(1,0));
  UTEST(VersionTag(1,0,6) == VersionTag(1,0,6));
  UTEST(VersionTag(0,0,0) == VersionTag(0,0,0));
  UTEST(VersionTag(33,44,55) == VersionTag(33,44,55));

  UTMSG("inequality");
  UTEST(VersionTag(1,0) != VersionTag(1,1));
  UTEST(VersionTag(1,0) != VersionTag(1,0,2));
  UTEST(VersionTag(0,0) != VersionTag(0,1));
  UTEST(VersionTag(42,42) != VersionTag(42,41));

  UTMSG("greater than");
  UTEST(VersionTag(1,0) > VersionTag(0,0));
  UTEST(VersionTag(10,0) > VersionTag(0,0));
  UTEST(VersionTag(1,1) > VersionTag(1,0));
  UTEST(VersionTag(2,32) > VersionTag(2,4));
  UTEST(VersionTag(9,1) > VersionTag(8,100));
  UTEST(VersionTag(9,1,70) > VersionTag(9,1,9));

  UTMSG("greater than or equal to");
  UTEST(VersionTag(9,1) >= VersionTag(8,100));
  UTEST(VersionTag(9,1) >= VersionTag(9,1));

  UTMSG("less than");
  UTEST(VersionTag(1,0) < VersionTag(2,0));
  UTEST(VersionTag(10,0) < VersionTag(13,0));
  UTEST(VersionTag(6,5) < VersionTag(7,6));
  UTEST(VersionTag(2,2) < VersionTag(2,12));
  UTEST(VersionTag(9,1) < VersionTag(12,0));
  UTEST(VersionTag(9,1,7) < VersionTag(9,1,10));

  UTMSG("less than or equal to");
  UTEST(VersionTag(2,2) <= VersionTag(2,12));
  UTEST(VersionTag(9,1) <= VersionTag(9,1));
  
  UTMSG("assignment");
  UTCOD(vp1 = vp2);
  UTEST(vp1.get_major() == 101);
  UTEST(vp1.get_minor() == 202);
  UTEST(vp1.get_sub() == 303);
}
