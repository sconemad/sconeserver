/* SconeServer (http://www.sconemad.com)

UNIT TESTS for Uniform Resource Identifier

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

#include <sconex/Uri.h>
#include <sconex/UnitTester.h>
using namespace scx;
  
void Uri_ut()
{
  UTSEC("construction");
  
  UTMSG("default");
  UTCOD(Uri u1);
  UTEST(u1.get_scheme() == "");
  UTEST(u1.get_host() == "");
  UTEST(u1.get_port() == 0);
  UTEST(u1.get_path() == "");
  UTEST(u1.get_query() == "");

  UTMSG("individual args");
  UTCOD(Uri u2("ftp","somewhere.com",0,"my/made/up/path.txt"));
  UTEST(u2.get_scheme() == "ftp");
  UTEST(u2.get_host() == "somewhere.com");
  UTEST(u2.get_port() == 21); // default ftp port
  UTEST(u2.get_path() == "my/made/up/path.txt");
  UTEST(u2.get_query() == "");

  UTCOD(Uri u3("http","elsewhere.com",9753,"a/web/page.html","doit=yes&why=whynot"));
  UTEST(u3.get_scheme() == "http");
  UTEST(u3.get_host() == "elsewhere.com");
  UTEST(u3.get_port() == 9753);
  UTEST(u3.get_path() == "a/web/page.html");
  UTEST(u3.get_query() == "doit=yes&why=whynot");

  UTMSG("string parsed");
  UTCOD(Uri u4("a-very.wierd.name.net"));
  UTEST(u4.get_scheme() == "");
  UTEST(u4.get_host() == "a-very.wierd.name.net");
  UTEST(u4.get_port() == 0);
  UTEST(u4.get_path() == "");
  UTEST(u4.get_query() == "");

  UTCOD(Uri u5("https://look.at.this"));
  UTEST(u5.get_scheme() == "https");
  UTEST(u5.get_host() == "look.at.this");
  UTEST(u5.get_port() == 443); // default https port
  UTEST(u5.get_path() == "");
  UTEST(u5.get_query() == "");

  UTCOD(Uri u6("look.here/path/squiggle/wibble"));
  UTEST(u6.get_scheme() == "");
  UTEST(u6.get_host() == "look.here");
  UTEST(u6.get_port() == 0);
  UTEST(u6.get_path() == "path/squiggle/wibble");
  UTEST(u6.get_query() == "");

  UTCOD(Uri u7("xmas.day:1225/presents?hohoho=santa"));
  UTEST(u7.get_scheme() == "");
  UTEST(u7.get_host() == "xmas.day");
  UTEST(u7.get_port() == 1225);
  UTEST(u7.get_path() == "presents");
  UTEST(u7.get_query() == "hohoho=santa");

  UTCOD(Uri u8("http://elsewhere.com:9753/a/web/page.html?doit=yes&why=whynot"));
  UTEST(u8.get_scheme() == "http");
  UTEST(u8.get_host() == "elsewhere.com");
  UTEST(u8.get_port() == 9753);
  UTEST(u8.get_path() == "a/web/page.html");
  UTEST(u8.get_query() == "doit=yes&why=whynot");

  UTCOD(Uri u9("localhost:5678"));
  UTEST(u9.get_scheme() == "");
  UTEST(u9.get_host() == "localhost");
  UTEST(u9.get_port() == 5678);
  UTEST(u9.get_path() == "");
  UTEST(u9.get_query() == "");

  UTSEC("operators");

  UTMSG("equality");
  UTEST(Uri("1","2",3,"4","5") == Uri("1","2",3,"4","5"));
  UTEST(Uri("6","7",8,"9","10") == Uri("6://7:8/9?10"));
  UTEST(u8 == u3);

  UTMSG("inequality");
  UTEST(Uri("1","2",3,"4","5") != Uri("x","2",3,"4","5"));
  UTEST(Uri("1","2",3,"4","5") != Uri("1","x",3,"4","5"));
  UTEST(Uri("1","2",3,"4","5") != Uri("1","2",0,"4","5"));
  UTEST(Uri("1","2",3,"4","5") != Uri("1","2",3,"x","5"));
  UTEST(Uri("1","2",3,"4","5") != Uri("1","2",3,"4","x"));
  UTEST(u7 != u6);

  UTMSG("assignment");
  UTCOD(u6 = u7);
  UTEST(u7 == u6);
  UTEST(u6.get_scheme() == "");
  UTEST(u6.get_host() == "xmas.day");
  UTEST(u6.get_port() == 1225);
  UTEST(u6.get_path() == "presents");
  UTEST(u6.get_query() == "hohoho=santa");

  UTSEC("utils");

  UTMSG("encode");
  UTEST(Uri::encode("one two") == "one%20two");

}
