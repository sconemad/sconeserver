/* SconeServer (http://www.sconemad.com)

UNIT TESTS for Num

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

#include "MathsFloat.h"
#include "MathsModule.h"
#include <sconex/UnitTester.h>
using namespace scx;

void Num_ut()
{
  ScriptRefTo<MathsModule> mod(new MathsModule());

  UTSEC("construction");
  
  UTCOD(MathsFloat i1(mod.object(),0));
  UTEST(i1.get_int() == 0);

  UTCOD(MathsFloat i2(mod.object(),1234567890));
  UTEST(i2.get_int() == 1234567890);

  UTCOD(MathsFloat i3(mod.object(),Mpfr("987654")));
  UTEST(i3.get_int() == 987654);

  //TODO: need to add more tests...

  UTSEC("ops");

  UTMSG("Power");

  // Compute 2^1000 - which produces quite a big number

  MathsFloat::Ref i4r(new MathsFloat(mod.object(),2));
  MathsFloat::Ref i5r(new MathsFloat(mod.object(),1e3));

  const char* _2pow1000 = 
    "10715086071862673209484250490600018105614048117055336074437503883703"
    "51051124936122493198378815695858127594672917553146825187145285692314"
    "04359845775746985748039345677748242309854210746050623711418779541821"
    "53046474983581941267398767559165543946077062914571196477686542167660"
    "429831652624386837205668069376";

  UTCOD(test_script_op(i4r,ScriptOp::Power,i5r,
		       MathsFloat(mod.object(),Mpfr(_2pow1000))));

}
