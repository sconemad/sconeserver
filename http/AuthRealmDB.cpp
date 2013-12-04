/* SconeServer (http://www.sconemad.com)

HTTP authentication using database storage

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


#include <http/AuthRealmDB.h>
#include <http/HTTPModule.h>
#include <sconex/User.h>
#include <memory>

namespace http {

//=============================================================================
HTTPUser::HTTPUser(HTTPModule* module,
		   scx::Database* db,
		   const std::string& name,
		   int id)
  : scx::DbProxy(db,"user","id",new scx::ScriptInt(id)),
    m_module(module),
    m_name(name)
{
  DEBUG_COUNT_CONSTRUCTOR(HTTPUser);
  m_parent = module;
}

//=============================================================================
HTTPUser::~HTTPUser()
{
  DEBUG_COUNT_DESTRUCTOR(HTTPUser);
}

//=============================================================================
std::string HTTPUser::get_string() const
{
  return m_name;
}

//=============================================================================
AuthRealmDB::AuthRealmDB(HTTPModule* module,
			 const scx::ScriptRef* args)
  : AuthRealm(module),
    m_db(0)
{
  DEBUG_COUNT_CONSTRUCTOR(AuthRealmDB);

  const scx::ScriptString* type =
    scx::get_method_arg<scx::ScriptString>(args,0,"type");

  if (type) {
    m_db = scx::Database::open(type->get_string(),args);
  } else {
    DEBUG_LOG("No DB type specified");
  }
}

//=============================================================================
AuthRealmDB::~AuthRealmDB()
{
  delete m_db;
  DEBUG_COUNT_DESTRUCTOR(AuthRealmDB);
}

//=============================================================================
std::string AuthRealmDB::lookup_hash(const std::string& username)
{
  if (!m_db) return "";

  std::auto_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "SELECT id,password FROM user WHERE username = ?"));
  
  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptString::new_ref(username));
  query->exec(&args);
  if (!query->next_result()) 
    return "";

  scx::ScriptRef* row_ref = query->result_list();
  scx::ScriptList* row = dynamic_cast<scx::ScriptList*>(row_ref->object());

  //TODO: We might want to cache the id to avoid two SELECTS for the normal
  // case of lookup_hash followed by lookup_data?
  //  scx::ScriptRef* a_id = row->get(0);
  //  int id = (a_id ? a_id->object()->get_int() : -1);

  scx::ScriptRef* a_pwentry = row->get(1);
  std::string pwentry = (a_pwentry ? a_pwentry->object()->get_string() : "");

  delete row_ref;
  return pwentry;
}

//=============================================================================
bool AuthRealmDB::update_hash(const std::string& username,
			      const std::string& hash)
{
  if (!m_db) return 0;

  std::auto_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "UPDATE user SET password = ? WHERE username = ?"));
  
  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptString::new_ref(hash));
  args.object()->give(scx::ScriptString::new_ref(username));
  return query->exec(&args);
}

//=============================================================================
scx::ScriptRef* AuthRealmDB::lookup_data(const std::string& username)
{
  if (!m_db) return 0;

  std::auto_ptr<scx::DbQuery> query(m_db->object()->new_query(
    "SELECT id FROM user WHERE username = ?"));
  
  scx::ScriptList::Ref args(new scx::ScriptList());
  args.object()->give(scx::ScriptString::new_ref(username));
  query->exec(&args);
  if (!query->next_result()) 
    return 0;

  scx::ScriptRef* row_ref = query->result_list();
  scx::ScriptList* row = dynamic_cast<scx::ScriptList*>(row_ref->object());

  scx::ScriptRef* a_id = row->get(0);
  int id = (a_id ? a_id->object()->get_int() : -1);

  delete row_ref;
  if (id < 0) return 0;

  HTTPUser* user = new HTTPUser(m_module.object(), 
				m_db->object(),
				username,id);
  return new scx::ScriptRef(user);
}

};
