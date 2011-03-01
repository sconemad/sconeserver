/* SconeServer (http://www.sconemad.com)

SQL Database Query

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef DbSqlQuery_h
#define DbSqlQuery_h

#include "DbSqlProfile.h"

namespace dbsql {

//=========================================================================
class DbSqlArg {

public:

  DbSqlArg();
  ~DbSqlArg();

  void init_param(MYSQL_BIND& bind, const scx::Arg* arg);
  void init_result(MYSQL_BIND& bind, MYSQL_FIELD& field);

  const std::string& get_name();
  scx::Arg* get_arg();

private:
  enum_field_types m_type;
  unsigned long m_length;
  my_bool m_is_null;
  std::string m_name;

  char* m_str_data;
  union {
    short short_data;
    long int  long_data;
    long long int longlong_data;
    float float_data;
    double double_data;
    MYSQL_TIME time_data;
  } m_data;

};

typedef std::vector<DbSqlArg> DbSqlArgList;


//=============================================================================
class DbSqlQuery : public scx::DbQuery {

public:

  DbSqlQuery(DbSqlProfile& profile, const std::string& query = "");
  DbSqlQuery(const DbSqlQuery& c);
  DbSqlQuery(RefType ref, DbSqlQuery& c);
  virtual ~DbSqlQuery();
  virtual scx::Arg* new_copy() const;
  virtual scx::Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::Arg* op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right);

  // DbQuery interface:
  virtual scx::Arg* exec(const scx::ArgList& args);
  virtual bool next_result();
  virtual scx::ArgMap* result() const;
  virtual scx::ArgList* result_list() const;
  
protected:

  void init();
  std::string log_error(const std::string& context, bool permanent);
  scx::Arg* get_error();

  DbSqlProfile& m_profile;
  scx::ModuleRef m_ref;
  std::string* m_query;
  MYSQL* m_conn;
  MYSQL_STMT* m_stmt;

  MYSQL_BIND* m_param_bind;
  DbSqlArgList* m_param_args;

  MYSQL_BIND* m_result_bind;
  DbSqlArgList* m_result_args;

  std::string* m_error;
};

};
#endif
