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

#ifndef SSLChannel_h
#define SSLChannel_h

#include <sconex/Stream.h>
#include <sconex/Module.h>
#include <sconex/ScriptBase.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>

class SSLModule;

//=========================================================================
// SSLChannel - A wrapper for an OpenSSL context
//
class SSLChannel : public scx::ScriptObject {
public:

  SSLChannel(SSLModule& mod,
	     const std::string& name,
	     bool client);

  ~SSLChannel();

  // Create an SSL connection object for this channel
  SSL* new_ssl();

  // Modify an existing SSL connection object to use this channel
  void change_ssl(SSL* ssl);
  
  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<SSLChannel> Ref;

protected:

private:

  SSLModule& m_mod;
  
  std::string m_name;

  SSL_CTX* m_ctx;
  
};

#endif
