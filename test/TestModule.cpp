/* SconeServer (http://www.sconemad.com)

Test module

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Arg.h"
#include "sconex/FilePath.h"
#include "sconex/FileDir.h"
#include "sconex/FileStat.h"

SCONESERVER_MODULE(TestModule);

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
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
         "Test module\n";
}

//=============================================================================
scx::Arg* TestModule::arg_lookup(const std::string& name)
{
  // Methods
  if ("filedir" == name) {
    return new scx::ArgObjectFunction(new scx::ArgModule(ref()),name);
  }      

  if ("counts" == name) {
    // Output instance counters
    std::ostringstream oss;
#ifdef _DEBUG
    static std::map<std::string,scx::DebugInstanceCounter> prev_counters;
      
    oss << "Instance counts:\n"
        << "CLASS NAME                     TOTAL    CURRENT  DELTA\n";
    const std::map<std::string,scx::DebugInstanceCounter>& counters =
      scx::Debug::get()->get_counters();
    std::map<std::string,scx::DebugInstanceCounter>::const_iterator it = counters.begin();
    while (it != counters.end()) {
      const scx::DebugInstanceCounter& counter = (*it).second;
      const std::string& class_name = (*it).first;
      int stat_max = counter.get_max();
      int stat_num = counter.get_num();
      int stat_delta = stat_num - prev_counters[class_name].get_num();

      oss << std::setw(30) << class_name << " "
          << std::setw(8) << stat_max << " "
          << std::setw(8) << stat_num << " "
          << std::setw(8) << stat_delta << "\n";
      
      it++;
    }
    prev_counters = counters;
#else
    oss << "Instance counts are only available in debug builds\n";
#endif
    return new scx::ArgString(oss.str());
  }
  
  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* TestModule::arg_function(
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);
  
  if ("filedir" == name) {
    // Test for filedir
    const scx::ArgString* a_arg1 =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_arg1) {
      return new scx::ArgError("test::run() File name must be specified");
    }
    std::string str1 = a_arg1->get_string();

    std::ostringstream oss;
    oss << "Listing:\n";
    scx::FileDir dir(str1);
    while (dir.next()) {
      oss << dir.path().path();
      if (dir.stat().is_dir()) {
        oss << "/";
      } else {
        oss << " (" << dir.stat().size() << " bytes)";
      }
      oss << "\n";
    }
    return new scx::ArgString(oss.str());
    
  }

  return SCXBASE Module::arg_function(name,args);
}
