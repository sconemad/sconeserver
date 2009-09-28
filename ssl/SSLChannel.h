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

#ifndef SSLChannel_h
#define SSLChannel_h

#include "sconex/Stream.h"
#include "sconex/Module.h"
#include "sconex/ArgObject.h"

#include <openssl/crypto.h>
#include <openssl/ssl.h>

class SSLModule;

//#########################################################################
class SSLChannel : public scx::ArgObjectInterface {

public:

  SSLChannel(
    SSLModule& mod,
    const std::string& name
  );

  ~SSLChannel();

  SSL* new_ssl();
  
  virtual std::string name() const;
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);

protected:

private:

  SSLModule& m_mod;
  
  std::string m_name;

  SSL_CTX* m_ctx;
  
};

#endif
