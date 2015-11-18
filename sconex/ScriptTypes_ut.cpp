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

#include <sconex/ScriptTypes.h>
#include <sconex/UnitTester.h>
using namespace scx;

void test_ScriptString()
{
  UTSEC("ScriptString");
  
  UTMSG("construction");

  UTCOD(ScriptString* as1 = new ScriptString(""));
  UTEST(as1->get_string() == "");
  UTEST(as1->get_int() == 0);

  UTCOD(ScriptString* as2 = new ScriptString("fish1234"));
  UTEST(as2->get_string() == "fish1234");
  UTEST(as2->get_int() == 1);

  UTCOD(ScriptString* as3 = new ScriptString(std::string("fowl5678")));
  UTEST(as3->get_string() == "fowl5678");
  UTEST(as3->get_int() == 1);

  UTMSG("copy");
  
  UTCOD(ScriptString* as4 = new ScriptString(*as2));
  UTEST(as4->get_string() == as2->get_string());
  UTEST(as4->get_int() == 1);

  UTCOD(ScriptObject* a1 = as4->new_copy());
  UTEST(a1 != 0);
  UTEST(a1->get_string() == as4->get_string());
  UTEST(a1->get_int() == 1);
  
  UTCOD(ScriptString* as5 = dynamic_cast<ScriptString*>(a1));
  UTEST(as5 != 0);
  UTEST(as5->get_string() == as4->get_string());
  UTEST(as5->get_int() == 1);

  delete a1;
  delete as4;

  UTMSG("ops");

  ScriptRef as1r(as1);
  ScriptRef as2r(as2);
  ScriptRef as3r(as3);

  UTCOD(test_script_op(as1r,ScriptOp::Add,as1r,ScriptString("")));
  UTCOD(test_script_op(as1r,ScriptOp::Add,as2r,ScriptString("fish1234")));
  UTCOD(test_script_op(as2r,ScriptOp::Add,as3r,ScriptString("fish1234fowl5678")));
  UTCOD(test_script_op(as3r,ScriptOp::Add,as2r,ScriptString("fowl5678fish1234")));

  UTCOD(test_script_op(as1r,ScriptOp::Equality,as1r,ScriptBool(1)));
  UTCOD(test_script_op(as2r,ScriptOp::Equality,as2r,ScriptBool(1)));
  UTCOD(test_script_op(as2r,ScriptOp::Equality,as3r,ScriptBool(0)));
  UTCOD(test_script_op(as1r,ScriptOp::Equality,as3r,ScriptBool(0)));

  UTCOD(test_script_op(as1r,ScriptOp::Inequality,as1r,ScriptBool(0)));
  UTCOD(test_script_op(as2r,ScriptOp::Inequality,as2r,ScriptBool(0)));
  UTCOD(test_script_op(as2r,ScriptOp::Inequality,as3r,ScriptBool(1)));
  UTCOD(test_script_op(as1r,ScriptOp::Inequality,as3r,ScriptBool(1)));
}

void test_ScriptInt()
{
  UTSEC("ScriptInt");

  UTMSG("construction");

  UTCOD(ScriptInt* ai1 = new ScriptInt(0));
  UTEST(ai1->get_int() == 0);
  UTEST(ai1->get_string() == "0");

  UTCOD(ScriptInt* ai2 = new ScriptInt(12345));
  UTEST(ai2->get_int() == 12345);
  UTEST(ai2->get_string() == "12345");

  UTCOD(ScriptInt* ai3 = new ScriptInt(-67890));
  UTEST(ai3->get_int() == -67890);
  UTEST(ai3->get_string() == "-67890");

  UTMSG("copy");

  UTCOD(ScriptInt* ai4 = new ScriptInt(*ai2));
  UTEST(ai4->get_int() == ai2->get_int());

  UTCOD(ScriptObject* a1 = ai4->new_copy());
  UTEST(a1 != 0);
  UTEST(a1->get_int() == 12345);
  
  UTCOD(ScriptInt* ai5 = dynamic_cast<ScriptInt*>(a1));
  UTEST(ai5 != 0);
  UTEST(ai5->get_string() == ai4->get_string());
  UTEST(ai5->get_int() == 12345);

  delete ai4;
  delete a1;

  ScriptRef ai1r(ai1);
  ScriptRef ai2r(ai2);
  ScriptRef ai3r(ai3);
  
  UTMSG("prefix ops");

  UTCOD(test_script_op(ScriptOp::Positive,ai1r,ScriptInt(0)));
  UTCOD(test_script_op(ScriptOp::Positive,ai2r,ScriptInt(12345)));
  UTCOD(test_script_op(ScriptOp::Positive,ai3r,ScriptInt(-67890)));

  UTCOD(test_script_op(ScriptOp::Negative,ai1r,ScriptInt(0)));
  UTCOD(test_script_op(ScriptOp::Negative,ai2r,ScriptInt(-12345)));
  UTCOD(test_script_op(ScriptOp::Negative,ai3r,ScriptInt(67890)));

  UTMSG("postfix ops");

  ScriptRef zero(new ScriptInt(0));
  ScriptRef one(new ScriptInt(1));
  ScriptRef five(new ScriptInt(5));

  UTCOD(test_script_op(one,ScriptOp::Factorial,ScriptInt(1)));
  UTCOD(test_script_op(five,ScriptOp::Factorial,ScriptInt(120)));

  UTMSG("binary ops");
  
  UTCOD(test_script_op(ai1r,ScriptOp::Add,ai1r,ScriptInt(0)));
  UTCOD(test_script_op(ai1r,ScriptOp::Add,ai2r,ScriptInt(12345)));
  UTCOD(test_script_op(ai2r,ScriptOp::Add,ai3r,ScriptInt(12345-67890)));
  UTCOD(test_script_op(ai3r,ScriptOp::Add,ai2r,ScriptInt(-67890+12345)));

  UTCOD(test_script_op(ai1r,ScriptOp::Subtract,ai1r,ScriptInt(0)));
  UTCOD(test_script_op(ai1r,ScriptOp::Subtract,ai2r,ScriptInt(-12345)));
  UTCOD(test_script_op(ai2r,ScriptOp::Subtract,ai3r,ScriptInt(12345+67890)));
  UTCOD(test_script_op(ai3r,ScriptOp::Subtract,ai2r,ScriptInt(-67890-12345)));

  UTCOD(test_script_op(ai1r,ScriptOp::Multiply,ai1r,ScriptInt(0)));
  UTCOD(test_script_op(ai1r,ScriptOp::Multiply,ai2r,ScriptInt(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::Multiply,ai3r,ScriptInt(12345*-67890)));
  UTCOD(test_script_op(ai3r,ScriptOp::Multiply,ai2r,ScriptInt(-67890*12345)));

  UTCOD(test_script_op(ai1r,ScriptOp::Divide,ai1r,ScriptError("Divide by zero")));
  UTCOD(test_script_op(ai1r,ScriptOp::Divide,ai2r,ScriptInt(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::Divide,ai3r,ScriptInt(12345/-67890)));
  UTCOD(test_script_op(ai3r,ScriptOp::Divide,ai2r,ScriptInt(-67890/12345)));

  UTCOD(test_script_op(ai1r,ScriptOp::GreaterThan,ai1r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::GreaterThan,ai1r,ScriptBool(1)));
  UTCOD(test_script_op(ai3r,ScriptOp::GreaterThan,ai2r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::GreaterThan,ai3r,ScriptBool(1)));
  UTCOD(test_script_op(ai3r,ScriptOp::GreaterThan,ai3r,ScriptBool(0)));

  UTCOD(test_script_op(ai1r,ScriptOp::LessThan,ai1r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::LessThan,ai1r,ScriptBool(0)));
  UTCOD(test_script_op(ai3r,ScriptOp::LessThan,ai2r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::LessThan,ai3r,ScriptBool(0)));
  UTCOD(test_script_op(ai3r,ScriptOp::LessThan,ai3r,ScriptBool(0)));

  UTCOD(test_script_op(ai1r,ScriptOp::GreaterThanOrEqualTo,ai1r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::GreaterThanOrEqualTo,ai1r,ScriptBool(1)));
  UTCOD(test_script_op(ai3r,ScriptOp::GreaterThanOrEqualTo,ai2r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::GreaterThanOrEqualTo,ai3r,ScriptBool(1)));
  UTCOD(test_script_op(ai3r,ScriptOp::GreaterThanOrEqualTo,ai3r,ScriptBool(1)));

  UTCOD(test_script_op(ai1r,ScriptOp::LessThanOrEqualTo,ai1r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::LessThanOrEqualTo,ai1r,ScriptBool(0)));
  UTCOD(test_script_op(ai3r,ScriptOp::LessThanOrEqualTo,ai2r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::LessThanOrEqualTo,ai3r,ScriptBool(0)));
  UTCOD(test_script_op(ai3r,ScriptOp::LessThanOrEqualTo,ai3r,ScriptBool(1)));
  
  UTCOD(test_script_op(ai1r,ScriptOp::Equality,ai1r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::Equality,ai2r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::Equality,ai3r,ScriptBool(0)));
  UTCOD(test_script_op(ai1r,ScriptOp::Equality,ai3r,ScriptBool(0)));

  UTCOD(test_script_op(ai1r,ScriptOp::Inequality,ai1r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::Inequality,ai2r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::Inequality,ai3r,ScriptBool(1)));
  UTCOD(test_script_op(ai1r,ScriptOp::Inequality,ai3r,ScriptBool(1)));
}

void test_ScriptBool()
{
  UTSEC("ScriptBool");

  UTMSG("construction");

  UTCOD(ScriptBool* ai1 = new ScriptBool(0));
  UTEST(ai1->get_int() == 0);
  UTEST(ai1->get_string() == "false");

  UTCOD(ScriptBool* ai2 = new ScriptBool(1));
  UTEST(ai2->get_int() == 1);
  UTEST(ai2->get_string() == "true");

  UTCOD(ScriptBool* ai3 = new ScriptBool(2));
  UTEST(ai3->get_int() == 1);
  UTEST(ai3->get_string() == "true");

  UTMSG("copy");

  UTCOD(ScriptBool* ai4 = new ScriptBool(*ai2));
  UTEST(ai4->get_int() == ai2->get_int());

  UTCOD(ScriptObject* a1 = ai4->new_copy());
  UTEST(a1 != 0);
  UTEST(a1->get_int() == 1);
  
  UTCOD(ScriptBool* ai5 = dynamic_cast<ScriptBool*>(a1));
  UTEST(ai5 != 0);
  UTEST(ai5->get_string() == ai4->get_string());
  UTEST(ai5->get_int() == 1);

  delete ai4;
  delete a1;

  ScriptRef ai1r(ai1);
  ScriptRef ai2r(ai2);
  
  UTMSG("prefix ops");

  UTCOD(test_script_op(ScriptOp::Not,ai1r,ScriptBool(1)));
  UTCOD(test_script_op(ScriptOp::Not,ai2r,ScriptBool(0)));

  UTMSG("binary ops");

  ScriptRef vfalse(new ScriptBool(0));
  ScriptRef vtrue(new ScriptBool(1));
  
  UTCOD(test_script_op(ai1r,ScriptOp::Equality,ai1r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::Equality,ai2r,ScriptBool(1)));
  UTCOD(test_script_op(ai1r,ScriptOp::Equality,ai2r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::Equality,ai1r,ScriptBool(0)));

  UTCOD(test_script_op(ai1r,ScriptOp::Inequality,ai1r,ScriptBool(0)));
  UTCOD(test_script_op(ai2r,ScriptOp::Inequality,ai2r,ScriptBool(0)));
  UTCOD(test_script_op(ai1r,ScriptOp::Inequality,ai2r,ScriptBool(1)));
  UTCOD(test_script_op(ai2r,ScriptOp::Inequality,ai1r,ScriptBool(1)));

  UTCOD(test_script_op(vfalse,ScriptOp::And,vfalse,ScriptBool(0)));
  UTCOD(test_script_op(vtrue,ScriptOp::And,vfalse,ScriptBool(0)));
  UTCOD(test_script_op(vfalse,ScriptOp::And,vtrue,ScriptBool(0)));
  UTCOD(test_script_op(vtrue,ScriptOp::And,vtrue,ScriptBool(1)));

  UTCOD(test_script_op(vfalse,ScriptOp::Or,vfalse,ScriptBool(0)));
  UTCOD(test_script_op(vtrue,ScriptOp::Or,vfalse,ScriptBool(1)));
  UTCOD(test_script_op(vfalse,ScriptOp::Or,vtrue,ScriptBool(1)));
  UTCOD(test_script_op(vtrue,ScriptOp::Or,vtrue,ScriptBool(1)));

  UTCOD(test_script_op(vfalse,ScriptOp::Xor,vfalse,ScriptBool(0)));
  UTCOD(test_script_op(vtrue,ScriptOp::Xor,vfalse,ScriptBool(1)));
  UTCOD(test_script_op(vfalse,ScriptOp::Xor,vtrue,ScriptBool(1)));
  UTCOD(test_script_op(vtrue,ScriptOp::Xor,vtrue,ScriptBool(0)));
}

void test_ScriptReal()
{
  UTSEC("ScriptReal");

  UTMSG("construction");

  UTCOD(ScriptReal* ar1 = new ScriptReal(0));
  UTEST(ar1->get_int() == 0);
  UTEST(ar1->get_string() == "0");

  UTCOD(ScriptReal* ar2 = new ScriptReal(1.2345));
  UTEST(ar2->get_int() == 1);
  UTEST(ar2->get_string() == "1.2345");

  UTCOD(ScriptReal* ar3 = new ScriptReal(-6.7890));
  UTEST(ar3->get_int() == -6);
  UTEST(ar3->get_string() == "-6.789");

  UTMSG("copy");

  UTCOD(ScriptReal* ar4 = new ScriptReal(*ar2));
  UTEST(ar4->get_int() == ar2->get_int());

  UTCOD(ScriptObject* a1 = ar4->new_copy());
  UTEST(a1 != 0);
  UTEST(a1->get_string() == "1.2345");
  
  UTCOD(ScriptReal* ar5 = dynamic_cast<ScriptReal*>(a1));
  UTEST(ar5 != 0);
  UTEST(ar5->get_string() == ar4->get_string());
  UTEST(ar5->get_string() == "1.2345");

  delete a1;
  
  UTMSG("ops");

  ScriptRef ar1r(ar1);
  ScriptRef ar2r(ar2);
  ScriptRef ar3r(ar3);

  UTCOD(test_script_op(ar1r,ScriptOp::Add,ar1r,ScriptReal(0.0)));
  UTCOD(test_script_op(ar1r,ScriptOp::Add,ar2r,ScriptReal(1.2345)));
  UTCOD(test_script_op(ar2r,ScriptOp::Add,ar3r,ScriptReal(1.2345-6.7890)));
  UTCOD(test_script_op(ar3r,ScriptOp::Add,ar2r,ScriptReal(-6.7890+1.2345)));

  UTCOD(test_script_op(ar1r,ScriptOp::Subtract,ar1r,ScriptReal(0.0)));
  UTCOD(test_script_op(ar1r,ScriptOp::Subtract,ar2r,ScriptReal(-1.2345)));
  UTCOD(test_script_op(ar2r,ScriptOp::Subtract,ar3r,ScriptReal(1.2345+6.7890)));
  UTCOD(test_script_op(ar3r,ScriptOp::Subtract,ar2r,ScriptReal(-6.7890-1.2345)));

  UTCOD(test_script_op(ar1r,ScriptOp::Multiply,ar1r,ScriptReal(0.0)));
  UTCOD(test_script_op(ar1r,ScriptOp::Multiply,ar2r,ScriptReal(0.0)));
  UTCOD(test_script_op(ar2r,ScriptOp::Multiply,ar3r,ScriptReal(1.2345*-6.789)));
  UTCOD(test_script_op(ar3r,ScriptOp::Multiply,ar2r,ScriptReal(-6.789*1.2345)));

  UTCOD(test_script_op(ar1r,ScriptOp::Divide,ar1r,ScriptReal(0.0/0.0)));
  UTCOD(test_script_op(ar1r,ScriptOp::Divide,ar2r,ScriptReal(0.0)));
  UTCOD(test_script_op(ar2r,ScriptOp::Divide,ar3r,ScriptReal(1.2345/-6.7890)));
  UTCOD(test_script_op(ar3r,ScriptOp::Divide,ar2r,ScriptReal(-6.789/1.2345)));

  UTCOD(test_script_op(ar1r,ScriptOp::Equality,ar1r,ScriptBool(1)));
  UTCOD(test_script_op(ar2r,ScriptOp::Equality,ar2r,ScriptBool(1)));
  UTCOD(test_script_op(ar2r,ScriptOp::Equality,ar3r,ScriptBool(0)));
  UTCOD(test_script_op(ar1r,ScriptOp::Equality,ar3r,ScriptBool(0)));

  UTCOD(test_script_op(ar1r,ScriptOp::Inequality,ar1r,ScriptBool(0)));
  UTCOD(test_script_op(ar2r,ScriptOp::Inequality,ar2r,ScriptBool(0)));
  UTCOD(test_script_op(ar2r,ScriptOp::Inequality,ar3r,ScriptBool(1)));
  UTCOD(test_script_op(ar1r,ScriptOp::Inequality,ar3r,ScriptBool(1)));
}

void test_ScriptList()
{
  UTSEC("ScriptList");

  UTMSG("construction");

  UTCOD(ScriptList* al1 = new ScriptList());
  UTEST(al1->get_int() == 0);
  UTEST(al1->get_string() == "[]");
  UTEST(al1->size() == 0);

  UTMSG("add elements to end");
  
  UTCOD(al1->give(new ScriptRef(new ScriptInt(1))));
  UTEST(al1->get_int() == 1);
  UTEST(al1->get_string() == "[1]");
  UTEST(al1->size() == 1);

  UTCOD(al1->give(new ScriptRef(new ScriptInt(2))));
  UTEST(al1->get_int() == 2);
  UTEST(al1->get_string() == "[1,2]");
  UTEST(al1->size() == 2);

  UTCOD(al1->give(new ScriptRef(new ScriptInt(3))));
  UTEST(al1->get_int() == 3);
  UTEST(al1->get_string() == "[1,2,3]");
  UTEST(al1->size() == 3);

  UTCOD(al1->give(new ScriptRef(new ScriptInt(4))));
  UTEST(al1->get_int() == 4);
  UTEST(al1->get_string() == "[1,2,3,4]");
  UTEST(al1->size() == 4);

  UTMSG("insert elements");

  UTCOD(al1->give(new ScriptRef(new ScriptInt(0)),0));
  UTEST(al1->get_int() == 5);
  UTEST(al1->get_string() == "[0,1,2,3,4]");
  UTEST(al1->size() == 5);

  UTCOD(al1->give(new ScriptRef(new ScriptInt(5)),5));
  UTEST(al1->get_int() == 6);
  UTEST(al1->get_string() == "[0,1,2,3,4,5]");
  UTEST(al1->size() == 6);

  UTCOD(al1->give(new ScriptRef(new ScriptString("hello")),3));
  UTEST(al1->get_int() == 7);
  UTEST(al1->get_string() == "[0,1,2,hello,3,4,5]");
  UTEST(al1->size() == 7);

  UTMSG("remove elements");

  UTCOD(ScriptRef* a1 = al1->take(4));
  UTEST(a1->object()->get_int() == 3);
  delete a1;
  
  UTEST(al1->get_int() == 6);
  UTEST(al1->get_string() == "[0,1,2,hello,4,5]");
  UTEST(al1->size() == 6);
  
  UTMSG("access elements");

  UTEST(al1->get(-1) == 0);
  UTEST(al1->get(0)->object()->get_int() == 0);
  UTEST(al1->get(1)->object()->get_int() == 1);
  UTEST(al1->get(2)->object()->get_int() == 2);
  UTEST(al1->get(3)->object()->get_string() == "hello");
  UTEST(al1->get(4)->object()->get_int() == 4);
  UTEST(al1->get(5)->object()->get_int() == 5);
  UTEST(al1->get(6) == 0);
  
  UTMSG("copy");

  UTCOD(ScriptList* al2 = new ScriptList(*al1));
  UTEST(al2->get_int() == 6);
  UTEST(al2->get_string() == "[0,1,2,hello,4,5]");
  UTEST(al2->size() == 6);

  UTCOD(ScriptObject* a2 = al2->new_copy());
  UTEST(a2 != 0);
  UTEST(a2->get_string() == "[0,1,2,hello,4,5]");
  
  UTCOD(ScriptList* al3 = dynamic_cast<ScriptList*>(a2));
  UTEST(al3 != 0);
  UTEST(al3->get_string() == al2->get_string());
  UTEST(al3->get_string() == "[0,1,2,hello,4,5]");

  delete al1;
  delete al2;
  delete a2;
}

void ScriptTypes_ut()
{
  test_ScriptString();
  test_ScriptInt();
  test_ScriptBool();
  test_ScriptReal();
  test_ScriptList();
}
