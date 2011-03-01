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
    m_username(username),
    m_password(password),
    m_num_connections(0),
    m_pool_max(5)
{
  std::string::size_type i1 = database.find_first_of(":");
  if (i1 != std::string::npos) {
    std::string::size_type i2 = database.find_first_of(":",i1+1);
    if (i2 != std::string::npos) {
      // host:port:db
      m_host = database.substr(0,i1);
      std::string s_port = database.substr(i1+1,i2-i1+1);
      m_port = (unsigned int)atoi(s_port.c_str());
      m_database = database.substr(i2+1);

    } else {
      // host:db
      m_host = database.substr(0,i1);
      m_database = database.substr(i1+1);
    }
  } else {
    // db
    m_database = database;
  }
}

//=========================================================================
DbSqlProfile::~DbSqlProfile()
{
  m_pool_mutex.lock();
  for (ConnectionList::iterator it = m_connection_pool.begin();
       it != m_connection_pool.end();
       ++it) {
    ::mysql_close(*it);
  }
  m_pool_mutex.unlock();
}

//=============================================================================
scx::DbQuery* DbSqlProfile::new_query(const std::string& query)
{
  return new DbSqlQuery(*this,query);
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
  if ("Query" == name ||
      "set_connection_pool" == name) {
    return new_method(name);
  }

  if ("connection_pool" == name) return new scx::ArgInt(m_pool_max);
  if ("connections" == name) return new scx::ArgInt(m_num_connections);
  
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

  if ("set_connection_pool" == name) {
    scx::ArgInt* a_num = dynamic_cast<scx::ArgInt*>(l->get(0));
    if (!a_num) return new scx::ArgError("set_connection_pool() No value given");
    m_pool_max = a_num->get_int();
    return 0;
  }
  
  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

//=============================================================================
MYSQL* DbSqlProfile::get_connection()
{
  m_pool_mutex.lock();
  if (m_connection_pool.size() > 0) {
    MYSQL* conn = m_connection_pool.front();
    m_connection_pool.pop_front();
    m_pool_mutex.unlock();
    return conn;
  }
  ++m_num_connections;
  m_pool_mutex.unlock();

  DEBUG_LOG("DBSQL profile "<<m_name<<" new connection (total:"<<m_num_connections<<")");
  
  MYSQL* conn = ::mysql_init(0);
  if (0 == ::mysql_real_connect(conn,
                                m_host.c_str(),
                                m_username.c_str(),
                                m_password.c_str(),
                                m_database.c_str(),
                                m_port,
                                0,
                                0)) {
    DEBUG_LOG("mysql_real_connect: Error occurred!");
    m_pool_mutex.lock();
    --m_num_connections;
    m_pool_mutex.unlock();
    return 0;
  }
  return conn;
}

//=============================================================================
void DbSqlProfile::release_connection(MYSQL* conn)
{
  m_pool_mutex.lock();
  if (m_connection_pool.size() >= m_pool_max) {
    --m_num_connections;
    DEBUG_LOG("DBSQL profile "<<m_name<<" closing connection (total:"<<m_num_connections<<")");
    ::mysql_close(conn);
  } else {
    m_connection_pool.push_back(conn);
  }
  m_pool_mutex.unlock();
}

};
