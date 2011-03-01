/* SconeServer (http://www.sconemad.com)

Service provider API

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

#include "sconex/sconex.h"
#include "sconex/Arg.h"

namespace scx {

//=============================================================================
template<class T> class Provider {
public:

  virtual T* create_new(const ArgMap& args) = 0;
  
};
  
//=============================================================================
template<class T> class SCONEX_API ProviderScheme {
public:
  
  typedef Provider<T> ProviderType;
  typedef std::map<std::string, ProviderType*> ProviderMap;
  
  T* create_new(const std::string& type, const ArgMap& args);
  
  void register_provider(const std::string& type, ProviderType* factory);
  
  void unregister_provider(const std::string& type);

protected:

  ProviderMap m_providers;

};

//=============================================================================
template<class T> T* ProviderScheme<T>::create_new(
  const std::string& type,
  const ArgMap& args
)
{
  typename ProviderMap::iterator it = m_providers.find(type);

  if (it == m_providers.end()) {
    return 0;
  }
  return it->second->create_new(args);
}

//=============================================================================
template<class T> void ProviderScheme<T>::register_provider(
  const std::string& type,
  Provider<T>* factory
)
{
  m_providers[type] = factory;
}

//=============================================================================
template<class T> void ProviderScheme<T>::unregister_provider(
  const std::string& type
)
{
  typename ProviderMap::iterator it = m_providers.find(type);
  if (it != m_providers.end()) {
    m_providers.erase(it);
  }
}

};
#endif
