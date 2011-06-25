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

#include "Num.h"
#include "MathsModule.h"

//=========================================================================
Num::Num(MathsModule* module,
	 const Mpfr& re, 
	 const Mpfr& im)
  : m_module(module),
    m_re(re),
    m_im(im)
{

}

//=========================================================================
Num::Num(const Num& c)
  : m_module(c.m_module),
    m_re(c.m_re),
    m_im(c.m_im)
{

}

//=========================================================================
Num::~Num()
{

}

//=========================================================================
scx::ScriptObject* Num::create(MathsModule* module,
			       const scx::ScriptRef* args)
{
  const scx::ScriptRef* re =
    scx::get_method_arg_ref(args,0,"re");
  const scx::ScriptRef* im =
    scx::get_method_arg_ref(args,1,"im");

  const scx::ScriptNum* nre = 
    re ? dynamic_cast<const scx::ScriptNum*>(re->object()) : 0;
  const scx::ScriptNum* nim = 
    im ? dynamic_cast<const scx::ScriptNum*>(im->object()) : 0;
  if (nre || nim) {
    return new Num(module,
		   nre ? nre->get_real() : 0.0,
		   nim ? nim->get_real() : 0.0);
  }
  
  return new Num(module,
		 re ? re->object()->get_string() : "0",
		 im ? im->object()->get_string() : "0");
}

//=========================================================================
scx::ScriptObject* Num::new_copy() const
{
  return new Num(*this);
}

//=========================================================================
std::string Num::get_string() const
{
  return to_string(m_module.object()->get_sf());
}

//=========================================================================
int Num::get_int() const
{
  return (int)mpfr_get_si(get_mod(),rnd);
}

//=========================================================================
double Num::get_real() const
{
  return mpfr_get_d(get_mod(),rnd);
}

//=========================================================================
scx::ScriptRef* Num::script_op(const scx::ScriptAuth& auth,
			       const scx::ScriptRef& ref,
			       const scx::ScriptOp& op,
			       const scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    const std::string name = right->object()->get_string();

    if ("string" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    if ("re" == name) 
      return new scx::ScriptRef(new Num(m_module.object(),m_re,0));
    if ("im" == name)
      return new scx::ScriptRef(new Num(m_module.object(),m_im,0));
    if ("mod" == name)
      return new scx::ScriptRef(new Num(m_module.object(),get_mod(),0));
    if ("arg" == name) 
      return new scx::ScriptRef(new Num(m_module.object(),get_arg(),0));
  }

  if (right) { // binary ops

    const Num* rc = 
      dynamic_cast<const Num*>(right->object());
    const scx::ScriptNum* rn = 
      dynamic_cast<const scx::ScriptNum*>(right->object());
    
    if (rc || rn) { // Num x Num (or converted) ops
      Num cc(m_module.object(),0,0);
      const Num* rvalue = 0;
      if (rc) {
	rvalue = rc;
      } else if (rn) {
	cc = Num(m_module.object(),rn->get_real(),0.0);
	rvalue = &cc;
      }

      switch (op.type()) {
        
      case scx::ScriptOp::Add:
	{
	  return new scx::ScriptRef(new Num(m_module.object(),
					    m_re + rvalue->get_re(),
					    m_im + rvalue->get_im()));
	}

      case scx::ScriptOp::Subtract:
	{
	  return new scx::ScriptRef(new Num(m_module.object(),
					    m_re - rvalue->get_re(),
					    m_im - rvalue->get_im()));
	}
	
      case scx::ScriptOp::Multiply:
	{
	  if (is_real() && rvalue->is_real()) {
	    return new scx::ScriptRef(new Num(m_module.object(),
					      m_re * rvalue->get_re()));
	  }
	  // (a+ib)(c+id) = ac + aid + ibc - bd = ac-bd + i(ad+bc)
	  return new scx::ScriptRef(new Num(m_module.object(),
	    m_re * rvalue->get_re() - m_im * rvalue->get_im(),
	    m_re * rvalue->get_im() + m_im * rvalue->get_re()));

	  // Alternatively
	  //	  Mpfr m; ::mpf_mul(m,get_mod(),rvalue->get_mod());
	  //	  double o = (get_arg() + rvalue->get_arg());
	  //	  Mpfr r; ::mpf_mul(r,m,Mpfr(cos(o)));
	  //	  Mpfr i; ::mpf_mul(i,m,Mpfr(sin(o)));
	  //	  return new scx::ScriptRef(new Num(m_module.object(),r,i));
	}
	
      case scx::ScriptOp::Divide:
	{
	  if (is_real() && rvalue->is_real()) {
	    return new scx::ScriptRef(new Num(m_module.object(),
					      m_re / rvalue->get_re()));
	  }
	  Mpfr n = rvalue->get_re() * rvalue->get_re() +
       	           rvalue->get_im() * rvalue->get_im();
	  return new scx::ScriptRef(new Num(m_module.object(),
	    (m_re * rvalue->get_re() + m_im * rvalue->get_im()) / n,
            (m_im * rvalue->get_re() - m_re * rvalue->get_im()) / n));
	}

      case scx::ScriptOp::Modulus:
	{
	  if (is_real() && rvalue->is_real()) {
	    Mpfr r; mpfr_fmod(r,m_re,rvalue->get_re(),rnd);
	    return new scx::ScriptRef(new Num(m_module.object(),r));
	  }
	  // Its all just too much...
	  return scx::ScriptError::new_ref("Must be real");
	}
	break;

      case scx::ScriptOp::Power:
	{
	  if (is_real() && rvalue->is_real() && mpfr_cmp_d(m_re,0) >= 0) {
	    Mpfr r; mpfr_pow(r,m_re,rvalue->get_re(),rnd);
	    return new scx::ScriptRef(new Num(m_module.object(),r));
	  }
	  // here's where is gets a bit complex...
	  // (a+bi)^(e+di) = (a^2 + b^2)^(e/2) * e^(-d*arg(a+bi)) *
	  //                 (cos(c*arg(a+bi) + 0.5*d*ln(a^2 + b^2)) 
	  //                  + i*sin(c*arg(a+bi) + 0.5*d*ln(a^2 + b^2)))
	  Mpfr z = (m_re*m_re + m_im*m_im);
	  Mpfr o = (rvalue->get_re() * get_arg()) + 
	    (Mpfr(0.5) * rvalue->get_im() * ::log(z));
	  Mpfr q = pow(z,Mpfr(0.5)*rvalue->get_re()) * 
	    exp(-rvalue->get_im()*get_arg());
	  Mpfr sino, coso; mpfr_sin_cos(sino,coso,o,rnd);
	  Mpfr lim(1e-12); // Round it a bit!
	  if (mpfr_cmpabs(sino,lim) < 0) sino = 0.0;
	  if (mpfr_cmpabs(coso,lim) < 0) coso = 0.0;
	  return new scx::ScriptRef(new Num(m_module.object(),
					    q * coso, q * sino));
	}

      case scx::ScriptOp::GreaterThan:
	return scx::ScriptInt::new_ref(get_mod() > rvalue->get_mod());
	
      case scx::ScriptOp::LessThan:
	return scx::ScriptInt::new_ref(get_mod() < rvalue->get_mod());
	
      case scx::ScriptOp::GreaterThanOrEqualTo:
	return scx::ScriptInt::new_ref(get_mod() >= rvalue->get_mod());
	
      case scx::ScriptOp::LessThanOrEqualTo:
	return scx::ScriptInt::new_ref(get_mod() <= rvalue->get_mod());
	
      case scx::ScriptOp::Equality:
	return scx::ScriptInt::new_ref(m_re == rvalue->get_re() &&
				       m_im == rvalue->get_im());
	
      case scx::ScriptOp::Inequality:
	return scx::ScriptInt::new_ref(m_re != rvalue->get_re() ||
				       m_im != rvalue->get_im());

      case scx::ScriptOp::Assign:
	if (!ref.is_const()) {
	  m_re = rvalue->get_re();
	  m_im = rvalue->get_im();
	}
	return ref.ref_copy();
          
      case scx::ScriptOp::AddAssign:
	if (!ref.is_const()) {
	  m_re = m_re + rvalue->get_re();
	  m_im = m_im + rvalue->get_im();
	}
	return ref.ref_copy();
	
      case scx::ScriptOp::SubtractAssign:
	if (!ref.is_const()) {
	  m_re = m_re - rvalue->get_re();
	  m_im = m_im - rvalue->get_im();
	}
	return ref.ref_copy();
      
      case scx::ScriptOp::MultiplyAssign:
	if (!ref.is_const()) {
	  if (is_real() && rvalue->is_real()) {
	    m_re = m_re * rvalue->get_re();
	  } else {
	    m_re = m_re * rvalue->get_re() - m_im * rvalue->get_im();
	    m_im = m_re * rvalue->get_im() + m_im * rvalue->get_re();
	  }
	}
	return ref.ref_copy();
	
      case scx::ScriptOp::DivideAssign:
	if (!ref.is_const()) {
	  if (is_real() && rvalue->is_real()) {
	    m_re = m_re / rvalue->get_re();
	  } else {
	    Mpfr n = rvalue->get_re() * rvalue->get_re() +
	      rvalue->get_im() * rvalue->get_im();
	    m_re = (m_re * rvalue->get_re() + m_im * rvalue->get_im()) / n;
	    m_im = (m_im * rvalue->get_re() - m_re * rvalue->get_im()) / n;
	  }
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
      return new scx::ScriptRef(new Num(m_module.object(),-m_re,-m_im));
      
    case scx::ScriptOp::Factorial:
      {
	if (!mpfr_integer_p(m_re) || !mpfr_integer_p(m_im)) {
	  return scx::ScriptError::new_ref("Must be integer");
	}
	if (is_real()) {
	  Mpfr r; mpfr_fac_ui(r,mpfr_get_ui(m_re,rnd),rnd);
	  return new scx::ScriptRef(new Num(m_module.object(),r));
	}
	Mpfr r; mpfr_fac_ui(r,mpfr_get_ui(m_re,rnd),rnd);
	Mpfr i; mpfr_fac_ui(i,mpfr_get_ui(m_im,rnd),rnd);
	return new scx::ScriptRef(new Num(m_module.object(),r,i));
      }
      break;

    default: 
      break;
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* Num::script_method(const scx::ScriptAuth& auth,
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

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
bool Num::is_real() const
{
  return mpfr_zero_p(m_im.value);
}

//=========================================================================
bool Num::is_imaginary() const
{
  return mpfr_zero_p(m_re.value) && !mpfr_zero_p(m_im.value);
}

//=========================================================================
bool Num::is_complex() const
{
  return !mpfr_zero_p(m_re.value) && !mpfr_zero_p(m_im.value);
}

//=========================================================================
const Mpfr& Num::get_re() const
{
  return m_re;
}

//=========================================================================
const Mpfr& Num::get_im() const
{
  return m_im;
}

//=========================================================================
Mpfr Num::get_mod() const
{ 
  if (is_real()) return abs(m_re);
  if (is_imaginary()) return abs(m_im);

  return sqrt(m_re*m_re + m_im*m_im);
}

//=========================================================================
Mpfr Num::get_arg() const 
{
  return atan2(m_im,m_re);
}

//=========================================================================
std::string Num::to_string(unsigned int dp) const
{
  if (!mpfr_number_p(m_re.value) ||
      !mpfr_number_p(m_im.value)) {
    return "nan";
  }

  bool r = !mpfr_zero_p(m_re.value);
  bool i = !mpfr_zero_p(m_im.value);

  const unsigned int maxlen = 64 + (r ? dp : 0) + (i ? dp : 0);
  char* cs = new char[maxlen];

  if (r && i) {
    ::mpfr_snprintf(cs,maxlen,"%-.*RNg%+.*RNg*i",dp,m_re.value,dp,m_im.value);
  } else if (i) {
    ::mpfr_snprintf(cs,maxlen,"%-.*RNg*i",dp,m_im.value);
  } else {
    ::mpfr_snprintf(cs,maxlen,"%-.*RNg",dp,m_re.value);
  }
  
  std::string str(cs);
  delete[] cs;
  return str;
}
