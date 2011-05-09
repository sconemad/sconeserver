/* SconeServer (http://www.sconemad.com)

Provider API templates

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

#ifndef scxProvider_h
#define scxProvider_h

#include <sconex/sconex.h>
#include <sconex/utils.h>
#include <sconex/ScriptBase.h>

// Uncomment to enable debug info
//#define Provider_DEBUG_LOG(m) DEBUG_LOG(type_name(typeid(*this))+"::"+m)

#ifndef Provider_DEBUG_LOG
#  define Provider_DEBUG_LOG(m)
#endif
  
namespace scx {

class ScriptRef;

//=============================================================================
// Provider - A class which can provide an implementation of a class T.
//
template<class T> class Provider {
public:

  // This method should return a new instance of class T in object, using
  // the specified arguments as required. The type parameter can be used to 
  // distinguish between different types if more than one was registered
  // using the same provider.
  //
  virtual void provide(const std::string& type,
		       const ScriptRef* args,
		       T*& object) = 0;
  
};
  
//=============================================================================
// ProviderScheme - A factory class which manages providers of class T.
//
template<class T> class SCONEX_API ProviderScheme {
public:

  typedef Provider<T> ProviderType;
  typedef std::map<std::string, ProviderType*> ProviderMap;
  
  T* provide(const std::string& type, const ScriptRef* args);
  
  void register_provider(const std::string& type, ProviderType* factory);
  
  void unregister_provider(const std::string& type, ProviderType* factory);

protected:

  ProviderMap m_providers;

};

//=============================================================================
template<class T> T* ProviderScheme<T>::provide(
  const std::string& type,
  const ScriptRef* args
)
{
  typename ProviderMap::iterator it = m_providers.find(type);
  Provider_DEBUG_LOG("provide("+type+","+
		     (args ? args->object()->get_string() : "NULL") +")");

  if (it == m_providers.end()) {
    return 0;
  }

  T* object = 0;
  it->second->provide(type,args,object);
  return object;
}

//=============================================================================
template<class T> void ProviderScheme<T>::register_provider(
  const std::string& type,
  Provider<T>* factory
)
{
  Provider_DEBUG_LOG("register_provider("+type+")");
  typename ProviderMap::iterator it = m_providers.find(type);
  if (it != m_providers.end()) {
    DEBUG_LOG("Type '"+type+"' already has a registered provider");
  } else {
    m_providers[type] = factory;
  }
}

//=============================================================================
template<class T> void ProviderScheme<T>::unregister_provider(
  const std::string& type,
  ProviderType* factory)
{
  Provider_DEBUG_LOG("unregister_provider("+type+")");
  typename ProviderMap::iterator it = m_providers.find(type);
  if (it != m_providers.end()) {
    if (it->second != factory) {
      DEBUG_LOG("Type '"+type+"' provider does not match already registered");
    } else {
      m_providers.erase(it);
    }
  }
}

};
#endif
