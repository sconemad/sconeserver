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

#ifndef httpSession_h
#define httpSession_h

#include "http/http.h"
#include "sconex/ArgObject.h"
#include "sconex/ArgStore.h"
namespace http {

class Session;
class HTTPModule;

//=============================================================================
class HTTP_API SessionManager : public scx::ArgObjectInterface {
public:

  SessionManager(HTTPModule& module);

  virtual ~SessionManager();

  Session* lookup_session(const std::string& id);
  Session* new_session();

  int check_sessions();

  virtual std::string name() const;
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);

 private:
  
  HTTPModule& m_module;
  scx::Mutex m_mutex;

  typedef HASH_TYPE<std::string,Session*> SessionMap;
  SessionMap m_sessions;

};
 
const scx::Time DEFAULT_SESSION_TIMEOUT = scx::Time(10 * 60);

//=============================================================================
class HTTP_API Session : public scx::ArgObjectInterface {
public:

  Session(
    HTTPModule& module,
    const std::string& id = ""
  );

  virtual ~Session();

  const std::string get_id() const;

  void reset_timeout(const scx::Time& time = DEFAULT_SESSION_TIMEOUT);
  bool valid() const;

  bool allow_upload() const;
  
  virtual std::string name() const;
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:

private:

  HTTPModule& m_module;

  std::string m_id;
  static unsigned long long int m_next_id;

  scx::Date m_timeout;

  scx::ArgStore m_vars;

};

};
#endif
