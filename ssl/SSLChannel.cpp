/* SconeServer (http://www.sconemad.com)

SSL Channel (wraps an OpenSSL context)

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/ConfigFile.h"

#include "SSLChannel.h"
#include "SSLModule.h"

//=========================================================================
SSLChannel::SSLChannel(
  SSLModule& mod,
  const std::string& name
)
  : m_mod(mod),
    m_name(name)
{
  m_ctx = SSL_CTX_new( SSLv23_server_method() );

  DEBUG_ASSERT(0 != m_ctx,"SSLChannel() Bad SSL context");
}

//=========================================================================
SSLChannel::~SSLChannel()
{
  SSL_CTX_free(m_ctx);
}

//=========================================================================
SSL* SSLChannel::new_ssl()
{
  return SSL_new(m_ctx);
}

//=============================================================================
std::string SSLChannel::name() const
{
  return std::string("CHANNEL:") + m_name;
}

//=============================================================================
scx::Arg* SSLChannel::arg_lookup(
  const std::string& name
)
{
  // Methods
  if ("load_key" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
scx::Arg* SSLChannel::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (!auth.admin()) return new scx::ArgError("Not permitted");

  if ("load_key" == name) {

    const scx::ArgString* a_key =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_key) {
      return new scx::ArgError("load_key() Key name must be supplied");
    }
    scx::FilePath key = m_mod.get_conf_path() + a_key->get_string();

    const scx::ArgString* a_cert =
      dynamic_cast<const scx::ArgString*>(l->get(1));
    scx::FilePath cert = m_mod.get_conf_path();
    if (a_cert) {
      cert = cert + a_cert->get_string();
    } else {
      cert = cert + std::string(a_key->get_string() + ".pub");
    }

    if (SSL_CTX_use_certificate_file(
          m_ctx,
          cert.path().c_str(),
          SSL_FILETYPE_PEM) <= 0) {
      return new scx::ArgError("load_key() Error loading certificate");
    }
    
    if (SSL_CTX_use_PrivateKey_file(
          m_ctx,
          key.path().c_str(),
          SSL_FILETYPE_PEM) <= 0) {
      return new scx::ArgError("load_key() Error loading private key");
    }
    
    if (!SSL_CTX_check_private_key(m_ctx)) {
      return new scx::ArgError("load_key() Loaded keys do not match");
    }
    
    return 0;
  }
  
  return SCXBASE ArgObjectInterface::arg_function(auth,name,args);
}
