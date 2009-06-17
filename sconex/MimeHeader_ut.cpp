/* SconeServer (http://www.sconemad.com)

UNIT TESTS for MIME Header

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

#include "sconex/MimeHeader.h"
#include "sconex/UnitTester.h"
using namespace scx;
  
void MimeHeader_ut()
{
  UTSEC("construction");
  
  UTMSG("default");
  UTCOD(MimeHeader mh1);
  UTEST(mh1.name() == "");

  UTSEC("parse");

  UTCOD(mh1.parse_line("Content-type: text/plain; fish=\"fowl\"; donkey=aminal"));
  UTCOD(const MimeHeaderValue* mhv1 = mh1.get_value());
  UTEST(mhv1->value() == "text/plain");
  UTCOD(std::string pfish);
  UTCOD(std::string pdonkey);
  UTCOD(mhv1->get_parameter("fish",pfish));
  UTCOD(mhv1->get_parameter("donkey",pdonkey));
  UTEST(pfish == "fowl");
  UTEST(pdonkey == "aminal");

  std::cout << mh1.get_string() << "\n";
  
}
