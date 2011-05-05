/* SconeServer (http://www.sconemad.com)

HTTP Authorisation

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

#ifndef httpAuthRealm_h
#define httpAuthRealm_h

#include "http/http.h"
#include "sconex/ScriptBase.h"
#include "sconex/ScriptTypes.h"
#include "sconex/FilePath.h"
#include "sconex/Date.h"
#include "sconex/Mutex.h"
namespace http {

class HTTPModule;
class MessageStream;

//=============================================================================
// HTTPUser - An authorised HTTP user
//
class HTTP_API HTTPUser : public scx::ScriptMap {
public:

  HTTPUser(HTTPModule& module,
	   const std::string& username,
	   const std::string& password,
	   const scx::FilePath& path);

  virtual ~HTTPUser();

  bool authorised(const std::string& password);

  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<HTTPUser> Ref;

private:

  HTTPModule& m_module;

  std::string m_username;
  std::string m_password;

};

//=============================================================================
// AuthRealm - An authorisation realm, essentially a collection of authorised 
// users. A realm can then be assigned to control access to one or more
// HTTP resources.
// 
//
class HTTP_API AuthRealm : public scx::ScriptObject {
public:

  // Create an authorisation realm for specified profile and root path
  AuthRealm(HTTPModule& module,
	    const std::string name,
	    const scx::FilePath& path);

  virtual ~AuthRealm();

  HTTPUser* lookup_user(const std::string& username);
  
  bool authorised(const std::string& username,
		  const std::string& password);

  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<AuthRealm> Ref;
  
protected:

  void refresh();

private:

  HTTPModule& m_module;
  std::string m_name;
  scx::FilePath m_path;
  scx::Date m_modtime;
  scx::Mutex m_mutex;

  typedef std::map<std::string,HTTPUser::Ref*> UserMap;
  UserMap m_users;
};

//=============================================================================
// AuthRealmManager - Manages the available authorisation realms.
//
//
class HTTP_API AuthRealmManager : public scx::ScriptObject {
public:

  AuthRealmManager(HTTPModule& module);
  
  virtual ~AuthRealmManager();
  
  AuthRealm* lookup_realm(const std::string& name);

  // ScriptObject methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<AuthRealmManager> Ref;
  
protected:

private:

  HTTPModule& m_module;

  typedef std::map<std::string,AuthRealm::Ref*> AuthRealmMap;
  AuthRealmMap m_realms;
};

};
#endif
