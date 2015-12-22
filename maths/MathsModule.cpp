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
#include <sconex/ScriptContext.h>

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
    m_sf(0)
{
  ::mp_set_memory_functions(alloc_func, realloc_func, free_func);
  m_funcs.insert("abs");
  m_funcs.insert("ceil");
  m_funcs.insert("floor");
  m_funcs.insert("trunc");
  m_funcs.insert("ln");
  m_funcs.insert("exp");
  m_funcs.insert("sin");
  m_funcs.insert("cos");
  m_funcs.insert("tan");
  m_funcs.insert("sinh");
  m_funcs.insert("cosh");
  m_funcs.insert("asin");
  m_funcs.insert("acos");
  m_funcs.insert("atan");
  m_funcs.insert("atan2");
  m_funcs.insert("sqrt");
  m_funcs.insert("gcd");
  m_funcs.insert("lcm");
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
      << "Using mpfr-" << mpfr_get_version() << " (mpfr.org)\n"
      << "Max float precision: " << MPFR_PREC_MAX << " bits\n";
  return oss.str();
}

//=============================================================================
int MathsModule::init()
{
  mpfr_set_default_prec(128);

  scx::StandardContext::register_type("MathsFloat",this);
  scx::StandardContext::register_type("MathsInt",this);

  return scx::Module::init();
}

//=============================================================================
bool MathsModule::close()
{
  scx::StandardContext::unregister_type("MathsFloat",this);
  scx::StandardContext::unregister_type("MathsInt",this);

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

    if ("set_prec" == name ||
	"set_sf" == name) 
      return new scx::ScriptMethodRef(ref,name);
    
    // Methods
    if (m_funcs.count(name)) return new scx::ScriptMethodRef(ref,name);
    
    // Properties
    MathsFloat* c = 0;

    if ("prec" == name) {
      return scx::ScriptInt::new_ref(mpfr_get_default_prec());
    }

    if ("sf" == name) {
      return scx::ScriptInt::new_ref(m_sf);
    }
    
    if ("PI" == name) {
      Mpfr(r); mpfr_const_pi(r, rnd);
      c = new MathsFloat(this, r);
    }

    else if ("e" == name) {
      Mpfr r(1);
      c = new MathsFloat(this, exp(r));
      mpfr_clear(r);
    }

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
  if ("set_prec" == name) {
    const scx::ScriptNum* a_prec = 
      scx::get_method_arg<scx::ScriptNum>(args,0,"value");
    if (!a_prec) return scx::ScriptError::new_ref("No value specified");
    mpfr_set_default_prec(a_prec->get_int());
    return 0;
  }
  
  if ("set_sf" == name) {
    const scx::ScriptNum* a_sf = 
      scx::get_method_arg<scx::ScriptNum>(args,0,"value");
    if (!a_sf) return scx::ScriptError::new_ref("No value specified");
    m_sf = a_sf->get_int();
    return 0;
  }

  if (m_funcs.count(name)) {
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
