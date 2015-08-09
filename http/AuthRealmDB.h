/* SconeServer (http://www.sconemad.com)

HTTP authentication using database storage

This reads usernames and password hashes from the 'user' table in the specified
database profile. Supports password updates and storing additional user data
(according to the database schema).

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
// AuthRealmDB - An authentication realm which uses database storage
//
class HTTP_API AuthRealmDB : public AuthRealm {
public:

  AuthRealmDB(HTTPModule* module,
	      const scx::ScriptRef* args);

  virtual ~AuthRealmDB();

protected:

  // AuthRealm methods  
  virtual std::string lookup_hash(const std::string& username);
  virtual bool update_hash(const std::string& username,
			   const std::string& hash);
  virtual scx::ScriptRef* lookup_data(const std::string& username);
  virtual bool add_user(const std::string& username,const std::string& hash);
  virtual bool remove_user(const std::string& username);  
  
private:

  void check_database();
  
  scx::Database::Ref* m_db;

};

};
#endif
