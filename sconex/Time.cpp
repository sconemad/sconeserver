/* SconeServer (http://www.sconemad.com)

Time and TimeZone

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


#include <sconex/Time.h>
#include <sconex/Date.h>
#include <sconex/ScriptTypes.h>
#include <sconex/utils.h>
namespace scx {

struct time_tok {
  int value;
  std::string sep;
};

//=============================================================================
Time::Time()
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
}

//=============================================================================
Time::Time(int t)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time.tv_sec = t;
}

//=============================================================================
Time::Time(const timeval& tv)
  : m_time(tv)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
}

//=============================================================================
Time::Time(int minutes,int seconds)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time.tv_sec = (abs(minutes)*SECONDS_PER_MINUTE) + abs(seconds);
  m_time.tv_sec *= (minutes<0) ? -1:1;
}

//=============================================================================
Time::Time(int hours,int minutes,int seconds)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time.tv_sec = (abs(hours)*SECONDS_PER_HOUR) + 
                  (abs(minutes)*SECONDS_PER_MINUTE) + 
                  abs(seconds);
  m_time.tv_sec *= (hours<0) ? -1:1;
}

//=============================================================================
Time::Time(int days,int hours,int minutes,int seconds)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time.tv_sec = (abs(days)*SECONDS_PER_DAY) + 
                  (abs(hours)*SECONDS_PER_HOUR) + 
                  (abs(minutes)*SECONDS_PER_MINUTE) + 
                  abs(seconds);
  m_time.tv_sec *= (days<0) ? -1:1;
}

//=============================================================================
Time::Time(int weeks,int days,int hours,int minutes,int seconds)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
  m_time.tv_sec = (abs(weeks)*SECONDS_PER_WEEK) + 
                  (abs(days)*SECONDS_PER_DAY) + 
                  (abs(hours)*SECONDS_PER_HOUR) + 
                  (abs(minutes)*SECONDS_PER_MINUTE) + 
                  abs(seconds);
  m_time.tv_sec *= (weeks<0) ? -1:1;
}

//=============================================================================
Time::Time(const ScriptRef* args)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);

  const ScriptInt* a_int = get_method_arg<ScriptInt>(args,0,"value");
  if (a_int) {
    m_time.tv_sec = a_int->get_int();
  }
}

//=============================================================================
Time::Time(const std::string& str)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
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
      case 0: m_time.tv_sec += sign*cur.value*SECONDS_PER_WEEK; break;
      case 1: m_time.tv_sec += sign*cur.value*SECONDS_PER_DAY; break;
      case 2: m_time.tv_sec += sign*cur.value*SECONDS_PER_HOUR; break;
      case 3: m_time.tv_sec += sign*cur.value*SECONDS_PER_MINUTE; break;
      case 4: m_time.tv_sec += sign*cur.value; break;
    }
    ts.pop();
  }
}

//=============================================================================
Time::Time(const Time& c)
  : ScriptObject(c),
    m_time(c.m_time)
{
  DEBUG_COUNT_CONSTRUCTOR(Time);
}

//=============================================================================
Time::~Time()
{
  DEBUG_COUNT_DESTRUCTOR(Time);
}

//=============================================================================
ScriptObject* Time::new_copy() const
{
  return new Time(*this);
}

//=============================================================================
int Time::seconds() const
{
  return (int)m_time.tv_sec;
}

//=============================================================================
double Time::to_microseconds() const
{
  return ((double)m_time.tv_sec * 1000000.0) + m_time.tv_usec;
}

//=============================================================================
double Time::to_milliseconds() const
{
  return ((double)m_time.tv_sec * 1000.0) + ((double)m_time.tv_usec / 1000.0);
}

//=============================================================================
double Time::to_seconds() const
{
  return m_time.tv_sec + ((double)m_time.tv_usec / 1000000.0);
}

//=============================================================================
double Time::to_weeks() const
{
  return m_time.tv_sec / (double)SECONDS_PER_WEEK;
}

//=============================================================================
double Time::to_days() const
{
  return m_time.tv_sec / (double)SECONDS_PER_DAY;
}

//=============================================================================
double Time::to_hours() const
{
  return m_time.tv_sec / (double)SECONDS_PER_HOUR;
}

//=============================================================================
double Time::to_minutes() const
{
  return m_time.tv_sec / (double)SECONDS_PER_MINUTE;
}

//=============================================================================
void Time::get(int& minutes,int& seconds) const
{
  seconds = m_time.tv_sec;
  
  minutes = (int)(seconds / SECONDS_PER_MINUTE);
  seconds -= minutes*SECONDS_PER_MINUTE;
}

//=============================================================================
void Time::get(int& hours,int& minutes,int& seconds) const
{
  seconds = m_time.tv_sec;

  hours = (int)(seconds / SECONDS_PER_HOUR);
  seconds -= hours*SECONDS_PER_HOUR;
  
  minutes = (int)(seconds / SECONDS_PER_MINUTE);
  seconds -= minutes*SECONDS_PER_MINUTE;
}

//=============================================================================
void Time::get(int& days,int& hours,int& minutes,int& seconds) const
{
  seconds = m_time.tv_sec;

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
  seconds = m_time.tv_sec;

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
int Time::microseconds() const
{
  return m_time.tv_usec;
}

//=============================================================================
std::string Time::string(Precision precision) const
{
  int weeks,days,hours,minutes,seconds;
  get(weeks,days,hours,minutes,seconds);
  weeks = abs(weeks);
  days = abs(days);
  hours = abs(hours);
  minutes = abs(minutes);
  seconds = abs(seconds);

  std::ostringstream oss;
  oss << (m_time.tv_sec < 0 ? "-" : "");
  bool space = false;

  if (weeks > 0 || precision == Weeks) {
    oss << weeks << " " << (weeks==1 ? "Week" : "Weeks");
    space = true;
  }

  if (weeks > 0 || days > 0 || precision == Days) {
    if (space) oss << " ";
    oss << days << " " << (days==1 ? "Day" : "Days");
    space = true;
  }

  if (precision >= Hours) {
    if (space) oss << " ";
    oss << std::setfill('0') << std::setw(2) << hours;

    if (precision >= Minutes) {
      oss << ":"
	  << std::setfill('0') << std::setw(2) << minutes;

      if (precision >= Seconds) {
	oss << ":"
	    << std::setfill('0') << std::setw(2) << seconds;

	if (precision == Milliseconds) {
	  oss << "." 
	      << std::setfill('0') << std::setw(3) 
	      << (int)(m_time.tv_usec / 1000);
	} else if (precision == Microseconds) {
	  oss << "."
	      << std::setfill('0') << std::setw(6) << m_time.tv_usec;
	}
      }
    }
  }
  
  return oss.str();
}

//=============================================================================
Time& Time::operator=(const Time& t)
{
  if (this != &t) {
    m_time = t.m_time;
  }
  return *this;
}

//=============================================================================
Time Time::operator+(const Time& t) const
{
  timeval tv;
  timeradd(&m_time, &t.m_time, &tv);
  return Time(tv);
}

//=============================================================================
Time Time::operator-(const Time& t) const
{
  timeval tv;
  timersub(&m_time, &t.m_time, &tv);
  return Time(tv);
}

//=============================================================================
std::string Time::get_string() const
{
  return string();
}

//=============================================================================
int Time::get_int() const
{
  return m_time.tv_sec;
}

//=============================================================================
ScriptRef* Time::script_op(const ScriptAuth& auth,
			   const ScriptRef& ref,
			   const ScriptOp& op,
			   const ScriptRef* right)
{
  if (right) { // binary ops
    const Time* rt = dynamic_cast<const Time*>(right->object());
    if (rt) { // Time X Time ops
      switch (op.type()) {
      case ScriptOp::Add:
	return new ScriptRef(new Time(*this + *rt));
      case ScriptOp::Subtract:
	return new ScriptRef(new Time(*this - *rt));
      default: break;
      }
    }
    
    if (ScriptOp::Lookup == op.type()) {
      std::string name = right->object()->get_string();
      if (name == "microseconds") return ScriptReal::new_ref(to_microseconds());
      if (name == "milliseconds") return ScriptReal::new_ref(to_milliseconds());
      if (name == "seconds") return ScriptReal::new_ref(to_seconds());
      if (name == "weeks") return ScriptReal::new_ref(to_weeks());
      if (name == "days") return ScriptReal::new_ref(to_days());
      if (name == "hours") return ScriptReal::new_ref(to_hours());
      if (name == "minutes") return ScriptReal::new_ref(to_minutes());

      if (name == "string") 
	return ScriptString::new_ref(string());
      if (name == "string_milliseconds") 
	return ScriptString::new_ref(string(Milliseconds));
      if (name == "string_microseconds") 
	return ScriptString::new_ref(string(Microseconds));
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
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
  m_time.tv_sec = (abs(hours)*SECONDS_PER_HOUR) +
                  (abs(minutes)*SECONDS_PER_MINUTE);
  m_time.tv_sec *= (hours<0 || minutes<0) ? -1:1;
}

//=============================================================================
TimeZone::TimeZone(const std::string& str)
  : Time(0)
{
  DEBUG_COUNT_CONSTRUCTOR(TimeZone);
  init_tables();
  parse_string(str);
}

//=============================================================================
TimeZone::TimeZone(const ScriptRef* args)
{
  DEBUG_COUNT_CONSTRUCTOR(TimeZone);
  init_tables();

  const ScriptInt* a_int = get_method_arg<ScriptInt>(args,0,"value");
  if (a_int) {
    m_time.tv_sec = 3600 * a_int->get_int();
    return;
  }
  
  const ScriptReal* a_real = get_method_arg<ScriptReal>(args,0,"value");
  if (a_real) {
    m_time.tv_sec = (int)(3600.0*a_real->get_real());
    return;
  }

  // Otherwise assume its a string
  const ScriptObject* a = get_method_arg<ScriptObject>(args,0,"string");
  if (a) {
    parse_string(a->get_string());
  }
}

//=============================================================================
TimeZone::TimeZone(const TimeZone& c)
  : Time(c)
{
  DEBUG_COUNT_CONSTRUCTOR(TimeZone);
  init_tables();
}

//=============================================================================
TimeZone::~TimeZone()
{
  DEBUG_COUNT_DESTRUCTOR(TimeZone);
}

//=============================================================================
ScriptObject* TimeZone::new_copy() const
{
  return new TimeZone(*this);
}

//=============================================================================
TimeZone TimeZone::utc()
{
  return TimeZone();
}

//=============================================================================
TimeZone TimeZone::local(const Date& date)
{
  struct tm tms;
  const time_t t = date.epoch_seconds();

  // Get local time on this date
  if (0 == localtime_r(&t, &tms)) {
    return TimeZone();
  }
  tms.tm_isdst=-1;
  time_t t_loc = mktime(&tms);

  // Get universal time on this date
  if (0 == gmtime_r(&t, &tms)) {
    return TimeZone();
  }
  tms.tm_isdst=-1;
  time_t t_gmt = mktime(&tms);

  // Local timezone is then the difference between these
  return TimeZone(t_loc - t_gmt);
}

//=============================================================================
TimeZone TimeZone::operator+(const Time& t) const
{
  return TimeZone(m_time.tv_sec + t.m_time.tv_sec);
}

//=============================================================================
TimeZone TimeZone::operator-(const Time& t) const
{
  return TimeZone(m_time.tv_sec - t.m_time.tv_sec);
}

//=============================================================================
std::string TimeZone::string() const
{
  int hours,minutes,seconds;
  get(hours,minutes,seconds);
  hours = abs(hours);
  minutes = abs(minutes);

  std::ostringstream oss;
  oss << (m_time.tv_sec < 0 ? "-" : "+")
      << std::setfill('0') << std::setw(2) << hours
      << std::setfill('0') << std::setw(2) << minutes;

  return oss.str();
}

//=============================================================================
void TimeZone::parse_string(const std::string& str)
{
  // First lookup in zone table
  TimeZoneOffsetMap::const_iterator zit = s_zone_table->find(str);
  if (zit != s_zone_table->end()) {
    m_time.tv_sec = zit->second;
    
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
        m_time.tv_sec = (hours*SECONDS_PER_HOUR) + (minutes*SECONDS_PER_MINUTE);
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
    m_time.tv_sec *= sign;
  }
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
