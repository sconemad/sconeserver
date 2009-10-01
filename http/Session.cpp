/* SconeServer (http://www.sconemad.com)

http Session

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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


#include "http/Session.h"
#include "http/HTTPModule.h"
#include "sconex/ConfigFile.h"
#include "sconex/Kernel.h"
namespace http {

class SessionManager;

//=========================================================================
class SessionCleanupJob : public scx::PeriodicJob {

public:

  SessionCleanupJob(SessionManager& manager, const scx::Time& period)
    : scx::PeriodicJob("http::SessionManager",period),
      m_manager(manager) {};

  virtual ~SessionCleanupJob() {};

  virtual bool run()
  {
    //    DEBUG_LOG("SessionCleanupJob running");
    m_manager.check_sessions();
    reset_timeout();
    return false;
  };

protected:
  SessionManager& m_manager;
};

//=========================================================================
SessionManager::SessionManager(HTTPModule& module)
  : m_module(module)
{
  m_job = scx::Kernel::get()->add_job(new SessionCleanupJob(*this,scx::Time(30)));
}

//=========================================================================
SessionManager::~SessionManager()
{
  scx::Kernel::get()->end_job(m_job);

  for (SessionMap::iterator it = m_sessions.begin();
       it != m_sessions.end();
       ++it) {
    Session* session = it->second;
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
    return it->second;
  }
  return 0;
}

//=========================================================================
Session* SessionManager::new_session()
{
  check_sessions();

  Session* s = new Session(m_module);

  scx::MutexLocker locker(m_mutex);
  m_sessions[s->get_id()] = s;
  return s;
}

//=========================================================================
int SessionManager::check_sessions()
{
  scx::MutexLocker locker(m_mutex);

  int n=0;
  for (SessionMap::iterator it = m_sessions.begin(); 
       it != m_sessions.end(); ) {
    Session* session = it->second;
    if (!session->valid() && session->get_num_refs() == 0) {
      DEBUG_LOG("Removing session " << session->get_id() << " due to timeout");
      delete session;
      m_sessions.erase(it++);
      ++n;
    } else {
      ++it;
    }
  }
  return n;
}

//=========================================================================
std::string SessionManager::name() const
{
  return "SESSION MANAGER";
}

//=============================================================================
scx::Arg* SessionManager::arg_lookup(
  const std::string& name
)
{
  // Methods
  
  if ("check" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Properties

  if ("list" == name) {
    scx::ArgList* list = new scx::ArgList();
    scx::MutexLocker locker(m_mutex);
    for (SessionMap::const_iterator it = m_sessions.begin();
	 it != m_sessions.end();
	 ++it) {
      list->give(new scx::ArgObject(it->second));
    }
    return list;
  }
  
  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* SessionManager::arg_resolve(const std::string& name)
{
  return SCXBASE ArgObjectInterface::arg_resolve(name);
}

//=============================================================================
scx::Arg* SessionManager::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("check" == name) {
    check_sessions();
    return 0;
  }

  return ArgObjectInterface::arg_function(auth,name,args);
}


unsigned long long int Session::m_next_id = 0x1234567812345678ULL;

//=========================================================================
Session::Session(
  HTTPModule& module,
  const std::string& id
) : m_module(module),
    m_id(id),
    m_vars("")
{
  if (id.empty()) {
    // Create a new session ID
    std::ostringstream oss;
    for (int i=0; i<16; ++i) {
      int c = rand() % 256;
      oss << std::hex << c;
    }
    oss << std::setw(16) << std::setfill('0') << std::hex << (m_next_id++);
    m_id = oss.str();
  }
}

//=========================================================================
Session::~Session()
{

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
bool Session::valid() const
{
  return (scx::Date::now() <= m_timeout);
}

//=========================================================================
bool Session::allow_upload() const
{
  Session* uc = const_cast<Session*>(this);

  const scx::Arg* a = uc->m_vars.arg_lookup("allow_upload");
  return (a && a->get_int());
}

//=========================================================================
std::string Session::name() const
{
  std::ostringstream oss;
  oss << "SESSION:" << get_id();
  return oss.str();
}

//=============================================================================
scx::Arg* Session::arg_lookup(
  const std::string& name
)
{
  // Methods
  
  if ("reset" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Properties
  
  if ("id" == name) return new scx::ArgString(m_id);
  if ("vars" == name) return new scx::ArgObject(&m_vars);

  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* Session::arg_resolve(const std::string& name)
{
  return SCXBASE ArgObjectInterface::arg_resolve(name);
}

//=============================================================================
scx::Arg* Session::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if ("reset" == name) {
    m_timeout = scx::Date(0);
    m_vars.reset();
    return 0;
  }

  return ArgObjectInterface::arg_function(auth,name,args);
}

};
