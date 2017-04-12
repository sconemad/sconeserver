/* SconeServer (http://www.sconemad.com)

http Host

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

#ifndef httpHost_h
#define httpHost_h

#include <http/http.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/FilePath.h>

namespace scx { class Descriptor; }

namespace http {

class HTTPModule;
class HostMapper;
class Request;
class Response;
class AuthRealm;
class MessageStream;
class HandlerMap;
  
//=============================================================================
// Host - An HTTP host configuration
//
class HTTP_API Host : public scx::ScriptObject {
public:

  // Create a host with id and dir
  Host(
    HTTPModule& module,
    HostMapper& mapper,
    const std::string id,
    const std::string hostname,
    const scx::FilePath& hostroot,
    const scx::FilePath& docroot
  );

  virtual ~Host();

  int init();

  // Process an incoming connection request
  bool connect_request(MessageStream* message,
		       Request& request,
		       Response& response);
  
  const std::string get_id() const;
  const std::string get_hostname() const;
  const scx::FilePath& get_hostroot() const;
  const scx::FilePath& get_docroot() const;

  // Map a file extension to a handler and arguments
  void add_extn_map(const std::string& pattern,
                    const std::string& handler,
                    scx::ScriptRef* args);
  
  // Lookup handler map to use for a particular file extension
  HandlerMap* lookup_extn_map(const std::string& name) const;

  // Map a path to a handler and arguments
  void add_path_map(const std::string& pattern,
                    const std::string& handler,
                    scx::ScriptRef* args);

  // Lookup handler to use for a path
  HandlerMap* lookup_path_map(const std::string& name,
			     std::string& pathinfo) const;

  // Map a path to an authentication realm
  void add_realm_map(const std::string& pattern,
                     const std::string& realm);

  // Lookup authentication realm to use for path
  std::string lookup_realm_map(const std::string& name) const;
  
  // Get/set parameter stored with this profile
  const scx::ScriptRef* get_param(const std::string& name) const;
  void set_param(const std::string& name,
		 scx::ScriptRef* value);


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

  typedef scx::ScriptRefTo<Host> Ref;
  
protected:

  // Check the path contained in the request URI is valid.
  bool check_path(const std::string& uripath);
  
  // Check HTTP authorization for this request.
  // Returns true if it passes.
  // Returns false if it fails, in which case the response headers are  set 
  // appropriately to request client authentication.
  bool check_auth(Request& request, Response& response);

  // Lookup/create/update the session for this request if required.
  void check_session(Request& request, Response& response);

private:

  HTTPModule& m_module;
  HostMapper& m_mapper;
  std::string m_id;
  std::string m_hostname;
  scx::FilePath m_hostroot;
  scx::FilePath m_docroot;

  typedef std::map<std::string,HandlerMap*> PatternMap;
  PatternMap m_extn_mods;
  PatternMap m_path_mods;

  typedef std::map<std::string,std::string> RealmMap;
  RealmMap m_realm_maps;

  scx::ScriptMap::Ref m_params;
};

};
#endif
