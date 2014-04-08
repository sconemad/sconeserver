/* SconeServer (http://www.sconemad.com)

HTTP Sessions

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

#ifndef httpSession_h
#define httpSession_h

#include <http/http.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Job.h>
namespace http {

class HTTPModule;
class SessionManager;
  
const scx::Time DEFAULT_SESSION_TIMEOUT = scx::Time(60 * 60);

//=============================================================================
// Session - Holds information about a user's interaction with a web site 
// from a particular client. Used to maintain state across (otherwise 
// stateless) HTTP protocol interactions.
//
class HTTP_API Session : public scx::ScriptObject {
public:

  // Create a session.
  // A new, random session id is created if not specified.
  Session(SessionManager& manager,
          const std::string& id = "");

  virtual ~Session();

  // Get the session id
  const std::string get_id() const;

  // Set/get session timeout, this is the time from when the session was last 
  // used until it expires.
  void set_timeout(const scx::Time& time);
  const scx::Time& get_timeout() const;

  // Set/get when the session was last used
  void set_last_used(const scx::Date& used = scx::Date::now());
  const scx::Date& get_last_used() const;
  
  // Get the expiry date
  scx::Date get_expiry() const;

  // Test if the session is valid (i.e. within its expiry date)
  bool valid() const;

  // Attempt to lock the session, returns false if already locked
  bool lock();

  // Unlock the session
  void unlock();

  // Test if the session has the named permission
  bool has_permission(const std::string& permission) const;

  // Add/remove permission
  void add_permission(const std::string& permission);
  void remove_permission(const std::string& permission);
  
  SessionManager& get_manager() const;
  
  // ScriptObject methods
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

  typedef scx::ScriptRefTo<Session> Ref;
  
protected:

private:

  SessionManager& m_manager;

  std::string m_id;

  scx::ScriptMap::Ref m_vars;
  std::set<std::string> m_perms;

  scx::Time m_timeout;
  scx::Date m_last_used;
  bool m_locked;
  
};

//=============================================================================
// SessionManager - Maintains a list of active HTTP sessions
//
class HTTP_API SessionManager : public scx::ScriptObject {
public:

  SessionManager(HTTPModule& module);

  virtual ~SessionManager();

  // Lookup an existing session by ID
  Session::Ref* lookup_session(const std::string& id);

  // Create a new session
  Session::Ref* new_session();

  // Check through sessions, removing any that have timed-out
  int check_sessions();

  // ScriptObject methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<SessionManager> Ref;

 private:

  friend class Session;
  
  HTTPModule& m_module;
  scx::Mutex m_mutex;
  
  typedef HASH_TYPE<std::string,Session::Ref*> SessionMap;
  SessionMap m_sessions;
  scx::JobID m_job;
  
};
 
};
#endif
