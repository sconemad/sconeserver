/* SconeServer (http://www.sconemad.com)

http Document root

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpDocRoot_h
#define httpDocRoot_h

#include "http/HTTPModule.h"
#include "sconex/ArgObject.h"
#include "sconex/ArgStore.h"
namespace http {

class HostMapper;
class Request;
class Response;
class AuthRealm;

//=============================================================================
class ModuleMap {
public:

  ModuleMap(
    HTTPModule& module,
    const std::string& name,
    scx::ArgList* args
  );
  
  ~ModuleMap();

  const std::string& get_name() const;
  
  bool connect_module(scx::Descriptor* endpoint);
  
private:
  HTTPModule& m_module;

  std::string m_name;
  scx::ArgList* m_args;
};

  
//=============================================================================
class HTTP_API DocRoot : public scx::ArgObjectInterface {
public:

  DocRoot(
    HTTPModule& module,
    Host& host,
    const std::string profile,
    const scx::FilePath& path
  );
  
  // Create a docroot for specified profile and root path

  virtual ~DocRoot();

  const std::string get_profile() const;

  bool connect_request(scx::Descriptor* endpoint, Request& request, Response& response);

  ModuleMap* lookup_extn_mod(const std::string& name) const;
  ModuleMap* lookup_path_mod(const std::string& name, std::string& pathinfo) const;

  std::string lookup_realm_map(const std::string& name) const;
  
  const scx::Arg* get_param(const std::string& name) const;
  void set_param(const std::string& name, scx::Arg* value);

  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);
  
protected:

  bool check_auth(Request& request, Response& response);
  
private:

  HTTPModule& m_module;
  Host& m_host;
  std::string m_profile;
  
  scx::FilePath m_path;

  typedef std::map<std::string,ModuleMap*> PatternMap;
  PatternMap m_extn_mods;
  PatternMap m_path_mods;

  typedef std::map<std::string,std::string> RealmMap;
  RealmMap m_realm_maps;

  scx::ArgStore m_params;
};

};
#endif
