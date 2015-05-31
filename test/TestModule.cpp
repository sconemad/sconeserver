/* SconeServer (http://www.sconemad.com)

Test module

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

#include "TestModule.h"

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/FilePath.h>
#include <sconex/FileDir.h>
#include <sconex/FileStat.h>
#include <sconex/Database.h>

SCONEX_MODULE(TestModule);

//=========================================================================
TestModule::TestModule(
)
  : scx::Module("test",scx::version())
{

}

//=========================================================================
TestModule::~TestModule()
{

}

//=========================================================================
std::string TestModule::info() const
{
  return "Test module";
}

//=============================================================================
scx::ScriptRef* TestModule::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("filedir" == name ||
	"db" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }      
    
    if ("counts" == name) {
      // Output instance counters
      std::ostringstream oss;
#ifdef _DEBUG
      static scx::Debug::InstanceCounterMap prev_counters;
      
      oss << "Instance counts:\n"
	  << "CLASS NAME                        TOTAL  CURRENT    DELTA   ALLOCS\n";
      scx::Debug::InstanceCounterMap counters;
      scx::Debug::get()->get_counters(counters);
      scx::Debug::InstanceCounterMap::iterator it = counters.begin();
      while (it != counters.end()) {
	const std::string& class_name = it->first;
	scx::DebugInstanceCounter& counter = it->second;
	int stat_max = counter.get_max();
	int stat_num = counter.get_num();
	int stat_delta = stat_num - prev_counters[class_name].get_num();
	
	oss << std::setw(30) << class_name << " "
	    << std::setw(8) << stat_max << " "
	    << std::setw(8) << stat_num << " "
	    << std::setw(8) << stat_delta << " | "
	    << counter.get_deltas() << "\n";
	
	it++;
      }
      prev_counters = counters;
      scx::Debug::get()->reset_counters();
#else
      oss << "Instance counts are only available in debug builds\n";
#endif
      return scx::ScriptString::new_ref(oss.str());
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* TestModule::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  const scx::ScriptList* argl = 
    dynamic_cast<const scx::ScriptList*>(args->object());
  
  if ("filedir" == name) {
    // Test for filedir
    const scx::ScriptString* path =
      scx::get_method_arg<scx::ScriptString>(args,0,"path");
    if (!path)
      return scx::ScriptError::new_ref("test::run() File name must be specified");
    std::ostringstream oss;
    oss << "Listing:\n";
    scx::FileDir dir(scx::FilePath(path->get_string()));
    while (dir.next()) {
      oss << dir.path().path();
      if (dir.stat().is_dir()) {
        oss << "/";
      } else {
        oss << " (" << dir.stat().size() << " bytes)";
      }
      oss << "\n";
    }
    return scx::ScriptString::new_ref(oss.str());
  }

  if ("db" == name) {
    const scx::ScriptString* type = 
      scx::get_method_arg<scx::ScriptString>(args,0,"type");
    if (!type) return 0;
    return scx::Database::open(type->get_string(),argl->get(1));
  }
  
  return scx::Module::script_method(auth,ref,name,args);
}
