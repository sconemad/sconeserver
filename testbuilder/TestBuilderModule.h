/* SconeServer (http://www.sconemad.com)

Test Builder Module

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef testbuilderModule_h
#define testbuilderModule_h

#include "sconex/Module.h"
#include "sconex/Thread.h"

class BuildProfile;

//#########################################################################
class TestBuilderModule : public scx::Module, public scx::Thread {

public:

  TestBuilderModule();
  virtual ~TestBuilderModule();

  virtual std::string info() const;
  
  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

  virtual void* run();
  
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);

protected:
  
private:

  std::map<std::string,BuildProfile*> m_profiles;
  // Test build profiles
  
};

#endif

