/* SconeServer (http://www.sconemad.com)

Statistics Module

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

#ifndef statModule_h
#define statModule_h

#include <sconex/Module.h>
#include "stat/StatChannel.h"

class StatModule;

//=============================================================================
class StatModule : public scx::Module,
                   public scx::Provider<scx::Stream> {
public:

  StatModule();
  virtual ~StatModule();

  virtual std::string info() const;
  
  StatChannel* find_channel(const std::string& name);
  StatChannel* add_channel(const std::string& name);
  bool remove_channel(const std::string& name);
  
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

protected:
  
private:

  // Statistics channels
  typedef HASH_TYPE<std::string,StatChannel::Ref*> ChannelMap;
  ChannelMap m_channels;
  
};

#endif

