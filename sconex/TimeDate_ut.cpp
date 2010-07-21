/* SconeServer (http://www.sconemad.com)

UNIT TESTS for Time, Date and TimeZone classes

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

#include "sconex/Time.h"
#include "sconex/Date.h"
#include "sconex/UnitTester.h"
using namespace scx;

void TimeDate_ut()
{
  UTSEC("time construction");
  
  UTMSG("default");
  UTCOD(Time t0);
  UTEST(t0.seconds() == 0);

  UTMSG("mins");
  UTCOD(Time t1(7));
  UTEST(t1.seconds() == 7);

  UTMSG("mins,secs");
  UTCOD(Time t2(7,6));
  UTEST(t2.seconds() == 426);
  
  UTMSG("hrs,mins,secs");
  UTCOD(Time t3(7,6,5));
  UTEST(t3.seconds() == 25565);

  UTMSG("days,hrs,mins,secs");
  UTCOD(Time t4(7,6,5,4));
  UTEST(t4.seconds() == 626704);

  UTMSG("wks,days,hrs,mins,secs");
  UTCOD(Time t5(7,6,5,4,3));
  UTEST(t5.seconds() == 4770243);

  UTMSG("string");
  UTEST(Time("0").seconds() == 0);
  UTEST(Time("0:0:0:0:0").seconds() == 0);
  UTEST(Time("-00:00").seconds() == 0);
  UTEST(Time("0w 0d 0h 0m 0s").seconds() == 0);
  UTEST(Time("432").seconds() == 432);
  UTEST(Time("12s").seconds() == 12);
  UTEST(Time("-52s").seconds() == -52);
  UTEST(Time("93m").seconds() == 5580);
  UTEST(Time("13h").seconds() == 46800);
  UTEST(Time("57d").seconds() == 4924800);
  UTEST(Time("25w").seconds() == 15120000);
  UTEST(Time("-4w").seconds() == -2419200);
  UTEST(Time("2:3").seconds() == 123);
  UTEST(Time("10:00").seconds() == 600);
  UTEST(Time("05:35").seconds() == 335);
  UTEST(Time("-04:15").seconds() == -255);
  UTEST(Time("06:07:08").seconds() == 22028);
  UTEST(Time("5:4:03:02").seconds() == 446582);
  UTEST(Time("99:88:77:66:55").seconds() == 67759615);
  UTEST(Time("45m23s").seconds() == 2723);
  UTEST(Time("5h 3m 7s").seconds() == 18187);
  UTEST(Time("10d 23h 59m 55s").seconds() == 950395);
  UTEST(Time("5w 41d 73h 359m 3343s").seconds() == 6854083);
  UTEST(Time("3w 7s").seconds() == 1814407);
  UTEST(Time("-1h 22s").seconds() == -3622);
  UTEST(Time("55d 12m 3s").seconds() == 4752723);

  UTMSG("copy");
  UTCOD(Time tc1(t1));
  UTEST(tc1.seconds() == 7);
  UTCOD(Time tc4(t4));
  UTEST(tc4.seconds() == 626704);

  
  UTSEC("time conversion");

  UTEST(t0.to_minutes() == 0);
  UTEST(t0.to_hours() == 0);
  UTEST(t0.to_days() == 0);
  UTEST(t0.to_weeks() == 0);
  
  UTEST(t1.to_minutes() > 0.11 && t1.to_minutes() < 0.12);
  UTEST(t1.to_hours() > 0.001 && t1.to_hours() < 0.002);
  UTEST(t1.to_days() > 8e-5 && t1.to_days() < 9e-5);
  UTEST(t1.to_weeks() > 1e-5 && t1.to_weeks() < 2e-5);

  UTEST(t2.to_minutes() > 7.0 && t2.to_minutes() < 7.2);
  UTEST(t2.to_hours() > 0.11 && t2.to_hours() < 0.12);
  UTEST(t2.to_days() > 0.004 && t2.to_days() < 0.005);
  UTEST(t2.to_weeks() > 0.0007 && t2.to_weeks() < 0.0008);

  UTEST(t3.to_minutes() > 426.08 && t3.to_minutes() < 426.09);
  UTEST(t3.to_hours() > 7.0 && t3.to_hours() < 7.2);
  UTEST(t3.to_days() > 0.295 && t3.to_days() < 0.296);
  UTEST(t3.to_weeks() > 0.0422 && t3.to_weeks() < 0.0423);
  
  UTEST(t4.to_minutes() > 10445.06 && t4.to_minutes() < 10445.07);
  UTEST(t4.to_hours() > 174.08 && t4.to_hours() < 174.09);
  UTEST(t4.to_days() > 7.2 && t4.to_days() < 7.3);
  UTEST(t4.to_weeks() > 1.036 && t4.to_weeks() < 1.037);

  UTEST(t5.to_minutes() > 79504.0 && t5.to_minutes() < 79504.1);
  UTEST(t5.to_hours() > 1325.0 && t5.to_hours() < 1325.1);
  UTEST(t5.to_days() > 55.21 && t5.to_days() < 55.22);
  UTEST(t5.to_weeks() > 7.88 && t5.to_weeks() < 7.89);

  UTSEC("time access");

  int tw,td,th,tm,ts;

  UTCOD(t1.get(tm,ts));
  UTEST(tm == 0 && ts == 7);
  UTCOD(t1.get(th,tm,ts));
  UTEST(th == 0 && tm == 0 && ts == 7);
  UTCOD(t1.get(td,th,tm,ts));
  UTEST(td == 0 && th == 0 && tm == 0 && ts == 7);
  UTCOD(t1.get(tw,td,th,tm,ts));
  UTEST(tw == 0 && td == 0 && th == 0 && tm == 0 && ts == 7);
  
  UTCOD(t2.get(tm,ts));
  UTEST(tm == 7 && ts == 6);
  UTCOD(t2.get(th,tm,ts));
  UTEST(th == 0 && tm == 7 && ts == 6);
  UTCOD(t2.get(td,th,tm,ts));
  UTEST(td == 0 && th == 0 && tm == 7 && ts == 6);
  UTCOD(t2.get(tw,td,th,tm,ts));
  UTEST(tw == 0 && td == 0 && th == 0 && tm == 7 && ts == 6);

  UTCOD(t3.get(tm,ts));
  UTEST(tm == 426 && ts == 5);
  UTCOD(t3.get(th,tm,ts));
  UTEST(th == 7 && tm == 6 && ts == 5);
  UTCOD(t3.get(td,th,tm,ts));
  UTEST(td == 0 && th == 7 && tm == 6 && ts == 5);
  UTCOD(t3.get(tw,td,th,tm,ts));
  UTEST(tw == 0 && td == 0 && th == 7 && tm == 6 && ts == 5);

  UTCOD(t4.get(tm,ts));
  UTEST(tm == 10445 && ts == 4);
  UTCOD(t4.get(th,tm,ts));
  UTEST(th == 174 && tm == 5 && ts == 4);
  UTCOD(t4.get(td,th,tm,ts));
  UTEST(td == 7 && th == 6 && tm == 5 && ts == 4);
  UTCOD(t4.get(tw,td,th,tm,ts));
  UTEST(tw == 1 && td == 0 && th == 6 && tm == 5 && ts == 4);

  UTCOD(t5.get(tm,ts));
  UTEST(tm == 79504 && ts == 3);
  UTCOD(t5.get(th,tm,ts));
  UTEST(th == 1325 && tm == 4 && ts == 3);
  UTCOD(t5.get(td,th,tm,ts));
  UTEST(td == 55 && th == 5 && tm == 4 && ts == 3);
  UTCOD(t5.get(tw,td,th,tm,ts));
  UTEST(tw == 7 && td == 6 && th == 5 && tm == 4 && ts == 3);

  
  UTSEC("time operators");

  UTEST((t0 + t1).seconds() == 7);
  UTEST((t1 + t2).seconds() == 433);
  UTEST((t2 + t3).seconds() == 25991);
  UTEST((t3 + t4).seconds() == 652269);
  UTEST((t4 + t5).seconds() == 5396947);
  UTEST((t5 + t0).seconds() == 4770243);
  UTEST((t0 + t1 + t2 + t3 + t4 + t5).seconds() == 5422945);

  UTEST((t3 - t2).seconds() == 25139);
  UTEST((t4 - t0).seconds() == 626704);
  UTEST((t4 + t1 - t2).seconds() == 626285);
  UTEST((t4 - t5).seconds() == -4143539);


  UTSEC("time to string");

  UTEST(t0.string() == "00:00:00");
  UTEST(t1.string() == "00:00:07");
  UTEST(t2.string() == "00:07:06");
  UTEST(t3.string() == "07:06:05");
  UTEST(t4.string() == "1 Week 0 Days 06:05:04");
  UTEST(t5.string() == "7 Weeks 6 Days 05:04:03");


  UTSEC("timezone construction");

  UTEST(TimeZone().seconds() == 0);
  UTEST(TimeZone(3,4).seconds() == 11040);
  UTCOD(TimeZone tz1(-7,145));
  UTEST(tz1.seconds() == -33900);
  UTEST(TimeZone("-1234").seconds() == -45240);
  UTCOD(TimeZone tz2("5678"));
  UTEST(tz2.seconds() == 206280);

  UTCOD(TimeZone tza("800"));
  UTEST(tza.seconds() == 28800);
  UTCOD(TimeZone tzb("745"));
  UTEST(tzb.seconds() == 27900);
  UTCOD(TimeZone tzc("1000"));
  UTEST(tzc.seconds() == 36000);


  UTSEC("timezone operators");

  UTCOD(TimeZone tz3 = tz1 + tz2);
  UTEST(tz3.seconds() == 172380);
  UTCOD(TimeZone tz4 = tz1 + t4);
  UTEST(tz4.seconds() == 592804);
  UTCOD(TimeZone tz5 = tz4 - tz1);
  UTEST(tz5.seconds() == 626704);
  UTCOD(TimeZone tz6 = tz5 - t1);
  UTEST(tz6.seconds() == 626697);
  UTEST((tzc + tzb).seconds() == 63900);


  UTSEC("time to string");
  
  UTEST(tz1.string() == "-0925");
  UTEST(tz2.string() == "+5718");
  UTEST(tz3.string() == "+4753");
  UTEST(tz4.string() == "+16440");
  UTEST(tz5.string() == "+17405");
  UTEST(tz6.string() == "+17404");

  UTEST(tza.string() == "+0800");
  UTEST(tzb.string() == "+0745");
  UTEST(tzc.string() == "+1000");
  UTEST((tza + tzb).string() == "+1545");
  UTEST((tza + tzc).string() == "+1800");
  UTEST((tzb + tzc).string() == "+1745");
  UTEST((tzc - tza).string() == "+0200");
  UTEST((tzc - tzb).string() == "+0215");
  UTEST((tzb - tza).string() == "-0015");
  UTEST((tza + tzb + tzc - tza - tzb - tzc).string() == "+0000");
  
  
  UTSEC("date construction");

  UTMSG("default/current");
  UTCOD(Date d0);
  UTCOD(Date dnow = Date::now());
  UTCOD(std::cout << " It is now "<< dnow.string() << "\n");
  UTEST(d0 < dnow);

  UTMSG("value");
  UTCOD(Date d1(1986,01,28,16,39,13));
  UTCOD(Date d2(2003,02,01,13,59,32));

  UTMSG("copy/assign");
  UTCOD(Date d3(d1));
  UTCOD(Date d4);
  UTCOD(d4 = d2);

  UTMSG("string");

  const int nds = 32;
  Date ds[nds];
  UTMSG("all the following should represent exactly the same date & time:");
  UTCOD(ds[0] = Date("Tue, 02 Aug 2005 12:34:56"));
  UTCOD(ds[1] = Date("Tue, 2 Aug 2005 12:34:56 GMT"));
  UTCOD(ds[2] = Date("Tue, 02 Aug 2005 12:34:56 +0000"));
  UTCOD(ds[3] = Date("Tue Aug 2 2005 12:34:56 UTC"));
  UTCOD(ds[4] = Date("Monday, 01-Aug-2005 22:34:56 -1400"));
  UTCOD(ds[5] = Date("Monday, 01-Aug-2005 23:34:56 -1300"));
  UTCOD(ds[6] = Date("Tuesday, 02-Aug-2005 0:34:56 -1200"));
  UTCOD(ds[7] = Date("Tuesday, 02-Aug-2005 1:34:56 -1100"));
  UTCOD(ds[8] = Date("02-Aug-2005 2:34:56 -1000"));
  UTCOD(ds[9] = Date("2-Aug-2005 4:34:56 AKDT"));
  UTCOD(ds[10] = Date("2-Aug-2005 5:34:56 PDT"));
  UTCOD(ds[11] = Date("2 August 2005 6:34:56 MDT"));
  UTCOD(ds[12] = Date("2-Aug-2005 7:34:56 CDT"));
  UTCOD(ds[13] = Date("2-Aug-2005 8:34:56 EDT"));
  UTCOD(ds[14] = Date("2-Aug-2005 9:34:56 -0300"));
  UTCOD(ds[15] = Date("2-Aug-2005 13:34:56 BST"));
  UTCOD(ds[16] = Date("2-Aug-2005 14:34:56 CEDT"));
  UTCOD(ds[17] = Date("2-Aug-2005 15:34:56 EEDT"));
  UTCOD(ds[18] = Date("2-Aug-2005 16:34:56 +0400"));
  UTCOD(ds[19] = Date("Tuesday, 02-Aug-2005 17:04:56 +0430"));
  UTCOD(ds[20] = Date("Tuesday; 02-Aug-2005 17:34:56 +0500"));
  UTCOD(ds[21] = Date("Tuesday August 02 2005 18:04:56 +0530"));
  UTCOD(ds[22] = Date("2005 Tuesday Aug 02 20-34-56 AWST"));
  UTCOD(ds[23] = Date("Aug 2 2005 21:34:56 JST"));
  UTCOD(ds[24] = Date("Tuesday, 02-Aug-2005 22:04:56 ACST"));
  UTCOD(ds[25] = Date("Tuesday, 02-Aug-2005 22:34:56 AEST"));
  UTCOD(ds[26] = Date("Tuesday, 02-Aug-2005 23:34:56 L"));
  UTCOD(ds[27] = Date("Wed 03 August 2005 00:34:56 M"));
  UTCOD(ds[28] = Date("Wed, 03-Aug-2005 01:34:56 +1300"));
  UTCOD(ds[29] = Date("Wednesday, 03-Aug-2005 02:34:56 +1400"));
  UTCOD(ds[30] = Date("2005-08-02 12:34:56"));
  UTCOD(ds[31] = Date("2005 Aug 02 12:34:56"));

  for (int ids=0; ids<nds; ++ids) {
    std::cout << " ds[" << ids << "] = " << ds[ids].string() << "\n";
    UTEST(ds[ids].epoch_seconds() == 1122982496);
    UTEST(ds[ids] == ds[0]);
  }
  
  UTSEC("date operators");

  UTEST(d1 == d3);
  UTEST(d2 == d4);
  UTEST(d1 != d4);
  UTEST(d2 != d3);

  UTCOD(Time td12 = d2 - d1);
  std::cout << " td12= " << td12.string() << "\n";
  UTCOD(td12.get(tw,td,th,tm,ts));
  UTEST(tw == 887 && td == 3 && th == 21 && tm == 20 && ts == 19);

  UTCOD(Date d5 = d1 + td12);
  std::cout << " d2= " << d2.string() << "\n";
  std::cout << " d5= " << d5.string() << "\n";
  UTEST(d5 == d2);

  UTCOD(d5 += Time(1));
  UTEST(d5 != d2);
  UTEST(d5 == (d2+Time(1)));

  UTCOD(d5 += Time(33,44,55));
  UTEST(d5 == (d2+Time(33,44,56)));

  UTCOD(d5 -= Time(0,44,56));
  UTEST(d5 == (d2+Time("1d9h")));
 
}
