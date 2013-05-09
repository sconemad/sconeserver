/* SconeServer (http://www.sconemad.com)

UNIT TESTS for utils

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/utils.h>
#include <sconex/UnitTester.h>
using namespace scx;

void utils_ut()
{
  std::string s;


  UTSEC("strup");

  UTCOD(s = "a"; strup(s));
  UTEST(s == "A");

  UTCOD(s = "B"; strup(s));
  UTEST(s == "B");

  UTCOD(s = "abcd"; strup(s));
  UTEST(s == "ABCD");

  UTCOD(s = "DonKeyShinDLeg"; strup(s));
  UTEST(s == "DONKEYSHINDLEG");

  UTCOD(s = "ONE 1 + TWO 2 # THREE 3 - four 4 . FIVE 5"; strup(s));
  UTEST(s == "ONE 1 + TWO 2 # THREE 3 - FOUR 4 . FIVE 5");
  

  UTSEC("strlow");

  UTCOD(s = "a"; strlow(s));
  UTEST(s == "a");

  UTCOD(s = "B"; strlow(s));
  UTEST(s == "b");

  UTCOD(s = "abcd"; strlow(s));
  UTEST(s == "abcd");

  UTCOD(s = "DonKeyShinDLeg"; strlow(s));
  UTEST(s == "donkeyshindleg");

  UTCOD(s = "ONE 1 + TWO 2 # THREE 3 - four 4 . FIVE 5"; strlow(s));
  UTEST(s == "one 1 + two 2 # three 3 - four 4 . five 5");


  UTSEC("new_c_str");

  UTCOD(char* cs1 = new_c_str("look at this"));
  UTEST(0 == strcmp(cs1,"look at this"));
  delete cs1;


  UTSEC("escape_quotes");

  UTCOD(std::string str = "dquote \"  squote '  lf \n  cr \r  bsp \b  tab \t  ff \f  a \a  v \v");
  UTCOD(std::cout << str << "\n");
  UTCOD(std::cout << escape_quotes(str) << "\n");
  UTEST(escape_quotes(str) == "dquote \\\"  squote \\'  lf \\n  cr \\r  bsp \\b  tab \\t  ff \\f  a \\a  v \\v");

  UTEST(escape_quotes("I am a \"funny\" robot that's what I am\n") == "I am a \\\"funny\\\" robot that\\'s what I am\\n");


  UTSEC("escape_html");

  UTEST(escape_html("One & One & One is > two, but < four") == "One &amp; One &amp; One is &gt; two, but &lt; four");


  UTSEC("random_hex_string");

  for (int i=0; i<100; ++i) {
    std::string str = random_hex_string(i);
    std::cout << "random_hex_string(" << i << ") = " << str << "\n";
    UTEST((int)str.length() == i);
  }
  
}
