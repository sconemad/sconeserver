/* SconeServer (http://www.sconemad.com)

Date

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


#include <sconex/Date.h>
#include <sconex/ScriptTypes.h>
#include <sconex/utils.h>
namespace scx {

Date::MonthNameMap* Date::s_month_table = 0;

//=============================================================================
Date::Date()
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
}

//=============================================================================
Date::Date(int t, bool local) 
  : m_time(),
    m_timezone(local ? TimeZone::local(t) : TimeZone())
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
  m_time.tv_sec = t;
}

//=============================================================================
Date::Date(const timeval& tv, bool local)
  : m_time(tv),
    m_timezone(local ? TimeZone::local(tv.tv_sec) : TimeZone())
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
}

//=============================================================================
Date::Date(int year, int month, int mday, int hour, int minute, int second,
	   bool local) 
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  struct tm tms;
  tms.tm_isdst = -1;
  tms.tm_wday = -1;
  tms.tm_yday = -1;
  tms.tm_sec = second;
  tms.tm_min = minute;
  tms.tm_hour = hour;
  tms.tm_mday = mday;
  tms.tm_mon = month-1;
  tms.tm_year = year-1900;

  if (local) {
    m_time.tv_sec = mktime(&tms);

    // Get local timezone
    m_timezone = TimeZone::local(m_time.tv_sec);

    // Adjust to UTC from local timezone
    m_time.tv_sec -= m_timezone.seconds();
    
  } else {
    m_time.tv_sec = mktime(&tms);
    m_time.tv_sec += TimeZone::local(m_time.tv_sec).seconds();
  }
}

//=============================================================================
Date::Date(const std::string& str, bool local)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
  parse_string(str,local);
}

//=============================================================================
Date::Date(const ScriptRef* args)
  : m_time()
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();

  const ScriptInt* a_local = get_method_arg<ScriptInt>(args,1,"local");
  bool local = false;
  if (a_local) local = a_local->get_int();

  const ScriptString* a_string = get_method_arg<ScriptString>(args,0,"string");
  if (a_string) {
    // Parse from string
    parse_string(a_string->get_string(),local);

  } else {
    const ScriptInt* a_int = get_method_arg<ScriptInt>(args,0,"value");
    if (a_int) {
      m_time.tv_sec = a_int->get_int();
      
    } else {
      ::gettimeofday(&m_time,0);
    }
    if (local) {
      m_timezone = TimeZone::local(m_time);

      // Adjust to UTC from local timezone
      m_time.tv_sec -= m_timezone.seconds();
    }
  }
}

//=============================================================================
Date::Date(const Date& c)
  : ScriptObject(c),
    m_time(c.m_time),
    m_timezone(c.m_timezone)
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
}

//=============================================================================
Date::~Date()
{
  DEBUG_COUNT_DESTRUCTOR(Date);
}

//=============================================================================
Date* Date::new_copy() const
{
  return new Date(*this);
}

//=============================================================================
Date Date::now(bool local)
{
  timeval tv;
  ::gettimeofday(&tv,0);
  return Date(tv,local);
}

//=============================================================================
bool Date::valid() const
{
  return m_time.tv_sec > 0 || m_time.tv_usec > 0;
}

//=============================================================================
int Date::microsecond() const
{
  return m_time.tv_usec;
}

//=============================================================================
int Date::second() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return tms.tm_sec;
  }
  return 0;
}
  
//=============================================================================
int Date::minute() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return tms.tm_min;
  }
  return 0;
}

//=============================================================================
int Date::hour() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return tms.tm_hour;
  }
  return 0;
}

//=============================================================================
int Date::mday() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return tms.tm_mday;
  }
  return 0;
}

//=============================================================================
Date::Month Date::month() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return (Date::Month)tms.tm_mon;
  }
  return Date::Jan;
}

//=============================================================================
int Date::year() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return 1900 + tms.tm_year;
  }
  return 0;
}

//=============================================================================
Date::Day Date::day() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return (Date::Day)tms.tm_wday;
  }
  return Date::Sun;
}

//=============================================================================
int Date::yday() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return tms.tm_yday;
  }
  return 0;
}

//=============================================================================
Time Date::time() const
{
  struct tm tms;
  if (get_tms(tms)) {
    return Time(tms.tm_hour,tms.tm_min,tms.tm_sec);
  }
  return Time();
}

//=============================================================================
Date Date::operator+(const Time& t) const
{
  //  Date d(m_time + t.m_time);
  timeval tv;
  timeradd(&m_time, &t.m_time, &tv);
  Date d(tv);
  d.m_timezone = m_timezone;
  return d;
}

//=============================================================================
Date Date::operator-(const Time& t) const
{
  timeval tv;
  timersub(&m_time, &t.m_time, &tv);
  Date d(tv);
  d.m_timezone = m_timezone;
  return d;
}

//=============================================================================
Time Date::operator-(const Date& t) const
{
  timeval tv;
  timersub(&m_time, &t.m_time, &tv);
  return Time(tv);
}

//=============================================================================
Date& Date::operator=(const Date& t)
{
  if (this != &t) {
    m_time = t.m_time;
    m_timezone = t.m_timezone;
  }
  return *this;
}

//=============================================================================
Date& Date::operator+=(const Time& t)
{
  timeradd(&m_time, &t.m_time, &m_time);
  return *this;
}

//=============================================================================
Date& Date::operator-=(const Time& t)
{
  timersub(&m_time, &t.m_time, &m_time);
  return *this;
}

//=============================================================================
bool Date::operator==(const Date& t) const
{
  return timercmp(&m_time,&t.m_time,==);
}

//=============================================================================
bool Date::operator!=(const Date& t) const
{
  return timercmp(&m_time,&t.m_time,!=);
}

//=============================================================================
bool Date::operator>(const Date& t) const
{
  return timercmp(&m_time,&t.m_time,>);
}

//=============================================================================
bool Date::operator>=(const Date& t) const
{
  return timercmp(&m_time,&t.m_time,>) || timercmp(&m_time,&t.m_time,==);
}

//=============================================================================
bool Date::operator<(const Date& t) const
{
  return timercmp(&m_time,&t.m_time,<);
}

//=============================================================================
bool Date::operator<=(const Date& t) const
{
  return timercmp(&m_time,&t.m_time,<) || timercmp(&m_time,&t.m_time,==);
}

//=============================================================================
std::string Date::string() const
{
  struct tm tms;
  if (!get_tms(tms)) {
    return std::string("ERROR");
  }

  std::string day;
  switch (tms.tm_wday) {
    case 0: day="Sun"; break;
    case 1: day="Mon"; break;
    case 2: day="Tue"; break;
    case 3: day="Wed"; break;
    case 4: day="Thu"; break;
    case 5: day="Fri"; break;
    case 6: day="Sat"; break;
  }

  std::string month;
  switch (tms.tm_mon) {
    case 0: month="Jan"; break;
    case 1: month="Feb"; break;
    case 2: month="Mar"; break;
    case 3: month="Apr"; break;
    case 4: month="May"; break;
    case 5: month="Jun"; break;
    case 6: month="Jul"; break;
    case 7: month="Aug"; break;
    case 8: month="Sep"; break;
    case 9: month="Oct"; break;
    case 10: month="Nov"; break;
    case 11: month="Dec"; break;
  }

  std::ostringstream oss;
  oss << std::setw(3) << day << ", "
      << std::setfill('0') << std::setw(2) << tms.tm_mday << " "
      << std::setw(3) << month << " "
      << std::setw(4) << (1900+tms.tm_year) << " ";
  oss << std::setfill('0') << std::setw(2) << tms.tm_hour << ":"
      << std::setfill('0') << std::setw(2) << tms.tm_min << ":"
      << std::setfill('0') << std::setw(2) << tms.tm_sec << " ";
  oss << timezone().string();

  return oss.str();
}

//=============================================================================
std::string Date::ansi_string() const
{
  struct tm tms;
  if (!get_tms(tms)) {
    return std::string("ERROR");
  }
  char str[32];
  if (0 == asctime_r(&tms, str)) {
    return std::string("ERROR");
  }
  return std::string(str);
}

//=============================================================================
std::string Date::code() const
{
  struct tm tms;
  if (!get_tms(tms)) {
    return std::string("ERROR");
  }

  std::ostringstream oss;
  oss << std::setw(4) << (1900+tms.tm_year) << "-"
      << std::setfill('0') << std::setw(2) << (1+tms.tm_mon) << "-"
      << std::setfill('0') << std::setw(2) << tms.tm_mday << " ";
  oss << std::setfill('0') << std::setw(2) << tms.tm_hour << ":"
      << std::setfill('0') << std::setw(2) << tms.tm_min << ":"
      << std::setfill('0') << std::setw(2) << tms.tm_sec << " "
      << timezone().string();

  return oss.str();
}

//=============================================================================
std::string Date::dcode() const
{
  struct tm tms;
  if (!get_tms(tms)) {
    return std::string("ERROR");
  }

  std::ostringstream oss;
  oss << std::setw(4) << (1900+tms.tm_year)
      << std::setfill('0') << std::setw(2) << (1+tms.tm_mon)
      << std::setfill('0') << std::setw(2) << tms.tm_mday;
  oss << std::setfill('0') << std::setw(2) << tms.tm_hour
      << std::setfill('0') << std::setw(2) << tms.tm_min
      << std::setfill('0') << std::setw(2) << tms.tm_sec;

  return oss.str();
}

//=============================================================================
std::string Date::format(const std::string& fmt) const
{
  struct tm tms;
  if (!get_tms(tms)) {
    return std::string("ERROR");
  }

  const int strmax = 256;
  char str[strmax];
  int len = ::strftime(str,strmax,fmt.c_str(),&tms);
  return std::string(str,len);
}
  
//=============================================================================
time_t Date::epoch_seconds() const
{
  return m_time.tv_sec;
}
 
//=============================================================================
const TimeZone& Date::timezone() const
{
  return m_timezone;
}

//=============================================================================
Date Date::to_zone(const TimeZone& timezone) const
{
  Date d(m_time);
  d.set_timezone(timezone);
  return d;
}
//=============================================================================
void Date::set_timezone(const TimeZone& timezone)
{
  m_timezone = timezone;
}

//=============================================================================
std::string Date::get_string() const
{
  return dcode();
}

//=============================================================================
int Date::get_int() const
{
  return m_time.tv_sec;
}
//=============================================================================
ScriptRef* Date::script_op(const ScriptAuth& auth,
			   const ScriptRef& ref,
			   const ScriptOp& op,
			   const ScriptRef* right)
{
  if (right) { // binary ops

    const Date* rd = dynamic_cast<const Date*>(right->object());
    if (rd) { // Date x Date ops
      switch (op.type()) {
      case ScriptOp::Subtract:
	return new ScriptRef(new Time(*this - *rd));
      case ScriptOp::Equality:
	return new ScriptRef(new ScriptInt(*this == *rd));
      case ScriptOp::Inequality:
	return new ScriptRef(new ScriptInt(*this != *rd));
      case ScriptOp::GreaterThan:
	return new ScriptRef(new ScriptInt(*this > *rd));
      case ScriptOp::GreaterThanOrEqualTo:
	return new ScriptRef(new ScriptInt(*this >= *rd));
      case ScriptOp::LessThan:
	return new ScriptRef(new ScriptInt(*this < *rd));
      case ScriptOp::LessThanOrEqualTo:
	return new ScriptRef(new ScriptInt(*this <= *rd));
      default: break;
      }
    }

    const Time* rt = dynamic_cast<const Time*>(right->object());
    if (rt) { // Date x Time ops
      switch (op.type()) {
      case ScriptOp::Add:
	return new ScriptRef(new Date(*this + *rt));
      case ScriptOp::Subtract:
	return new ScriptRef(new Date(*this - *rt));
      default: break;
      }
    }
    
    if (ScriptOp::Lookup == op.type()) {
      std::string name = right->object()->get_string();
      if (name == "second") return ScriptInt::new_ref(second());
      if (name == "minute") return ScriptInt::new_ref(minute());
      if (name == "hour") return ScriptInt::new_ref(hour());
      if (name == "mday") return ScriptInt::new_ref(mday());
      if (name == "month") return ScriptInt::new_ref(month());
      if (name == "year") return ScriptInt::new_ref(year());
      if (name == "day") return ScriptInt::new_ref(day());
      if (name == "yday") return ScriptInt::new_ref(yday());
      if (name == "code") return ScriptString::new_ref(code());
      if (name == "string") return ScriptString::new_ref(string());
      if (name == "ansi") return ScriptString::new_ref(ansi_string());
      if (name == "epoch_seconds") return ScriptInt::new_ref(m_time.tv_sec);
      if (name == "timezone") 
	return new ScriptRef(m_timezone.new_copy());

      if (name == "format" ||
	  name == "to_zone") {
	return new ScriptMethodRef(ref,name);
      }
    }
  }
  
  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* Date::script_method(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const std::string& name,
			       const ScriptRef* args)
{
  if ("format" == name) {
    const ScriptString* a_fmt = get_method_arg<ScriptString>(args,0,"format");
    if (!a_fmt) return ScriptError::new_ref("No format specified");
    return ScriptString::new_ref(format(a_fmt->get_string()));
  }
  
  if ("to_zone" == name) {
    const TimeZone* a_zone = get_method_arg<TimeZone>(args,0,"timezone");
    if (!a_zone) 
      return ScriptError::new_ref("to_zone() Must specify a valid timezone");
    return new ScriptRef(to_zone(*a_zone).new_copy());
  }
  
  return ScriptObject::script_method(auth,ref,name,args);
}


//=============================================================================
bool Date::get_tms(struct tm& tms) const
{
  time_t tadj = m_time.tv_sec + m_timezone.seconds();
  
  if (0 == gmtime_r(&tadj, &tms)) {
    return false;
  }

  return true;
}

//=============================================================================
void Date::parse_string(const std::string& str, bool local)
{
  int hour=-1;
  int minute=-1;
  int second=-1;
  int date=-1;
  int month=-1;
  int year=-1;
  bool got_zone=false;
  bool year_first=false;
  char pre=' ';
  std::string::const_iterator it = str.begin();

  while (it < str.end()) {

    if (isalnum(*it)) {
      std::string::const_iterator it1(it);

      if (isalpha(*it)) {
        while (it < str.end() && isalpha(*it)) ++it;
        std::string token(it1,it);
        strup(token);
        if (month<0) {
          MonthNameMap::const_iterator m_it = s_month_table->find(token);
          if (m_it != s_month_table->end()) {
            month = m_it->second;
            continue;
          }
        }
        
        if (!got_zone) {
          TimeZone::TimeZoneOffsetMap::const_iterator z_it =
            TimeZone::s_zone_table->find(token);
          if (z_it != TimeZone::s_zone_table->end()) {
            m_timezone = TimeZone(std::string(token));
            continue;
          }
        }
        

      } else {
        while (it < str.end() && isdigit(*it)) ++it;
        std::string token(it1,it);
        int num = atoi(token.c_str());

        if (!year_first && date<0 && num>0 && num<=31) {
          date=num;
        } else if (!year_first && month<0 && num>0 && num<=12) {
          month=num-1;
        } else if (year_first && month<0 && num>0 && num<=12) {
          month=num-1;
        } else if (year_first && date<0 && num>0 && num<=31) {
          date=num;
        } else if (year<0 && num>=1970) {
          year=num;
	  year_first = (date<0 && month<0);
        } else if (hour<0 && num>=0 && num<24) {
          hour=num;
        } else if (minute<0 && num>=0 && num<60) {
          minute=num;
        } else if (second<0 && num>=0 && num<60) {
          second=num;
        } else if (year<0 && num>=70 && num<=99) {
          year=1900+num;
        } else if (year<0 && num>=0 && num<=99) {
          year=2000+num;
        } else if (!got_zone && token.length()==4 &&
                   (pre=='-' || pre=='+')) {
          char ps[2] = {pre,'\0'};
          m_timezone = TimeZone(std::string(ps) + token);
          got_zone = true;
        }
      }
    } else {
      pre = (*it);
      ++it;
    }
  }

  if (month<0 && year<0 && date>0 && hour>0 && minute>0) {
    second=minute;
    minute=hour;
    hour=date;
    date=-1;
  } else if (month<0 && year<0 && date>0 && hour>0) {
    minute=hour;
    hour=date;
    date=-1;
  }

  struct tm tms;
  tms.tm_isdst = -1;
  tms.tm_wday = -1;
  tms.tm_yday = -1;
  tms.tm_sec = second<0 ? 0 : second;
  tms.tm_min = minute<0 ? 0 : minute;
  tms.tm_hour = hour<0 ? 0 : hour;
  tms.tm_mday = date<0 ? 1 : date;
  tms.tm_mon = month<0 ? 0 : month;
  tms.tm_year = year<1970 ? 70 : (year-1900);

  if (local && !got_zone) {
    m_time.tv_sec = mktime(&tms);
  } else {
    m_time.tv_sec = mktime(&tms);
    m_time.tv_sec += TimeZone::local(m_time.tv_sec).seconds();
  }

  // Adjust to UTC from given timezone
  m_time.tv_sec -= m_timezone.seconds();
}

//=============================================================================
void Date::init_tables()
{
  if (s_month_table == 0) {
    s_month_table = new MonthNameMap();
    
    (*s_month_table)["JAN"]=0;
    (*s_month_table)["JANUARY"]=0;
    (*s_month_table)["JANVIER"]=0;
    (*s_month_table)["JANUAR"]=0;

    (*s_month_table)["FEB"]=1;
    (*s_month_table)["FEBRUARY"]=1;
    (*s_month_table)["FÉVRIER"]=1;
    (*s_month_table)["FEVRIER"]=1;
    (*s_month_table)["FEBRUAR"]=1;
    
    (*s_month_table)["MAR"]=2;
    (*s_month_table)["MARCH"]=2;
    (*s_month_table)["MARS"]=2;
    (*s_month_table)["MÄRZ"]=2;
    (*s_month_table)["MARZ"]=2;
      
    (*s_month_table)["APR"]=3;
    (*s_month_table)["APRIL"]=3;
    (*s_month_table)["AVRIL"]=3;
    
    (*s_month_table)["MAY"]=4;
    (*s_month_table)["MAI"]=4;
    
    (*s_month_table)["JUN"]=5;
    (*s_month_table)["JUNE"]=5;
    (*s_month_table)["JUIN"]=5;
    (*s_month_table)["JUNI"]=5;
    
    (*s_month_table)["JUL"]=6;
    (*s_month_table)["JULY"]=6;
    (*s_month_table)["JUILLET"]=6;
    (*s_month_table)["JULI"]=6;
    
    (*s_month_table)["AUG"]=7;
    (*s_month_table)["AUGUST"]=7;
    (*s_month_table)["AOÛT"]=7;
    (*s_month_table)["AOUT"]=7;
    
    (*s_month_table)["SEP"]=8;
    (*s_month_table)["SEPTEMBER"]=8;
    (*s_month_table)["SEPTEMBRE"]=8;
    
    (*s_month_table)["OCT"]=9;
    (*s_month_table)["OCTOBER"]=9;
    (*s_month_table)["OCTOBRE"]=9;
    (*s_month_table)["OKTOBER"]=9;

    
    (*s_month_table)["NOV"]=10;
    (*s_month_table)["NOVEMBER"]=10;
    (*s_month_table)["NOVEMBRE"]=10;
    
    (*s_month_table)["DEC"]=11;
    (*s_month_table)["DECEMBER"]=11;
    (*s_month_table)["DÉCEMBRE"]=11;
    (*s_month_table)["DECEMBRE"]=11;
    (*s_month_table)["DEZEMBER"]=11;
  }
}

};
