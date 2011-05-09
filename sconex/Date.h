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

#ifndef scxDate_h
#define scxDate_h

#include <sconex/Time.h>
#include <sconex/ScriptBase.h>
namespace scx {

//=============================================================================
class SCONEX_API Date : public ScriptObject {
public:

  enum Day { Sun, Mon, Tue, Wed, Thu, Fri, Sat };
  enum Month { Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec };

  Date();
  Date(int t, bool local=false);
  Date(const timeval& tv, bool local=false);
  Date(int year, int month, int mday, int hour, int minute, int second,
       bool local=false);
  Date(const std::string& str, bool local=false);
  Date(const ScriptRef* args);

  Date(const Date& c);
  ~Date();

  virtual ScriptObject* new_copy() const;

  static Date now(bool local=false);

  // Is this a valid date?
  bool valid() const;

  int microsecond() const;
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
  
  // Return a string in the standard form "YYYY-MM-DD hh:mm:ss +zhzm"
  std::string code() const;

  // Return a string in the compact standard form "YYYYMMDDhhmmss"
  std::string dcode() const;

  std::string format(const std::string& fmt) const;
  
  const TimeZone& timezone() const;
  Date to_zone(const TimeZone& timezone) const;
  void set_timezone(const TimeZone& timezone);
  
  time_t epoch_seconds() const;

  // ScriptObject methods:
  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  typedef ScriptRefTo<Date> Ref;

protected:

  bool get_tms(struct tm& tms) const;
  void parse_string(const std::string& str, bool local);
  
  timeval m_time;
  TimeZone m_timezone;
  
  static void init_tables();
  typedef std::map<std::string,int> MonthNameMap;
  static MonthNameMap* s_month_table;

};

};
#endif
