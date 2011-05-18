/* SconeServer (http://www.sconemad.com)

UNIT TESTS for FilePath

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

#include <sconex/FilePath.h>
#include <sconex/UnitTester.h>
using namespace scx;
  
void FilePath_ut()
{
  UTSEC("construction");
  
  UTMSG("default");
  UTCOD(FilePath fp);
  UTEST(fp.path() == "");

  UTMSG("initialised");
  UTCOD(FilePath fpr("/the"));
  UTEST(fpr.path() == "/the");


  UTSEC("operators");

  UTMSG("+=");

  UTCOD(fp += "one");
  UTEST(fp.path() == "one");
  UTCOD(fp += "two");
  UTEST(fp.path() == "one/two");

  UTCOD(fpr += "quick");
  UTEST(fpr.path() == "/the/quick");
  UTCOD(fpr += "brown/fox/");
  UTEST(fpr.path() == "/the/quick/brown/fox");

  UTMSG("+");

  UTCOD(FilePath fp2 = fp + fpr);
  UTEST(fp2 == "/the/quick/brown/fox");

  UTCOD(FilePath fp3 = fpr + fp);
  UTEST(fp3 == "/the/quick/brown/fox/one/two");

  UTMSG("==");

  UTEST(fp == fp);
  UTEST(fpr == fpr);
  UTEST(fp2 == fp + fpr);
  UTEST(fp3 == fpr + fp);


  UTSEC("methods");

  UTMSG("pop");

  UTEST(fp.pop() == "two");
  UTEST(fp.path() == "one");

  UTEST(fp.pop() == "one");
  UTEST(fp.path() == "");

  UTEST(fp.pop() == "");
  UTEST(fp.path() == "");

  UTEST(fpr.pop() == "fox");
  UTEST(fpr.path() == "/the/quick/brown");

  UTEST(fpr.pop() == "brown");
  UTEST(fpr.path() == "/the/quick");

  UTEST(fpr.pop() == "quick");
  UTEST(fpr.path() == "/the");

  UTEST(fpr.pop() == "the");
  UTEST(fpr.path() == "/");

  UTEST(fpr.pop() == "");
  UTEST(fpr.path() == "/");
}
