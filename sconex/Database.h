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

#ifndef scxDatabase_h
#define scxDatabase_h

#include <sconex/Provider.h>
#include <sconex/ScriptBase.h>

namespace scx {

class DbQuery;
  
//=========================================================================
class Database : public ScriptObject {

public:

  typedef scx::ScriptRefTo<Database> Ref;
  
  // Open a database of the specified type
  static Database::Ref* open(const std::string& type,
			     const ScriptRef* args);
  
  Database();
  Database(const Database& c);
  virtual ~Database();

  virtual DbQuery* new_query(const std::string& query) =0;
  
  static void register_provider(const std::string& type,
                                Provider<Database::Ref>* factory);
  static void unregister_provider(const std::string& type,
				  Provider<Database::Ref>* factory);

protected:

  static void init();
  
  static ProviderScheme<Database::Ref>* s_providers;

};

//=========================================================================
class DbQuery : public ScriptObject {

public:

  DbQuery();
  DbQuery(const DbQuery& c);
  virtual ~DbQuery();
  
  virtual bool exec(const ScriptRef* args) =0;
  virtual bool next_result() =0;
  virtual ScriptRef* result() const =0;
  virtual ScriptRef* result_list() const =0;

  typedef scx::ScriptRefTo<DbQuery> Ref;
  
};
  
};
#endif
