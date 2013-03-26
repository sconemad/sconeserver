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

#include <sconex/ModuleInterface.h>
#include <sconex/StreamDebugger.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Log.h>

#include <openssl/ssl.h>

SCONEX_MODULE(SSLModule);

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
  scx::Stream::register_stream("ssl",this);
}

//=========================================================================
SSLModule::~SSLModule()
{  
  scx::Stream::unregister_stream("ssl",this);

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
  oss << "Secure socket layer\n"
      << "Using " << SSLeay_version(SSLEAY_VERSION);
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
bool SSLModule::close()
{
  if (!scx::Module::close()) return false;

  for (ChannelMap::const_iterator it = m_channels.begin();
       it != m_channels.end();
       it++) {
    delete it->second;
  }
  m_channels.clear();
  return true;
}

//=========================================================================
SSLChannel* SSLModule::find_channel(const std::string& name)
{
  ChannelMap::const_iterator it = m_channels.find(name);
  
  if (it != m_channels.end()) {
    return it->second->object();
  }
  
  return 0;
}

//=============================================================================
SSLChannel* SSLModule::lookup_channel_for_host(const std::string& host)
{
  HostMap::const_iterator it = m_hostmap.find(host);
  if (it != m_hostmap.end()) {
    return find_channel(it->second);
  }
  return 0;
}

//=============================================================================
scx::ScriptRef* SSLModule::script_op(const scx::ScriptAuth& auth,
				     const scx::ScriptRef& ref,
				     const scx::ScriptOp& op,
				     const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
	"remove" == name ||
        "map" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("channels" == name) {
      scx::ScriptList* list = new scx::ScriptList();
      for (ChannelMap::const_iterator it = m_channels.begin();
	   it != m_channels.end();
	   ++it) {
	list->give(it->second->ref_copy(ref.reftype()));
      }
      return new scx::ScriptRef(list);
    }

    // Sub-objects
    SSLChannel* channel = find_channel(name);
    if (channel) {
      return new scx::ScriptRef(channel);
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SSLModule::script_method(const scx::ScriptAuth& auth,
					 const scx::ScriptRef& ref,
					 const std::string& name,
					 const scx::ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");
  
  if ("add" == name) {
    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    bool b_client = false;
    const scx::ScriptInt* a_client = 
      scx::get_method_arg<scx::ScriptInt>(args,1,"client");
    if (a_client) b_client = (0 != a_client->get_int());

    // Check channel doesn't already exist
    if (find_channel(s_name))
      return scx::ScriptError::new_ref("Channel '" + s_name + "' exists");

    SSLChannel* channel = new SSLChannel(*this,s_name,b_client);
    m_channels[s_name] = new SSLChannel::Ref(channel);

    return new SSLChannel::Ref(channel);
  }
  
  if ("remove" == name) {
    const scx::ScriptString* a_name = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_name) 
      return scx::ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    // Find and remove channel
    ChannelMap::iterator it = m_channels.find(s_name);
    if (it == m_channels.end())
      return scx::ScriptError::new_ref("Channel '" + s_name + 
				       "' does not exist");

    delete it->second;
    m_channels.erase(it);

    return 0;
  }

  if ("map" == name) {
    const scx::ScriptString* a_host = 
      scx::get_method_arg<scx::ScriptString>(args,0,"host");
    if (!a_host) 
      return scx::ScriptError::new_ref("Host pattern must be specified");
    std::string s_host = a_host->get_string();

    const scx::ScriptString* a_channel = 
      scx::get_method_arg<scx::ScriptString>(args,1,"channel");
    if (!a_channel) 
      return scx::ScriptError::new_ref("Channel name must be specified");
    std::string s_channel = a_channel->get_string();

    m_hostmap[s_host] = s_channel;
  }
  
  return scx::Module::script_method(auth,ref,name,args);
}

//=========================================================================
void SSLModule::provide(const std::string& type,
			const scx::ScriptRef* args,
			scx::Stream*& object)
{
  const scx::ScriptString* channel =
    scx::get_method_arg<scx::ScriptString>(args,0,"channel");
  if (!channel) {
    scx::Log("ssl").submit("No SSL channel specified, aborting connection");
    return;
  }

  object = new SSLStream(this,channel->get_string());
}
