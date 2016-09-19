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


#include "MySqlQuery.h"
#include "MySqlModule.h"
#include <sconex/utils.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Date.h>

namespace mysql {

// Uncomment to enable debug info
//#define MySqlQuery_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef MySqlQuery_DEBUG_LOG
#  define MySqlQuery_DEBUG_LOG(m)
#endif

//=========================================================================
MySqlArg::MySqlArg()
  : m_str_data(0)
{
  DEBUG_COUNT_CONSTRUCTOR(MySqlArg);
}

//=========================================================================
MySqlArg::MySqlArg(const MySqlArg& c)
  : m_type(c.m_type),
    m_length(c.m_length),
    m_is_null(c.m_is_null),
    m_name(c.m_name),
    m_str_data(0),
    m_data(c.m_data)
{
  DEBUG_COUNT_CONSTRUCTOR(MySqlArg);
  if (c.m_str_data) {
    m_str_data = scx::new_c_str(c.m_str_data);
  }
}

//=========================================================================
MySqlArg::~MySqlArg()
{
  delete[] m_str_data;
  DEBUG_COUNT_DESTRUCTOR(MySqlArg);
}
  
//=========================================================================
void MySqlArg::init_param(MYSQL_BIND& bind, const scx::ScriptRef* arg)
{
  memset(&bind,0,sizeof(MYSQL_BIND));
  m_type = MYSQL_TYPE_LONG;
  m_is_null = false;
  m_length = 0;
  bind.buffer = (void*)&m_data;

  if (arg == 0) {
    m_is_null = true;
    MySqlQuery_DEBUG_LOG("Binding NULL param");

  } else {
    const scx::ScriptObject* obj = arg->object();
    const std::type_info& ti = typeid(*obj);

    if (typeid(scx::ScriptString) == ti) {
      m_type = MYSQL_TYPE_STRING;
      const std::string value = obj->get_string();
      delete[] m_str_data;
      m_str_data = scx::new_c_str(value);
      m_length = value.size();
      bind.buffer = (void*)m_str_data;
      MySqlQuery_DEBUG_LOG("Binding string param '" << value << "'");

    } else if (typeid(scx::ScriptBool) == ti) {
      m_type = MYSQL_TYPE_SHORT;
      short value = (obj->get_int() == 0) ? 0 : 1;
      m_data.short_data = value;
      m_length = sizeof(value);
      MySqlQuery_DEBUG_LOG("Binding short int param '" << value << "'");
      
    } else if (typeid(scx::ScriptInt) == ti) {
      m_type = MYSQL_TYPE_LONG;
      long value = obj->get_int();
      m_data.long_data = value;
      m_length = sizeof(value);
      MySqlQuery_DEBUG_LOG("Binding long int param '" << value << "'");

    } else if (typeid(scx::ScriptNum) == ti) {
      m_type = MYSQL_TYPE_DOUBLE;
      double value = ((scx::ScriptNum*)obj)->get_real();
      m_data.double_data = value;
      m_length = sizeof(value);
      MySqlQuery_DEBUG_LOG("Binding double param '" << value << "'");
      
    } else if (typeid(scx::Date) == ti) {
      m_type = MYSQL_TYPE_DATETIME;
      const scx::Date* value = dynamic_cast<const scx::Date*>(obj);
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
      MySqlQuery_DEBUG_LOG("Binding datetime param '" << value->code() << "'");
      
    } else {
      m_is_null = true;
      MySqlQuery_DEBUG_LOG("Unsupported param binding, using NULL");
    }
  }

  bind.buffer_type = m_type;
  bind.buffer_length = m_length;
  bind.is_null = &m_is_null;
  bind.length = &m_length;
}
  
//=========================================================================
void MySqlArg::init_result(MYSQL_BIND& bind, MYSQL_FIELD& field)
{
  MySqlQuery_DEBUG_LOG("Binding result '" << field.name << "' type " 
		       << field.type);

  memset(&bind,0,sizeof(MYSQL_BIND));
  m_type = field.type;
  m_is_null = false;
  m_length = 0;
  m_name = field.name;
  
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
const std::string& MySqlArg::get_name() const
{
  return m_name;
}
  
//=========================================================================
scx::ScriptRef* MySqlArg::get_arg() const
{
  if (m_is_null) 
    return scx::ScriptError::new_ref("NULL");

  switch (m_type) {
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_VAR_STRING:
  case MYSQL_TYPE_BLOB:
    if (m_length >= 0) {
      if (m_length >= 1024) m_length = 1024-1;
      m_str_data[m_length] = '\0';
      return scx::ScriptString::new_ref(m_str_data);
    }
    break;
    
  case MYSQL_TYPE_SHORT:
    return scx::ScriptInt::new_ref(m_data.short_data);
    
  case MYSQL_TYPE_LONG:
    return scx::ScriptInt::new_ref(m_data.long_data);
    
  case MYSQL_TYPE_LONGLONG:
    return scx::ScriptInt::new_ref(m_data.longlong_data);
    
  case MYSQL_TYPE_FLOAT:
    return scx::ScriptReal::new_ref(m_data.float_data);
    
  case MYSQL_TYPE_DOUBLE:
    return scx::ScriptReal::new_ref(m_data.double_data);

  case MYSQL_TYPE_DATETIME:
    return new scx::ScriptRef(new scx::Date(m_data.time_data.year,
					    m_data.time_data.month,
					    m_data.time_data.day,
					    m_data.time_data.hour,
					    m_data.time_data.minute,
					    m_data.time_data.second,
					    false));
    
  default:
    break;
  }
  return scx::ScriptError::new_ref("Unknown type");
}


//=========================================================================
MySqlQuery::MySqlQuery(MySqlProfile* profile,
		       const std::string& query)
  : m_profile(profile),
    m_query(query),
    m_conn(profile->get_connection()),
    m_stmt(0),
    m_param_bind(0),
    m_param_args(0),
    m_result_bind(0),
    m_result_args(0),
    m_affected_rows(0),
    m_invalid(false)
{
  DEBUG_COUNT_CONSTRUCTOR(MySqlQuery);
  init();
}

//=========================================================================
MySqlQuery::MySqlQuery(const MySqlQuery& c)
  : scx::DbQuery(c),
    m_profile(c.m_profile),
    m_query(c.m_query),
    m_conn(m_profile.object()->get_connection()),
    m_stmt(0),
    m_param_bind(0),
    m_param_args(0),
    m_result_bind(0),
    m_result_args(0),
    m_affected_rows(0),
    m_invalid(false)
{
  DEBUG_COUNT_CONSTRUCTOR(MySqlQuery);
  init();
}

//=========================================================================
MySqlQuery::~MySqlQuery()
{
  if (m_stmt) {
    ::mysql_stmt_close(m_stmt);
    delete[] m_param_bind;
    delete m_param_args;
    delete[] m_result_bind;
    delete m_result_args;
  }

  m_profile.object()->release_connection(m_conn);

  DEBUG_COUNT_DESTRUCTOR(MySqlQuery);
}

//=========================================================================
scx::ScriptObject* MySqlQuery::new_copy() const
{
  return new MySqlQuery(*this);
}

//=========================================================================
std::string MySqlQuery::get_string() const
{
  return m_query;
}

//=========================================================================
int MySqlQuery::get_int() const
{
  // Return false in the case of a permanent error
  return (!m_invalid);
}

//=========================================================================
scx::ScriptRef* MySqlQuery::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    if (name == "exec" ||
	name == "next_result") {
      return new scx::ScriptMethodRef(ref,name);
    }

    if (name == "error") return scx::ScriptString::new_ref(m_error_string);
    if (name == "result") return result();
    if (name == "result_list") return result_list();
    if (name == "affected_rows") 
      return scx::ScriptInt::new_ref(m_affected_rows);
    if (name == "insert_id")
      return scx::ScriptInt::new_ref(insert_id());
  }
  
  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* MySqlQuery::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if ("exec" == name) {
    if (!exec(args)) {
      return scx::ScriptError::new_ref(m_error_string);
    }
    // Return 'true' to indicate success
    return scx::ScriptInt::new_ref(1);
  }
  
  if ("next_result" == name) {
    if (m_invalid) return scx::ScriptError::new_ref(m_error_string);
    if (!m_stmt) return scx::ScriptError::new_ref("No statement");
    return scx::ScriptInt::new_ref( next_result() );
  }
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
bool MySqlQuery::exec(const scx::ScriptRef* args)
{
  MySqlQuery_DEBUG_LOG("{"<<m_profile.object()->m_name<<"} exec: "<<m_query);
  
  const scx::ScriptList* argsl = 
    (args ? dynamic_cast<const scx::ScriptList*>(args->object()) : 0);

  if (m_invalid) {
    // If the query is invalid, don't go any further
    return false;
  }
  
  if (m_param_bind) {
    // If the statement has parameters, check and bind
    if (!argsl || argsl->size() != (int)m_param_args->size()) {
      log_error("Incorrect number of parameters",false);
      return false;
    }
    
    for (int i=0; i<(int)m_param_args->size(); ++i) {
      (*m_param_args)[i].init_param(m_param_bind[i],argsl->get(i));
    }
    
    if (::mysql_stmt_bind_param(m_stmt,m_param_bind)) {
      log_error("mysql_stmt_bind_param",false);
      return false;
    }
  }
  
  // Execute the statement
  
  if (::mysql_stmt_execute(m_stmt)) {
    log_error("mysql_stmt_execute",false);
    return false;
  }
  
  m_affected_rows = ::mysql_stmt_affected_rows(m_stmt);
  MySqlQuery_DEBUG_LOG("Affected rows: " << m_affected_rows);

  return true;
}

//=========================================================================
bool MySqlQuery::next_result()
{
  if (m_invalid || !m_stmt) return false;
  
  int err = ::mysql_stmt_fetch(m_stmt);
  if (err == 1) {
    log_error("mysql_stmt_fetch",false);
    return false;
  }
  
  if (err == MYSQL_DATA_TRUNCATED) {
    MySqlQuery_DEBUG_LOG("mysql_stmt_fetch() data truncated");
  }

  return (err != MYSQL_NO_DATA);
}
 
//=========================================================================
scx::ScriptRef* MySqlQuery::result() const
{
  scx::ScriptMap* row = new scx::ScriptMap();
  for (int i=0; i<(int)m_result_args->size(); ++i) {
    const MySqlArg& rarg = (*m_result_args)[i];
    row->give(rarg.get_name(),rarg.get_arg());
  }
  return new scx::ScriptRef(row);
}

//=========================================================================
scx::ScriptRef* MySqlQuery::result_list() const
{
  scx::ScriptList* row = new scx::ScriptList();
  for (int i=0; i<(int)m_result_args->size(); ++i) {
    const MySqlArg& rarg = (*m_result_args)[i];
    row->give(rarg.get_arg());
  }
  return new scx::ScriptRef(row);
}

//=========================================================================
int MySqlQuery::insert_id() const
{
  return ::mysql_insert_id(m_conn);
}

//=========================================================================
void MySqlQuery::init()
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
  
  if (::mysql_stmt_prepare(m_stmt,m_query.c_str(),m_query.size())) {
    log_error("mysql_stmt_prepare",true);
    return;
  }
  
  // Setup the parameter (input) bindings
  
  int params = ::mysql_stmt_param_count(m_stmt);
  MySqlQuery_DEBUG_LOG("Statement has " << params << " parameters");

  if (params > 0) {
    m_param_bind = new MYSQL_BIND[params];
    m_param_args = new MySqlArgList(params);
  }

  // Setup the result (output) bindings
  
  MYSQL_RES* res = ::mysql_stmt_result_metadata(m_stmt);
  if (res) {
    int fields = ::mysql_num_fields(res);
    MySqlQuery_DEBUG_LOG("Result contains " << fields << " fields");
    
    m_result_bind = new MYSQL_BIND[fields];
    m_result_args = new MySqlArgList(fields);
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
void MySqlQuery::log_error(const std::string& context, bool invalid)
{
  m_error_string = context;
  if (m_stmt) {
    m_error_string += std::string(": ") + ::mysql_stmt_error(m_stmt);
  }
  MySqlQuery_DEBUG_LOG(m_error_string);

  if (invalid) {
    // Error makes the query invalid (i.e. this is a permanant error)
    m_invalid = true;
  }
}

};
