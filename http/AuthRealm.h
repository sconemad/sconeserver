/* SconeServer (http://www.sconemad.com)

HTTP Authentication

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

#ifndef httpAuthRealm_h
#define httpAuthRealm_h

#include <http/http.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/Mutex.h>
#include <sconex/Provider.h>
#include <sconex/Password.h>
namespace http {

class HTTPModule;
class MessageStream;

//=============================================================================
// AuthRealm - An authentication realm, an object which can determine whether 
// a specified username/password combination is valid.
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

  AuthRealm(HTTPModule* module);

  virtual ~AuthRealm();

  void set_name(const std::string& name);

  // Returns a 'good' scriptref if the specified username/password combination
  // is valid, or a 'bad' scriptref (can be NULL) otherwise. In the succesful
  // case, the scriptref might be an object representing the user.
  scx::ScriptRef* authenticate(const std::string& username,
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

  AuthRealm(const AuthRealm& c);
  AuthRealm& operator=(const AuthRealm& v);
  // Prohibit copy

  // Lookup authentication hash for specified user
  // Must return an empty string if the user doesn't exist
  virtual std::string lookup_hash(const std::string& username) = 0;

  // Update authentication hash for specified user
  // (if supported by realm)
  virtual bool update_hash(const std::string& username,
			   const std::string& hash);

  // Lookup data for specified user 
  // (if supported by realm)
  virtual scx::ScriptRef* lookup_data(const std::string& username);

  // Add a user to the realm with the specified password hash
  // (if supported by realm)
  virtual bool add_user(const std::string& username, const std::string& hash);

  // Remove a user from the realm
  // (if supported by realm)
  virtual bool remove_user(const std::string& username);

  scx::ScriptRefTo<HTTPModule> m_module;
  std::string m_name;
  scx::PasswordHash::Ref m_hash_method;

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
  void register_realm_type(const std::string& type,
			   scx::Provider<AuthRealm>* factory);
  void unregister_realm_type(const std::string& type,
			     scx::Provider<AuthRealm>* factory);

  // We also provide some standard realm types
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       AuthRealm*& object);

  typedef scx::ScriptRefTo<AuthRealmManager> Ref;

private:

  AuthRealmManager(const AuthRealmManager& c);
  AuthRealmManager& operator=(const AuthRealmManager& v);
  // Prohibit copy

  HTTPModule* m_module;

  typedef std::map<std::string,AuthRealm::Ref*> AuthRealmMap;
  AuthRealmMap m_realms;

  // AuthRealm providers
  scx::ProviderScheme<AuthRealm> m_realm_types;

};

};
#endif
