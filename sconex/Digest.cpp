/* SconeServer (http://www.sconemad.com)

Message Digest calculation

Copyright (c) 2000-2016 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/Digest.h>
#include <sconex/ScriptTypes.h>
#include <sconex/utils.h>
namespace scx {

const char* DEFAULT_DIGEST = "SHA1";
  
ProviderScheme<Digest>* Digest::s_providers = 0;

//=============================================================================
Digest* Digest::create(const std::string& type, 
                       const ScriptRef* args)
{
  init();
  return s_providers->provide(type.empty() ? DEFAULT_DIGEST : type, args);
}

//=============================================================================
Digest::~Digest()
{
}

//=============================================================================
const Buffer& Digest::get_digest() const
{
  return m_digest;
}

//=============================================================================
std::string Digest::get_digest_string() const
{
  std::ostringstream oss;
  for (const unsigned char* c = (const unsigned char*)m_digest.head();
       c < (const unsigned char*)m_digest.tail(); ++c) {
    oss << std::setw(2) << std::setfill('0') << std::hex << (int)(*c);
  }
  return oss.str();
}
  
//=============================================================================
void Digest::register_method(const std::string& type,
                             Provider<Digest>* factory)
{
  init();
  s_providers->register_provider(type,factory);
}

//=============================================================================
void Digest::unregister_method(const std::string& type,
                               Provider<Digest>* factory)
{
  init();
  s_providers->unregister_provider(type,factory);
}

//=============================================================================
std::string Digest::get_string() const
{
  return m_method + ":" + get_digest_string();
}

//=============================================================================
int Digest::get_int() const
{
  return m_digest.used();
}
  
//=============================================================================
ScriptRef* Digest::script_op(const ScriptAuth& auth,
                             const ScriptRef& ref,
                             const ScriptOp& op,
                             const ScriptRef* right)
{
  if (op.type() == ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    if ("method" == name)
      return ScriptString::new_ref(m_method);
    if ("digest" == name)
      return ScriptString::new_ref(get_digest_string());
    
    // Methods
    if ("update" == name ||
	"finish" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* Digest::script_method(const ScriptAuth& auth,
                                 const ScriptRef& ref,
                                 const std::string& name,
                                 const ScriptRef* args)
{
  if ("update" == name) {
    const ScriptString* a_data = 
      get_method_arg<ScriptString>(args,0,"data");
    if (!a_data) return ScriptError::new_ref("No data specified");
    std::string data = a_data->get_string();
    update(data.c_str(), data.length());
    return 0;
  }

  if ("finish" == name) {
    finish();
    return 0;
  }
 
  return ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
void Digest::init()
{
  if (!s_providers) {
    s_providers = new ProviderScheme<Digest>();
  }
}

//=============================================================================
Digest::Digest(const std::string& method)
  : m_method(method),
    m_digest(0)
{
}

//=============================================================================
Digest::Digest(const Digest& c)
  : m_method(c.m_method),
    m_digest(c.m_digest)
{
}
  
};
