/* SconeServer (http://www.sconemad.com)

UNIT TESTS for basic SconeScript objects

Copyright (c) 2000-2007 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/Arg.h"
#include "sconex/UnitTester.h"
using namespace scx;

void test_op(
  const Arg& left,
  Arg::OpType optype,
  const std::string& opname,
  const Arg& right,
  const Arg& expected)
{
  UTCOD(Arg* result = const_cast<Arg&>(left).op(optype,opname,&const_cast<Arg&>(right)));
  UTEST(result != 0);
  std::cout << " result '" << result->get_string()
            << "' type '" << typeid(*result).name() << "'\n";
  std::cout << " expected '" << expected.get_string()
            << "' type '" << typeid(expected).name() << "'\n";
  UTEST(typeid(*result) == typeid(expected));
  UTEST(result->get_int() == expected.get_int());
  UTEST(result->get_string() == expected.get_string());
  delete result;
}

void test_op(
  const Arg& left,
  const std::string& opname,
  const Arg& right,
  const Arg& expected)
{
  test_op(left,Arg::Binary,opname,right,expected);
}

void test_op(
  const std::string& opname,
  const Arg& arg,
  const Arg& expected)
{
  ArgInt dummy(0);
  test_op(arg,Arg::Prefix,opname,dummy,expected);
}

void test_op(
  const Arg& arg,
  const std::string& opname,
  const Arg& expected)
{
  ArgInt dummy(0);
  test_op(arg,Arg::Postfix,opname,dummy,expected);
}

void test_ArgString()
{
  UTSEC("ArgString");
  
  UTMSG("construction");

  UTCOD(ArgString as1(""));
  UTEST(as1.get_string() == "");
  UTEST(as1.get_int() == 1);

  UTCOD(ArgString as2("fish1234"));
  UTEST(as2.get_string() == "fish1234");
  UTEST(as2.get_int() == 1);

  UTCOD(ArgString as3(std::string("fowl5678")));
  UTEST(as3.get_string() == "fowl5678");
  UTEST(as3.get_int() == 1);

  UTMSG("copy");
  
  UTCOD(ArgString as4(as2));
  UTEST(as4.get_string() == as2.get_string());
  UTEST(as4.get_int() == 1);

  UTCOD(Arg* a1 = as4.new_copy());
  UTEST(a1 != 0);
  UTEST(a1->get_string() == as4.get_string());
  UTEST(a1->get_int() == 1);
  
  UTCOD(ArgString* as5 = dynamic_cast<ArgString*>(a1));
  UTEST(as5 != 0);
  UTEST(as5->get_string() == as4.get_string());
  UTEST(as5->get_int() == 1);

  delete a1;

  UTMSG("ops");

  UTCOD(test_op(as1,"+",as1,ArgString("")));
  UTCOD(test_op(as1,"+",as2,ArgString("fish1234")));
  UTCOD(test_op(as2,"+",as3,ArgString("fish1234fowl5678")));
  UTCOD(test_op(as3,"+",as2,ArgString("fowl5678fish1234")));

  UTCOD(test_op(as1,"==",as1,ArgInt(1)));
  UTCOD(test_op(as2,"==",as2,ArgInt(1)));
  UTCOD(test_op(as2,"==",as3,ArgInt(0)));
  UTCOD(test_op(as1,"==",as3,ArgInt(0)));

  UTCOD(test_op(as1,"!=",as1,ArgInt(0)));
  UTCOD(test_op(as2,"!=",as2,ArgInt(0)));
  UTCOD(test_op(as2,"!=",as3,ArgInt(1)));
  UTCOD(test_op(as1,"!=",as3,ArgInt(1)));
}

void test_ArgInt()
{
  UTSEC("ArgInt");

  UTMSG("construction");

  UTCOD(ArgInt ai1(0));
  UTEST(ai1.get_int() == 0);
  UTEST(ai1.get_string() == "0");

  UTCOD(ArgInt ai2(12345));
  UTEST(ai2.get_int() == 12345);
  UTEST(ai2.get_string() == "12345");

  UTCOD(ArgInt ai3(-67890));
  UTEST(ai3.get_int() == -67890);
  UTEST(ai3.get_string() == "-67890");

  UTMSG("copy");

  UTCOD(ArgInt ai4(ai2));
  UTEST(ai4.get_int() == ai2.get_int());

  UTCOD(Arg* a1 = ai4.new_copy());
  UTEST(a1 != 0);
  UTEST(a1->get_int() == 12345);
  
  UTCOD(ArgInt* ai5 = dynamic_cast<ArgInt*>(a1));
  UTEST(ai5 != 0);
  UTEST(ai5->get_string() == ai4.get_string());
  UTEST(ai5->get_int() == 12345);

  delete a1;
  
  UTMSG("prefix ops");

  UTCOD(test_op("+",ai1,ArgInt(0)));
  UTCOD(test_op("+",ai2,ArgInt(12345)));
  UTCOD(test_op("+",ai3,ArgInt(-67890)));

  UTCOD(test_op("-",ai1,ArgInt(0)));
  UTCOD(test_op("-",ai2,ArgInt(-12345)));
  UTCOD(test_op("-",ai3,ArgInt(67890)));

  UTMSG("postfix ops");

  UTCOD(test_op(ArgInt(0),"!",ArgInt(1)));
  UTCOD(test_op(ArgInt(1),"!",ArgInt(1)));
  UTCOD(test_op(ArgInt(2),"!",ArgInt(2)));
  UTCOD(test_op(ArgInt(3),"!",ArgInt(6)));
  UTCOD(test_op(ArgInt(4),"!",ArgInt(24)));
  UTCOD(test_op(ArgInt(5),"!",ArgInt(120)));

  UTMSG("binary ops");
  
  UTCOD(test_op(ai1,"+",ai1,ArgInt(0)));
  UTCOD(test_op(ai1,"+",ai2,ArgInt(12345)));
  UTCOD(test_op(ai2,"+",ai3,ArgInt(12345-67890)));
  UTCOD(test_op(ai3,"+",ai2,ArgInt(-67890+12345)));

  UTCOD(test_op(ai1,"-",ai1,ArgInt(0)));
  UTCOD(test_op(ai1,"-",ai2,ArgInt(-12345)));
  UTCOD(test_op(ai2,"-",ai3,ArgInt(12345+67890)));
  UTCOD(test_op(ai3,"-",ai2,ArgInt(-67890-12345)));

  UTCOD(test_op(ai1,"*",ai1,ArgInt(0)));
  UTCOD(test_op(ai1,"*",ai2,ArgInt(0)));
  UTCOD(test_op(ai2,"*",ai3,ArgInt(12345*-67890)));
  UTCOD(test_op(ai3,"*",ai2,ArgInt(-67890*12345)));

  UTCOD(test_op(ai1,"/",ai1,ArgError("Divide by zero")));
  UTCOD(test_op(ai1,"/",ai2,ArgInt(0)));
  UTCOD(test_op(ai2,"/",ai3,ArgInt(12345/-67890)));
  UTCOD(test_op(ai3,"/",ai2,ArgInt(-67890/12345)));

  UTCOD(test_op(ai1,">",ai1,ArgInt(0)));
  UTCOD(test_op(ai2,">",ai1,ArgInt(1)));
  UTCOD(test_op(ai3,">",ai2,ArgInt(0)));
  UTCOD(test_op(ai2,">",ai3,ArgInt(1)));
  UTCOD(test_op(ai3,">",ai3,ArgInt(0)));

  UTCOD(test_op(ai1,"<",ai1,ArgInt(0)));
  UTCOD(test_op(ai2,"<",ai1,ArgInt(0)));
  UTCOD(test_op(ai3,"<",ai2,ArgInt(1)));
  UTCOD(test_op(ai2,"<",ai3,ArgInt(0)));
  UTCOD(test_op(ai3,"<",ai3,ArgInt(0)));

  UTCOD(test_op(ai1,">=",ai1,ArgInt(1)));
  UTCOD(test_op(ai2,">=",ai1,ArgInt(1)));
  UTCOD(test_op(ai3,">=",ai2,ArgInt(0)));
  UTCOD(test_op(ai2,">=",ai3,ArgInt(1)));
  UTCOD(test_op(ai3,">=",ai3,ArgInt(1)));

  UTCOD(test_op(ai1,"<=",ai1,ArgInt(1)));
  UTCOD(test_op(ai2,"<=",ai1,ArgInt(0)));
  UTCOD(test_op(ai3,"<=",ai2,ArgInt(1)));
  UTCOD(test_op(ai2,"<=",ai3,ArgInt(0)));
  UTCOD(test_op(ai3,"<=",ai3,ArgInt(1)));
  
  UTCOD(test_op(ai1,"==",ai1,ArgInt(1)));
  UTCOD(test_op(ai2,"==",ai2,ArgInt(1)));
  UTCOD(test_op(ai2,"==",ai3,ArgInt(0)));
  UTCOD(test_op(ai1,"==",ai3,ArgInt(0)));

  UTCOD(test_op(ai1,"!=",ai1,ArgInt(0)));
  UTCOD(test_op(ai2,"!=",ai2,ArgInt(0)));
  UTCOD(test_op(ai2,"!=",ai3,ArgInt(1)));
  UTCOD(test_op(ai1,"!=",ai3,ArgInt(1)));

  UTCOD(test_op(ArgInt(0),"&",ArgInt(0),ArgInt(0)));
  UTCOD(test_op(ArgInt(1),"&",ArgInt(0),ArgInt(0)));
  UTCOD(test_op(ArgInt(0),"&",ArgInt(1),ArgInt(0)));
  UTCOD(test_op(ArgInt(1),"&",ArgInt(1),ArgInt(1)));

  UTCOD(test_op(ArgInt(0),"|",ArgInt(0),ArgInt(0)));
  UTCOD(test_op(ArgInt(1),"|",ArgInt(0),ArgInt(1)));
  UTCOD(test_op(ArgInt(0),"|",ArgInt(1),ArgInt(1)));
  UTCOD(test_op(ArgInt(1),"|",ArgInt(1),ArgInt(1)));

  UTCOD(test_op(ArgInt(0),"xor",ArgInt(0),ArgInt(0)));
  UTCOD(test_op(ArgInt(1),"xor",ArgInt(0),ArgInt(1)));
  UTCOD(test_op(ArgInt(0),"xor",ArgInt(1),ArgInt(1)));
  UTCOD(test_op(ArgInt(1),"xor",ArgInt(1),ArgInt(0)));
}

void test_ArgReal()
{
  UTSEC("ArgReal");

  UTMSG("construction");

  UTCOD(ArgReal ar1(0));
  UTEST(ar1.get_int() == 0);
  UTEST(ar1.get_string() == "0");

  UTCOD(ArgReal ar2(1.2345));
  UTEST(ar2.get_int() == 1);
  UTEST(ar2.get_string() == "1.2345");

  UTCOD(ArgReal ar3(-6.7890));
  UTEST(ar3.get_int() == -6);
  UTEST(ar3.get_string() == "-6.789");

  UTMSG("copy");

  UTCOD(ArgReal ar4(ar2));
  UTEST(ar4.get_int() == ar2.get_int());

  UTCOD(Arg* a1 = ar4.new_copy());
  UTEST(a1 != 0);
  UTEST(a1->get_string() == "1.2345");
  
  UTCOD(ArgReal* ar5 = dynamic_cast<ArgReal*>(a1));
  UTEST(ar5 != 0);
  UTEST(ar5->get_string() == ar4.get_string());
  UTEST(ar5->get_string() == "1.2345");

  delete a1;
  
  UTMSG("ops");

  UTCOD(test_op(ar1,"+",ar1,ArgReal(0.0)));
  UTCOD(test_op(ar1,"+",ar2,ArgReal(1.2345)));
  UTCOD(test_op(ar2,"+",ar3,ArgReal(1.2345-6.7890)));
  UTCOD(test_op(ar3,"+",ar2,ArgReal(-6.7890+1.2345)));

  UTCOD(test_op(ar1,"-",ar1,ArgReal(0.0)));
  UTCOD(test_op(ar1,"-",ar2,ArgReal(-1.2345)));
  UTCOD(test_op(ar2,"-",ar3,ArgReal(1.2345+6.7890)));
  UTCOD(test_op(ar3,"-",ar2,ArgReal(-6.7890-1.2345)));

  UTCOD(test_op(ar1,"*",ar1,ArgReal(0.0)));
  UTCOD(test_op(ar1,"*",ar2,ArgReal(0.0)));
  UTCOD(test_op(ar2,"*",ar3,ArgReal(1.2345*-6.789)));
  UTCOD(test_op(ar3,"*",ar2,ArgReal(-6.789*1.2345)));

  UTCOD(test_op(ar1,"/",ar1,ArgReal(0.0/0.0)));
  UTCOD(test_op(ar1,"/",ar2,ArgReal(0.0)));
  UTCOD(test_op(ar2,"/",ar3,ArgReal(1.2345/-6.7890)));
  UTCOD(test_op(ar3,"/",ar2,ArgReal(-6.789/1.2345)));

  UTCOD(test_op(ar1,"==",ar1,ArgInt(1)));
  UTCOD(test_op(ar2,"==",ar2,ArgInt(1)));
  UTCOD(test_op(ar2,"==",ar3,ArgInt(0)));
  UTCOD(test_op(ar1,"==",ar3,ArgInt(0)));

  UTCOD(test_op(ar1,"!=",ar1,ArgInt(0)));
  UTCOD(test_op(ar2,"!=",ar2,ArgInt(0)));
  UTCOD(test_op(ar2,"!=",ar3,ArgInt(1)));
  UTCOD(test_op(ar1,"!=",ar3,ArgInt(1)));
}

void test_ArgList()
{
  UTSEC("ArgList");

  UTMSG("construction");

  UTCOD(ArgList al1);
  UTEST(al1.get_int() == 0);
  UTEST(al1.get_string() == "()");
  UTEST(al1.size() == 0);

  UTMSG("add elements to end");
  
  UTCOD(al1.give(new ArgInt(1)));
  UTEST(al1.get_int() == 1);
  UTEST(al1.get_string() == "(1)");
  UTEST(al1.size() == 1);

  UTCOD(al1.give(new ArgInt(2)));
  UTEST(al1.get_int() == 2);
  UTEST(al1.get_string() == "(1,2)");
  UTEST(al1.size() == 2);

  UTCOD(al1.give(new ArgInt(3)));
  UTEST(al1.get_int() == 3);
  UTEST(al1.get_string() == "(1,2,3)");
  UTEST(al1.size() == 3);

  UTCOD(al1.give(new ArgInt(4)));
  UTEST(al1.get_int() == 4);
  UTEST(al1.get_string() == "(1,2,3,4)");
  UTEST(al1.size() == 4);

  UTMSG("insert elements");

  UTCOD(al1.give(new ArgInt(0),0));
  UTEST(al1.get_int() == 5);
  UTEST(al1.get_string() == "(0,1,2,3,4)");
  UTEST(al1.size() == 5);

  UTCOD(al1.give(new ArgInt(5),5));
  UTEST(al1.get_int() == 6);
  UTEST(al1.get_string() == "(0,1,2,3,4,5)");
  UTEST(al1.size() == 6);

  UTCOD(al1.give(new ArgString("hello"),3));
  UTEST(al1.get_int() == 7);
  UTEST(al1.get_string() == "(0,1,2,hello,3,4,5)");
  UTEST(al1.size() == 7);

  UTMSG("remove elements");

  UTCOD(Arg* a1 = al1.take(4));
  UTEST(a1->get_int() == 3);
  delete a1;
  
  UTEST(al1.get_int() == 6);
  UTEST(al1.get_string() == "(0,1,2,hello,4,5)");
  UTEST(al1.size() == 6);
  
  UTMSG("access elements");

  UTEST(al1.get(-1) == 0);
  UTEST(al1.get(0)->get_int() == 0);
  UTEST(al1.get(1)->get_int() == 1);
  UTEST(al1.get(2)->get_int() == 2);
  UTEST(al1.get(3)->get_string() == "hello");
  UTEST(al1.get(4)->get_int() == 4);
  UTEST(al1.get(5)->get_int() == 5);
  UTEST(al1.get(6) == 0);
  
  UTMSG("copy");

  UTCOD(ArgList al2(al1));
  UTEST(al2.get_int() == 6);
  UTEST(al2.get_string() == "(0,1,2,hello,4,5)");
  UTEST(al2.size() == 6);

  UTCOD(Arg* a2 = al2.new_copy());
  UTEST(a2 != 0);
  UTEST(a2->get_string() == "(0,1,2,hello,4,5)");
  
  UTCOD(ArgList* al3 = dynamic_cast<ArgList*>(a2));
  UTEST(al3 != 0);
  UTEST(al3->get_string() == al2.get_string());
  UTEST(al3->get_string() == "(0,1,2,hello,4,5)");

  delete a2;
}

void Arg_ut()
{
  test_ArgString();
  test_ArgInt();
  test_ArgReal();
  test_ArgList();
}
