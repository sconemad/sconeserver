/* SconeServer (http://www.sconemad.com)

SQL Database Profile

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef DbSqlProfile_h
#define DbSqlProfile_h

#include "DbSqlModule.h"
#include "sconex/ArgObject.h"

#include <mysql.h>

namespace dbsql {

class DbSqlModule;

//=========================================================================
class DbSqlProfile : public scx::ArgObjectInterface {

public:

  DbSqlProfile(DbSqlModule& module,
	       const std::string& name,
	       const std::string& database,
	       const std::string& username,
	       const std::string& password);
  
  ~DbSqlProfile();

  virtual std::string name() const;
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);

  MYSQL* new_connection();

private:

  friend class DbSqlQuery;

  DbSqlModule& m_module;

  std::string m_name;
  std::string m_database;
  std::string m_username;
  std::string m_password;

};

};
#endif
