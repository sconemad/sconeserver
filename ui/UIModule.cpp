/* SconeServer (http://www.sconemad.com)

User interface module

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


#include "UIModule.h"
#include "UIDisplay.h"
#include "UIWindow.h"
#include <sconex/Kernel.h>
#include <sconex/ScriptContext.h>

SCONEX_MODULE(UIModule);

//=========================================================================
UIModule::UIModule() 
  : scx::Module("ui",scx::version()),
    m_display(0),
    m_window(0)
{
  XInitThreads();
  scx::StandardContext::register_type("Window",this);
}

//=========================================================================
UIModule::~UIModule()
{
  scx::StandardContext::unregister_type("Window",this);
}

//=========================================================================
std::string UIModule::info() const
{
  return "User interface module";
}

//=============================================================================
int UIModule::init()
{
  m_display = new UIDisplay(this);
  m_window = new UIWindow(m_display);
  return 0;
}

//=============================================================================
bool UIModule::close()
{
  if (!scx::Module::close()) return false;

  if (m_display) {
    delete m_window; m_window = 0;
    m_display->close();
  }

  return true;
}

//=============================================================================
void UIModule::display_closing(UIDisplay* display)
{
  m_display = 0;
}

//=============================================================================
scx::ScriptRef* UIModule::script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();

    // Methods
    if ("plot" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

  }
	
  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* UIModule::script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args)
{
  if ("plot" == name) {
    const scx::ScriptRef* a_x = scx::get_method_arg_ref(args,0,"x");
    const scx::ScriptRef* a_y = scx::get_method_arg_ref(args,1,"y");
    const scx::ScriptRef* a_val = scx::get_method_arg_ref(args,2,"value");
    if (!a_x || !a_y) 
      return scx::ScriptError::new_ref("Must specify x and y");
      
    m_window->plot(a_x->object()->get_int(), 
		   a_y->object()->get_int(),
		   a_val ? a_val->object()->get_int() : 1);

    return 0;
  }

  return scx::Module::script_method(auth,ref,name,args);
}

//=============================================================================
void UIModule::provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object)
{
  if (type == "Window") 
    object = scx::ScriptInt::create(args);
}
