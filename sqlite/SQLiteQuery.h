/* SconeServer (http://www.sconemad.com)

SQLite database query

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef SQLiteQuery_h
#define SQLiteQuery_h

#include "SQLiteProfile.h"

namespace sqlite {

//=============================================================================
class SQLiteQuery : public scx::DbQuery {

public:

  SQLiteQuery(SQLiteProfile* profile, 
              const std::string& query = "");
  SQLiteQuery(const SQLiteQuery& c);
  virtual ~SQLiteQuery();

  // ScriptObject methods
  virtual scx::ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // DbQuery methods
  virtual bool exec(const scx::ScriptRef* args);
  virtual bool next_result();
  virtual scx::ScriptRef* result() const;
  virtual scx::ScriptRef* result_list() const;

  typedef scx::ScriptRefTo<SQLiteQuery> Ref;
  
protected:

  void init();
  void log_error(const std::string& context, bool invalid);

  void bind_param(int param, const scx::ScriptRef* arg);
  scx::ScriptRef* get_result(int col) const;
  
  scx::ScriptRefTo<SQLiteProfile> m_profile;

  std::string m_query;
  sqlite3* m_db;
  sqlite3_stmt* m_stmt;

  int m_step_err;
  bool m_first;
  bool m_invalid;
  std::string m_error_string;
};

};
#endif
