/* SconeServer (http://www.sconemad.com)

http Request

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

#ifndef httpRequest_h
#define httpRequest_h

#include "http/DocRoot.h"
#include "sconex/ArgObject.h"
#include "sconex/VersionTag.h"
#include "sconex/Uri.h"
#include "sconex/MimeHeader.h"
namespace http {

class Session;

//=============================================================================
class HTTP_API Request : public scx::ArgObjectInterface {

public:

  Request(const std::string& profile);  
  virtual ~Request();

  const std::string& get_method() const;
  const scx::Uri& get_uri() const;
  const scx::VersionTag& get_version() const;
  bool parse_request(const std::string& str, bool secure);

  std::string get_header(const std::string& name) const;
  scx::MimeHeader get_header_parsed(const std::string& name) const;
  bool parse_header(const std::string& str);


  void set_host(Host* host);
  const Host* get_host() const;

  const std::string& get_profile() const;

  void set_docroot(DocRoot* docroot);
  const DocRoot* get_docroot() const;

  void set_session(Session* session);
  const Session* get_session() const;
  Session* get_session();

  void set_path(const scx::FilePath& path);
  const scx::FilePath& get_path() const;

  void set_auth_user(const std::string& user);
  const std::string& get_auth_user() const;

  void set_path_info(const std::string& pathinfo);
  const std::string& get_path_info() const;  
  
  void set_param(const std::string& name, scx::Arg* value);
  void set_param(const std::string& name, const std::string& value);
  std::string get_param(const std::string& name) const;
  bool is_param(const std::string& name) const;

  // ArgObject interface
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);
  
private:

  // ----
  // The following data comes directly from the HTTP Message, which looks like:
  // <METHOD> <URI> HTTP/<VERSION><CR><LF>
  // <HEADER-NAME>: <HEADER-VALUE><CR><LF>
  // <HEADER-NAME>: <HEADER-VALUE><CR><LF>
  // ...
  // <CR><LF>
  //

  std::string m_method;
  // The request method (GET|POST|HEAD)

  scx::Uri m_uri;
  // The uniform resource indicator being requested

  scx::VersionTag m_version;
  // The HTTP version being used by the client
  
  scx::MimeHeaderTable m_headers;
  // Table containing request headers


  // ----
  // Decoded data:
  
  Host* m_host;
  // The HTTP Host

  std::string m_profile;
  // The profile

  DocRoot* m_docroot;
  // The document root corresponding to the above profile

  Session* m_session;
  // The HTTP session (if applicable)

  scx::FilePath m_path;
  // Local file corresponding to the remote URI (if applicable)
  
  std::string m_auth_user;
  // Username authenticated using HTTP authentication (empty if not authenticated)

  std::string m_pathinfo;
  // Extra path information following path of invoked object
    
  scx::ArgMap m_params;
  // Parameters sent with the request
  
};

};
#endif
