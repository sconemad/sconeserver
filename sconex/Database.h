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

#include "sconex/Provider.h"
#include "sconex/ArgObject.h"

namespace scx {

class DbQuery;
  
//=========================================================================
class Database : public scx::ArgObjectInterface {

public:

  static Database* create_new(
    const std::string& type,
    const ArgMap& args);

  Database();
  virtual ~Database();

  virtual DbQuery* new_query(const std::string& query) =0;
  
  virtual std::string name() const =0;
  virtual scx::Arg* arg_lookup(const std::string& name) =0;
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args) =0;

  static void register_provider(const std::string& type,
                                Provider<Database>* factory);
  static void unregister_provider(const std::string& type);
  
protected:

  static void init();
  
  static ProviderScheme<Database>* s_providers;

};

//=========================================================================
class DbQuery : public scx::Arg {

public:

  DbQuery();
  DbQuery(const DbQuery& c);
  DbQuery(RefType ref, DbQuery& c);
  
  virtual Arg* exec(const ArgList& args) =0;
  virtual bool next_result() =0;
  virtual ArgMap* result() const =0;
  virtual ArgList* result_list() const =0;
  
};
  
};
#endif
