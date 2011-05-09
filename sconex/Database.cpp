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


#include <Database.h>

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
  
};
