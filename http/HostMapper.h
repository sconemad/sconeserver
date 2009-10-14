/* SconeServer (http://www.sconemad.com)

HTTP Host mapper

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpHostMapper_h
#define httpHostMapper_h

#include "http/http.h"
#include "sconex/ArgObject.h"
namespace http {

class Host;
class HTTPModule;
class Request;
class Response;

//=============================================================================
class HTTP_API HostMapper : public scx::ArgObjectInterface {
public:

  HostMapper(HTTPModule& module);
  virtual ~HostMapper();
  
  bool connect_request(scx::Descriptor* endpoint, Request& request, Response& response);

  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);

protected:

  typedef HASH_TYPE<std::string,std::string> HostNameMap;
  bool lookup(const HostNameMap& map, const std::string& pattern, std::string& result);
  
private:

  HTTPModule& m_module;

  typedef std::map<std::string,Host*> HostMap;
  HostMap m_hosts;

  HostNameMap m_aliases;
  HostNameMap m_redirects;

};

};
#endif
