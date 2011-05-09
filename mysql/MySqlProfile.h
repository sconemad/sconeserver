/* SconeServer (http://www.sconemad.com)

MySQL Database Profile

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

#ifndef MySqlProfile_h
#define MySqlProfile_h

#include <sconex/Database.h>
#include <sconex/Mutex.h>

#include <mysql.h>

namespace mysql {

class MySqlModule;

//=========================================================================
// MySqlProfile - A MySQL database connection profile
//
class MySqlProfile : public scx::Database {
public:

  MySqlProfile(MySqlModule& module,
	       const std::string& name,
	       const std::string& database,
	       const std::string& username,
	       const std::string& password);
  
  ~MySqlProfile();

  MYSQL* get_connection();
  void release_connection(MYSQL* conn);
  
  // scx::Database methods
  virtual scx::DbQuery* new_query(const std::string& query);
  
    // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					scx::ScriptRef* args);

  typedef scx::ScriptRefTo<MySqlProfile> Ref;

private:

  friend class MySqlQuery;

  MySqlModule& m_module;

  std::string m_name;

  std::string m_database;
  std::string m_host;
  int m_port;
  std::string m_username;
  std::string m_password;

  typedef std::list<MYSQL*> ConnectionList;
  ConnectionList m_connection_pool;

  scx::Mutex m_pool_mutex;

  int m_num_connections;
  int m_pool_max;
};

};
#endif
