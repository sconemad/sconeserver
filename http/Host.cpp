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


#include <http/Host.h>
#include <http/HostMapper.h>
#include <http/DocRoot.h>
#include <http/MessageStream.h>
#include <http/Request.h>
#include <http/AuthRealm.h>
#include <http/Session.h>
#include <sconex/ConfigFile.h>
#include <sconex/Log.h>
namespace http {

#define LOG(msg) scx::Log("http.hosts").attach("id", m_id).submit(msg);

//=========================================================================
Host::Host(
  HTTPModule& module,
  HostMapper& mapper,
  const std::string id,
  const std::string hostname,
  const scx::FilePath& dir
) : m_module(module),
    m_mapper(mapper),
    m_id(id),
    m_hostname(hostname),
    m_dir(dir)
{
  m_parent = &mapper;
}

//=========================================================================
Host::~Host()
{
  for (DocRootMap::const_iterator it = m_docroots.begin();
       it != m_docroots.end();
       ++it) {
    delete it->second;
  }
}

//=========================================================================
int Host::init()
{
  scx::ConfigFile config(m_dir + "host.conf");
  scx::ScriptRef* ctx = new scx::ScriptRef(this);
  int err = config.load(ctx);
  return err;
}

//=========================================================================
bool Host::connect_request(scx::Descriptor* endpoint,
			   Request& request,
			   Response& response)
{
  std::string profile = request.get_profile();
  DocRoot* docroot = get_docroot(profile);
  if (docroot == 0) {
    // Unknown profile
    LOG("Unknown profile '" + profile + "' for host '" + m_hostname + "'");
    response.set_status(http::Status::NotFound);
    return false;
  }

  request.set_docroot(docroot);
  return docroot->connect_request(endpoint,request,response);
}

//=========================================================================
const std::string Host::get_id() const
{
  return m_id;
}

//=========================================================================
const std::string Host::get_hostname() const
{
  return m_hostname;
}

//=========================================================================
const scx::FilePath& Host::get_path() const
{
  return m_dir;
}

//=========================================================================
DocRoot* Host::get_docroot(const std::string& profile)
{
  DocRootMap::const_iterator it = m_docroots.find(profile);
  if (it != m_docroots.end()) {
    return it->second->object();
  }

  return 0;
}

//=========================================================================
std::string Host::get_string() const
{
  return get_id();
}

//=============================================================================
scx::ScriptRef* Host::script_op(const scx::ScriptAuth& auth,
				const scx::ScriptRef& ref,
				const scx::ScriptOp& op,
				const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
	"remove" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    
    if ("id" == name) 
      return scx::ScriptString::new_ref(m_id);
  
    if ("hostname" == name) 
      return scx::ScriptString::new_ref(m_hostname);
    
    if ("path" == name) 
      return scx::ScriptString::new_ref(m_dir.path());
    
    if ("docroots" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      scx::ScriptRef* list_ref = new scx::ScriptRef(list);
      for (DocRootMap::const_iterator it = m_docroots.begin();
	   it != m_docroots.end();
	   ++it) {
	DocRoot::Ref* docroot = it->second;
	list->give(docroot->ref_copy(ref.reftype()));
      }
      return list_ref;
    }

    // Sub-objects
    
    DocRootMap::const_iterator it = m_docroots.find(name);
    if (it != m_docroots.end()) {
      DocRoot::Ref* r = it->second;
      return r->ref_copy(ref.reftype());
    }      
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* Host::script_method(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const std::string& name,
				    const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("add" == name) {
    const scx::ScriptString* a_profile =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_profile) {
      return scx::ScriptError::new_ref("add() No profile name specified");
    }
    std::string s_profile = a_profile->get_string();
    
    const scx::ScriptString* a_path =
      scx::get_method_arg<scx::ScriptString>(args,1,"path");
    if (!a_path) {
      return scx::ScriptError::new_ref("add() No path specified");
    }

    DocRootMap::const_iterator it = m_docroots.find(s_profile);
    if (it != m_docroots.end()) {
      return scx::ScriptError::new_ref("add() Profile already exists");
    }
        
    scx::FilePath path = m_dir + a_path->get_string();
    LOG("Adding profile '" + s_profile + "' dir '" + path.path() + "'");
    DocRoot* profile = new DocRoot(m_module,*this,s_profile,path.path());
    m_docroots[s_profile] = new DocRoot::Ref(profile);
    return new DocRoot::Ref(profile);
  }

  if ("remove" == name) {
    const scx::ScriptString* a_profile =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_profile) {
      return scx::ScriptError::new_ref("remove() No profile name specified");
    }
    std::string s_profile = a_profile->get_string();

    DocRootMap::iterator it = m_docroots.find(s_profile);
    if (it == m_docroots.end()) {
      return scx::ScriptError::new_ref("remove() Profile not found");
    }
        
    LOG("Removing profile '" + s_profile + "'");
    delete (*it).second;
    m_docroots.erase(it);
    return 0;
  }
    
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

};
