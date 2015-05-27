/* SconeServer (http://www.sconemad.com)

http Document root

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

#ifndef httpDocRoot_h
#define httpDocRoot_h

#include <http/http.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/FilePath.h>

namespace scx { class Descriptor; }

namespace http {

class HTTPModule;
class Host;
class HostMapper;
class Request;
class Response;
class AuthRealm;

//=============================================================================
// StreamMap - Connects an incoming HTTP request to a stream for handling.
//
class StreamMap {
public:

  StreamMap(
    HTTPModule& module,
    const std::string& type,
    scx::ScriptRef* args
  );
  
  ~StreamMap();

  const std::string& get_type() const;
  
  bool connect(scx::Descriptor* endpoint);
  
private:
  StreamMap(const StreamMap& c);
  StreamMap& operator=(const StreamMap& v);
  // Prohibit copy

  HTTPModule& m_module;

  std::string m_type;
  scx::ScriptRef* m_args;
};

  
//=============================================================================
// DocRoot - Represents an HTTP document root for a given host and profile
// configuration.
//
class HTTP_API DocRoot : public scx::ScriptObject {
public:

  // Create a docroot for specified host, profile and root path
  DocRoot(
    HTTPModule& module,
    Host& host,
    const std::string profile,
    const scx::FilePath& path
  );
  
  virtual ~DocRoot();

  const std::string get_profile() const;

  // Process an incoming connection request
  bool connect_request(scx::Descriptor* endpoint,
		       Request& request,
		       Response& response);

  // Map a file extension to a stream type and arguments
  void add_extn_map(const std::string& pattern,
                    const std::string& stream,
                    scx::ScriptRef* args);
  
  // Lookup stream map to use for a particular file extension
  StreamMap* lookup_extn_map(const std::string& name) const;

  // Map a path to a stream type and arguments
  void add_path_map(const std::string& pattern,
                    const std::string& stream,
                    scx::ScriptRef* args);

  // Lookup stream map to use for a path
  StreamMap* lookup_path_map(const std::string& name,
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

  typedef scx::ScriptRefTo<DocRoot> Ref;
  
protected:

  DocRoot(const DocRoot& c);
  DocRoot& operator=(const DocRoot& v);
  // Prohibit copy

  // Check HTTP authorization for this request.
  // Returns true if it passes.
  // Returns false if it fails, in which case the response headers are  set 
  // appropriately to request client authentication.
  bool check_auth(Request& request, 
		  Response& response);
  
private:

  HTTPModule& m_module;
  Host& m_host;
  std::string m_profile;
  
  scx::FilePath m_path;

  typedef std::map<std::string,StreamMap*> PatternMap;
  PatternMap m_extn_mods;
  PatternMap m_path_mods;

  typedef std::map<std::string,std::string> RealmMap;
  RealmMap m_realm_maps;

  scx::ScriptMap::Ref m_params;
};

};
#endif
