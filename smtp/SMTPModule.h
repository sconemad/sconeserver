/* SconeServer (http://www.sconemad.com)

SMTP (Simple Mail Transfer Protocol) Module

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef smtpModule_h
#define smtpModule_h

#include "sconex/Module.h"
#include "sconex/Descriptor.h"
#include "sconex/Uri.h"
#include "sconex/StreamSocket.h"
#include "sconex/ScriptBase.h"

namespace smtp {

//=============================================================================
class SMTPModule : public scx::Module,
                   public scx::Provider<scx::ScriptObject> {

public:

  SMTPModule();
  virtual ~SMTPModule();

  virtual std::string info() const;

  virtual int init();
  virtual void close();
  
  const scx::ScriptRef* get_server() const;

  scx::StreamSocket* new_server_connection();

  // ScriptObject methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  // Provider<ScriptObject> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object);

protected:

private:

  scx::ScriptRef* m_server;
  
};

};
#endif

