/* SconeServer (http://www.sconemad.com)

SconeX Time and Date classes

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxTimeDate_h
#define scxTimeDate_h

#include "sconex/sconex.h"
#include <time.h>
namespace scx {

// Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
// Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
// Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

const int SECONDS_PER_WEEK   = 604800;
const int SECONDS_PER_DAY    = 86400;
const int SECONDS_PER_HOUR   = 3600;
const int SECONDS_PER_MINUTE = 60;

class Date;

//=============================================================================
class SCONEX_API Time {

public:

  explicit Time(time_t t=0);
  Time(int minutes,int seconds);
  Time(int hours,int minutes,int seconds);
  Time(int days,int hours,int minutes,int seconds);
  Time(int weeks,int days,int hours,int minutes,int seconds);
  Time(const std::string& str);
  Time(const Time& c);
  ~Time();

  // Get the time in seconds
  int seconds() const;

  // These return the total time converted to these units
  double to_weeks() const;
  double to_days() const;
  double to_hours() const;
  double to_minutes() const;

  void get(int& minutes,int& seconds) const;
  void get(int& hours,int& minutes,int& seconds) const;
  void get(int& days,int& hours,int& minutes,int& seconds) const;
  void get(int& weeks,int& days,int& hours,int& minutes,int& seconds) const;

  Time operator+(const Time& t) const;
  Time operator-(const Time& t) const;

  std::string string() const;

protected:
  friend class Date;
  friend class TimeZone;
  time_t m_time;
  
};


//=============================================================================
class SCONEX_API TimeZone : public Time {

public:

  explicit TimeZone(time_t t=0);
  TimeZone(int hours,int minutes);
  TimeZone(const std::string& str);
  ~TimeZone();

  static TimeZone utc();
  // Get the UTC timezone (i.e. +0000 or GMT)
  
  static TimeZone local(const Date& date);
  // Get the local timezone including DST correction for this date
  
  TimeZone operator+(const Time& t) const;
  TimeZone operator-(const Time& t) const;
  
  std::string string() const;

protected:
  friend class Date;
  
  void init_tables();
  static std::map<std::string,int>* s_zone_table;

};


//=============================================================================
class SCONEX_API Date {

public:

  enum Day { Sun, Mon, Tue, Wed, Thu, Fri, Sat };
  enum Month { Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec };

  Date(
    time_t t=0,
    bool local=false
  );
  
  Date(
    int year,
    int month,
    int mday,
    int hour,
    int minute,
    int second,
    bool local=false
  );
  
  Date(
    const std::string& str,
    bool local=false
  );
  
  Date(const Date& c);
  
  ~Date();

  static Date now(bool local=false);

  // Is this a valid date?
  bool valid() const;
  
  int second() const;
  int minute() const;
  int hour() const;
  int mday() const;
  Month month() const;
  int year() const;

  Day day() const;
  int yday() const;

  Time time() const;
  
  Date operator+(const Time& t) const;
  Date operator-(const Time& t) const;
  Time operator-(const Date& t) const;

  Date& operator=(const Date& t);
  Date& operator+=(const Time& t);
  Date& operator-=(const Time& t);

  bool operator==(const Date& t) const;
  bool operator!=(const Date& t) const;
  bool operator>(const Date& t) const;
  bool operator>=(const Date& t) const;
  bool operator<(const Date& t) const;
  bool operator<=(const Date& t) const;

  std::string string() const;
  std::string ansi_string() const;
  
  std::string code() const;
  // "YYYY-MM-DD hh:mm:ss +zhzm"

  std::string dcode() const;
  // "YYYYMMDDhhmmss"
    
  const bool is_local() const;
  void set_local(bool yesno);

  TimeZone timezone() const;
  
  time_t epoch_seconds() const;
 
protected:

  bool get_tms(struct tm& tms) const;
  
  time_t m_time;
  bool m_local;
  
  void init_tables();
  static std::map<std::string,int>* s_month_table;

};

};
#endif
