/* SconeServer (http://www.sconemad.com)

SQL Database Query

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


#include "DbSqlQuery.h"

namespace dbsql {

//=========================================================================
DbSqlQuery::DbSqlQuery(DbSqlProfile& profile,
		       const std::string& query)
  : m_profile(profile),
    m_ref(m_profile.m_module.ref()),
    m_query(new std::string(query))
{
  DEBUG_COUNT_CONSTRUCTOR(DbSqlQuery);
}

//=========================================================================
DbSqlQuery::DbSqlQuery(const DbSqlQuery& c)
  : Arg(c),
    m_profile(c.m_profile),
    m_ref(c.m_ref),
    m_query(new std::string(*c.m_query))
{
  DEBUG_COUNT_CONSTRUCTOR(DbSqlQuery);
}

//=========================================================================
DbSqlQuery::DbSqlQuery(RefType ref, DbSqlQuery& c)
  : Arg(ref,c),
    m_profile(c.m_profile),
    m_ref(c.m_ref),
    m_query(c.m_query)
{
  DEBUG_COUNT_CONSTRUCTOR(DbSqlQuery);
}

//=========================================================================
DbSqlQuery::~DbSqlQuery()
{
  if (last_ref()) {
    delete m_query;
  }
  DEBUG_COUNT_DESTRUCTOR(DbSqlQuery);
}

//=========================================================================
scx::Arg* DbSqlQuery::new_copy() const
{
  return new DbSqlQuery(*this);
}

//===========================================================================
scx::Arg* DbSqlQuery::ref_copy(RefType ref)
{
  return new DbSqlQuery(ref,*this);
}

//=========================================================================
std::string DbSqlQuery::get_string() const
{
  return *m_query;
}

//=========================================================================
int DbSqlQuery::get_int() const
{
  return !m_query->empty();
}

//=========================================================================
scx::Arg* DbSqlQuery::op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right)
{
  if (is_method_call(optype,opname)) {

    scx::ArgList* l = dynamic_cast<scx::ArgList*>(right);
    
    if ("exec" == m_method) {
      if (is_const()) return new scx::ArgError("Not permitted");
      
      MYSQL* conn = m_profile.m_connection;

      m_profile.m_module.log("{"+m_profile.m_name+"} exec: "+*m_query);
      if (::mysql_query(m_profile.m_connection,m_query->c_str())) {
	// Error
	return new scx::ArgError(::mysql_error(conn));
      }
      
      MYSQL_RES* res = ::mysql_use_result(conn);
      
      scx::ArgList* list = new scx::ArgList();
      MYSQL_ROW row;
      while ((row = ::mysql_fetch_row(res)) != 0) {
	unsigned int n = ::mysql_num_fields(res);
	scx::ArgList* row_list = new scx::ArgList();
	for (unsigned int i=0; i<n; ++i) {
	  row_list->give( new scx::ArgString(row[i]) );
	}
	list->give(row_list);
      }
      
      ::mysql_free_result(res);
      return list;
    }
    
  } else if (scx::Arg::Binary == optype) {

    if ("." == opname) {
      std::string name = right->get_string();

      if (name == "exec") return new_method(name);
    }

  }
  return SCXBASE Arg::op(auth,optype,opname,right);
}

};
