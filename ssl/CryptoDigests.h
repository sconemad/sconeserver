/* SconeServer (http://www.sconemad.com)

Interface to OpenSSL/libcrypto message digests

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

#ifndef sslCryptoDigests_h
#define sslCryptoDigests_h

#include <sconex/Digest.h>

// Includes from OpenSSL shared library
#include <openssl/evp.h>

class SSLModule;

//=============================================================================
class CryptoDigests : public scx::Digest {
public:

  CryptoDigests(SSLModule* module,
                const std::string& method);
  CryptoDigests(const CryptoDigests& c);
  virtual ~CryptoDigests();

  virtual scx::ScriptObject* new_copy() const;
  
  // scx::Digest interface:
  virtual void update(const void* buffer, int n);
  virtual void finish();

protected:
  
  scx::ScriptRefTo<SSLModule> m_module;
  const EVP_MD* m_md;
  EVP_MD_CTX* m_ctx;

};

#endif
