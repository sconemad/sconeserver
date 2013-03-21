/* SconeServer (http://www.sconemad.com)

MySQL Database Query

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

#ifndef MySqlQuery_h
#define MySqlQuery_h

#include "MySqlProfile.h"

namespace mysql {

//=========================================================================
// MySqlArg - Represents a parameter argument to, or a result from, a query
//
class MySqlArg {
public:

  MySqlArg();
  MySqlArg(const MySqlArg& c);
  ~MySqlArg();

  void init_param(MYSQL_BIND& bind, const scx::ScriptRef* arg);
  void init_result(MYSQL_BIND& bind, MYSQL_FIELD& field);

  const std::string& get_name() const;
  scx::ScriptRef* get_arg() const;

private:
  enum_field_types m_type;
  mutable unsigned long m_length;
  my_bool m_is_null;
  std::string m_name;

  mutable char* m_str_data;
  union {
    short short_data;
    long int  long_data;
    long long int longlong_data;
    float float_data;
    double double_data;
    MYSQL_TIME time_data;
  } m_data;

};

typedef std::vector<MySqlArg> MySqlArgList;


//=============================================================================
class MySqlQuery : public scx::DbQuery {

public:

  MySqlQuery(MySqlProfile* profile, 
	     const std::string& query = "");
  MySqlQuery(const MySqlQuery& c);
  virtual ~MySqlQuery();

  // ScriptObject methods
  virtual scx::ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // DbQuery methods
  virtual bool exec(const scx::ScriptRef* args);
  virtual bool next_result();
  virtual scx::ScriptRef* result() const;
  virtual scx::ScriptRef* result_list() const;

  typedef scx::ScriptRefTo<MySqlQuery> Ref;
  
protected:

  void init();
  void log_error(const std::string& context, bool invalid);

  scx::ScriptRefTo<MySqlProfile> m_profile;

  std::string m_query;
  MYSQL* m_conn;
  MYSQL_STMT* m_stmt;

  MYSQL_BIND* m_param_bind;
  MySqlArgList* m_param_args;

  MYSQL_BIND* m_result_bind;
  MySqlArgList* m_result_args;

  int m_affected_rows;
  bool m_invalid;
  std::string m_error_string;
};

};
#endif
