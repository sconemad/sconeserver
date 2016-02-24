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

#include "CryptoDigests.h"
#include "SSLModule.h"

// These were renamed in OpenSSL 1.1
#ifndef EVP_MD_CTX_new
#define EVP_MD_CTX_new EVP_MD_CTX_create
#endif
#ifndef EVP_MD_CTX_free
#define EVP_MD_CTX_free EVP_MD_CTX_destroy
#endif

#include <sconex/ScriptTypes.h>
#include <sconex/Log.h>

//=============================================================================
CryptoDigests::CryptoDigests(SSLModule* module,
                             const std::string& method) 
  : Digest(method),
    m_module(module),
    m_md(0),
    m_ctx(0)
{
  m_md = EVP_get_digestbyname(method.c_str());
  if (!m_md) {
    DEBUG_LOG("Failed to create digest");
    return;
  }
  m_ctx = EVP_MD_CTX_new();
  if (!m_ctx) {
    DEBUG_LOG("Failed to create digest context");
    return;
  }
  EVP_DigestInit_ex(m_ctx, m_md, 0);
}

//=============================================================================
CryptoDigests::CryptoDigests(const CryptoDigests& c)
  : Digest(c),
    m_module(c.m_module),
    m_md(c.m_md),
    m_ctx(0)
{
  m_ctx = EVP_MD_CTX_new();
  if (!m_ctx) {
    DEBUG_LOG("Failed to create digest context");
    return;
  }
  if (c.m_ctx) EVP_MD_CTX_copy(m_ctx, c.m_ctx);
}

//=============================================================================
scx::ScriptObject* CryptoDigests::new_copy() const
{
  return new CryptoDigests(*this);
}

//=============================================================================
CryptoDigests::~CryptoDigests()
{
  if (m_ctx) EVP_MD_CTX_free(m_ctx);
}

//=============================================================================
void CryptoDigests::update(const void* buffer, int n)
{
  if (!m_ctx) return;
  EVP_DigestUpdate(m_ctx, buffer, n);
}

//=============================================================================
void CryptoDigests::finish()
{
  if (!m_ctx) return;
  m_digest.resize(EVP_MAX_MD_SIZE);
  unsigned int len = 0;
  EVP_DigestFinal(m_ctx, (unsigned char*)m_digest.tail(), &len);
  m_digest.push(len);

  if (m_ctx) {
    EVP_MD_CTX_free(m_ctx);
    m_ctx = 0;
  }
}
