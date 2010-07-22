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


#include "DbSqlProfile.h"
#include "DbSqlQuery.h"

namespace dbsql {

//=========================================================================
DbSqlProfile::DbSqlProfile(
  DbSqlModule& module,
  const std::string& name,
  const std::string& database,
  const std::string& username,
  const std::string& password
) : m_module(module),
    m_name(name),
    m_database(database),
    m_username(username),
    m_password(password)
{

}

//=========================================================================
DbSqlProfile::~DbSqlProfile()
{

}

//=============================================================================
std::string DbSqlProfile::name() const
{
  return m_name;
}

//=============================================================================
scx::Arg* DbSqlProfile::arg_lookup(
  const std::string& name
)
{
  // Methods
  if ("Query" == name) {
    return new_method(name);
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* DbSqlProfile::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  // Allow this for now
  //  if (!auth.trusted()) return new scx::ArgError("Not permitted");

  if ("Query" == name) {
    scx::Arg* a_str = l->get(0);
    std::string s_str;
    if (a_str) s_str = a_str->get_string();
    return new DbSqlQuery(*this,s_str);
  }
  
  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

//=============================================================================
MYSQL* DbSqlProfile::new_connection()
{
  std::string db = "";
  std::string host = "";
  unsigned int port = 0;

  std::string::size_type i1 = m_database.find_first_of(":");
  if (i1 != std::string::npos) {
    std::string::size_type i2 = m_database.find_first_of(":",i1+1);
    if (i2 != std::string::npos) {
      // host:port:db
      host = m_database.substr(0,i1);
      std::string s_port = m_database.substr(i1+1,i2-i1+1);
      port = (unsigned int)atoi(s_port.c_str());
      db = m_database.substr(i2+1);

    } else {
      // host:db
      host = m_database.substr(0,i1);
      db = m_database.substr(i1+1);
    }
  } else {
    // db
    db = m_database;
  }

  DEBUG_LOG("Connecting to db '" << host << ":" << port << ":" << db <<
            "' as user '" << m_username << "'");
  
  MYSQL* conn = ::mysql_init(0);
  if (0 == ::mysql_real_connect(conn,
                                host.c_str(),
                                m_username.c_str(),
                                m_password.c_str(),
                                db.c_str(),
                                port,
                                0,
                                0)) {
    DEBUG_LOG("mysql_real_connect: Error occurred!");
  }
  return conn;
}

};
