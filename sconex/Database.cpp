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


#include "Database.h"

namespace scx {

template class Provider<Database>;
template class ProviderScheme<Database>;
  
ProviderScheme<Database>* Database::s_providers = 0;
  
//=========================================================================
Database* Database::create_new(
  const std::string& type,
  const ArgMap& args
)
{
  init();
  return s_providers->create_new(type,args);
}

//=========================================================================
Database::Database()
{

}

//=========================================================================
Database::~Database()
{

}

//=========================================================================
void Database::register_provider(const std::string& type,
                                 Provider<Database>* factory)
{
  s_providers->register_provider(type,factory);
}
  
//=========================================================================
void Database::unregister_provider(const std::string& type)
{
  s_providers->unregister_provider(type);
}
  
//=========================================================================
void Database::init()
{
  if (!s_providers) {
    s_providers = new ProviderScheme<Database>();
  }
}

  
//=========================================================================
DbQuery::DbQuery()
{

}
  
//=========================================================================
DbQuery::DbQuery(const DbQuery& c)
  : Arg(c)
{

}
  
//=========================================================================
DbQuery::DbQuery(RefType ref, DbQuery& c)
  : Arg(ref,c)
{

}
  
};
