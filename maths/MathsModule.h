/* SconeServer (http://www.sconemad.com)

Maths module

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

#ifndef mathsMathsModule_h
#define mathsMathsModule_h

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/ScriptExpr.h>

//=========================================================================
class MathsModule : public scx::Module,
		    public scx::Provider<scx::ScriptObject> {
public:

  MathsModule();
  virtual ~MathsModule();

  virtual std::string info() const;

  virtual int init();
  virtual bool close();
  
  unsigned int get_sf() const;

  // ScriptObject methods  
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // Provider<ScriptObject> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object);

private:
  unsigned int m_sf;

  std::set<std::string> m_funcs;

};

#endif
