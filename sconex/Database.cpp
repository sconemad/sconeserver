/* SconeServer (http://www.sconemad.com)

Database

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


#include <sconex/Database.h>

#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <memory>
  
namespace scx {

template class Provider<Database>;
template class ProviderScheme<Database>;
  
ProviderScheme<Database::Ref>* Database::s_providers = 0;
  
//=========================================================================
Database::Ref* Database::open(const std::string& type,
			      const ScriptRef* args)
{
  init();
  return s_providers->provide(type,args);
}

//=========================================================================
Database::Database()
{
  DEBUG_COUNT_CONSTRUCTOR(Database);
}

//=========================================================================
Database::Database(const Database& c)
  : ScriptObject(c)
{
  DEBUG_COUNT_CONSTRUCTOR(Database);
}

//=========================================================================
Database::~Database()
{
  DEBUG_COUNT_DESTRUCTOR(Database);
}

//=========================================================================
bool Database::simple_query(const std::string& query)
{
  std::auto_ptr<DbQuery> q(new_query(query));
  return q->exec(0);
}

//=========================================================================
int Database::simple_query_num(const std::string& query)
{
  std::auto_ptr<DbQuery> q(new_query(query));
  if (!q->exec(0)) return -1;
  if (!q->next_result()) return -1;
  ScriptRef* row_ref = q->result_list();
  ScriptList* row = dynamic_cast<ScriptList*>(row_ref->object());
  ScriptRef* a_num = row->get(0);
  int result = a_num ? a_num->object()->get_int() : -1;
  delete row_ref;
  return result;
}
  
//=========================================================================
void Database::register_provider(const std::string& type,
                                 Provider<Database::Ref>* factory)
{
  init();
  s_providers->register_provider(type,factory);
}
  
//=========================================================================
void Database::unregister_provider(const std::string& type,
				   Provider<Database::Ref>* factory)
{
  init();
  s_providers->unregister_provider(type,factory);
}
  
//=========================================================================
void Database::init()
{
  if (!s_providers) {
    s_providers = new ProviderScheme<Database::Ref>();
  }
}

  
//=========================================================================
DbQuery::DbQuery()
{
  DEBUG_COUNT_CONSTRUCTOR(DbQuery);
}
  
//=========================================================================
DbQuery::DbQuery(const DbQuery& c)
  : ScriptObject(c)
{
  DEBUG_COUNT_CONSTRUCTOR(DbQuery);
}

//=========================================================================
DbQuery::~DbQuery()
{
  DEBUG_COUNT_DESTRUCTOR(DbQuery);
}



//=========================================================================
DbProxy::DbProxy(Database* db,
		 const std::string& table,
		 const std::string& key_field,
		 ScriptObject* key)
  : m_db(db),
    m_table(table),
    m_key_field(key_field),
    m_key(key,ScriptRef::ConstRef)
{

}

//=========================================================================
DbProxy::DbProxy(const DbProxy& c)
  : ScriptObject(c),
    m_db(c.m_db),
    m_table(c.m_table),
    m_key_field(c.m_key_field),
    m_key(c.m_key)
{
  
}

//=========================================================================
DbProxy::~DbProxy()
{

}

//=========================================================================
ScriptObject* DbProxy::new_copy() const
{
  return new DbProxy(*this);
}

/*
//=========================================================================
std::string DbProxy::get_string() const
{
  return "";
}

//=========================================================================
int DbProxy::get_int() const
{
  return 1;
}
*/

//=========================================================================
ScriptRef* DbProxy::script_op(const ScriptAuth& auth,
			      const ScriptRef& ref,
			      const ScriptOp& op,
			      const ScriptRef* right)
{
  if (op.type() == ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("set" == name) {
      return new ScriptMethodRef(ref,name);
    }

    std::auto_ptr<DbQuery> query(m_db.object()->new_query(
      "SELECT "+name+" FROM "+m_table+" WHERE "+m_key_field+" = ?"));
    
    ScriptList::Ref args(new ScriptList());
    args.object()->give(m_key.ref_copy());
    query->exec(&args);
    
    ScriptRef* result = 0;
    if (query->next_result()) {
      ScriptRef* row_ref = query->result_list();
      ScriptList* row = dynamic_cast<ScriptList*>(row_ref->object());
      if (row) result = row->take(0);
      delete row_ref;
    }
    return result;
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
ScriptRef* DbProxy::script_method(const ScriptAuth& auth,
				  const ScriptRef& ref,
				  const std::string& name,
				  const ScriptRef* args)
{
  if (name == "set") {
    const ScriptString* a_name = get_method_arg<ScriptString>(args,0,"name");
    if (!a_name)
      return ScriptError::new_ref("No name specified");
    std::string s_name = a_name->get_string();
    if (s_name.empty())
      return ScriptError::new_ref("No name specified");

    const ScriptRef* value = get_method_arg_ref(args,1,"value");
    
    std::auto_ptr<DbQuery> query(m_db.object()->new_query(
      "UPDATE "+m_table+" SET "+s_name+" = ? WHERE "+m_key_field+" = ?"));
    
    ScriptList::Ref args(new ScriptList());
    args.object()->give(value->ref_copy());
    args.object()->give(m_key.ref_copy());
    if (!query->exec(&args))
      return ScriptError::new_ref("Failed to update value");

    return 0;
  }

  return ScriptObject::script_method(auth,ref,name,args);
}

};
