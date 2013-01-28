/* SconeServer (http://www.sconemad.com)

SQLite database profile

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


#include "SQLiteProfile.h"
#include "SQLiteModule.h"
#include "SQLiteQuery.h"
#include <sconex/ScriptTypes.h>

namespace sqlite {

// Uncomment to enable debug info
//#define SQLiteProfile_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef SQLiteProfile_DEBUG_LOG
#  define SQLiteProfile_DEBUG_LOG(m)
#endif

//=========================================================================
SQLiteProfile::SQLiteProfile(
  SQLiteModule* module,
  const std::string& name,
  const std::string& dbfile
) : m_module(module),
    m_name(name),
    m_dbfile(dbfile),
    m_db(0),
    m_used(false)
{
  m_parent = module;
}

//=========================================================================
SQLiteProfile::~SQLiteProfile()
{
  m_db_mutex.lock();
  if (m_db) {
    ::sqlite3_close(m_db);
    m_db = 0;
  }
  m_db_mutex.unlock();
}

//=============================================================================
sqlite3* SQLiteProfile::get_db()
{
  m_db_mutex.lock();

  if (m_used) {
    m_db_mutex.unlock();
    return 0;
  }

  // Not in use, can use it
  ::sqlite3_open_v2(m_dbfile.c_str(), &m_db,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
  m_used = true;
  m_db_mutex.unlock();
  return m_db;
}

//=============================================================================
void SQLiteProfile::release_db()
{
  m_db_mutex.lock();
  DEBUG_ASSERT(m_used, "Releasing SQLite DB that isn't in use");
  m_used = false;
  m_db_mutex.unlock();
}

//=============================================================================
scx::DbQuery* SQLiteProfile::new_query(const std::string& query)
{
  return new SQLiteQuery(this,query);
}

//=============================================================================
std::string SQLiteProfile::get_string() const
{
  return m_name;
}

//=============================================================================
scx::ScriptRef* SQLiteProfile::script_op(const scx::ScriptAuth& auth,
                                         const scx::ScriptRef& ref,
                                         const scx::ScriptOp& op,
                                         const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("Query" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
    
    if ("used" == name) 
      return scx::ScriptInt::new_ref((int)m_used);
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SQLiteProfile::script_method(const scx::ScriptAuth& auth,
                                             const scx::ScriptRef& ref,
                                             const std::string& name,
                                             const scx::ScriptRef* args)
{
  // Allow this for now
  //  if (!auth.trusted()) return new scx::ArgError("Not permitted");

  if ("Query" == name) {
    const scx::ScriptString* a_str = 
      scx::get_method_arg<scx::ScriptString>(args,0,"query");
    std::string s_str = (a_str ? a_str->get_string() : "");
    return new SQLiteQuery::Ref(new SQLiteQuery(this,s_str));
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

};
