/* SconeServer (http://www.sconemad.com)

SSL secure stream using OpenSSL

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

#ifndef sslStream_h
#define sslStream_h

#include "sconex/Stream.h"

// Includes from OpenSSL shared library
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

class SSLModule;

//=============================================================================
class SSLStream : public scx::Stream {

public:

  SSLStream(
    SSLModule& mod,
    const std::string& channel
  );

  virtual ~SSLStream();

  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);

  virtual scx::Condition event(scx::Stream::Event e);

  void set_last_read_cond(scx::Condition c);
  void set_last_write_cond(scx::Condition c);

protected:

  SSLModule& m_mod;
  std::string m_channel;
  
  SSL* m_ssl;
  X509* m_client_cert;
  BIO* m_bio;

  int m_seq;
  int m_init_retries;
  scx::Condition m_last_read_cond;
  scx::Condition m_last_write_cond;

private:

};

#endif
