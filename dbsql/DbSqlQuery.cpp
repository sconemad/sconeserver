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


#include "DbSqlQuery.h"
#include "sconex/utils.h"

namespace dbsql {

// Uncomment to enable debug info
//#define DbSqlQuery_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef DbSqlQuery_DEBUG_LOG
#  define DbSqlQuery_DEBUG_LOG(m)
#endif

//=========================================================================
DbSqlArg::DbSqlArg()
  : m_str_data(0)
{
    
};

//=========================================================================
DbSqlArg::~DbSqlArg()
{
  delete[] m_str_data;
};
  
//=========================================================================
void DbSqlArg::init_param(MYSQL_BIND& bind, const scx::Arg* arg)
{
  memset(&bind,0,sizeof(MYSQL_BIND));
  m_type = MYSQL_TYPE_LONG;
  m_is_null = false;
  m_length = 0;
  bind.buffer = (void*)&m_data;

  if (arg == 0) {
    m_is_null = true;
    DbSqlQuery_DEBUG_LOG("Binding NULL param");

  } else {
    const std::type_info& ti = typeid(*arg);

    if (typeid(scx::ArgString) == ti) {
      m_type = MYSQL_TYPE_STRING;
      const std::string value = arg->get_string();
      m_str_data = scx::new_c_str(value);
      m_length = value.size();
      bind.buffer = (void*)m_str_data;
      DbSqlQuery_DEBUG_LOG("Binding string param '" << value << "'");

    } else if (typeid(scx::ArgInt) == ti) {
      m_type = MYSQL_TYPE_LONG;
      long value = arg->get_int();
      m_data.long_data = value;
      m_length = sizeof(value);
      DbSqlQuery_DEBUG_LOG("Binding long int param '" << value << "'");
      
    } else if (typeid(scx::ArgReal) == ti) {
      m_type = MYSQL_TYPE_DOUBLE;
      double value = ((scx::ArgReal*)arg)->get_real();
      m_data.double_data = value;
      m_length = sizeof(value);
      DbSqlQuery_DEBUG_LOG("Binding double param '" << value << "'");
      
    } else if (typeid(scx::Date) == ti) {
      m_type = MYSQL_TYPE_DATETIME;
      const scx::Date* value = dynamic_cast<const scx::Date*>(arg);
      m_data.time_data.year = value->year();
      m_data.time_data.month = (unsigned int)(value->month() + 1);
      m_data.time_data.day = value->mday();
      m_data.time_data.hour = value->hour();
      m_data.time_data.minute = value->minute();
      m_data.time_data.second = value->second();
      m_data.time_data.second_part = 0;
      m_data.time_data.neg = 0;
      m_data.time_data.time_type = MYSQL_TIMESTAMP_DATETIME;
      m_length = sizeof(MYSQL_TIME);
      DbSqlQuery_DEBUG_LOG("Binding datetime param '" << value->code() << "'");
      
    } else {
      m_is_null = true;
      DbSqlQuery_DEBUG_LOG("Unsupported param binding, using NULL");
    }
  }

  bind.buffer_type = m_type;
  bind.buffer_length = m_length;
  bind.is_null = &m_is_null;
  bind.length = &m_length;
}
  
//=========================================================================
void DbSqlArg::init_result(MYSQL_BIND& bind, MYSQL_FIELD& field)
{
  memset(&bind,0,sizeof(MYSQL_BIND));
  m_type = field.type;
  m_is_null = false;
  m_length = 0;
  m_name = field.name;
  
  DbSqlQuery_DEBUG_LOG("Binding result '" << field.name << "' type " << field.type);
  switch (m_type) {
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_VAR_STRING:
  case MYSQL_TYPE_BLOB:
    m_length = 1024;
    m_str_data = new char[m_length];
    memset(m_str_data,0,m_length);
    bind.buffer = (void*)m_str_data;
    break;
    
  default:
    bind.buffer = (void*)&m_data;
    break;
  }
  
  bind.buffer_type = m_type;
  bind.buffer_length = m_length;
  bind.is_null = &m_is_null;
  bind.length = &m_length;
}

//=========================================================================
const std::string& DbSqlArg::get_name()
{
  return m_name;
}
  
//=========================================================================
scx::Arg* DbSqlArg::get_arg()
{
  if (m_is_null) return 0;

  switch (m_type) {
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_VAR_STRING:
  case MYSQL_TYPE_BLOB:
    if (m_length >= 0) {
      if (m_length >= 1024) m_length = 1024-1;
      m_str_data[m_length] = '\0';
      return new scx::ArgString(m_str_data);
    }
    break;
    
  case MYSQL_TYPE_SHORT:
    return new scx::ArgInt(m_data.short_data);
    
  case MYSQL_TYPE_LONG:
    return new scx::ArgInt(m_data.long_data);
    
  case MYSQL_TYPE_LONGLONG:
    return new scx::ArgInt(m_data.longlong_data);
    
  case MYSQL_TYPE_FLOAT:
    return new scx::ArgReal(m_data.float_data);
    
  case MYSQL_TYPE_DOUBLE:
    return new scx::ArgReal(m_data.double_data);

  case MYSQL_TYPE_DATETIME:
    return new scx::Date(m_data.time_data.year,
			 m_data.time_data.month,
			 m_data.time_data.day,
			 m_data.time_data.hour,
			 m_data.time_data.minute,
			 m_data.time_data.second,
			 false);
    
  default:
    break;
  }
  return new scx::ArgError("Unknown type");
}


//=========================================================================
DbSqlQuery::DbSqlQuery(DbSqlProfile& profile,
		       const std::string& query)
  : m_profile(profile),
    m_ref(m_profile.m_module.ref()),
    m_query(new std::string(query)),
    m_conn(profile.get_connection()),
    m_stmt(0),
    m_param_bind(0),
    m_param_args(0),
    m_result_bind(0),
    m_result_args(0),
    m_error(0)

{
  DEBUG_COUNT_CONSTRUCTOR(DbSqlQuery);
  init();
}

//=========================================================================
DbSqlQuery::DbSqlQuery(const DbSqlQuery& c)
  : scx::DbQuery(c),
    m_profile(c.m_profile),
    m_ref(c.m_ref),
    m_query(new std::string(*c.m_query)),
    m_conn(c.m_profile.get_connection()),
    m_stmt(0),
    m_param_bind(0),
    m_param_args(0),
    m_result_bind(0),
    m_result_args(0),
    m_error(0)
{
  DEBUG_COUNT_CONSTRUCTOR(DbSqlQuery);
  init();
}

//=========================================================================
DbSqlQuery::DbSqlQuery(RefType ref, DbSqlQuery& c)
  : scx::DbQuery(ref,c),
    m_profile(c.m_profile),
    m_ref(c.m_ref),
    m_query(c.m_query),
    m_conn(c.m_conn),
    m_stmt(c.m_stmt),
    m_param_bind(c.m_param_bind),
    m_param_args(c.m_param_args),
    m_result_bind(c.m_result_bind),
    m_result_args(c.m_result_args),
    m_error(c.m_error)
{
  DEBUG_COUNT_CONSTRUCTOR(DbSqlQuery);
}

//=========================================================================
DbSqlQuery::~DbSqlQuery()
{
  if (last_ref()) {
    delete m_query;

    if (m_stmt) {
      ::mysql_stmt_close(m_stmt);
      delete[] m_param_bind;
      delete m_param_args;
      delete[] m_result_bind;
      delete m_result_args;
    }

    m_profile.release_connection(m_conn);
    delete m_error;
  }
  DEBUG_COUNT_DESTRUCTOR(DbSqlQuery);
}

//=========================================================================
scx::Arg* DbSqlQuery::new_copy() const
{
  return new DbSqlQuery(*this);
}

//===========================================================================
scx::Arg* DbSqlQuery::ref_copy(RefType ref)
{
  return new DbSqlQuery(ref,*this);
}

//=========================================================================
std::string DbSqlQuery::get_string() const
{
  return *m_query;
}

//=========================================================================
int DbSqlQuery::get_int() const
{
  // Return false in the case of a permanent error
  return (m_error == 0);
}

//=========================================================================
scx::Arg* DbSqlQuery::op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right)
{
  if (is_method_call(optype,opname)) {

    scx::ArgList* l = dynamic_cast<scx::ArgList*>(right);
    
    if ("exec" == m_method) {
      return exec(*l);
    }

    if ("next_result" == m_method) {
      if (m_error) return get_error();
      if (!m_stmt) return new scx::ArgError("No statement");
      return new scx::ArgInt( next_result() );
    }
    
  } else if (scx::Arg::Binary == optype) {

    if ("." == opname) {
      std::string name = right->get_string();

      if (name == "error") return get_error();
      if (name == "result") return result();
      if (name == "result_list") return result_list();

      if (name == "exec" ||
	  name == "next_result") return new_method(name);
    }

  }
  return SCXBASE Arg::op(auth,optype,opname,right);
}

//=========================================================================
scx::Arg* DbSqlQuery::exec(const scx::ArgList& args)
{
  //  m_profile.m_module.log("{"+m_profile.m_name+"} exec: "+*m_query);
  
  if (m_error) {
    // If there is a permanent error, don't execute the query, return the error
    return get_error();
  }
  
  if (m_param_args) {
    // If the statement has parameters, check and bind
    if (args.size() != (int)m_param_args->size()) {
	  return new scx::ArgError(log_error("Incorrect number of parameters",false));
    }
    
    for (int i=0; i<(int)m_param_args->size(); ++i) {
      (*m_param_args)[i].init_param(m_param_bind[i],args.get(i));
    }
    
    if (::mysql_stmt_bind_param(m_stmt,m_param_bind)) {
      return new scx::ArgError(log_error("mysql_stmt_bind_param",false));
    }
  }
  
  // Execute the statement
  
  if (::mysql_stmt_execute(m_stmt)) {
    return new scx::ArgError(log_error("mysql_stmt_bind_param",false));
  }
  
  int affected_rows = ::mysql_stmt_affected_rows(m_stmt);
  if (affected_rows >= 0) {
    // This sort of query has no results, just return the number of affected rows
    DbSqlQuery_DEBUG_LOG("Affected rows: " << affected_rows);
    return new scx::ArgInt(affected_rows);
  }
  
  return new scx::ArgInt(1);
}

//=========================================================================
bool DbSqlQuery::next_result()
{
  if (m_error || !m_stmt) return false;
  
  int err = ::mysql_stmt_fetch(m_stmt);
  if (err == 1) {
    log_error("mysql_stmt_fetch",false);
    return false;
  }
  if (err == MYSQL_DATA_TRUNCATED) {
    DbSqlQuery_DEBUG_LOG("mysql_stmt_fetch() data truncated");
  }
  return (err != MYSQL_NO_DATA);
}

//=========================================================================
scx::ArgMap* DbSqlQuery::result() const
{
  scx::ArgMap* row = new scx::ArgMap();
  for (int i=0; i<(int)m_result_args->size(); ++i) {
    DbSqlArg& rarg = (*m_result_args)[i];
    row->give(rarg.get_name(),rarg.get_arg());
  }
  return row;
}

//=========================================================================
scx::ArgList* DbSqlQuery::result_list() const
{
  scx::ArgList* row = new scx::ArgList();
  for (int i=0; i<(int)m_result_args->size(); ++i) {
    DbSqlArg& rarg = (*m_result_args)[i];
    row->give(rarg.get_arg());
  }
  return row;
}

//=========================================================================
void DbSqlQuery::init()
{
  // Initialise and prepare statement
  
  if (m_conn == 0) {
    log_error("mysql_real_connect",true);
    return;
  }

  m_stmt = ::mysql_stmt_init(m_conn);
  if (!m_stmt) {
    log_error("mysql_stmt_init",true);
    return;
  }
  
  if (::mysql_stmt_prepare(m_stmt,m_query->c_str(),m_query->size())) {
    log_error("mysql_stmt_prepare",true);
    return;
  }
  
  // Setup the parameter (input) bindings
  
  int params = ::mysql_stmt_param_count(m_stmt);
  DbSqlQuery_DEBUG_LOG("Statement has " << params << " parameters");

  if (params > 0) {
    m_param_bind = new MYSQL_BIND[params];
    m_param_args = new DbSqlArgList(params);
  }

  // Setup the result (output) bindings
  
  MYSQL_RES* res = ::mysql_stmt_result_metadata(m_stmt);
  if (res) {
    int fields = ::mysql_num_fields(res);
    DbSqlQuery_DEBUG_LOG("Result contains " << fields << " fields");
    
    m_result_bind = new MYSQL_BIND[fields];
    m_result_args = new DbSqlArgList(fields);
    for (int i=0; i<fields; ++i) {
      MYSQL_FIELD* field = ::mysql_fetch_field(res);
      (*m_result_args)[i].init_result(m_result_bind[i],*field);
    }
    ::mysql_free_result(res);
    
    if (::mysql_stmt_bind_result(m_stmt,m_result_bind)) {
      log_error("mysql_stmt_bind_result",true);
      return;
    }
  }
}

//=========================================================================
std::string DbSqlQuery::log_error(const std::string& context, bool permanent)
{
  std::string error = context;
  if (m_stmt) {
    error += std::string(": ") + ::mysql_stmt_error(m_stmt);
  }
  DbSqlQuery_DEBUG_LOG(error);

  if (permanent) {
    // Save the error if its permanent
    if (m_error == 0) {
      m_error = new std::string();
    }
    (*m_error) = error;
  }

  return error;
}

//=========================================================================
scx::Arg* DbSqlQuery::get_error()
{
  if (m_error) {
    return new scx::ArgError(*m_error);
  }
  return 0;
}
  
};
