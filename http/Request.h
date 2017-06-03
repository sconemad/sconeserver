/* SconeServer (http://www.sconemad.com)

HTTP Request

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

#ifndef httpRequest_h
#define httpRequest_h

#include <http/Session.h>
#include <sconex/ScriptBase.h>
#include <sconex/VersionTag.h>
#include <sconex/Uri.h>
#include <sconex/MimeHeader.h>
#include <sconex/FilePath.h>
namespace http {

class Host;

//=============================================================================
// Request - Represents a request message from an HTTP client.
//
class HTTP_API Request : public scx::ScriptObject {
public:

  Request(const std::string& id);
  virtual ~Request();

  void set_method(const std::string& method);
  const std::string& get_method() const;

  void set_uri(const scx::Uri& uri);
  const scx::Uri& get_uri() const;
  bool is_secure() const;
  
  void set_version(const scx::VersionTag& ver);
  const scx::VersionTag& get_version() const;

  bool parse_request(const std::string& str, bool secure);

  void set_header(const std::string& name, const std::string& value);
  bool remove_header(const std::string& name);
  std::string get_header(const std::string& name) const;
  scx::MimeHeader get_header_parsed(const std::string& name) const;
  bool parse_header(const std::string& str);

  void set_host(Host* host);
  const Host* get_host() const;

  const std::string& get_id() const;

  void give_session(Session::Ref* session);
  const Session* get_session() const;
  Session* get_session();

  void set_path(const scx::FilePath& path);
  const scx::FilePath& get_path() const;

  void set_auth_user(const std::string& user);
  const std::string& get_auth_user() const;

  void set_path_info(const std::string& pathinfo);
  const std::string& get_path_info() const;  
  
  void set_param(const std::string& name, scx::ScriptRef* value);
  void set_param(const std::string& name, const std::string& value);
  std::string get_param(const std::string& name) const;
  bool is_param(const std::string& name) const;

  std::string build_header_string();

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

  typedef scx::ScriptRefTo<Request> Ref;
  
private:

  // ----
  // The following data comes directly from the HTTP Message, which looks like:
  // <METHOD> <URI> HTTP/<VERSION><CR><LF>
  // <HEADER-NAME>: <HEADER-VALUE><CR><LF>
  // <HEADER-NAME>: <HEADER-VALUE><CR><LF>
  // ...
  // <CR><LF>
  //

  // The request method (GET|POST|HEAD)
  std::string m_method;

  // The uniform resource indicator being requested
  scx::Uri m_uri;

  // The HTTP version being used by the client
  scx::VersionTag m_version;
  
  // Table containing request headers
  scx::MimeHeaderTable m_headers;


  // ----
  // Decoded data:
  
  // The HTTP Host
  Host* m_host;

  // Unique identifier for this request
  std::string m_id;
  
  // The HTTP session (if applicable)
  Session::Ref* m_session;

  // Local file corresponding to the remote URI (if applicable)
  scx::FilePath m_path;
  
  // Username authenticated using HTTP authentication
  // (empty if not authenticated)
  std::string m_auth_user;

  // Extra path information following path of invoked object
  std::string m_pathinfo;
    
  // Parameters sent with the request
  scx::ScriptMap::Ref m_params;
  
};

};
#endif
