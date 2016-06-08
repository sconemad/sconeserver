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

#ifndef mathsInt_h
#define mathsInt_h

#include <sconex/ScriptTypes.h>

#include <gmp.h>

class MathsModule;

//=========================================================================
// MathsInt - A mathematical integer (as if there were any other sort)
//
class MathsInt : public scx::ScriptNum {
public:

  // Convenience method to create a new ScriptRef to a new MathsInt
  static scx::ScriptRef* new_ref(MathsModule* module, mpz_t value);
  
  MathsInt(MathsModule* module);
  MathsInt(MathsModule* module, const std::string& value);
  MathsInt(MathsModule* module, long value);
  MathsInt(MathsModule* module, double value);
  MathsInt(MathsModule* module, const mpz_t value);
  MathsInt(MathsModule* module, const scx::ScriptObject* value);
  MathsInt(const MathsInt& c);

  virtual ~MathsInt();

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

  const mpz_t& get_value() const;
  
  std::string to_string(int base, int group) const;
  
  typedef scx::ScriptRefTo<MathsInt> Ref;

protected:

  scx::ScriptRef* gcd_list(const scx::ScriptList* l);
  scx::ScriptRef* lcm_list(const scx::ScriptList* l);
  
  scx::ScriptRefTo<MathsModule> m_module;
  mpz_t m_value;

};

#endif
