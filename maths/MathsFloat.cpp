/* SconeServer (http://www.sconemad.com)

Number

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

#include "MathsFloat.h"
#include "MathsModule.h"

//=========================================================================
MathsFloat::MathsFloat(MathsModule* module,
	 const Mpfr& value)
  : m_module(module),
    m_value(value)
{

}

//=========================================================================
MathsFloat::MathsFloat(const MathsFloat& c)
  : m_module(c.m_module),
    m_value(c.m_value)
{

}

//=========================================================================
MathsFloat::~MathsFloat()
{

}

//=========================================================================
scx::ScriptObject* MathsFloat::create(MathsModule* module,
			       const scx::ScriptRef* args)
{
  const scx::ScriptRef* value =
    scx::get_method_arg_ref(args,0,"value");

  const scx::ScriptNum* nvalue = 
    value ? dynamic_cast<const scx::ScriptNum*>(value->object()) : 0;
  if (nvalue) {
    return new MathsFloat(module,nvalue->get_real());
  }
  
  return new MathsFloat(module,value ? value->object()->get_string() : "0");
}

//=========================================================================
scx::ScriptObject* MathsFloat::new_copy() const
{
  return new MathsFloat(*this);
}

//=========================================================================
std::string MathsFloat::get_string() const
{
  return to_string(m_module.object()->get_sf());
}

//=========================================================================
int MathsFloat::get_int() const
{
  return (int)mpfr_get_si(m_value,rnd);
}

//=========================================================================
double MathsFloat::get_real() const
{
  return mpfr_get_d(m_value,rnd);
}

//=========================================================================
scx::ScriptRef* MathsFloat::script_op(const scx::ScriptAuth& auth,
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

  if (right) { // binary ops

    const MathsFloat* rc = 
      dynamic_cast<const MathsFloat*>(right->object());
    const scx::ScriptNum* rn = 
      dynamic_cast<const scx::ScriptNum*>(right->object());
    
    if (rc || rn) { // MathsFloat x MathsFloat (or converted) ops
      MathsFloat cc(m_module.object(),0);
      const MathsFloat* rvalue = 0;
      if (rc) {
	rvalue = rc;
      } else if (rn) {
	cc = MathsFloat(m_module.object(),rn->get_real());
	rvalue = &cc;
      }

      switch (op.type()) {
        
      case scx::ScriptOp::Add:
	{
	  return new scx::ScriptRef(new MathsFloat(m_module.object(),
					    m_value + rvalue->get_value()));
	}

      case scx::ScriptOp::Subtract:
	{
	  return new scx::ScriptRef(new MathsFloat(m_module.object(),
					    m_value - rvalue->get_value()));
	}
	
      case scx::ScriptOp::Multiply:
	{
	  return new scx::ScriptRef(new MathsFloat(m_module.object(),
					    m_value * rvalue->get_value()));
	}
	
      case scx::ScriptOp::Divide:
	{
	  return new scx::ScriptRef(new MathsFloat(m_module.object(),
					    m_value / rvalue->get_value()));
	}

      case scx::ScriptOp::Modulus:
	{
	  Mpfr r; mpfr_fmod(r,m_value,rvalue->get_value(),rnd);
	  return new scx::ScriptRef(new MathsFloat(m_module.object(),r));
	}
	break;

      case scx::ScriptOp::Power:
	{
	  Mpfr r; mpfr_pow(r,m_value,rvalue->get_value(),rnd);
	  return new scx::ScriptRef(new MathsFloat(m_module.object(),r));
	}

      case scx::ScriptOp::GreaterThan:
	return scx::ScriptBool::new_ref(m_value > rvalue->get_value());
	
      case scx::ScriptOp::LessThan:
	return scx::ScriptBool::new_ref(m_value < rvalue->get_value());
	
      case scx::ScriptOp::GreaterThanOrEqualTo:
	return scx::ScriptBool::new_ref(m_value >= rvalue->get_value());
	
      case scx::ScriptOp::LessThanOrEqualTo:
	return scx::ScriptBool::new_ref(m_value <= rvalue->get_value());
	
      case scx::ScriptOp::Equality:
	return scx::ScriptBool::new_ref(m_value == rvalue->get_value());
	
      case scx::ScriptOp::Inequality:
	return scx::ScriptBool::new_ref(m_value != rvalue->get_value());

      case scx::ScriptOp::Assign:
	if (!ref.is_const()) {
	  m_value = rvalue->get_value();
	}
	return ref.ref_copy();
          
      case scx::ScriptOp::AddAssign:
	if (!ref.is_const()) {
	  m_value = m_value + rvalue->get_value();
	}
	return ref.ref_copy();
	
      case scx::ScriptOp::SubtractAssign:
	if (!ref.is_const()) {
	  m_value = m_value - rvalue->get_value();
	}
	return ref.ref_copy();
      
      case scx::ScriptOp::MultiplyAssign:
	if (!ref.is_const()) {
	  m_value = m_value * rvalue->get_value();
	}
	return ref.ref_copy();
	
      case scx::ScriptOp::DivideAssign:
	if (!ref.is_const()) {
	  m_value = m_value / rvalue->get_value();
	}
	return ref.ref_copy();

      default: 
	break;
      }
    }

  } else { // prefix or postfix ops

    switch (op.type()) {
      
    case scx::ScriptOp::Positive:
      return new scx::ScriptRef(new_copy()); 
      
    case scx::ScriptOp::Negative:
      return new scx::ScriptRef(new MathsFloat(m_module.object(),-m_value));
      
    case scx::ScriptOp::PreIncrement:
      if (!ref.is_const()) {
	m_value = m_value + Mpfr(1);
      }
      return ref.ref_copy();
      
    case scx::ScriptOp::PreDecrement:
      if (!ref.is_const()) {
	m_value = m_value - Mpfr(1);
      }
      return ref.ref_copy();

    case scx::ScriptOp::Factorial:
      {
	if (!mpfr_integer_p(m_value)) {
	  return scx::ScriptError::new_ref("Must be integer");
	}
	Mpfr r; mpfr_fac_ui(r,mpfr_get_ui(m_value,rnd),rnd);
	return new scx::ScriptRef(new MathsFloat(m_module.object(),r));
      }
      break;

    case scx::ScriptOp::PostIncrement: 
      {
	Mpfr pre_value = m_value;
	if (!ref.is_const()) {
	  m_value = m_value + Mpfr(1);
	}
	return new scx::ScriptRef(new MathsFloat(m_module.object(),pre_value));
      }
      
    case scx::ScriptOp::PostDecrement: 
      {
	Mpfr pre_value = m_value;
	if (!ref.is_const()) {
	  m_value = m_value - Mpfr(1);
	}
	return new scx::ScriptRef(new MathsFloat(m_module.object(),pre_value));
      }
      
    default: 
      break;
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* MathsFloat::script_method(const scx::ScriptAuth& auth,
				   const scx::ScriptRef& ref,
				   const std::string& name,
				   const scx::ScriptRef* args)
{
  if ("string" == name) {
    const scx::ScriptInt* a_sf = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"sf");
    int sf = a_sf ? a_sf->get_int() : m_module.object()->get_sf();
    if (sf < 0) sf = 0;
    
    return scx::ScriptString::new_ref(to_string(sf));
  }

  MathsModule* m = m_module.object();
  MathsFloat* r = 0;

  if ("abs" == name) {
    r = new MathsFloat(m, abs(m_value));

  } else if ("ceil" == name) {
    r = new MathsFloat(m, ceil(m_value));

  } else if ("floor" == name) {
    r = new MathsFloat(m, floor(m_value));

  } else if ("trunc" == name) {
    r = new MathsFloat(m, trunc(m_value));

  } else if ("ln" == name) {
    r = new MathsFloat(m, ::log(m_value));

  } else if ("exp" == name) {
    r = new MathsFloat(m, ::exp(m_value));

  } else if ("sin" == name) {
    r = new MathsFloat(m, sin(m_value));

  } else if ("cos" == name) {
    r = new MathsFloat(m, cos(m_value));

  } else if ("tan" == name) {
    r = new MathsFloat(m, tan(m_value));

  } else if ("sinh" == name) {
    r = new MathsFloat(m, sinh(m_value));

  } else if ("cosh" == name) {
    r = new MathsFloat(m, cosh(m_value));

  } else if ("asin" == name) {
    r = new MathsFloat(m, asin(m_value));

  } else if ("acos" == name) {
    r = new MathsFloat(m, acos(m_value));

  } else if ("atan" == name) {
    r = new MathsFloat(m, atan(m_value));

  } else if ("atan2" == name) {
    const MathsFloat* y = scx::get_method_arg<MathsFloat>(args,0,"y");
    const MathsFloat* x = scx::get_method_arg<MathsFloat>(args,1,"x");
    if (!y || !x) 
      return scx::ScriptError::new_ref("No x or y value specified");
    r = new MathsFloat(m,atan2(y->get_value(), x->get_value()));
  }

  if (r) return new scx::ScriptRef(r);
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
const Mpfr& MathsFloat::get_value() const
{
  return m_value;
}

//=========================================================================
std::string MathsFloat::to_string(unsigned int dp) const
{
  if (!mpfr_number_p(m_value.value)) {
    return "nan";
  }

  char* cs = 0;
  ::mpfr_asprintf(&cs,"%-.*RNg",dp,m_value.value);
  
  std::string str(cs);
  mpfr_free_str(cs);
  return str;
}
