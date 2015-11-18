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


#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/ScriptExpr.h>

#include "MathsModule.h"
#include "MathsInt.h"
#include "MathsFloat.h"

#include <gmp.h>
#include <mpfr.h>

SCONEX_MODULE(MathsModule);

const size_t ALLOC_LIMIT = 1000000;

//=========================================================================
void* alloc_func(size_t alloc_size)
{
  if (alloc_size > ALLOC_LIMIT) 
    throw "Out of memory for gmp type";

  return ::malloc(alloc_size);
}

//=========================================================================
void* realloc_func(void* ptr, size_t /* old_size */, size_t new_size)
{
  if (new_size > ALLOC_LIMIT) 
    throw "Out of memory for gmp type";

  return ::realloc(ptr, new_size);
}

//=========================================================================
void free_func(void* ptr, size_t /* size */)
{
  ::free(ptr);
}

//=========================================================================
MathsModule::MathsModule() 
  : scx::Module("maths",scx::version()),
    m_sf(16)
{
  ::mp_set_memory_functions(alloc_func, realloc_func, free_func);
}

//=========================================================================
MathsModule::~MathsModule()
{

}

//=========================================================================
std::string MathsModule::info() const
{
  std::ostringstream oss;
  oss << "Mathematics\n"
      << "Using gmp-" << gmp_version << " (gmplib.org)\n"
      << "Using mpfr-" << mpfr_get_version() << " (mpfr.org)\n";
  return oss.str();
}

//=============================================================================
int MathsModule::init()
{
  //  mpfr_set_default_prec(1024);

  scx::ScriptExpr::register_type("MathsFloat",this);
  scx::ScriptExpr::register_type("MathsInt",this);

  return scx::Module::init();
}

//=============================================================================
bool MathsModule::close()
{
  scx::ScriptExpr::unregister_type("MathsFloat",this);
  scx::ScriptExpr::unregister_type("MathsInt",this);

  return scx::Module::close();
}

//=============================================================================
unsigned int MathsModule::get_sf() const
{
  return m_sf;
}

//=============================================================================
scx::ScriptRef* MathsModule::script_op(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const scx::ScriptOp& op,
				       const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();

    // Methods

    if ("abs" == name || 
	"ceil" == name ||
	"floor" == name ||
	"trunc" == name ||
	"ln" == name || 
	"exp" == name ||
	"sin" == name || 
	"cos" == name || 
	"tan" == name ||
	"sinh" == name || 
	"cosh" == name ||
	"asin" == name || 
	"acos" == name || 
	"atan" == name || 
	"atan2" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    MathsFloat* c = 0;

    if ("PI" == name) 
      c = new MathsFloat(this,M_PI);

    else if ("e" == name) 
      c = new MathsFloat(this,M_E);

    if (c) return new scx::ScriptRef(c,scx::ScriptRef::ConstRef);

  }
	
  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* MathsModule::script_method(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const std::string& name,
					   const scx::ScriptRef* args)
{
  if ("abs" == name || 
      "ceil" == name ||
      "floor" == name ||
      "trunc" == name ||
      "ln" == name || 
      "exp" == name ||
      "sin" == name || 
      "cos" == name || 
      "tan" == name ||
      "sinh" == name || 
      "cosh" == name ||
      "asin" == name || 
      "acos" == name || 
      "atan" == name || 
      "atan2" == name) {
    const scx::ScriptNum* n =
      scx::get_method_arg<scx::ScriptNum>(args,0,"value");
    if (!n) return scx::ScriptError::new_ref("No value specified");
    scx::ScriptNum* nc = const_cast<scx::ScriptNum*>(n);
    return nc->script_method(auth, ref, name, args);
  }
  
  return scx::Module::script_method(auth,ref,name,args);
}

//=============================================================================
void MathsModule::provide(const std::string& type,
			  const scx::ScriptRef* args,
			  scx::ScriptObject*& object)
{
  if (type == "MathsFloat")
    object = MathsFloat::create(this,args);
  if (type == "MathsInt")
    object = MathsInt::create(this,args);
}
