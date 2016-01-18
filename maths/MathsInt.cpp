/* SconeServer (http://www.sconemad.com)

MathsInt - Maths Integer type using gmp mpz

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

#include "MathsInt.h"
#include "MathsFloat.h"
#include "MathsModule.h"

//===========================================================================
scx::ScriptRef* MathsInt::new_ref(MathsModule* module, mpz_t value)
{
  MathsInt* o = new MathsInt(module);
  mpz_swap(o->m_value, value);
  mpz_clear(value);
  return new scx::ScriptRef(o);
}

//=========================================================================
MathsInt::MathsInt(MathsModule* module)
  : m_module(module)
{
  mpz_init(m_value);
}

//=========================================================================
MathsInt::MathsInt(MathsModule* module, const std::string& value)
  : m_module(module)
{
  mpz_init_set_str(m_value, value.c_str(), 0);
}

//=========================================================================
MathsInt::MathsInt(MathsModule* module, long value)
  : m_module(module)
{
  mpz_init_set_si(m_value, value);
}

//=========================================================================
MathsInt::MathsInt(MathsModule* module, double value)
  : m_module(module)
{
  mpz_init_set_d(m_value, value);
}

//=========================================================================
MathsInt::MathsInt(MathsModule* module, const mpz_t value)
  : m_module(module)
{
  mpz_init_set(m_value, value);
}

//=========================================================================
MathsInt::MathsInt(const MathsInt& c)
  : m_module(c.m_module)
{
  mpz_init_set(m_value, c.m_value);
}

//=========================================================================
MathsInt::~MathsInt()
{
  mpz_clear(m_value);
}

//=========================================================================
scx::ScriptObject* MathsInt::create(MathsModule* module,
				    const scx::ScriptRef* args)
{
  const scx::ScriptRef* value =
    scx::get_method_arg_ref(args,0,"value");
  return new MathsInt(module, value ? value->object()->get_string() : "0");
}

//=========================================================================
scx::ScriptObject* MathsInt::new_copy() const
{
  return new MathsInt(*this);
}

//=========================================================================
std::string MathsInt::get_string() const
{
  return to_string();
}

//=========================================================================
int MathsInt::get_int() const
{
  return (int)mpz_get_si(m_value);
}

//=========================================================================
double MathsInt::get_real() const
{
  return mpz_get_d(m_value);
}

//=========================================================================
scx::ScriptRef* MathsInt::script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    const std::string name = right->object()->get_string();
    
    if ("string" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  }
  
  MathsModule* m = m_module.object();
  
  if (right) { // binary ops
    
    const MathsInt* rmi = dynamic_cast<const MathsInt*>(right->object());
    if (!rmi) {
      // Promote to float to handle mixed ops
      scx::ScriptRef fl(new MathsFloat(m_module.object(), get_string()));
      return fl.object()->script_op(auth, ref, op, right);
    }
    
    // MathsInt x MathsInt ops
    const mpz_t& rvalue = rmi->get_value();
    
    switch (op.type()) {
    case scx::ScriptOp::Add: {
      mpz_t r; mpz_init(r); mpz_add(r, m_value, rvalue);
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::Subtract: {
      mpz_t r; mpz_init(r); mpz_sub(r, m_value, rvalue);
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::Multiply: {
      mpz_t r; mpz_init(r); mpz_mul(r, m_value, rvalue);
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::Divide: {
      if (mpz_divisible_p(m_value, rvalue)) {
	mpz_t r; mpz_init(r); mpz_tdiv_q(r, m_value, rvalue);
	return MathsInt::new_ref(m, r);
      }
      // Promote to float to handle non-integer division
      scx::ScriptRef fl(new MathsFloat(m_module.object(), get_string()));
      return fl.object()->script_op(auth, ref, op, right);
    }
    case scx::ScriptOp::Modulus: {
      mpz_t r; mpz_init(r); mpz_tdiv_r(r, m_value, rvalue);
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::Power: {
      mpz_t r; mpz_init(r); mpz_pow_ui(r, m_value, mpz_get_ui(rvalue));
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::GreaterThan: {
      return scx::ScriptBool::new_ref(mpz_cmp(m_value, rvalue) > 0);
    }
    case scx::ScriptOp::LessThan: {
      return scx::ScriptBool::new_ref(mpz_cmp(m_value, rvalue) < 0);
    }
    case scx::ScriptOp::GreaterThanOrEqualTo: {
      return scx::ScriptBool::new_ref(mpz_cmp(m_value, rvalue) >= 0);
    }
    case scx::ScriptOp::LessThanOrEqualTo: {
      return scx::ScriptBool::new_ref(mpz_cmp(m_value, rvalue) <= 0);
    }
    case scx::ScriptOp::Equality: {
      return scx::ScriptBool::new_ref(mpz_cmp(m_value, rvalue) == 0);
    }
    case scx::ScriptOp::Inequality: {
      return scx::ScriptBool::new_ref(mpz_cmp(m_value, rvalue) != 0);
    }
    case scx::ScriptOp::Assign: {
      if (!ref.is_const()) mpz_set(m_value, rvalue);
      return ref.ref_copy();
    }
    case scx::ScriptOp::AddAssign: {
      if (!ref.is_const()) mpz_add(m_value, m_value, rvalue);
      return ref.ref_copy();
    }
    case scx::ScriptOp::SubtractAssign: {
      if (!ref.is_const()) mpz_sub(m_value, m_value, rvalue);
      return ref.ref_copy();
    }
    case scx::ScriptOp::MultiplyAssign: {
      if (!ref.is_const()) mpz_mul(m_value, m_value, rvalue);
      return ref.ref_copy();
    }
    case scx::ScriptOp::DivideAssign: {
      //XXX promote to float if not integer
      if (!ref.is_const()) mpz_tdiv_q(m_value, m_value, rvalue);
      return ref.ref_copy();
    }
    default:
      break;
    }
    
  } else { // prefix or postfix ops
    
    switch (op.type()) {
    case scx::ScriptOp::Positive: {
      return new scx::ScriptRef(new_copy()); 
    }
    case scx::ScriptOp::Negative: {
      mpz_t r; mpz_init(r); mpz_neg(r, m_value);
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::PreIncrement: {
      if (!ref.is_const()) mpz_add_ui(m_value, m_value, 1);
      return ref.ref_copy();
    }
    case scx::ScriptOp::PreDecrement: {
      if (!ref.is_const()) mpz_sub_ui(m_value, m_value, 1);
      return ref.ref_copy();
    }
    case scx::ScriptOp::Factorial: {
      mpz_t r; mpz_init(r); mpz_fac_ui(r, mpz_get_ui(m_value));
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::PostIncrement: {
      mpz_t r; mpz_init_set(r, m_value);
      if (!ref.is_const()) mpz_add_ui(m_value, m_value, 1);
      return MathsInt::new_ref(m, r);
    }
    case scx::ScriptOp::PostDecrement: {
      mpz_t r; mpz_init_set(r, m_value);
      if (!ref.is_const()) mpz_sub_ui(m_value, m_value, 1);
      return MathsInt::new_ref(m, r);
    }
    default: 
      break;
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* MathsInt::script_method(const scx::ScriptAuth& auth,
				   const scx::ScriptRef& ref,
				   const std::string& name,
				   const scx::ScriptRef* args)
{
  if ("string" == name) {
    return scx::ScriptString::new_ref(to_string());
  }

  MathsModule* m = m_module.object();
  
  if ("abs" == name) {
    mpz_t r; mpz_init(r); mpz_abs(r, m_value);
    return MathsInt::new_ref(m, r);
  }

  if ("gcd" == name) {
    const scx::ScriptList* al = scx::get_method_arg<scx::ScriptList>(args,0,"list");
    if (al) return gcd_list(al);
    return gcd_list(dynamic_cast<const scx::ScriptList*>(args->object()));
  }

  if ("lcm" == name) {
    const scx::ScriptList* al = scx::get_method_arg<scx::ScriptList>(args,0,"list");
    if (al) return lcm_list(al);
    return lcm_list(dynamic_cast<const scx::ScriptList*>(args->object()));
  }

  if ("fib" == name) {
    mpz_t r; mpz_init(r); mpz_fib_ui(r, mpz_get_ui(m_value));
    return MathsInt::new_ref(m, r);
  }

  // Promote to float to handle other methods
  scx::ScriptRef fl(new MathsFloat(m_module.object(), get_string()));
  return fl.object()->script_method(auth, ref, name, args);
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
const mpz_t& MathsInt::get_value() const
{
  return m_value;
}

//=========================================================================
std::string MathsInt::to_string() const
{
  char* cs = mpz_get_str(0, 10, m_value);
  std::string str(cs);
  ::free(cs);
  return str;
}

//=========================================================================
scx::ScriptRef* MathsInt::gcd_list(const scx::ScriptList* l)
{
  if (!l || l->size() == 0) return scx::ScriptError::new_ref("No arguments specified");

  const scx::ScriptRef* a = l->get(0);
  const MathsInt* ai = a ? dynamic_cast<const MathsInt*>(a->object()) : 0;
  mpz_t r; mpz_init_set(r, ai->get_value());
  
  for (int i=1; i<l->size(); ++i) {
    const scx::ScriptRef* b = l->get(i);
    const MathsInt* bi = b ? dynamic_cast<const MathsInt*>(b->object()) : 0;
    if (!bi) {
      mpz_clear(r);
      return scx::ScriptError::new_ref("Non integer value specified");
    }
    mpz_gcd(r, r, bi->get_value());
  }

  return MathsInt::new_ref(m_module.object(), r);
}
      
//=========================================================================
scx::ScriptRef* MathsInt::lcm_list(const scx::ScriptList* l)
{
  if (!l || l->size() == 0) return scx::ScriptError::new_ref("No arguments specified");

  const scx::ScriptRef* a = l->get(0);
  const MathsInt* ai = a ? dynamic_cast<const MathsInt*>(a->object()) : 0;
  mpz_t r; mpz_init_set(r, ai->get_value());
  
  for (int i=1; i<l->size(); ++i) {
    const scx::ScriptRef* b = l->get(i);
    const MathsInt* bi = b ? dynamic_cast<const MathsInt*>(b->object()) : 0;
    if (!bi) {
      mpz_clear(r);
      return scx::ScriptError::new_ref("Non integer value specified");
    }
    mpz_lcm(r, r, bi->get_value());
  }

  return MathsInt::new_ref(m_module.object(), r);
}
    
