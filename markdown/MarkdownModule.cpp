/* SconeServer (http://www.sconemad.com)

Sconesite Markdown module

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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

#include "MarkdownModule.h"
#include "MarkdownDoc.h"
#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/FilePath.h>
#include <sconex/FileStat.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Log.h>
#include <cmark.h>

#include <cmark.h>

SCONEX_MODULE(MarkdownModule);

#define LOG(msg) scx::Log("markdown").submit(msg);

//=========================================================================
MarkdownModule::MarkdownModule()
  : scx::Module("markdown",scx::version())
{
  scs::Document::register_document_type("md",this);
}

//=========================================================================
MarkdownModule::~MarkdownModule()
{
  scs::Document::unregister_document_type("md",this);
}

//=========================================================================
std::string MarkdownModule::info() const
{
  std::ostringstream oss;
  oss << "Markdown document handler\n"
      << "Using cmark version " << cmark_version_string()
      << " (commonmark.org)";
  return oss.str();
}

//=========================================================================
int MarkdownModule::init()
{
  return Module::init();
}

//=============================================================================
scx::ScriptRef* MarkdownModule::script_op(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const scx::ScriptOp& op,
				       const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("test" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* MarkdownModule::script_method(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const std::string& name,
					   const scx::ScriptRef* args)
{
  if (name == "test") {

  }

  return scx::Module::script_method(auth,ref,name,args);
}

//=========================================================================
void MarkdownModule::provide(const std::string& type,
                             const scx::ScriptRef* args,
                             scs::Document*& object)
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
  
  if ("md" == type) {
    object = new MarkdownDoc(name->get_string(),
                             root->get_string(), 
                             file->get_string());
  }
}
