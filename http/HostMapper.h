/* SconeServer (http://www.sconemad.com)

HTTP Host mapper

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

#ifndef httpHostMapper_h
#define httpHostMapper_h

#include <http/http.h>
#include <http/Host.h>
#include <sconex/ScriptBase.h>
namespace scx { class Descriptor; };

namespace http {

class HTTPModule;
class Request;
class Response;

//=============================================================================
// HostMapper - Maps host names to host configurations defined on the server
//
class HTTP_API HostMapper : public scx::ScriptObject {
public:

  HostMapper(HTTPModule& module);
  virtual ~HostMapper();
  
  // Process an incoming connection request
  bool connect_request(scx::Descriptor* endpoint,
		       Request& request,
		       Response& response);

  Host* lookup_host(const std::string& name);

  // ScriptObject methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);
  
  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<HostMapper> Ref;
  
protected:

  typedef HASH_TYPE<std::string,std::string> HostNameMap;

  bool lookup(const HostNameMap& map,
	      const std::string& pattern,
	      std::string& result);
  
private:

  HTTPModule& m_module;

  typedef std::map<std::string,Host::Ref*> HostMap;
  HostMap m_hosts;

  HostNameMap m_aliases;
  HostNameMap m_redirects;

};

};
#endif
