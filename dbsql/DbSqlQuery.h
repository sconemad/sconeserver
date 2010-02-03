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

#ifndef DbSqlQuery_h
#define DbSqlQuery_h

#include "DbSqlProfile.h"
#include "sconex/Arg.h"

namespace dbsql {

//=============================================================================
class DbSqlQuery : public scx::Arg {

public:

  DbSqlQuery(DbSqlProfile& profile, const std::string& query = "");
  DbSqlQuery(const DbSqlQuery& c);
  DbSqlQuery(RefType ref, DbSqlQuery& c);
  virtual ~DbSqlQuery();
  virtual scx::Arg* new_copy() const;
  virtual scx::Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::Arg* op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right);

protected:

  DbSqlProfile& m_profile;
  scx::ModuleRef m_ref;
  mysqlpp::Query* m_query;

};

};
#endif
