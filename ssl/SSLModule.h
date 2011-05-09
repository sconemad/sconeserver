/* SconeServer (http://www.sconemad.com)

SSL connection module

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

#ifndef sslModule_h
#define sslModule_h

#include "SSLChannel.h"
#include <sconex/Module.h>
#include <sconex/Descriptor.h>
#include <sconex/Stream.h>

//=============================================================================
// SSLModule - A module providing Secure Socket Layer encryption streams
//
class SSLModule : public scx::Module,
                  public scx::Provider<scx::Stream> {
public:

  SSLModule();
  virtual ~SSLModule();

  virtual std::string info() const;

  virtual int init();

  SSLChannel* find_channel(const std::string& name);
  
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);

  typedef scx::ScriptRefTo<SSLModule> Ref;
  
protected:

private:

  typedef std::map<std::string,SSLChannel::Ref*> ChannelMap;
  ChannelMap m_channels;
  
};

#endif

