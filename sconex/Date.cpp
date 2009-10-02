/* SconeServer (http://www.sconemad.com)

Date

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


#include "sconex/Date.h"
#include "sconex/utils.h"
namespace scx {

Date::MonthNameMap* Date::s_month_table = 0;

//=============================================================================
Date::Date()
  : m_time(new time_t(0)),
    m_local(new bool(false))
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
}

//=============================================================================
Date::Date(
  int t,
  bool local
)
  : m_time(new time_t(t)),
    m_local(new bool(local))
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
}

//=============================================================================
Date::Date(
  int year,
  int month,
  int mday,
  int hour,
  int minute,
  int second,
  bool local
)
  : m_time(new time_t()),
    m_local(new bool(local))
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

  *m_time = mktime(&tms) - timezone().seconds();
}

//=============================================================================
Date::Date(
  const std::string& str,
  bool local
)
  : m_time(new time_t(0)),
    m_local(new bool(local))
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
  int hour=-1;
  int minute=-1;
  int second=-1;
  int date=-1;
  int month=-1;
  int year=-1;
  bool got_zone=false;
  TimeZone tz;
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
            tz = TimeZone(std::string(token));
            continue;
          }
        }
        

      } else {
        while (it < str.end() && isdigit(*it)) ++it;
        std::string token(it1,it);
        int num = atoi(token.c_str());

        if (date<0 && num>0 && num<=31) {
          date=num;
        } else if (month<0 && num>0 && num<=12) {
          month=num-1;
        } else if (year<0 && num>=1970) {
          year=num;
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
          tz = TimeZone(std::string(ps) + token);
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

  *m_local = local;
  *m_time = mktime(&tms);

  if (!local || got_zone) {
    // Adjust mktime results from local
    *m_time += TimeZone::local(Date(*m_time)).seconds();
  }

  // Adjust to UTC from given timezone
  *m_time -= tz.seconds();
}

//=============================================================================
Date::Date(Arg* args)
  : m_time(new time_t()),
    m_local(new bool())
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();

  ArgList* l = dynamic_cast<ArgList*>(args);

  Arg* a = l->get(0);
  ArgInt* a_int = dynamic_cast<ArgInt*>(a);
  if (a_int) {
    *m_time = a->get_int();
    
  } else {
    time_t tmp;
    *m_time = ::time(&tmp);
  }

  Arg* local = l->get(1);
  *m_local = (local ? local->get_int() : 0);
}

//=============================================================================
Date::Date(const Date& c)
  : Arg(c),
    m_time(new time_t(*c.m_time)),
    m_local(new bool(*c.m_local))
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
}

//=============================================================================
Date::Date(RefType ref, Date& c)
  : Arg(ref,c),
    m_time(c.m_time),
    m_local(c.m_local)
{
  DEBUG_COUNT_CONSTRUCTOR(Date);
  init_tables();
}

//=============================================================================
Date::~Date()
{
  if (last_ref()) {
    delete m_time;
    delete m_local;
  }
  DEBUG_COUNT_DESTRUCTOR(Date);
}

//=============================================================================
Arg* Date::new_copy() const
{
  return new Date(*this);
}

//=============================================================================
Arg* Date::ref_copy(RefType ref)
{
  return new Date(ref,*this);
}

//=============================================================================
Date Date::now(bool local)
{
  time_t tmp;
  return Date(::time(&tmp),local);
}

//=============================================================================
bool Date::valid() const
{
  return *m_time > 0;
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
  return Date(*m_time + *t.m_time,*m_local);
}

//=============================================================================
Date Date::operator-(const Time& t) const
{
  return Date(*m_time - *t.m_time,*m_local);
}

//=============================================================================
Time Date::operator-(const Date& t) const
{
  return Time(*m_time - *t.m_time);
}

//=============================================================================
Date& Date::operator=(const Date& t)
{
  *m_time = *t.m_time;
  *m_local = *t.m_local;
  return *this;
}

//=============================================================================
Date& Date::operator+=(const Time& t)
{
  *m_time += *t.m_time;
  return *this;
}

//=============================================================================
Date& Date::operator-=(const Time& t)
{
  *m_time -= *t.m_time;
  return *this;
}

//=============================================================================
bool Date::operator==(const Date& t) const
{
  return (*m_time == *t.m_time);
}

//=============================================================================
bool Date::operator!=(const Date& t) const
{
  return (*m_time != *t.m_time);
}

//=============================================================================
bool Date::operator>(const Date& t) const
{
  return (*m_time > *t.m_time);
}

//=============================================================================
bool Date::operator>=(const Date& t) const
{
  return (*m_time >= *t.m_time);
}

//=============================================================================
bool Date::operator<(const Date& t) const
{
  return (*m_time < *t.m_time);
}

//=============================================================================
bool Date::operator<=(const Date& t) const
{
  return (*m_time <= *t.m_time);
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
  return std::string(asctime(&tms));
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
time_t Date::epoch_seconds() const
{
  return *m_time;
}
 
//=============================================================================
const bool Date::is_local() const
{
  return *m_local;
}

//=============================================================================
void Date::set_local(bool yesno)
{
  *m_local = yesno;
}

//=============================================================================
TimeZone Date::timezone() const
{
  if (*m_local) {
    return TimeZone::local(*this);
  }
  return TimeZone::utc();
}

//=============================================================================
std::string Date::get_string() const
{
  return dcode();
}

//=============================================================================
int Date::get_int() const
{
  return *m_time;
}
//=============================================================================
Arg* Date::op(const Auth& auth, OpType optype, const std::string& opname, Arg* right)
{
  if (optype == Arg::Binary) {
    
    Date* rd = dynamic_cast<Date*>(right);
    if (rd) { // Date x Date ops
      if ("-" == opname) { // Minus (difference in time)
	return new Time(*this - *rd);
      } else if ("==" == opname) { // Equality
	return new ArgInt(*this == *rd);
      } else if ("==" == opname) { // Inequality
	return new ArgInt(*this != *rd);
      } else if (">" == opname) { // Greater than
	return new ArgInt(*this > *rd);
      } else if (">=" == opname) { // Greater than or equal to
	return new ArgInt(*this >= *rd);
      } else if ("<" == opname) { // Less than
	return new ArgInt(*this < *rd);
      } else if ("<=" == opname) { // Less than or equal to
	return new ArgInt(*this <= *rd);
      }
    }

    Time* rt = dynamic_cast<Time*>(right);
    if (rt) { // Date x Time ops
      if ("+" == opname) { // Plus
	return new Date(*this + *rt);
      } else if ("-" == opname) { // Minus
	return new Date(*this - *rt);
      }
    }
    
    if ("." == opname) { // Scope resolution
      std::string name = right->get_string();
      if (name == "second") return new ArgInt(second());
      if (name == "minute") return new ArgInt(minute());
      if (name == "hour") return new ArgInt(hour());
      if (name == "mday") return new ArgInt(mday());
      if (name == "month") return new ArgInt(month());
      if (name == "year") return new ArgInt(year());
      if (name == "day") return new ArgInt(day());
      if (name == "yday") return new ArgInt(yday());
      if (name == "timezone") return new ArgString(timezone().string());
      if (name == "code") return new ArgString(code());
      if (name == "string") return new ArgString(string());
      if (name == "ansi") return new ArgString(ansi_string());
      if (name == "epoch_seconds") return new ArgInt(*m_time);
    }
  }
  return Arg::op(auth,optype,opname,right);
}

//=============================================================================
bool Date::get_tms(struct tm& tms) const
{
  struct tm* tmr;
  if (m_local) {
    tmr = localtime(m_time);
  } else {
    tmr = gmtime(m_time);
  }

  if (tmr == 0) {
    return false;
  }

  memcpy(&tms,tmr,sizeof(tm));
  return true;
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
    (*s_month_table)["FEBRUAR"]=1;
    
    (*s_month_table)["MAR"]=2;
    (*s_month_table)["MARCH"]=2;
    (*s_month_table)["MARS"]=2;
    (*s_month_table)["MÄRZ"]=2;
      
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
    (*s_month_table)["DEZEMBER"]=11;
  }
}

};
