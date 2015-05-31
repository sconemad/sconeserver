/* SconeServer (http://www.sconemad.com)

SSL Channel

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

#include <sconex/ConfigFile.h>

#include "SSLChannel.h"
#include "SSLModule.h"
#include "SSLStream.h"
#include <sconex/ScriptTypes.h>
#include <sconex/Kernel.h>

//=========================================================================
SSLChannel::SSLChannel(SSLModule& mod,
		       const std::string& name,
		       bool client
) : m_mod(mod),
    m_name(name)
{
  m_parent = &m_mod;

  if (client) {
    m_ctx = SSL_CTX_new( SSLv23_client_method() );
    DEBUG_ASSERT(0 != m_ctx,"SSLChannel() Bad SSL context");
  } else {
    m_ctx = SSL_CTX_new( SSLv23_server_method() );
    DEBUG_ASSERT(0 != m_ctx,"SSLChannel() Bad SSL context");
    SSL_CTX_set_tlsext_servername_callback(m_ctx, SSLStream::sni_callback);
  }

  // Disallow old SSL protocols
  SSL_CTX_set_options(m_ctx, SSL_OP_NO_SSLv2);
  SSL_CTX_set_options(m_ctx, SSL_OP_NO_SSLv3);
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

//=========================================================================
void SSLChannel::change_ssl(SSL* ssl)
{
  SSL_set_SSL_CTX(ssl, m_ctx);
}

//=============================================================================
std::string SSLChannel::get_string() const
{
  return m_name;
}

//=============================================================================
scx::ScriptRef* SSLChannel::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("load_key" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SSLChannel::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  if ("load_key" == name) {

    const scx::ScriptString* a_key =
      scx::get_method_arg<scx::ScriptString>(args,0,"key");
    if (!a_key)
      return scx::ScriptError::new_ref("Key name must be supplied");
    scx::FilePath conf = scx::Kernel::get()->get_conf_path();
    scx::FilePath key = conf + scx::FilePath(a_key->get_string());

    const scx::ScriptString* a_cert =
      scx::get_method_arg<scx::ScriptString>(args,1,"cert");
    scx::FilePath cert = conf;
    if (a_cert) {
      cert = cert + scx::FilePath(a_cert->get_string());
    } else {
      cert = cert + scx::FilePath(std::string(a_key->get_string() + ".pub"));
    }

    if (SSL_CTX_use_certificate_file(
          m_ctx,
          cert.path().c_str(),
          SSL_FILETYPE_PEM) <= 0) {
      return scx::ScriptError::new_ref("Error loading certificate");
    }
    
    if (SSL_CTX_use_PrivateKey_file(
          m_ctx,
          key.path().c_str(),
          SSL_FILETYPE_PEM) <= 0) {
      return scx::ScriptError::new_ref("Error loading private key");
    }
    
    if (!SSL_CTX_check_private_key(m_ctx)) {
      return scx::ScriptError::new_ref("Loaded keys do not match");
    }
    
    return 0;
  }
  
  return scx::ScriptObject::script_method(auth,ref,name,args);
}
