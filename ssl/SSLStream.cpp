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

#include "SSLStream.h"
#include "SSLModule.h"
#include "SSLChannel.h"

#include <sconex/StreamSocket.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Log.h>

// Uncomment to enable debug info for the SSL Stream
//#define SSLStream_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef SSLStream_DEBUG_LOG
#  define SSLStream_DEBUG_LOG(m)
#endif

// Uncomment to enable debug info for the OpenSSL BIO interface
//#define scxsp_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef scxsp_DEBUG_LOG
#  define scxsp_DEBUG_LOG(m)
#endif

#define LOG(msg) scx::Log("ssl").                               \
  attach("id",scx::ScriptInt::new_ref(endpoint().uid())).       \
  submit(msg);

BIO *BIO_new_scxsp(SSLStream* scxsp,int close_flag);

//=============================================================================
SSLStream::SSLStream(SSLModule* module,
		     const std::string& channel) 
  : Stream("ssl"),
    m_module(module),
    m_channel(channel),
    m_client(false),
    m_ssl(0),
    m_seq(0),
    m_init_retries(0),
    m_last_read_cond(scx::Ok),
    m_last_write_cond(scx::Ok)
{

}

//=============================================================================
SSLStream::~SSLStream()
{
  if (m_ssl) {
    SSL_free(m_ssl);
  }
}

//=============================================================================
scx::Condition SSLStream::read(void* buffer,
			       int n,
			       int& na)
{
  if (m_seq == Start) {
    na=0;
    return scx::Ok;
  }

  na = SSL_read(m_ssl,(char*)buffer,n);
  int e = SSL_get_error(m_ssl,na);

  SSLStream_DEBUG_LOG("read() SSLStream::read(buff," << n << ") read " << na
                      << " error=" << e);

  if (na<0 || e == SSL_ERROR_ZERO_RETURN) {
    na=0;
    return scx::End;
  }

  if (na==0) {
    return m_last_read_cond;
  }

  return scx::Ok;
}

//=============================================================================
scx::Condition SSLStream::write(const void* buffer,
				int n,
				int& na)
{
  if (m_seq == Start) {
    na=0;
    return scx::Ok;
  }

  na = SSL_write(m_ssl,(char*)buffer,n);

  SSLStream_DEBUG_LOG("read() SSLStream::write(buff," << n << ") wrote "
                      << na);

  if (na<0) {
    na=0;
    return scx::Error;
  }

  if (na==0) {
    return m_last_write_cond;
  }

  return scx::Ok;
}


//=============================================================================
scx::Condition SSLStream::event(scx::Stream::Event e)
{
  switch (e) {
  
    case scx::Stream::Opening: { // OPENING
      if (m_seq == Start) {
	// Determine if this is a client connection by examining the socket
	scx::StreamSocket* socket = 
	  dynamic_cast<scx::StreamSocket*>(&endpoint());
	if (socket) {
	  m_client = socket->is_client();
	}
	if (m_client) {
	  enable_event(scx::Stream::Writeable,true);
	} else {
	  enable_event(scx::Stream::Readable,true);
	}
	// Set a 1 minute timeout for SSL connecting stage
	endpoint().set_timeout(scx::Time(60));
	return init_ssl();

      } else if (m_seq == Connecting) {
	return scx::Wait;

      } else { // Connected
	// Remove endpoint timeout
	endpoint().set_timeout(scx::Time(0));
      }
    } break;

    case scx::Stream::Closing: { // CLOSING
      SSL_free(m_ssl);
      m_ssl=0;
    } break;
    
    case scx::Stream::Readable: { // READABLE
      if (m_seq == Connecting) {
	return connect_ssl(e);
      }   
    } break;

    case scx::Stream::Writeable: { // WRITEABLE
      if (m_seq == Connecting) {
	return connect_ssl(e);
      }   
    } break;

    default:
      break;
  }  
    
  return scx::Ok;
}

//=============================================================================
scx::Condition SSLStream::init_ssl()
{
  m_last_read_cond=scx::Ok;
  m_last_write_cond=scx::Ok;
  
  SSLChannel* channel = m_module.object()->find_channel(m_channel);
  if (!channel) {
    LOG("init_ssl: Invalid SSL channel");
    return scx::Error;
  }
  m_ssl = channel->new_ssl();
  if (m_ssl == 0) {
    LOG("init_ssl: Invalid SSL object");
    return scx::Error;
  }
  
  m_bio = BIO_new_scxsp(this,0);
  SSL_set_bio(m_ssl,m_bio,m_bio);
  BIO_set_ssl(m_bio,m_ssl,BIO_CLOSE);
  BIO_set_nbio(m_bio,1);

  m_seq = Connecting;
  return scx::Wait;
}

//=============================================================================
scx::Condition SSLStream::connect_ssl(scx::Stream::Event e)
{
  int ret = 0;
  if (m_client) {
    // Client mode - we initiate connection
    ret = SSL_connect(m_ssl);
  } else {
    // Server mode - we accept connection
    ret = SSL_accept(m_ssl);
  }
  int err = SSL_get_error(m_ssl,ret);
        
  SSLStream_DEBUG_LOG("connect_ssl " << (m_client ? "client" : "server") 
		      << " attempt " << m_init_retries 
		      << " returned " << ret 
		      << " error " << err);
  
  if (++m_init_retries > 10) {
    DEBUG_LOG("connect_ssl aborted after 10 retries");
    return scx::Error;
  }
  
  if (err != 0) {
    if (e == scx::Stream::Writeable) {
      enable_event(scx::Stream::Writeable,false);
      enable_event(scx::Stream::Readable,true);
    }
    return scx::Wait;
  }
  
  SSLStream_DEBUG_LOG("Opened secure connection using " << SSL_get_cipher(m_ssl));

  m_seq = Connected;
  enable_event(scx::Stream::Opening,true);
  enable_event(scx::Stream::Readable,false);
  enable_event(scx::Stream::Writeable,false);
  return scx::Ok;
}

//=============================================================================
std::string SSLStream::stream_status() const
{
  std::ostringstream oss;
  oss << m_channel 
      << " seq:";
  switch (m_seq) {
  case Start: oss << "START"; break;
  case Connecting: oss << "CONNECTING"; break;
  case Connected: oss << "CONNECTED"; break;
  default: oss << "UNKNOWN!"; break;
  }
  if (m_ssl) {
    const SSL_CIPHER* cipher = SSL_get_current_cipher(m_ssl);
    if (cipher) {
      oss << " cipher:" << SSL_CIPHER_get_name(cipher);
    }
  }
  return oss.str();
}

//=============================================================================
void SSLStream::set_last_read_cond(scx::Condition c)
{
  m_last_read_cond=c;
}

//=============================================================================
void SSLStream::set_last_write_cond(scx::Condition c)
{
  m_last_write_cond=c;
}

//=============================================================================
int SSLStream::sni_callback(SSL *ssl, int *ad, void *arg)
{
  const char* host = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  SSLStream_DEBUG_LOG("SSL SNI hostname=" << (host ? host : "NULL"));
  if (host) {
    BIO* b = SSL_get_rbio(ssl);
    SSLStream* scxsp = (SSLStream*)b->ptr;
    scxsp->got_hostname(host);
  }
  return SSL_TLSEXT_ERR_OK;
}

//=============================================================================
void SSLStream::got_hostname(const std::string& host)
{
  SSLChannel* c = m_module.object()->lookup_channel_for_host(host);
  if (c) {
    std::string cn = c->get_string();
    if (cn != m_channel) {
      // Change to the new channel
      LOG("Changing channel to '" + cn +
          "' due to SNI hostname '" + host + "'");
      m_channel = cn;
      c->change_ssl(m_ssl);
    }
  }
}

//=============================================================================
// SSLStream BIO - OpenSSL callback interface
//=============================================================================
static int scxsp_write(BIO *h, const char *buf, int num);
static int scxsp_read(BIO *h, char *buf, int size);
static int scxsp_puts(BIO *h, const char *str);
static long scxsp_ctrl(BIO *h, int cmd, long arg1, void *arg2);
static int scxsp_new(BIO *h);
static int scxsp_free(BIO *data);

//=============================================================================
static BIO_METHOD methods_scxsp=
{
  BIO_TYPE_SOCKET,
  "Sconex pipe",
  scxsp_write,
  scxsp_read,
  scxsp_puts,
  0, /* scxsp_gets, */
  scxsp_ctrl,
  scxsp_new,
  scxsp_free,
  NULL,
};

//=============================================================================
BIO_METHOD *BIO_s_scxsp(void)
{
  return(&methods_scxsp);
}

//=============================================================================
BIO *BIO_new_scxsp(SSLStream* scxsp,int close_flag)
{
  BIO *b = BIO_new(BIO_s_scxsp());
  if (b==0) {
    return 0;
  }
  BIO_set_fd(b,1,close_flag);
  b->ptr=scxsp;
  return b;
}

//=============================================================================
static int scxsp_new(BIO *b)
{
  b->init=0;
  b->num=0;
  b->ptr=0;
  b->flags=0;

  scxsp_DEBUG_LOG("scxsp_new(" << (void*)b << ")");

  return 1;
}

//=============================================================================
static int scxsp_free(BIO *b)
{
  scxsp_DEBUG_LOG("scxsp_free(" << (void*)b << ")");

  return 0;
}

//=============================================================================
static int scxsp_read(BIO *b,char *buffer,int n)
{
  SSLStream* scxsp = (SSLStream*)b->ptr;
  DEBUG_ASSERT(buffer!=0,"scxsp_read() Invalid buffer");
  DEBUG_ASSERT(scxsp!=0,"scxsp_read() Invalid ptr");

  int na=0;
  scx::Condition c = scxsp->Stream::read((void*)buffer,n,na);

  scxsp_DEBUG_LOG("scxsp_read(" << (void*)b << ",buff,"<< n << ") read " << na);

  BIO_clear_retry_flags(b);
  scxsp->set_last_read_cond(c);
  if (c==scx::Ok) {

  } else if (c==scx::Wait) {
    BIO_set_retry_read(b);

  } else {
    na=-1;
  }

  return na;
}

//=============================================================================
static int scxsp_write(BIO *b, const char *buffer, int n)
{
  SSLStream* scxsp = (SSLStream*)b->ptr;
  DEBUG_ASSERT(buffer!=0,"scxsp_write() Invalid buffer");
  DEBUG_ASSERT(scxsp!=0,"scxsp_write() Invalid ptr");

  int na=0;
  scx::Condition c = scxsp->Stream::write((const void*)buffer,n,na);

  scxsp_DEBUG_LOG("scxsp_write(" << (void*)b << ",buff," << n << ") wrote "
                  << na);

  BIO_clear_retry_flags(b);
  scxsp->set_last_write_cond(c);
  if (c==scx::Ok) {

  } else if (c==scx::Wait) {
    BIO_set_retry_write(b);
    
  } else {
    na=-1;
  }

  return na;
}

//=============================================================================
static long scxsp_ctrl(BIO *b, int cmd, long num, void *ptr)
{
  long ret=1;
  int *ip;

  switch (cmd) {
  case BIO_CTRL_RESET:
    num=0;
  case BIO_C_FILE_SEEK:
    ret=0;
    break;
  case BIO_C_FILE_TELL:
  case BIO_CTRL_INFO:
    ret=0;
    break;
  case BIO_C_SET_FD:
    b->num= *((int *)ptr);
    b->shutdown=(int)num;
    b->init=1;
    break;
  case BIO_C_GET_FD:
    if (b->init) {
      ip=(int *)ptr;
      if (ip != NULL) {
        *ip=b->num;
      }
      ret=b->num;
    }
    else
      ret= -1;
    break;
  case BIO_CTRL_GET_CLOSE:
    ret=b->shutdown;
    break;
  case BIO_CTRL_SET_CLOSE:
    b->shutdown=(int)num;
    break;
  case BIO_CTRL_PENDING:
  case BIO_CTRL_WPENDING:
    ret=0;
    break;
  case BIO_CTRL_DUP:
    ret=1;
    break;

  case BIO_CTRL_FLUSH: {
//      SSLStream* scxsp = (SSLStream*)b->ptr;
//      DEBUG_ASSERT(scxsp!=0,"scxsp_read() Invalid ptr");
//      scxsp->Stream::flush();
//      ret = 1;
    }
    break;

  default:
    ret=0;
    break;
  }

  scxsp_DEBUG_LOG("scxsp_ctrl(" << (void*)b << "," << cmd << "," << num
                  << "," << (void*)ptr << ") returning " << ret);

  return(ret);
}

//=============================================================================
static int scxsp_puts(BIO *bp, const char *str)
{
  int n,ret;

  n=strlen(str);
  ret=scxsp_write(bp,str,n);
  return ret;
}


/*
//	printf("SSL connection using %s\n",SSL_get_cipher(m_ssl));
  // Get client's certificate (note: beware of dynamic allocation) - opt
  m_client_cert = SSL_get_peer_certificate(m_ssl);
  if (m_client_cert != 0) {
    char*    str;
    char     buf [4096];
    printf("Client certificate:\n");
    str = X509_NAME_oneline(X509_get_subject_name(m_client_cert),0,0);

    printf("\t subject: %s\n", str);
    delete str;

    str = X509_NAME_oneline(X509_get_issuer_name(m_client_cert),0,0);

    printf("\t issuer: %s\n", str);
    delete str;

    // We could do all sorts of certificate verification stuff here before
    // deallocating the certificate.
    X509_free(m_client_cert);
  } else {
    printf ("Client does not have certificate.\n");
  }
*/
