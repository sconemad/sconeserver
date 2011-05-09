/* SconeServer (http://www.sconemad.com)

Server module

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

#ifndef scxServerModule_h
#define scxServerModule_h

#include <sconex/Module.h>
#include <sconex/Stream.h>
#include "ConnectionChain.h"


//=============================================================================
// ServerModule - Handles listening for incoming connections and building the
// appropriate stream chains to handle these.
//
class ServerModule : public scx::Module,
                     public scx::Provider<scx::Stream> {
public:

  ServerModule();
  virtual ~ServerModule();

  virtual std::string info() const;

  virtual bool connect(scx::Descriptor* endpoint,
		       const scx::ScriptRef* args);

  void add(const std::string& name,ConnectionChain* c);
  ConnectionChain::Ref* find(const std::string& name);

  // ScriptObject methods  
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // Provider<scx::Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);

private:

  typedef HASH_TYPE<std::string,ConnectionChain::Ref*> ConnectionChainMap;
  ConnectionChainMap m_chains;
  
};
  
#endif
