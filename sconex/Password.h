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

#ifndef scxPassword_h
#define scxPassword_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
#include <sconex/Provider.h>
namespace scx {

class StandardPasswordHashes;

//=============================================================================
// PasswordHash - A hash method for password verification
//
class SCONEX_API PasswordHash : public ScriptObject {
public:

  // Create a hash method of the specified type
  static PasswordHash* create(const std::string& type,
			      const ScriptRef* args);

  PasswordHash();
  virtual ~PasswordHash();

  // Verify the password is correct
  // (i.e. it hashes to the same value as the supplied hash)
  // rehash_req is set to true if the password needs re-hashing, i.e. this 
  // hash method supports a more secure version of the hashed form.
  virtual bool verify(const std::string& password,
		      const std::string& hash,
		      bool& rehash_req) = 0;

  // Generate a hashed entry for the password
  virtual std::string rehash(const std::string& password) = 0;

  // Interface for registering new hash methods
  static void register_method(const std::string& type,
			      scx::Provider<PasswordHash>* factory);
  static void unregister_method(const std::string& type,
				scx::Provider<PasswordHash>* factory);

  typedef ScriptRefTo<PasswordHash> Ref;

 protected:

  PasswordHash(const PasswordHash& original);
  PasswordHash& operator=(const PasswordHash& rhs);
  // Prohibit copy

private:

  static void init();

  static ProviderScheme<PasswordHash>* s_providers;
  static StandardPasswordHashes* s_standard_provider;
};


//=============================================================================
// PasswordHashCrypt - Password hashing method using crypt
//
class SCONEX_API PasswordHashCrypt : public PasswordHash {
public:

  PasswordHashCrypt(const std::string& type,
		    const ScriptRef* args);
  
  virtual ~PasswordHashCrypt();

  virtual std::string get_string() const;
  
  virtual bool verify(const std::string& password,
		      const std::string& hash,
		      bool& rehash_req);

  virtual std::string rehash(const std::string& password);

  static std::string crypt(const std::string& password,
			   const std::string& hash);

 protected:

  PasswordHashCrypt(const PasswordHashCrypt& original);
  PasswordHashCrypt& operator=(const PasswordHashCrypt& rhs);
  // Prohibit copy

private:

  std::string m_type;
  std::string m_code;

};

};

#endif
