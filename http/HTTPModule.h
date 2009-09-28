/* SconeServer (http://www.sconemad.com)

HTTP (HyperText Transfer Protocol) Module

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

#ifndef httpModule_h
#define httpModule_h

#include "http/http.h"
#include "http/HostMapper.h"
#include "http/AuthRealm.h"
#include "http/Session.h"
#include "sconex/Module.h"
#include "sconex/Descriptor.h"

namespace http {

class HostMapper;
class HTTPModule;
  
//=============================================================================
class HTTP_API HTTPModule : public scx::Module {
public:

  HTTPModule();
  virtual ~HTTPModule();

  virtual std::string info() const;

  virtual int init();

  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

  HostMapper& get_hosts();
  AuthRealmManager& get_realms();
  SessionManager& get_sessions();

  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);
  
protected:

private:

  HostMapper m_hosts;
  AuthRealmManager m_realms;
  SessionManager m_sessions;

};

};
#endif

