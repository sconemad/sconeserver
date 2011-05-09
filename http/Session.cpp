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


#include <http/Session.h>
#include <http/HTTPModule.h>
#include <sconex/ConfigFile.h>
#include <sconex/Kernel.h>
namespace http {

class SessionManager;

#define SESSION_JOB_TIMEOUT 30

//=========================================================================
// SessionCleanupJob - A periodic job which causes the session manager to
// check for timed-out sessions on a regular basis.
//
class SessionCleanupJob : public scx::PeriodicJob {

public:

  SessionCleanupJob(SessionManager& manager, const scx::Time& period)
    : scx::PeriodicJob("http Session cleanup",period),
      m_manager(manager) {};

  virtual ~SessionCleanupJob() {};

  virtual bool run()
  {
    m_manager.check_sessions();
    reset_timeout();
    return false;
  };

protected:
  SessionManager& m_manager;
};

unsigned long long int Session::m_next_id = 0;

//=========================================================================
Session::Session(
  HTTPModule& module,
  const std::string& id
) : m_module(module),
    m_id(id)
{
  DEBUG_COUNT_CONSTRUCTOR(Session);

  m_parent = &m_module;

  if (m_next_id == 0) {
    // Seed the id counter with a big random-ish value.
    for (int i=0; i<100; ++i) {
      m_next_id += rand();
      m_next_id *= rand();
    }
  }

  if (id.empty()) {
    // Create a new session ID
    std::ostringstream oss;
    for (int i=0; i<16; ++i) {
      int c = rand() % 256;
      oss << std::setw(2) << std::setfill('0') << std::hex << c;
    }
    oss << std::setw(16) << std::setfill('0') << std::hex << (m_next_id++);
    m_id = oss.str();
  }

  reset_timeout();
}

//=========================================================================
Session::~Session()
{
  DEBUG_COUNT_DESTRUCTOR(Session);
}

//=========================================================================
const std::string Session::get_id() const
{
  return m_id;
}

//=========================================================================
void Session::reset_timeout(const scx::Time& time)
{
  m_timeout = scx::Date::now() + time;
}

//=========================================================================
const scx::Date& Session::get_timeout() const
{
  return m_timeout;
}
  
//=========================================================================
bool Session::valid() const
{
  return (scx::Date::now() <= m_timeout);
}

//=========================================================================
bool Session::allow_upload() const
{
  const scx::ScriptRef* a = lookup("allow_upload");
  return (a && a->object()->get_int());
}

//=========================================================================
std::string Session::get_string() const
{
  return get_id();
}

//=============================================================================
scx::ScriptRef* Session::script_op(const scx::ScriptAuth& auth,
				   const scx::ScriptRef& ref,
				   const scx::ScriptOp& op,
				   const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("reset" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
    
    // Properties
    if ("id" == name) return scx::ScriptString::new_ref(m_id);

  }
    
  return scx::ScriptMap::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* Session::script_method(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const std::string& name,
				       const scx::ScriptRef* args)
{
  if ("reset" == name) {
    m_timeout = scx::Date(0);
    clear();
    return 0;
  }

  return scx::ScriptMap::script_method(auth,ref,name,args);
}


//=========================================================================
SessionManager::SessionManager(HTTPModule& module)
  : m_module(module)
{
  m_parent = &m_module;

#ifndef DISABLE_JOBS
  m_job = scx::Kernel::get()->add_job(
    new SessionCleanupJob(*this,scx::Time(SESSION_JOB_TIMEOUT)));
#endif
}

//=========================================================================
SessionManager::~SessionManager()
{
#ifndef DISABLE_JOBS
  scx::Kernel::get()->end_job(m_job);
#endif

  for (SessionMap::iterator it = m_sessions.begin();
       it != m_sessions.end();
       ++it) {
    Session::Ref* session = it->second;
    delete session;
  }
}

//=========================================================================
Session* SessionManager::lookup_session(const std::string& id)
{
  check_sessions();

  scx::MutexLocker locker(m_mutex);
  SessionMap::iterator it = m_sessions.find(id);
  if (it != m_sessions.end()) {
    return it->second->object();
  }
  return 0;
}

//=========================================================================
Session* SessionManager::new_session()
{
  check_sessions();

  Session* session = new Session(m_module);

  scx::MutexLocker locker(m_mutex);
  m_sessions[session->get_id()] = new Session::Ref(session);
  return session;
}

//=========================================================================
int SessionManager::check_sessions()
{
  scx::MutexLocker locker(m_mutex);

  int n=0;
  for (SessionMap::iterator it = m_sessions.begin(); 
       it != m_sessions.end(); ) {
    Session* session = it->second->object();
    if (!session->valid() && session->num_refs() == 1) {
      log("Removing session " + session->get_id() + " due to timeout");
      delete session;
      m_sessions.erase(it++);
      ++n;
    } else {
      ++it;
    }
  }
  return n;
}

//=============================================================================
scx::ScriptRef* SessionManager::script_op(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const scx::ScriptOp& op,
					  const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("check" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
    
    // Properties
    if ("list" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      scx::ScriptRef* list_ref = new scx::ScriptRef(list);
      scx::MutexLocker locker(m_mutex);
      for (SessionMap::const_iterator it = m_sessions.begin();
	   it != m_sessions.end();
	   ++it) {
	Session::Ref* session_ref = it->second;
	list->give(session_ref->ref_copy(ref.reftype()));
      }
      return list_ref;
    }
  }
  
  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SessionManager::script_method(const scx::ScriptAuth& auth,
					      const scx::ScriptRef& ref,
					      const std::string& name,
					      const scx::ScriptRef* args)
{
  if ("check" == name) {
    check_sessions();
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

};
