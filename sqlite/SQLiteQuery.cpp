/* SconeServer (http://www.sconemad.com)

SQLite database query

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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


#include "SQLiteQuery.h"
#include "SQLiteModule.h"
#include <sconex/utils.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Date.h>

namespace sqlite {

// Uncomment to enable debug info
//#define SQLiteQuery_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef SQLiteQuery_DEBUG_LOG
#  define SQLiteQuery_DEBUG_LOG(m)
#endif

//=========================================================================
SQLiteQuery::SQLiteQuery(SQLiteProfile* profile,
		       const std::string& query)
  : m_profile(profile),
    m_query(query),
    m_db(profile->get_db()),
    m_stmt(0),
    m_invalid(false)
{
  DEBUG_COUNT_CONSTRUCTOR(SQLiteQuery);
  init();
}

//=========================================================================
SQLiteQuery::SQLiteQuery(const SQLiteQuery& c)
  : scx::DbQuery(c),
    m_profile(c.m_profile),
    m_query(c.m_query),
    m_db(c.m_db), //XXX problem?
    m_stmt(0),
    m_invalid(false)
{
  DEBUG_COUNT_CONSTRUCTOR(SQLiteQuery);
  init();
}

//=========================================================================
SQLiteQuery::~SQLiteQuery()
{
  if (m_stmt) {
    ::sqlite3_finalize(m_stmt);
  }

  DEBUG_COUNT_DESTRUCTOR(SQLiteQuery);
}

//=========================================================================
scx::ScriptObject* SQLiteQuery::new_copy() const
{
  return new SQLiteQuery(*this);
}

//=========================================================================
std::string SQLiteQuery::get_string() const
{
  return m_query;
}

//=========================================================================
int SQLiteQuery::get_int() const
{
  // Return false in the case of a permanent error
  return (!m_invalid);
}

//=========================================================================
scx::ScriptRef* SQLiteQuery::script_op(const scx::ScriptAuth& auth,
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
  }
  
  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* SQLiteQuery::script_method(const scx::ScriptAuth& auth,
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
bool SQLiteQuery::exec(const scx::ScriptRef* args)
{
  SQLiteQuery_DEBUG_LOG("{" << m_profile.object()->m_name <<
                        "} exec: " << m_query);
  
  const scx::ScriptList* argsl = 
    (args ? dynamic_cast<const scx::ScriptList*>(args->object()) : 0);

  if (m_invalid) {
    // If the query is invalid, don't go any further
    return false;
  }

  ::sqlite3_reset(m_stmt);
  
  // If the statement has parameters, check and bind
  int params = ::sqlite3_bind_parameter_count(m_stmt);
  if (params > 0) {
    ::sqlite3_clear_bindings(m_stmt);
    if (!argsl || argsl->size() != params) {
      log_error("Incorrect number of parameters", false);
      return false;
    }
    for (int i=0; i<params; ++i) {
      bind_param(1+i, argsl->get(i));
    }
  }

  // Execute the statement
  m_step_err = ::sqlite3_step(m_stmt);
  if (m_step_err != SQLITE_DONE &&
      m_step_err != SQLITE_ROW) {
    log_error("sqlite3_step (init)", false);
  }
  m_first = true;
  
  return true;
}

//=========================================================================
bool SQLiteQuery::next_result()
{
  if (m_invalid || !m_stmt) return false;

  // The first row will already have been loaded in exec(),
  // for subsequent rows call step again.
  if (!m_first) {
    m_step_err = ::sqlite3_step(m_stmt);
    if (m_step_err != SQLITE_DONE &&
        m_step_err != SQLITE_ROW) {
      log_error("sqlite3_step", false);
    }
  }
  m_first = false;

  // Return true if we have a row of results
  return (m_step_err == SQLITE_ROW);
}
 
//=========================================================================
scx::ScriptRef* SQLiteQuery::result() const
{
  int count = ::sqlite3_data_count(m_stmt);
  scx::ScriptMap* row = new scx::ScriptMap();
  for (int i=0; i<count; ++i) {
    row->give(::sqlite3_column_name(m_stmt, i), get_result(i));
  }
  return new scx::ScriptRef(row);
}

//=========================================================================
scx::ScriptRef* SQLiteQuery::result_list() const
{
  int count = ::sqlite3_data_count(m_stmt);
  scx::ScriptList* row = new scx::ScriptList();
  for (int i=0; i<count; ++i) {
    row->give(get_result(i));
  }
  return new scx::ScriptRef(row);
}

//=========================================================================
void SQLiteQuery::init()
{
  // Check the database is available
  if (m_db == 0) {
    log_error("sqlite3_open_v2", true);
    return;
  }

  // Prepare the statement
  int err = ::sqlite3_prepare_v2(m_db, m_query.c_str(), m_query.size()+1,
                                 &m_stmt, 0);
  if (SQLITE_OK != err) {
    log_error("sqlite3_prepare_v2", true);
  }
}

//=========================================================================
void SQLiteQuery::log_error(const std::string& context, bool invalid)
{
  std::ostringstream oss;
  oss << context;
  int code = ::sqlite3_errcode(m_db);
  if (code != SQLITE_OK) {
    oss << " err=" << code;
    // Ideally we'd print the error string, but this function doesn't seem
    // to be available everywhere, and sqlite3_errmsg is not thread-safe.
    //oss << " (" << ::sqlite3_errstr(code) << ")";
   }
  m_error_string = oss.str();
  SQLiteQuery_DEBUG_LOG(m_error_string);

  if (invalid) {
    // Error makes the query invalid (i.e. this is a permanant error)
    m_invalid = true;
  }
}

//=========================================================================
void SQLiteQuery::bind_param(int param, const scx::ScriptRef* arg)
{
  const scx::ScriptObject* obj = arg->object();
  const std::type_info& ti = typeid(*obj);
  int err = 0;
  
  if (typeid(scx::ScriptString) == ti) {
    const std::string value = obj->get_string();
    err = ::sqlite3_bind_text(m_stmt, param, value.c_str(), value.size(),
                              SQLITE_TRANSIENT);
    SQLiteQuery_DEBUG_LOG("Binding string param '" << value << "'");

  } else if (typeid(scx::ScriptInt) == ti) {
    int value = obj->get_int();
    err = ::sqlite3_bind_int(m_stmt, param, value);
    SQLiteQuery_DEBUG_LOG("Binding int param '" << value << "'");
      
  } else if (typeid(scx::ScriptNum) == ti) {
    double value = ((scx::ScriptNum*)obj)->get_real();
    err = ::sqlite3_bind_double(m_stmt, param, value);
    SQLiteQuery_DEBUG_LOG("Binding double param '" << value << "'");

  } else if (typeid(scx::Date) == ti) {
    // Note: Using UNIX epoch date format with sqlite?
    scx::Date* date = (scx::Date*)obj;
    int value = date->epoch_seconds();
    err = ::sqlite3_bind_int(m_stmt, param, value);
    SQLiteQuery_DEBUG_LOG("Binding date param as int '" << value << "'");
    
  } else {
    err = ::sqlite3_bind_null(m_stmt, param);
    SQLiteQuery_DEBUG_LOG("Unsupported param binding, using NULL");
  }
  
  if (err != SQLITE_OK) {
    log_error("sqlite3_bind_", false);
  }
}

//=========================================================================
scx::ScriptRef* SQLiteQuery::get_result(int col) const
{
  int type = ::sqlite3_column_type(m_stmt, col);
  scx::ScriptRef* val = 0;
  switch (type) {
    case SQLITE_INTEGER:
      val = scx::ScriptInt::new_ref(::sqlite3_column_int(m_stmt, col));
      break;
    case SQLITE_FLOAT:
      val = scx::ScriptReal::new_ref(::sqlite3_column_double(m_stmt, col));
      break;
    case SQLITE_TEXT:
      val = scx::ScriptString::new_ref(
        (const char*)::sqlite3_column_text(m_stmt, col));
      break;
    case SQLITE_NULL:
      val  = scx::ScriptError::new_ref("NULL");
      break;
    default:
      val  = scx::ScriptError::new_ref("Unknown type");
      break;
  }
  SQLiteQuery_DEBUG_LOG("Got result type " << type <<
                        " value '" << val->object()->get_string() << "'");
  return val;
}

};
