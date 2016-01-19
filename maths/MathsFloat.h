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

#ifndef mathsFloat_h
#define mathsFloat_h

#include <sconex/ScriptTypes.h>

#include <mpfr.h>

class MathsModule;

const mpfr_rnd_t rnd = GMP_RNDN;

//=========================================================================
// Mpfr - A simple C++ wrapper around the mpfr_t type.
//
class Mpfr {
public: 

  // Construction

  Mpfr() 
  { ::mpfr_init(value); }

  Mpfr(long double ld) 
  { mpfr_init_set_ld(value, ld, rnd); }

  Mpfr(const std::string& s, int base=10) 
  { mpfr_init_set_str(value,s.c_str(),base,rnd); }

  Mpfr(const mpfr_t& f) 
  { mpfr_init_set(value,f,rnd); };

  Mpfr(const mpz_t& z) 
  { mpfr_init_set_z(value,z,rnd); };

  Mpfr(const Mpfr& c) 
  { mpfr_init_set(value,c.value,rnd); }

  ~Mpfr() 
  { mpfr_clear(value); }

  // Operators

  Mpfr operator-() const
  { Mpfr r; mpfr_neg(r,value,rnd); return r; }

  Mpfr operator+(const Mpfr& c) const
  { Mpfr r; mpfr_add(r,value,c.value,rnd); return r; }

  Mpfr operator-(const Mpfr& c) const
  { Mpfr r; mpfr_sub(r,value,c.value,rnd); return r; }
 
  Mpfr operator*(const Mpfr& c) const
    { Mpfr r; mpfr_mul(r,value,c.value,rnd); return r; }

  Mpfr operator/(const Mpfr& c) const
  { Mpfr r; mpfr_div(r,value,c.value,rnd); return r; }
  
  Mpfr& operator=(const Mpfr& c)
  { if (this != &c) mpfr_set(value,c.value,rnd); return *this; }

  int operator>(const Mpfr& c)
  { return ::mpfr_greater_p(value,c.value); }

  int operator<(const Mpfr& c)
  { return ::mpfr_less_p(value,c.value); }

  int operator>=(const Mpfr& c)
  { return ::mpfr_greaterequal_p(value,c.value); }

  int operator<=(const Mpfr& c)
  { return ::mpfr_lessequal_p(value,c.value); }

  int operator==(const Mpfr& c)
  { return ::mpfr_equal_p(value,c.value); }

  int operator!=(const Mpfr& c)
  { return !::mpfr_equal_p(value,c.value); }

  // Conversions

  operator mpfr_t&() 
  { return value; };
  
  operator const mpfr_t&() const 
  { return value; };

  // Data

  mpfr_t value;
};

// Basic maths functions for Mpfr
inline Mpfr abs(const Mpfr& v) { Mpfr r; mpfr_abs(r,v,rnd); return r; }
inline Mpfr ceil(const Mpfr& v) { Mpfr r; mpfr_ceil(r,v); return r; }
inline Mpfr floor(const Mpfr& v) { Mpfr r; mpfr_floor(r,v); return r; }
inline Mpfr trunc(const Mpfr& v) { Mpfr r; mpfr_trunc(r,v); return r; }
inline Mpfr log(const Mpfr& v) { Mpfr r; mpfr_log(r,v,rnd); return r; }
inline Mpfr exp(const Mpfr& v) { Mpfr r; mpfr_exp(r,v,rnd); return r; }
inline Mpfr pow(const Mpfr& v, const Mpfr& e) 
{ Mpfr r; mpfr_pow(r,v,e,rnd); return r; }
inline Mpfr sqrt(const Mpfr& v) { Mpfr r; mpfr_sqrt(r,v,rnd); return r; }

inline Mpfr cos(const Mpfr& v) { Mpfr r; mpfr_cos(r,v,rnd); return r; }
inline Mpfr sin(const Mpfr& v) { Mpfr r; mpfr_sin(r,v,rnd); return r; }
inline Mpfr tan(const Mpfr& v) { Mpfr r; mpfr_tan(r,v,rnd); return r; }
inline Mpfr acos(const Mpfr& v) { Mpfr r; mpfr_acos(r,v,rnd); return r; }
inline Mpfr asin(const Mpfr& v) { Mpfr r; mpfr_asin(r,v,rnd); return r; }
inline Mpfr atan(const Mpfr& v) { Mpfr r; mpfr_atan(r,v,rnd); return r; }
inline Mpfr atan2(const Mpfr& y, const Mpfr& x) 
{ Mpfr r; mpfr_atan2(r,y,x,rnd); return r; }
inline Mpfr cosh(const Mpfr& v) { Mpfr r; mpfr_cosh(r,v,rnd); return r; }
inline Mpfr sinh(const Mpfr& v) { Mpfr r; mpfr_sinh(r,v,rnd); return r; }
inline Mpfr tanh(const Mpfr& v) { Mpfr r; mpfr_tanh(r,v,rnd); return r; }


//=========================================================================
// MathsFloat - A mathematical number (as if there were any other sort)
//
class MathsFloat : public scx::ScriptNum {
public:

  MathsFloat(MathsModule* module,
      const Mpfr& value = 0.0);

  MathsFloat(MathsModule* module, const scx::ScriptObject* value);
  MathsFloat(const MathsFloat& c);

  virtual ~MathsFloat();

  static scx::ScriptObject* create(MathsModule* module, 
				   const scx::ScriptRef* args);
  virtual scx::ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;
  virtual double get_real() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  const Mpfr& get_value() const;
  
  std::string to_string(unsigned int dp) const;
  
  typedef scx::ScriptRefTo<MathsFloat> Ref;

protected:

  scx::ScriptRef* mean_list(const scx::ScriptList* l);

  scx::ScriptRefTo<MathsModule> m_module;
  Mpfr m_value;

};

#endif
