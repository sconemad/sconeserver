/* SconeServer (http://www.sconemad.com)

Time and TimeZone

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


#include "sconex/Time.h"
#include "sconex/Date.h"
#include "sconex/utils.h"
namespace scx {

//=============================================================================
Time::Time(int t)
  : m_time(t)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
}

//=============================================================================
Time::Time(int minutes,int seconds)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time = (abs(minutes)*SECONDS_PER_MINUTE) + 
           abs(seconds);
  m_time *= (minutes<0) ? -1:1;
}

//=============================================================================
Time::Time(int hours,int minutes,int seconds)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time = (abs(hours)*SECONDS_PER_HOUR) + 
           (abs(minutes)*SECONDS_PER_MINUTE) + 
           abs(seconds);
  m_time *= (hours<0) ? -1:1;
}

//=============================================================================
Time::Time(int days,int hours,int minutes,int seconds)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time = (abs(days)*SECONDS_PER_DAY) + 
           (abs(hours)*SECONDS_PER_HOUR) + 
           (abs(minutes)*SECONDS_PER_MINUTE) + 
           abs(seconds);
  m_time *= (days<0) ? -1:1;
}

//=============================================================================
Time::Time(int weeks,int days,int hours,int minutes,int seconds)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time = (abs(weeks)*SECONDS_PER_WEEK) + 
           (abs(days)*SECONDS_PER_DAY) + 
           (abs(hours)*SECONDS_PER_HOUR) + 
           (abs(minutes)*SECONDS_PER_MINUTE) + 
           abs(seconds);
  m_time *= (weeks<0) ? -1:1;
}

//=============================================================================
Time::Time(Arg* args)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);

  ArgList* l = dynamic_cast<ArgList*>(args);
  Arg* a = l->get(0);
  ArgInt* a_int = dynamic_cast<ArgInt*>(a);
  if (a_int) {
    m_time = a->get_int();
  } else {
    m_time = 0;
  }
}

//=============================================================================
Time::Time(const Time& c)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time=c.m_time;
}

struct time_tok {
  int value;
  std::string sep;
};

//=============================================================================
Time::Time(const std::string& str)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time=0;
  std::queue<time_tok> ts;
  std::string::const_iterator it = str.begin();
  time_tok cur;
  cur.value=0;
  int unq=0;
  int sign=1;
  bool got=false;

  while (it < str.end()) {

    if (isdigit(*it)) {
      if (got) {
        ts.push(cur);
        unq += (!isalpha(cur.sep[0]));
      }
      cur.value=0; cur.sep="";
      std::string::const_iterator it1(it);
      do ++it; while (it < str.end() && isdigit(*it));
      std::string token(it1,it);
      cur.value = atoi(token.c_str());
      got=true;

    } else if (isalpha(*it)) {
      std::string::const_iterator it1(it);
      do ++it; while (it < str.end() && isalpha(*it));
      cur.sep = std::string(it1,it);
      strup(cur.sep);
      
    } else if ((*it)=='-') {
      sign = -1;
      ++it;

    } else {
      ++it;
    }
  }
  if (got) {
    ts.push(cur);
    unq += (!isalpha(cur.sep[0]));
  }

  int seq=2;
  if (unq>0 && unq<6) seq = 5-unq;

  while (!ts.empty()) {
    const time_tok& cur = ts.front();
    if (cur.sep=="S" || cur.sep=="SEC" ||
        cur.sep=="SECS" || cur.sep=="SECOND" || cur.sep=="SECONDS") {
      seq=4;
    } else if (cur.sep=="M" || cur.sep=="MIN" ||
               cur.sep=="MINS" || cur.sep=="MINUTE" || cur.sep=="MINUTES") {
      seq=3;
    } else if (cur.sep=="H" || cur.sep=="HR" ||
               cur.sep=="HRS" || cur.sep=="HOUR" || cur.sep=="HOURS") {
      seq=2;
    } else if (cur.sep=="D" || cur.sep=="DY" ||
               cur.sep=="DYS" || cur.sep=="DAY" || cur.sep=="DAYS") {
      seq=1;
    } else if (cur.sep=="W" || cur.sep=="WK" ||
               cur.sep=="WKS" || cur.sep=="WEEK" || cur.sep=="WEEKS") {
      seq=0;
    }
    switch (seq++) {
      case 0: m_time += sign*cur.value*SECONDS_PER_WEEK; break;
      case 1: m_time += sign*cur.value*SECONDS_PER_DAY; break;
      case 2: m_time += sign*cur.value*SECONDS_PER_HOUR; break;
      case 3: m_time += sign*cur.value*SECONDS_PER_MINUTE; break;
      case 4: m_time += sign*cur.value; break;
    }
    ts.pop();
  }
}

//=============================================================================
Time::~Time()
{
  DEBUG_COUNT_DESTRUCTOR(Time);
}

//=============================================================================
int Time::seconds() const
{
  return (int)m_time;
}

//=============================================================================
double Time::to_weeks() const
{
  return m_time / (double)SECONDS_PER_WEEK;
}

//=============================================================================
double Time::to_days() const
{
  return m_time / (double)SECONDS_PER_DAY;
}

//=============================================================================
double Time::to_hours() const
{
  return m_time / (double)SECONDS_PER_HOUR;
}

//=============================================================================
double Time::to_minutes() const
{
  return m_time / (double)SECONDS_PER_MINUTE;
}

//=============================================================================
void Time::get(int& minutes,int& seconds) const
{
  seconds = m_time;
  
  minutes = (int)(seconds / SECONDS_PER_MINUTE);
  seconds -= minutes*SECONDS_PER_MINUTE;
}

//=============================================================================
void Time::get(int& hours,int& minutes,int& seconds) const
{
  seconds = m_time;

  hours = (int)(seconds / SECONDS_PER_HOUR);
  seconds -= hours*SECONDS_PER_HOUR;
  
  minutes = (int)(seconds / SECONDS_PER_MINUTE);
  seconds -= minutes*SECONDS_PER_MINUTE;
}

//=============================================================================
void Time::get(int& days,int& hours,int& minutes,int& seconds) const
{
  seconds = m_time;

  days = (int)(seconds / SECONDS_PER_DAY);
  seconds -= days*SECONDS_PER_DAY;

  hours = (int)(seconds / SECONDS_PER_HOUR);
  seconds -= hours*SECONDS_PER_HOUR;
  
  minutes = (int)(seconds / SECONDS_PER_MINUTE);
  seconds -= minutes*SECONDS_PER_MINUTE;
}

//=============================================================================
void Time::get(int& weeks,int& days,int& hours,int& minutes,int& seconds) const
{
  seconds = m_time;

  weeks = (int)(seconds / SECONDS_PER_WEEK);
  seconds -= weeks*SECONDS_PER_WEEK;

  days = (int)(seconds / SECONDS_PER_DAY);
  seconds -= days*SECONDS_PER_DAY;

  hours = (int)(seconds / SECONDS_PER_HOUR);
  seconds -= hours*SECONDS_PER_HOUR;
  
  minutes = (int)(seconds / SECONDS_PER_MINUTE);
  seconds -= minutes*SECONDS_PER_MINUTE;
}

//=============================================================================
std::string Time::string() const
{
  int weeks,days,hours,minutes,seconds;
  get(weeks,days,hours,minutes,seconds);
  weeks = abs(weeks);
  days = abs(days);
  hours = abs(hours);
  minutes = abs(minutes);
  seconds = abs(seconds);

  std::ostringstream oss;
  oss << (m_time < 0 ? "-" : "");
  
  if (weeks > 0) {
    oss << weeks << " " << (weeks==1 ? "Week" : "Weeks") << " ";
  }

  if (weeks > 0 || days > 0) {
    oss << days << " " << (days==1 ? "Day" : "Days") << " ";
  }
  
  oss << std::setfill('0') << std::setw(2) << hours << ":"
      << std::setfill('0') << std::setw(2) << minutes << ":"
      << std::setfill('0') << std::setw(2) << seconds;
  
  return oss.str();
}

//=============================================================================
Time Time::operator+(const Time& t) const
{
  return Time(m_time+t.m_time);
}

//=============================================================================
Time Time::operator-(const Time& t) const
{
  return Time(m_time-t.m_time);
}

//=============================================================================
Arg* Time::new_copy() const
{
  return new Time(*this);
}

//=============================================================================
std::string Time::get_string() const
{
  return string();
}

//=============================================================================
int Time::get_int() const
{
  return m_time;
}
//=============================================================================
Arg* Time::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (optype == Arg::Binary) {
    Time* rt = dynamic_cast<Time*>(right);
    if (rt) { // Time X Time ops
      if ("+" == opname) { // Plus
	return new Time(*this + *rt);
      } else if ("-" == opname) { // Minus
	return new Time(*this - *rt);
      }
    }
    
    if ("." == opname) { // Scope resolution
      std::string name = right->get_string();
      if (name == "seconds") return new ArgInt(seconds());
      if (name == "weeks") return new ArgReal(to_weeks());
      if (name == "days") return new ArgReal(to_days());
      if (name == "hours") return new ArgInt(to_hours());
      if (name == "minutes") return new ArgInt(to_minutes());
      if (name == "string") return new ArgString(string());
    }
  }
  return Arg::op(auth,optype,opname,right);
}


TimeZone::TimeZoneOffsetMap* TimeZone::s_zone_table = 0;

//=============================================================================
TimeZone::TimeZone(time_t t)
  : Time(t)
{
  DEBUG_COUNT_CONSTRUCTOR(TimeZone);
  init_tables();
}

//=============================================================================
TimeZone::TimeZone(int hours,int minutes)
{
  DEBUG_COUNT_CONSTRUCTOR(TimeZone);
  init_tables();
  m_time = (abs(hours)*SECONDS_PER_HOUR) +
           (abs(minutes)*SECONDS_PER_MINUTE);
  m_time *= (hours<0 || minutes<0) ? -1:1;
}

//=============================================================================
TimeZone::TimeZone(const std::string& str)
{
  DEBUG_COUNT_CONSTRUCTOR(TimeZone);
  init_tables();
  m_time=0;

  // First lookup in zone table
  TimeZoneOffsetMap::const_iterator zit = s_zone_table->find(str);
  if (zit != s_zone_table->end()) {
    m_time = zit->second;
    
  } else {
    std::string::const_iterator it = str.begin();
    int sign=1;

    while (it < str.end()) {

      if (isdigit(*it)) {
        std::string::const_iterator it1(it);
        do ++it; while (it < str.end() && isdigit(*it));
        std::string token(it1,it);
        int value = atoi(token.c_str());
        int hours = value/100;
        int minutes = value - (hours*100);
        m_time = (hours*SECONDS_PER_HOUR) + (minutes*SECONDS_PER_MINUTE);
        break;
        
      } else if (isalpha(*it)) {
        std::string::const_iterator it1(it);
        do ++it; while (it < str.end() && isalpha(*it));
        std::string token(it1,it);
        strup(token);
        
      } else if ((*it)=='-') {
        sign = -1;
        ++it;
        
      } else {
        ++it;
      }
    }
    m_time *= sign;
  }

}

//=============================================================================
TimeZone::~TimeZone()
{
  DEBUG_COUNT_DESTRUCTOR(TimeZone);
}

//=============================================================================
TimeZone TimeZone::utc()
{
  return TimeZone(0);
}

//=============================================================================
TimeZone TimeZone::local(const Date& date)
{
  struct tm* tms = 0;
  const time_t t = date.epoch_seconds();

  // Get local time on this date
  tms = localtime(&t);
  if (tms == 0) {
    return TimeZone();
  }
  tms->tm_isdst=-1;
  time_t t_loc = mktime(tms);

  // Get universal time on this date
  tms = gmtime(&t);
  if (tms == 0) {
    return TimeZone();
  }
  tms->tm_isdst=-1;
  time_t t_gmt = mktime(tms);

  // Local timezone is then the difference between these
  return TimeZone(t_loc - t_gmt);
}

//=============================================================================
TimeZone TimeZone::operator+(const Time& t) const
{
  return TimeZone(m_time+t.m_time);
}

//=============================================================================
TimeZone TimeZone::operator-(const Time& t) const
{
  return TimeZone(m_time-t.m_time);
}

//=============================================================================
std::string TimeZone::string() const
{
  int hours,minutes,seconds;
  get(hours,minutes,seconds);
  hours = abs(hours);
  minutes = abs(minutes);

  std::ostringstream oss;
  oss << (m_time < 0 ? "-" : "+")
      << std::setfill('0') << std::setw(2) << hours
      << std::setfill('0') << std::setw(2) << minutes;

  return oss.str();
}

//=============================================================================
void TimeZone::init_tables()
{
  if (s_zone_table == 0) {
    s_zone_table = new TimeZoneOffsetMap();
    
    add_tz("AKST",-9);
    add_tz("AKDT",-8);
    add_tz("PST", -8);
    add_tz("PDT", -7);
    add_tz("MST", -7);
    add_tz("MDT", -6);
    add_tz("CST", -6);
    add_tz("CDT", -5);
    add_tz("EST", -5);
    add_tz("EDT", -4);
    add_tz("AST", -4);
    add_tz("NST", -3.5);
    add_tz("ADT", -3);
    add_tz("NDT", -2.5);
    
    add_tz("UT",  0);
    add_tz("UTC", 0);
    add_tz("GMT", 0);
    add_tz("WET", 0);
    add_tz("BST", 1);
    add_tz("IST", 1);
    add_tz("WEDT",1);
    add_tz("CET", 1);
    add_tz("CEDT",2);
    add_tz("EET", 2);
    add_tz("EEDT",3);

    add_tz("CXT", 7);
    add_tz("AWST",8);
    add_tz("JST", 9);
    add_tz("ACST",9.5);
    add_tz("AEST",10);
    add_tz("ACDT",10.5);

    // Military
    add_tz("Y",   -12);
    add_tz("X",   -11);
    add_tz("W",   -10);
    add_tz("V*",  -9.5);
    add_tz("V",   -9);
    add_tz("U",   -8);
    add_tz("T",   -7);
    add_tz("S",   -6);
    add_tz("R",   -5);
    add_tz("Q",   -4);
    add_tz("P*",  -3.5);
    add_tz("P",   -3);
    add_tz("O",   -2);
    add_tz("N",   -1);
    add_tz("Z",   0);
    add_tz("A",   1);
    add_tz("B",   2);
    add_tz("C",   3);
    add_tz("C*",  3.5);
    add_tz("D",   4);
    add_tz("D*",  4.5);
    add_tz("E",   5);
    add_tz("E*",  5.5);
    add_tz("F",   6);
    add_tz("F*",  6.5);
    add_tz("G",   7);
    add_tz("H",   8);
    add_tz("I",   9);
    add_tz("I*",  9.5);
    add_tz("K",   10);
    add_tz("K*",  10.5);
    add_tz("L",   11);
    add_tz("L*",  11.5);
    add_tz("M",   12);
    add_tz("M*" , 13);
    add_tz("M**", 14);
  }
}

//=============================================================================
void TimeZone::add_tz(const std::string& name,double hrs)
{
  (*s_zone_table)[name] = (int)((double)SECONDS_PER_HOUR * hrs);
}

};
