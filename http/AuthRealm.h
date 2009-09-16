/* SconeServer (http://www.sconemad.com)

http Authorisation Realm

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

#ifndef httpAuthRealm_h
#define httpAuthRealm_h

#include "http/http.h"
#include "sconex/ArgObject.h"
#include "sconex/FilePath.h"
namespace http {

class HTTPModule;
class MessageStream;

/*  
//=============================================================================
class HTTP_API HTTPUser : public scx::ArgObjectInterface {
public:

  HTTPUser(const std::string& name,const scx::FilePath& path);
  virtual ~HTTPUser();

  bool authorised(const std::string& pass);

  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);

private:

  std::string m_name;
  scx::ArgStore m_vars;

};
*/

//=============================================================================
class HTTP_API AuthRealm : public scx::ArgObjectInterface {
public:

  AuthRealm(
    HTTPModule& module,
    const std::string name,
    const scx::FilePath& path
  );
  
  // Create an authrealm for specified profile and root path

  virtual ~AuthRealm();

  const std::string& get_realm() const;
  
  bool authorised(const std::string& user, const std::string& pass);

  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:

  void refresh();

private:

  HTTPModule& m_module;
  std::string m_name;
  scx::FilePath m_path;
  scx::Date m_modtime;
  scx::Mutex m_mutex;

  std::map<std::string,std::string> m_users;
};

//=============================================================================
class HTTP_API AuthRealmManager : public scx::ArgObjectInterface {
public:

  AuthRealmManager(HTTPModule& module);
  
  virtual ~AuthRealmManager();
  
  AuthRealm* lookup_realm(const std::string& name);

  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:

private:

  HTTPModule& m_module;

  std::map<std::string,AuthRealm*> m_realms;
};



};
#endif
