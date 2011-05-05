/* SconeServer (http://www.sconemad.com)

MySQL Database Module

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

#ifndef MySqlModule_h
#define MySqlModule_h

#include "MySqlProfile.h"
#include "sconex/Database.h"
#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Provider.h"

namespace mysql {

class MySqlProfile;
class MySqlQuery;

//=============================================================================
class MySqlModule : public scx::Module,
                    public scx::Provider<scx::Database::Ref>
{
public:

  MySqlModule();
  virtual ~MySqlModule();

  virtual std::string info() const;

  virtual int init();
  virtual void close();
  
  MySqlProfile* lookup_profile(const std::string& profile);

  MySqlQuery* new_query(const std::string& profile,const std::string& query);

  // ScriptObject methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // scx::Provider<scx::Database::Ref> methods
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Database::Ref*& object);
  
protected:

private:

  typedef HASH_TYPE<std::string,MySqlProfile::Ref*> MySqlProfileMap;
  MySqlProfileMap m_profiles;

};

};
#endif
