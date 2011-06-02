/* SconeServer (http://www.sconemad.com)

HTTP Authorisation using database

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

#ifndef httpAuthRealmDB_h
#define httpAuthRealmDB_h

#include <http/AuthRealm.h>
#include <sconex/Database.h>
namespace http {

//=============================================================================
// HTTPUser - An authorised HTTP user
//
class HTTP_API HTTPUser : public scx::DbProxy {
public:

  HTTPUser(HTTPModule* module,
	   scx::Database* db,
	   const std::string& name,
	   int id);

  virtual ~HTTPUser();

  // ScriptObject methods
  virtual std::string get_string() const;

private:

  scx::ScriptRefTo<HTTPModule> m_module;
  std::string m_name;

};


//=============================================================================
// AuthRealmDB - An authorisation realm which uses a database
//
class HTTP_API AuthRealmDB : public AuthRealm {
public:

  AuthRealmDB(HTTPModule* module,
	      const scx::ScriptRef* args);

  virtual ~AuthRealmDB();

  // AuthRealm methods  
  virtual scx::ScriptRef* authorised(const std::string& username,
				     const std::string& password);

private:

  scx::ScriptRefTo<HTTPModule> m_module;
  scx::Database::Ref* m_db;

};

};
#endif
