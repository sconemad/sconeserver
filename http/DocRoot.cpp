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


#include <http/DocRoot.h>
#include <http/Host.h>
#include <http/MessageStream.h>
#include <http/Request.h>
#include <http/AuthRealm.h>
#include <sconex/Uri.h>
#include <sconex/Base64.h>
#include <sconex/utils.h>
#include <sconex/Date.h>
#include <sconex/FileStat.h>
#include <sconex/Log.h>
namespace http {

#define LOGGER() scx::Log("http.hosts") \
  .attach("id", m_host.get_id() + "." + m_profile)
#define LOG(msg) LOGGER().submit(msg);

//=========================================================================
StreamMap::StreamMap(
  const std::string& type,
  scx::ScriptRef* args
) : m_type(type),
    m_args(args)
{

}
  
//=========================================================================
StreamMap::~StreamMap()
{
  delete m_args;
}

//=========================================================================
const std::string& StreamMap::get_type() const
{
  return m_type;
}
  
//=========================================================================
bool StreamMap::connect(scx::Descriptor* d)
{
  scx::Stream* stream = scx::Stream::create_new(m_type,m_args);
  if (!stream) {
    DEBUG_LOG("Failed to create stream of type " << m_type <<
	      " args " << m_args->object()->get_string());
    return false;
  }

  d->add_stream(stream);
  return true;
}


  
//=========================================================================
DocRoot::DocRoot(
  HTTPModule& module,
  Host& host,
  const std::string profile,
  const scx::FilePath& path
) : m_module(module),
    m_host(host),
    m_profile(profile),
    m_path(path),
    m_params(new scx::ScriptMap())
{
  m_parent = &host;

  // A bit of a hack really, but set up default module mappings so it will
  // do something sensible if there is no host config file present.
  add_extn_map(".","dirindex",0);
  add_extn_map("!","errorpage",0);
  add_extn_map("*","getfile",0);
}

//=========================================================================
DocRoot::~DocRoot()
{
  PatternMap::iterator it; 
  for (it = m_extn_mods.begin(); it != m_extn_mods.end(); ++it) {
    delete it->second;
  }
  for (it = m_path_mods.begin(); it != m_path_mods.end();  ++it) {
    delete it->second;
  }
}

//=========================================================================
bool DocRoot::connect_request(scx::Descriptor* endpoint,
			      Request& request,
			      Response& response)
{
  StreamMap* strmap = 0;

  if (response.get_status().code() != http::Status::Ok) {
    strmap = lookup_extn_map("!");
    
  } else {
    const scx::Uri& uri = request.get_uri();
    const std::string& uripath = scx::Uri::decode(uri.get_path());

    // Check valid path
    if (!check_path(uripath)) {
      response.set_status(http::Status::Forbidden);
      return false;
    }
    
    // Check http authorization
    if (!check_auth(request,response)) {
      response.set_status(http::Status::Unauthorized);
      return false;
    }

    scx::FilePath path = m_path + uripath;
    request.set_path(path);

    std::string pathinfo;
    strmap = lookup_path_map(uripath,pathinfo);
    if (strmap) {
      // Path mapped module
      request.set_path_info(pathinfo);
    } else {
      // Normal file mapping
      scx::FileStat stat(path);
      if (!stat.exists()) {
        response.set_status(http::Status::NotFound);
        return false;
      } else if (stat.is_dir()) {
        strmap = lookup_extn_map(".");
      } else {
        strmap = lookup_extn_map(uripath);
      }
    }

    check_session(request,response);
  }

  // Check we have a stream map
  if (strmap == 0) {
    LOG("No stream map found to handle request");
    response.set_status(http::Status::ServiceUnavailable);
    return false;
  }

  // Connect the stream
  if (!strmap->connect(endpoint)) {
    LOG("Failed to connect stream to handle request");
    response.set_status(http::Status::ServiceUnavailable);
    return false;
  }

  // Success
  return true;
}

//=========================================================================
void DocRoot::add_extn_map(const std::string& pattern,
                           const std::string& stream,
                           scx::ScriptRef* args)
{
  LOG("Mapping extension '" + pattern + 
      "' to stream type " + stream +
      " " + (args ? args->object()->get_string() : "NULL"));

  PatternMap::iterator it = m_extn_mods.find(pattern);
  if (it != m_extn_mods.end()) delete it->second;
  m_extn_mods[pattern] = new StreamMap(stream,args);
}

//=========================================================================
StreamMap* DocRoot::lookup_extn_map(const std::string& name) const
{
  int bailout=100;
  std::string::size_type idot;
  std::string key=name;
  while (--bailout > 0) {

    PatternMap::const_iterator it = m_extn_mods.find(key);
    if (it != m_extn_mods.end()) {
      return it->second;
    }
    
    if (key.size()<=0 || key=="*") {
      return 0;
    }

    if (key[0]=='*') {
      idot = key.find(".",2);
    } else {
      idot = key.find_first_of(".");
    }
    
    if (idot==key.npos) {
      key="*";
    } else {
      key = "*" + key.substr(idot);
    }
    
  }
  DEBUG_LOG("lookup_extn_map() Pattern match bailout");
  return 0; // Bailed out
}

//=========================================================================
void DocRoot::add_path_map(const std::string& pattern,
                           const std::string& stream,
                           scx::ScriptRef* args)
{
  LOG("Mapping path '" + pattern + 
      "' to stream type " + stream +
      " " + (args ? args->object()->get_string() : "NULL"));
  
  PatternMap::iterator it = m_path_mods.find(pattern);
  if (it != m_path_mods.end()) delete it->second;
  m_path_mods[pattern] = new StreamMap(stream,args);
}

//=========================================================================
StreamMap* DocRoot::lookup_path_map(const std::string& name,
				    std::string& pathinfo) const
{
  std::string key="/"+name;
  pathinfo = "";

  for (PatternMap::const_iterator it = m_path_mods.begin();
       it != m_path_mods.end();
       ++it) {
    DEBUG_LOG("lookup_path_map "<<key<<" matching "<<it->first);
    if (key.find(it->first) == 0) {
      pathinfo = key.substr(it->first.length());
      DEBUG_LOG("  found "<<it->second->get_type());
      return it->second;
    }
  }
  return 0;
}

//=========================================================================
void DocRoot::add_realm_map(const std::string& pattern,
                            const std::string& realm)
{
  LOG("Mapping path '" + pattern + "' to realm " + realm);
  m_realm_maps[pattern] = realm;
}

//=========================================================================
std::string DocRoot::lookup_realm_map(const std::string& name) const
{
  std::string key="/"+name;

  for (RealmMap::const_iterator it = m_realm_maps.begin();
       it != m_realm_maps.end();
       ++it) {
    if (key.find(it->first) == 0) {
      return it->second;
    }
  }
  return "";
}

//=========================================================================
const std::string DocRoot::get_profile() const
{
  return m_profile;
}

//=============================================================================
const scx::ScriptRef* DocRoot::get_param(const std::string& name) const
{
  return m_params.object()->lookup(name);
}

//=============================================================================
void DocRoot::set_param(const std::string& name, scx::ScriptRef* value)
{
  m_params.object()->give(name,value);
}

//=========================================================================
std::string DocRoot::get_string() const
{
  return get_profile();
}

//=============================================================================
scx::ScriptRef* DocRoot::script_op(const scx::ScriptAuth& auth,
				   const scx::ScriptRef& ref,
				   const scx::ScriptOp& op,
				   const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("map" == name ||
	"map_path" == name ||
	"add_realm" == name ||
	"map_realm" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Sub-objects
    if ("profile" == name) return scx::ScriptString::new_ref(m_profile);
    if ("path" == name) return scx::ScriptString::new_ref(m_path.path());
    if ("params" == name) return m_params.ref_copy(ref.reftype());
  }
  
  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* DocRoot::script_method(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const std::string& name,
				       const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("map" == name) {
    const scx::ScriptString* a_pattern =
      scx::get_method_arg<scx::ScriptString>(args,0,"pattern");
    if (!a_pattern) {
      return scx::ScriptError::new_ref("map() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ScriptString* a_stream =
      scx::get_method_arg<scx::ScriptString>(args,1,"stream");
    if (!a_stream) {
      return scx::ScriptError::new_ref("No stream type specified");
    }
    std::string s_stream = a_stream->get_string();

    // Transfer the remaining arguments to a new list, to be stored with
    // the module map.
    scx::ScriptList::Ref* ml = new scx::ScriptList::Ref(new scx::ScriptList());
    int pos=2;
    const scx::ScriptRef* arg = 0;
    while (0 != (arg = scx::get_method_arg_ref(args,pos++))) {
      ml->object()->give(arg->ref_copy());
    }

    add_extn_map(s_pattern,s_stream,ml);
    return 0;
  }

  if ("map_path" == name) {
    const scx::ScriptString* a_pattern =
      scx::get_method_arg<scx::ScriptString>(args,0,"pattern");
    if (!a_pattern) {
      return scx::ScriptError::new_ref("No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ScriptString* a_stream =
      scx::get_method_arg<scx::ScriptString>(args,1,"stream");
    if (!a_stream) {
      return scx::ScriptError::new_ref("No stream type specified");
    }
    std::string s_stream = a_stream->get_string();

    // Transfer the remaining arguments to a new list, to be stored with
    // the module map.
    scx::ScriptList::Ref* ml = new scx::ScriptList::Ref(new scx::ScriptList());
    int pos=2;
    const scx::ScriptRef* arg = 0;
    while (0 != (arg = scx::get_method_arg_ref(args,pos++))) {
      ml->object()->give(arg->ref_copy());
    }

    add_path_map(s_pattern,s_stream,ml);
    return 0;
  }

  if ("map_realm" == name) {
    const scx::ScriptString* a_pattern =
      scx::get_method_arg<scx::ScriptString>(args,0,"pattern");
    if (!a_pattern) {
      return scx::ScriptError::new_ref("map_realm() No pattern specified");
    }
    std::string s_pattern = a_pattern->get_string();

    const scx::ScriptString* a_realm =
      scx::get_method_arg<scx::ScriptString>(args,1,"realm");
    if (!a_realm) {
      return scx::ScriptError::new_ref("map_realm() No realm specified");
    }
    std::string s_realm = a_realm->get_string();

    add_realm_map(s_pattern,s_realm);
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
bool DocRoot::check_path(const std::string& uripath)
{
  if (uripath.length() > 1 && uripath[0] == '/') {
    LOG("Request uri starts with /");
    return false;
  }

  if (uripath.find("..") != std::string::npos) {
    LOG("Request uri contains ..");
    return false;
  }
 
  return true;
}

//=============================================================================
bool DocRoot::check_auth(Request& request, Response& response)
{
  // Check auth
  const scx::Uri& uri = request.get_uri();
  const std::string& uripath = uri.get_path();
  
  std::string realm_name = lookup_realm_map(uripath);
  AuthRealm* realm = m_module.get_realms().lookup_realm(realm_name);
  if (realm) {
    std::string auth = request.get_header("AUTHORIZATION");
    std::string user;
    std::string pass;
    if (!auth.empty()) {
      std::string::size_type is = auth.find(" ");
      std::string method;
      std::string enc;
      if (is != std::string::npos) {
        method = auth.substr(0,is);
        scx::strup(method);
        enc = auth.substr(is+1);
      }
      
      if (method == "BASIC") {
        std::istringstream auth64in(enc);
        std::ostringstream auth64out;
        scx::Base64::decode(auth64in,auth64out);
        auth = auth64out.str();
        std::string::size_type ic = auth.find(":");
        if (ic != std::string::npos) {
          user = auth.substr(0,ic);
          pass = auth.substr(ic+1);
        }
      }
    }
    scx::ScriptRef* result = realm->authenticate(user,pass);
    bool passed = !BAD_SCRIPTREF(result);
    delete result;
    if (passed) {
      LOG("Auth passed for realm '" + realm_name + "' user='" + user + "'");
      request.set_auth_user(user);
    } else {
      LOG("Auth failed for realm '" + realm_name + "' user='" + user + "'");
      response.set_header("WWW-AUTHENTICATE",
                          "Basic realm=\"" + realm_name + "\"");
      return false;
    }
  }
  
  return true;
}

//=============================================================================
void DocRoot::check_session(Request& request, Response& response)
{
  if (request.get_session()) return; // Already set
  
  // Check for session cookie
  std::string cookie = request.get_header("Cookie");
  std::string scxid;
  if (!cookie.empty()) {
    const std::string pattern = "scxid=";
    std::string::size_type start = cookie.find(pattern);
    if (start != std::string::npos) {
      start += pattern.length();
      std::string::size_type end = cookie.find_first_of(" ;",start);
      if (end != std::string::npos) {
        scxid = cookie.substr(start,end-start);
      } else {
        scxid = cookie.substr(start);
      }
    }
  }
  
  const scx::ScriptRef* a_auto_session = get_param("auto_session");
  bool b_auto_session = 
    (a_auto_session && a_auto_session->object()->get_int());
  
  // Lookup the session
  Session::Ref* s = m_module.get_sessions().lookup_session(scxid);
  if (s != 0) {
    // Existing session
    LOGGER().attach("session",scxid).submit("Existing session");
    s->object()->set_last_used();
    
  } else {
    if (!scxid.empty()) {
      // Timed-out session
    }
    
    // Create new session if auto_session enabled
    if (b_auto_session) {
      s = m_module.get_sessions().new_session();
      scxid = s->object()->get_id();
      LOGGER().attach("session",scxid).submit("New session");
    }
  }
  
  if (s) {
    // Update cookie
    std::string cookie = "scxid=" + scxid +
      "; expires=" + s->object()->get_expiry().string() +
      "; path=/";
    response.set_header("Set-Cookie",cookie);
    request.give_session(s);
  }
}
 
};
