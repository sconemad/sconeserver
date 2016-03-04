/* SconeServer (http://www.sconemad.com)

Sconesite Module

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

#include <sconesite/SconesiteModule.h>
#include <sconesite/SconesiteStream.h>
#include <sconesite/Profile.h>
#include <sconesite/XMLDoc.h>
#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Stream.h>
#include <sconex/Kernel.h>
#include <sconex/File.h>
#include <sconex/Log.h>
namespace scs {
  
SCONEX_MODULE(SconesiteModule);

#define SCONESITE_JOB_PERIOD 7

#define LOG(msg) scx::Log("sconesite").submit(msg);

//=========================================================================
class SconesiteJob : public scx::PeriodicJob {
public:

  SconesiteJob(SconesiteModule* module, 
	       const scx::Time& period)
    : scx::PeriodicJob("sconesite Refresh",period),
      m_module(module) {};

  virtual bool run()
  {
    m_module.object()->refresh();
    return false;
  };

protected:
  SconesiteModule::Ref m_module;
};

//=========================================================================
SconesiteModule::SconesiteModule()
  : scx::Module("sconesite",scx::version())
{
  scx::Stream::register_stream(name(),this);
  Document::register_document_type("xml",this);

  m_job = scx::Kernel::get()->add_job(
    new SconesiteJob(this,scx::Time(SCONESITE_JOB_PERIOD)));
}

//=========================================================================
SconesiteModule::~SconesiteModule()
{
  scx::Stream::unregister_stream(name(),this);
  Document::unregister_document_type("xml",this);
}

//=========================================================================
std::string SconesiteModule::info() const
{
  return "Sconesite content management system";
}

//=========================================================================
bool SconesiteModule::close()
{
  if (!scx::Module::close()) return false;

  scx::Kernel::get()->end_job(m_job);

  for (ProfileMap::const_iterator it = m_profiles.begin();
       it != m_profiles.end();
       ++it) {
    delete it->second;
  }
  m_profiles.clear();
  return true;
}

//=========================================================================
void SconesiteModule::provide(const std::string& type,
			      const scx::ScriptRef* args,
			      scx::Stream*& object)
{
  const scx::ScriptString* profile_name =
    scx::get_method_arg<scx::ScriptString>(args,0,"profile");
  if (!profile_name) {
    LOG("No profile specified, aborting connection");
  }

  Profile* profile = lookup_profile(profile_name->get_string());
  if (!profile) {
    LOG("Unknown profile '"+profile_name->get_string()+
	"' specified, aborting connection");
  }
  
  object = new SconesiteStream(this,*profile);
}

//=========================================================================
void SconesiteModule::provide(const std::string& type,
			      const scx::ScriptRef* args,
			      Document*& object)
{
  const scx::ScriptString* name =
    scx::get_method_arg<scx::ScriptString>(args,0,"name");
  if (!name) {
    LOG("No document name specified");
    return;
  }  

  const scx::ScriptString* root =
    scx::get_method_arg<scx::ScriptString>(args,1,"root");
  if (!root) {
    LOG("No document root specified");
    return;
  }  

  const scx::ScriptString* file =
    scx::get_method_arg<scx::ScriptString>(args,2,"file");
  if (!file) {
    LOG("No document file specified");
    return;
  }  
  
  if ("xml" == type) {
    object = new XMLDoc(name->get_string(),
			root->get_string(), 
			file->get_string());

    // If the article file doesn't exist, create a default one.
    if (!scx::FileStat(object->get_filepath()).exists()) {
      scx::File file;
      if (scx::Ok != file.open(object->get_filepath(),
                               scx::File::Write|scx::File::Create,00660)) {
        DEBUG_LOG_ERRNO("Unable to create new article '" <<
                        object->get_filepath().path() << "'");
        delete object; object = 0;
      }
      file.write("<article>\n");
      file.write("\n<p>\nwrite something here...\n</p>\n\n");
      file.write("</article>\n");
      file.close();
    }
  }
}

//=========================================================================
void SconesiteModule::refresh()
{
  m_templates.refresh();
  for (ProfileMap::iterator it = m_profiles.begin();
       it != m_profiles.end();
       ++it) {
    Profile* profile = it->second->object();
    profile->refresh();
  }
}

//=========================================================================
Profile* SconesiteModule::lookup_profile(const std::string& profile)
{
  ProfileMap::const_iterator it = m_profiles.find(profile);
  if (it != m_profiles.end()) {
    return it->second->object();
  }
  return 0;
}

//=============================================================================
Template* SconesiteModule::lookup_template(const std::string& name)
{
  return m_templates.lookup(name);
}
  
//=============================================================================
scx::ScriptRef* SconesiteModule::script_op(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const scx::ScriptOp& op,
					   const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
        "add_templates" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("document_types" == name) {
      scx::ScriptList::Ref* list = 
	new scx::ScriptList::Ref(new scx::ScriptList());
      const scx::ProviderScheme<Document>* providers =
        Document::get_document_providers();
      scx::ProviderScheme<Document>::ProviderMap::const_iterator it;
      for (it = providers->providers().begin();
           it != providers->providers().end(); ++it) {
        list->object()->give(scx::ScriptString::new_ref(it->first));
      }
      return list;
    }
    
    // Sub-objects
    ProfileMap::const_iterator it = m_profiles.find(name);
    if (it != m_profiles.end()) {
      return it->second->ref_copy(ref.reftype());
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SconesiteModule::script_method(const scx::ScriptAuth& auth,
					       const scx::ScriptRef& ref,
					       const std::string& name,
					       const scx::ScriptRef* args)
{
  if ("add" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_profile =
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_profile) {
      return scx::ScriptError::new_ref("No profile name specified");
    }
    std::string s_profile = a_profile->get_string();

    // There must be a corresponding http Host with the same name as the
    // sconesite profile.
    scx::Module::Ref httpmod = scx::Kernel::get()->get_module("http");
    http::HTTPModule* http = dynamic_cast<http::HTTPModule*>(httpmod.object());
    http::Host* host = http ? http->get_hosts().lookup_host(s_profile) : 0;
    if (!host)
      return scx::ScriptError::new_ref("No http host for this profile");

    ProfileMap::const_iterator it = m_profiles.find(s_profile);
    if (it != m_profiles.end())
      return scx::ScriptError::new_ref("Profile already exists");

    scx::Database* a_db =
      const_cast<scx::Database*>(
        scx::get_method_arg<scx::Database>(args,1,"db"));
    if (!a_db)
      return scx::ScriptError::new_ref("No database specified");
    
    LOG("Adding profile '" + s_profile);
    Profile* profile = new Profile(*this,s_profile,host,a_db);
    m_profiles[s_profile] = new Profile::Ref(profile);

    return new Profile::Ref(profile);
  }

  if ("add_templates" == name) {
    if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_path =
      scx::get_method_arg<scx::ScriptString>(args,0,"path");
    if (!a_path) {
      return scx::ScriptError::new_ref("No path specified");
    }
    m_templates.add(a_path->get_string());

    return 0;
  }
  
  return scx::Module::script_method(auth,ref,name,args);
}

};
