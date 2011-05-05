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

#ifndef scxTime_h
#define scxTime_h

#include "sconex/sconex.h"
#include "sconex/ScriptBase.h"
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
class SCONEX_API Time : public ScriptObject {
public:

  Time();
  Time(int t);
  Time(const timeval& tv);
  Time(int minutes,int seconds);
  Time(int hours,int minutes,int seconds);
  Time(int days,int hours,int minutes,int seconds);
  Time(int weeks,int days,int hours,int minutes,int seconds);
  Time(const std::string& str);
  Time(const ScriptRef* args);

  Time(const Time& c);
  ~Time();

  virtual ScriptObject* new_copy() const;

  // Get the time in seconds (ignoring partial seconds)
  int seconds() const;
  
  // These return the total time converted to these units
  double to_microseconds() const;
  double to_milliseconds() const;
  double to_seconds() const;
  double to_weeks() const;
  double to_days() const;
  double to_hours() const;
  double to_minutes() const;

  void get(int& minutes,int& seconds) const;
  void get(int& hours,int& minutes,int& seconds) const;
  void get(int& days,int& hours,int& minutes,int& seconds) const;
  void get(int& weeks,int& days,int& hours,int& minutes,int& seconds) const;

  // Get microsecond component
  int microseconds() const;

  Time& operator=(const Time& t);

  Time operator+(const Time& t) const;
  Time operator-(const Time& t) const;

  enum Precision { 
    Weeks, Days, Hours, Minutes, Seconds, Milliseconds, Microseconds 
  };

  virtual std::string string(Precision precision=Seconds) const;

  // ScriptObject methods:
  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  typedef ScriptRefTo<Time> Ref;

protected:
  friend class Date;
  friend class TimeZone;
  timeval m_time;
  
};


//=============================================================================
class SCONEX_API TimeZone : public Time {
public:

  explicit TimeZone(time_t t=0);
  TimeZone(int hours,int minutes);
  TimeZone(const std::string& str);
  TimeZone(const ScriptRef* args);

  TimeZone(const TimeZone& c);
  ~TimeZone();

  virtual ScriptObject* new_copy() const;

  static TimeZone utc();
  // Get the UTC timezone (i.e. +0000 or GMT)
  
  static TimeZone local(const Date& date);
  // Get the local timezone including DST correction for this date
  
  TimeZone operator+(const Time& t) const;
  TimeZone operator-(const Time& t) const;
  
  virtual std::string string() const;

  typedef ScriptRefTo<TimeZone> Ref;

protected:
  friend class Date;
  
  void parse_string(const std::string& str);

  static void init_tables();
  static void add_tz(const std::string& name,double hrs);

  typedef std::map<std::string,int> TimeZoneOffsetMap;
  static TimeZoneOffsetMap* s_zone_table;

};

};
#endif
