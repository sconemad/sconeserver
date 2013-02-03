/* SconeServer (http://www.sconemad.com)

Connection chain

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

#ifndef ConnectionChain_h
#define ConnectionChain_h

#include <sconex/ScriptBase.h>

namespace scx { class Descriptor; class Module; };
class ConnectionNode;
class ServerModule;

//=============================================================================
// ConnectionChain - This is essentially a list of ConnectionNode, which are
// applied in turn to an incoming descriptor in order to setup the required 
// streams.
//
class ConnectionChain : public scx::ScriptObject {
public:

  ConnectionChain(ServerModule* module,
		  const std::string& name);

  virtual ~ConnectionChain();

  bool connect(
    scx::Descriptor* d
  );
  
  void add(ConnectionNode* n);

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
  
  virtual std::string get_log_context() const;
  
  typedef scx::ScriptRefTo<ConnectionChain> Ref;
  
protected:

  scx::ScriptRefTo<ServerModule> m_module;

  std::string m_name;
  
  typedef std::list<ConnectionNode*> ConnectionNodeList;
  ConnectionNodeList m_nodes;
  
private:

};

//=============================================================================
// ConnectionNode - An individual setup item in a connection chain, this holds
// the name of the stream type to use and any associated arguments.
//
class ConnectionNode {
public:
  
  ConnectionNode(ServerModule* module,
		 const std::string& type,
		 scx::ScriptRef* args);
  
  ~ConnectionNode();

  bool connect(scx::Descriptor* d);

  const std::string& get_name() const;
  const scx::ScriptRef* get_args() const;
  
protected:

  scx::ScriptRefTo<ServerModule> m_module;
  
  std::string m_type;
  scx::ScriptRef* m_args;

};

#endif
