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

#include <http/http.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/Mutex.h>
#include <sconex/Provider.h>
namespace http {

class HTTPModule;
class MessageStream;

//=============================================================================
// AuthRealm - An authorisation realm, essentially an object which determines
// whether a specified username/password combination is valid.
//
// This is the base class, from which realm implementations are derived.
// The default realm implementations can be suplemented using the realm
// provider interface in AuthRealmManager below.
//
// A realm can be assigned to control access to one or more HTTP resources
// via HTTP basic authentication, or queried directly from a script to provide
// HTML-form based authentication.
//
class HTTP_API AuthRealm : public scx::ScriptObject {
public:

  // Create an authorisation realm for specified profile and root path
  AuthRealm(const std::string name);

  virtual ~AuthRealm();

  // Returns a 'good' scriptref if the specified username/password combination
  // is valid, or a 'bad' scriptref (can be NULL) otherwise. In the succesful
  // case, the scriptref might be an object representing the user.
  virtual scx::ScriptRef* authorised(const std::string& username,
				     const std::string& password) = 0;

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

  std::string m_name;

};

//=============================================================================
// AuthRealmManager - Manages the available authorisation realms, and allows
// additional realm types to be registered and used.
//
class HTTP_API AuthRealmManager : public scx::ScriptObject,
                                  public scx::Provider<AuthRealm> {
public:

  AuthRealmManager(HTTPModule* module);
  
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

  // AuthRealm provider interface for registering extension realm types
  void register_realm(const std::string& type,
		      scx::Provider<AuthRealm>* factory);
  void unregister_realm(const std::string& type,
			scx::Provider<AuthRealm>* factory);

  // We also provide some standard realm types
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       AuthRealm*& object);

  typedef scx::ScriptRefTo<AuthRealmManager> Ref;

private:

  HTTPModule* m_module;

  typedef std::map<std::string,AuthRealm::Ref*> AuthRealmMap;
  AuthRealmMap m_realms;

  // AuthRealm providers
  scx::ProviderScheme<AuthRealm> m_providers;

};

};
#endif
