/* SconeServer (http://www.sconemad.com)

SSL connection module

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


#include "SSLModule.h"
#include "SSLStream.h"
#include "SSLChannel.h"

#include "sconex/ModuleInterface.h"
#include "sconex/StreamDebugger.h"
#include "sconex/Arg.h"

#include <openssl/ssl.h>

SCONESERVER_MODULE(SSLModule);

//=========================================================================
// Thread synchronization data and callbacks required by OpenSSL
//
static pthread_mutex_t lock_cs[CRYPTO_NUM_LOCKS];
static long lock_count[CRYPTO_NUM_LOCKS];

unsigned long pthreads_thread_id()
{
  return (unsigned long)pthread_self();
}

void pthreads_locking_callback(int mode, int type, const char* file, int line)
{
#ifdef undef
  //SSL thread debug info:
  fprintf(stderr,"thread=%4d mode=%s lock=%s %s:%d\n",
          CRYPTO_thread_id(),
          (mode&CRYPTO_LOCK)?"l":"u",
          (type&CRYPTO_READ)?"r":"w",file,line);
#endif

  if (mode & CRYPTO_LOCK) {
    pthread_mutex_lock(&(lock_cs[type]));
    lock_count[type]++;
  } else {
    pthread_mutex_unlock(&(lock_cs[type]));
  }
}

//=========================================================================
SSLModule::SSLModule()
  : scx::Module("ssl",scx::version())
{

}

//=========================================================================
SSLModule::~SSLModule()
{  
  for (ChannelMap::const_iterator it = m_channels.begin();
       it != m_channels.end();
       it++) {
    delete it->second;
  }

  CRYPTO_set_locking_callback(NULL);
  for (int i=0; i<CRYPTO_NUM_LOCKS; i++) {
    pthread_mutex_destroy(&(lock_cs[i]));
    //    fprintf(stderr,"%8ld:%s\n",lock_count[i],CRYPTO_get_lock_name(i));
  }
}

//=========================================================================
std::string SSLModule::info() const
{
  std::ostringstream oss;
  oss << "Copyright (c) 2000-2009 Andrew Wedgbury\n"
      << "Secure socket layer\n"
      << "Using " << SSLeay_version(SSLEAY_VERSION) << "\n";
  return oss.str();
}

//=========================================================================
int SSLModule::init()
{
  SSL_load_error_strings();
  SSL_library_init();

  for (int i=0; i<CRYPTO_NUM_LOCKS; i++) {
    lock_count[i]=0;
    pthread_mutex_init(&(lock_cs[i]),NULL);
  }

  CRYPTO_set_id_callback((unsigned long (*)())pthreads_thread_id);
  CRYPTO_set_locking_callback(pthreads_locking_callback);
  
  return Module::init();
}

//=========================================================================
bool SSLModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgString* channel =
    dynamic_cast<const scx::ArgString*>(args->get(0));
  if (!channel) {
    log("No SSL channel specified, aborting connection");
    return false;
  }

  SSLStream* s = new SSLStream(*this,channel->get_string());
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}

//=========================================================================
SSLChannel* SSLModule::find_channel(const std::string& name)
{
  ChannelMap::const_iterator it = m_channels.find(name);
  
  if (it != m_channels.end()) {
    return it->second;
  }
  
  return 0;
}

//=============================================================================
scx::Arg* SSLModule::arg_lookup(const std::string& name)
{
  // Methods

  if ("add" == name ||
      "remove" == name) {
    return new scx::ArgObjectFunction(
      new scx::ArgModule(ref()),name);
  }      

  // Properties
  
  if ("list" == name) {
    std::ostringstream oss;
    for (ChannelMap::const_iterator it = m_channels.begin();
	 it != m_channels.end();
	 ++it) {
      oss << it->first << "\n";
    }
    return new scx::ArgString(oss.str());
  }

  // Sub-objects
  
  SSLChannel* channel = find_channel(name);
  if (channel) {
    return new scx::ArgObject(channel);
  }
  
  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* SSLModule::arg_function(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (!auth.admin()) return new scx::ArgError("Not permitted");
  
  if ("add" == name) {
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("ssl::add() Name must be specified");
    }

    // Check channel doesn't already exist
    if (find_channel(s_name)) {
      return new scx::ArgError("ssl::add() Channel '" + s_name +
                               "' already exists");
    }

    m_channels[s_name] = new SSLChannel(*this,s_name);
    return 0;
  }
  
  if ("remove" == name) {
    std::string s_name;
    const scx::ArgString* a_name =
      dynamic_cast<const scx::ArgString*>(l->get(0));
    if (a_name) {
      s_name = a_name->get_string();
    } else {
      return new scx::ArgError("ssl::remove() Name must be specified");
    }

    // Remove channel
    SSLChannel* channel = find_channel(s_name);
    if (!channel) {
      return new scx::ArgError("ssl::remove() Channel '" + s_name +
                               "' does not exist");
    }
      
    delete channel;
    m_channels.erase(s_name);
    return 0;
  }

  return SCXBASE Module::arg_function(auth,name,args);
}
