/* SconeServer (http://www.sconemad.com)

HTTP (HyperText Transfer Protocol) Module

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

#ifndef httpModule_h
#define httpModule_h

#include <http/http.h>
#include <http/HostMapper.h>
#include <http/AuthRealm.h>
#include <http/Session.h>
#include <sconex/Module.h>
#include <sconex/Descriptor.h>
#include <sconex/Uri.h>
#include <sconex/Stream.h>

namespace http {

//=============================================================================
// HTTPModule - Implements a HyperText Transfer Protocol client and server.
//
class HTTP_API HTTPModule : public scx::Module,
                            public scx::Provider<scx::Stream>,
                            public scx::Provider<scx::ScriptObject> {
public:

  HTTPModule();
  virtual ~HTTPModule();

  virtual std::string info() const;

  virtual int init();
  virtual bool close();

  HostMapper& get_hosts();
  AuthRealmManager& get_realms();
  SessionManager& get_sessions();

  unsigned int get_idle_timeout() const;

  const scx::Uri& get_client_proxy() const;

  // ScriptObject methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // Provider<Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);

  // Provider<ScriptObject> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object);

  typedef scx::ScriptRefTo<HTTPModule> Ref;

protected:

private:

  HostMapper::Ref* m_hosts;
  AuthRealmManager::Ref* m_realms;
  SessionManager::Ref* m_sessions;

  unsigned int m_idle_timeout;
  scx::Uri m_client_proxy;

};

};
#endif

