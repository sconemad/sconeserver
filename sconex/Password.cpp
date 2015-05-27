/* SconeServer (http://www.sconemad.com)

Password hashing methods

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/Password.h>
#include <sconex/ScriptTypes.h>
#include <sconex/utils.h>
#include <sconex/Mutex.h>
namespace scx {

const char* DEFAULT_HASH = "sha512_crypt";

ProviderScheme<PasswordHash>* PasswordHash::s_providers = 0;
StandardPasswordHashes* PasswordHash::s_standard_provider = 0;

//=============================================================================
// StandardPasswordHashes - provider for standard password hashes
//
class StandardPasswordHashes : public Provider<PasswordHash> {
public:

  StandardPasswordHashes() {
    PasswordHash::register_method("sha512_crypt", this);
    PasswordHash::register_method("sha256_crypt", this);
    PasswordHash::register_method("md5_crypt", this);
  }

  virtual void provide(const std::string& type,
		       const ScriptRef* args,
		       PasswordHash*& object) {
    object = new PasswordHashCrypt(type,args);
  }
};


//=============================================================================
PasswordHash* PasswordHash::create(const std::string& type, 
				   const ScriptRef* args)
{
  init();
  return s_providers->provide(type.empty() ? DEFAULT_HASH : type, args);
}

//=============================================================================
PasswordHash::PasswordHash()
{

}

//=============================================================================
PasswordHash::~PasswordHash()
{

}

//=============================================================================
void PasswordHash::register_method(const std::string& type,
				   Provider<PasswordHash>* factory)
{
  s_providers->register_provider(type,factory);
}

//=============================================================================
void PasswordHash::unregister_method(const std::string& type,
				     Provider<PasswordHash>* factory)
{
  s_providers->unregister_provider(type,factory);
}


//=============================================================================
void PasswordHash::init()
{
  if (!s_providers) {
    s_providers = new ProviderScheme<PasswordHash>();
    s_standard_provider = new StandardPasswordHashes();
  }
}
  
//=============================================================================
PasswordHashCrypt::PasswordHashCrypt(
  const std::string& type,
  __attribute__((unused)) const scx::ScriptRef* args
)
  : m_type(type)
{
  DEBUG_COUNT_CONSTRUCTOR(PasswordHashCrypt);

  if (type == "sha512_crypt") 
    m_code = "$6$";
  else if (type == "sha256_crypt") 
    m_code = "$5$";
  else if (type == "md5_crypt")
    m_code = "$1$";
}

//=============================================================================
PasswordHashCrypt::~PasswordHashCrypt()
{
  DEBUG_COUNT_DESTRUCTOR(PasswordHashCrypt);
}

//=============================================================================
std::string PasswordHashCrypt::get_string() const
{
  return m_type;
}

//=============================================================================
bool PasswordHashCrypt::verify(const std::string& password,
			       const std::string& hash,
			       bool& rehash_req)
{
  // Flag that a rehash required if the hash does not start with our code
  rehash_req = (0 != hash.compare(0,m_code.length(),m_code,
				  0,m_code.length()));

  return (hash == crypt(password,hash));
}

//=============================================================================
std::string PasswordHashCrypt::rehash(const std::string& password)
{
  static const char* salt_chars = 
    "./0123456789ABCD"
    "EFGHIJKLMNOPQRST"
    "UVWXYZabcdefghij"
    "klmnopqrstuvwxyz";

  std::ostringstream oss;
  oss << m_code << scx::random_string(16,salt_chars) << "$";
  return crypt(password, oss.str());
}

//=============================================================================
std::string PasswordHashCrypt::crypt(const std::string& password,
				     const std::string& hash)
{
#ifdef HAVE_CRYPT_R
  struct crypt_data data;
  memset(&data,0,sizeof(data));
  data.initialized = 0;
  return ::crypt_r(password.c_str(), hash.c_str(), &data);

#else
  //NOTE: crypt_r not available, protect non-reenterant crypt call with mutex
  static Mutex* mutex = new Mutex();
  MutexLocker locker(*mutex);
  return ::crypt(password.c_str(), hash.c_str());
#endif
}


};
