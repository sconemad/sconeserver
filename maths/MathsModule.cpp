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
#include "Num.h"

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
  return "Maths module using the GNU multiple precision arithmetic library";
}

//=============================================================================
int MathsModule::init()
{
  //mpfr_set_default_prec(1024);

  scx::ScriptExpr::register_type("Num",this);

  return scx::Module::init();
}

//=============================================================================
bool MathsModule::close()
{
  scx::ScriptExpr::unregister_type("Num",this);

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
	"atan" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    Num* c = 0;

    if ("i" == name) {
      c = new Num(this,0,1);

    } else if ("PI" == name) 
      c = new Num(this,M_PIl);

    else if ("e" == name) 
      c = new Num(this,M_El);

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
  Num* r = 0;

  if ("abs" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");
    r = new Num(this, abs(n->get_re()), abs(n->get_im()));
  }

  if ("ceil" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");
    r = new Num(this, ceil(n->get_re()), ceil(n->get_im()));
  }

  if ("floor" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");
    r = new Num(this, floor(n->get_re()), floor(n->get_im()));
  }

  if ("trunc" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");
    r = new Num(this, trunc(n->get_re()), trunc(n->get_im()));
  }

  if ("ln" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this, ::log(n->get_re()),0);

    } else { // Complex case ln(r*e^(i*a)) = ln(r) + i*a
      r = new Num(this, ::log(n->get_mod()), n->get_arg());
    }
  }

  if ("exp" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this, ::exp(n->get_re()),0);

    } else { // Complex case exp(a+bi) = (e^a)cos(b) + i(e^a)sin(b)
      Mpfr ea = exp(n->get_re());
      r = new Num(this,
		  ea * cos(n->get_im()),
		  ea * sin(n->get_im()));
    }
  }

  if ("sin" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,sin(n->get_re()),0);

    } else { // Complex case sin(a+bi) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
      r = new Num(this, 
		  sin(n->get_re()) * cosh(n->get_im()),
		  cos(n->get_re()) * sinh(n->get_im()));
    }
  }

  if ("cos" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,cos(n->get_re()),0);

    } else { // Complex case cos(a+bi) = cos(a)*cosh(b) - i*sin(a)*sinh(b)
      r = new Num(this, 
		  cos(n->get_re()) * cosh(n->get_im()),
		  -sin(n->get_re()) * sinh(n->get_im()));
    }
  }

  if ("tan" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,tan(n->get_re()),0);

    } else { // Complex case tan(a+bi) = sin(a+bi) / cos(a+bi)
      Mpfr sr = sin(n->get_re()) * cosh(n->get_im());
      Mpfr si = cos(n->get_re()) * sinh(n->get_im());
      Mpfr cr = cos(n->get_re()) * cosh(n->get_im());
      Mpfr ci = -sin(n->get_re()) * sinh(n->get_im());
      //   (sr +i si) / (cr +i ci) 
      // = ((cr +i ci) (cr -i ci)) / ((sr +i si) (cr -i ci))
      // = ( (sr*cr + si*ci + i (si*cr - sr*ci) ) / (cr*cr +ci*ci)
      Mpfr d = cr*cr + ci*ci;
      r = new Num(this, 
		  (sr*cr + si*ci) / d, 
		  (si*cr - sr*ci) / d);
    }
  }

  if ("sinh" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,sinh(n->get_re()),0);

    } else { // Complex case sinh(a+bi) = sinh(a)*cos(b) + i*cosh(a)*sin(b)
      r = new Num(this,
		  sinh(n->get_re()) * cos(n->get_im()),
		  cosh(n->get_re()) * sin(n->get_im()));
    }
  }

  if ("cosh" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,cosh(n->get_re()),0);

    } else { // Complex case cosh(a+bi) = cosh(a)*cos(b) + i*sinh(a)*sin(b)
      r = new Num(this,
		  cosh(n->get_re()) * cos(n->get_im()),
		  sinh(n->get_re()) * sin(n->get_im()));
    }
  }

  if ("asin" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,asin(n->get_re()),0);

    } else { // Complex case asin(N) = -i ln( iN +/- sqrt(1 - N^2) )
      Mpfr o = n->get_im() * n->get_im();
      Mpfr p = n->get_re() + Mpfr(1);
      Mpfr q = n->get_re() - Mpfr(1);
      Mpfr v = Mpfr(0.5) * sqrt( p*p + o );
      Mpfr w = Mpfr(0.5) * sqrt( q*q + o );
      Mpfr a = v + w;
      Mpfr b = v - w;
      if (mpfr_sgn(n->get_im().value) >= 0) {
	r = new Num(this, asin(b), ::log(a + sqrt(a*a - Mpfr(1))) );
      } else {
	r = new Num(this, asin(b), -::log(a + sqrt(a*a - Mpfr(1))) );
      }
    }
  }

  if ("acos" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,acos(n->get_re()),0);

    } else { // Complex case acos(N) = -i ln( N +/- i*sqrt(1 - N^2) )
      Mpfr o = n->get_im() * n->get_im();
      Mpfr p = n->get_re() + Mpfr(1);
      Mpfr q = n->get_re() - Mpfr(1);
      Mpfr v = Mpfr(0.5) * sqrt( p*p + o );
      Mpfr w = Mpfr(0.5) * sqrt( q*q + o );
      Mpfr a = v + w;
      Mpfr b = v - w;
      if (mpfr_sgn(n->get_im().value) >= 0) {
	r = new Num(this, acos(b), -::log(a + sqrt(a*a - Mpfr(1))) );
      } else {
	r = new Num(this, acos(b), ::log(a + sqrt(a*a - Mpfr(1))) );
      }
    }
  }

  if ("atan" == name) {
    const Num* n = scx::get_method_arg<Num>(args,0,"value");
    if (!n) 
      return scx::ScriptError::new_ref("No value specified");

    if (n->is_real()) { // Real case
      r = new Num(this,atan(n->get_re()),0);

    } else { // Complex case atan(X) = (i/2) ln((i+X)/(i-X))
      //   (i+a+ib)/(i-a-ib)
      // = [(i(1+b) + a) (i(b-1) -a) ] / [ (i(1-b) -a) (i(b-1) -a) ]
      // = [ -(1+b)(b-1) -aa +i(a(b-1) -(1+b)a) ] / [ -(1-b)(b-1) +aa ]
      // = [ 1 -bb -aa -2ai ] / [ 1 + aa -2b +bb ]
      Mpfr d = Mpfr(1) + n->get_re()*n->get_re() 
	- Mpfr(2)*n->get_im() + n->get_im()*n->get_im();
      Mpfr yr = (Mpfr(1)-n->get_im()*n->get_im()-n->get_re()*n->get_re()) / d;
      Mpfr yi = Mpfr(-2) * n->get_re() / d;
      // ln(r*e^(i*a)) = ln(r) + i*a
      Mpfr zr = ::log(sqrt(yr*yr + yi*yi));
      Mpfr zi = atan2(yi,yr);
      r = new Num(this, Mpfr(-0.5)*zi, Mpfr(0.5)*zr);
    }
  }

  if (r) return new scx::ScriptRef(r);

  return scx::Module::script_method(auth,ref,name,args);
}

//=============================================================================
void MathsModule::provide(const std::string& type,
			  const scx::ScriptRef* args,
			  scx::ScriptObject*& object)
{
  if (type == "Num")
    object = Num::create(this,args);
}
