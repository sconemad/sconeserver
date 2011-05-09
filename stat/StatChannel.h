/* SconeServer (http://www.sconemad.com)

Statistics Channel

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

#ifndef statChannel_h
#define statChannel_h

#include <sconex/Stream.h>
#include <sconex/Module.h>
#include <sconex/ScriptBase.h>

//=============================================================================
// StatChannel - A statistic-collection channel
//
class StatChannel : public scx::ScriptObject {
public:

  StatChannel(const std::string& name);
  ~StatChannel();

  // Increment the named stat type by value
  void inc_stat(const std::string& type, long value);

  // Get the named stat value
  long get_stat(const std::string& type) const;

  // Clear stats
  void clear();
  
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

  typedef scx::ScriptRefTo<StatChannel> Ref;

protected:

private:

  std::string m_name;

  typedef HASH_TYPE<std::string,long> StatMap;
  StatMap m_stats;
  
};

#endif
