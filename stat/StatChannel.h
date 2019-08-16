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

class StatModule;

//=============================================================================
// Stats - A set of connection statistics
//
class Stats : public scx::ScriptObject {
public:

  enum Type {
    Connections = 0,
    Reads,
    Writes,
    Errors,
    BytesRead,
    BytesWritten,
    StatTypeMax
  };

  static const char* stat_name(Type type);

  Stats();

  // Increment the named stat type by value
  void inc_stat(Type type, long value);

  // Get the named stat value
  long get_stat(Type type) const;

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

  typedef scx::ScriptRefTo<Stats> Ref;

private:
  std::vector<long> m_stats;
};


//=============================================================================
// StatChannel - A statistic-collection channel
//
class StatChannel : public scx::ScriptObject {
public:

  StatChannel(StatModule* module, const std::string& name);
  ~StatChannel();

  const Stats* get_stats() const;
  void inc_stat(Stats::Type type, long value);

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

  scx::ScriptRefTo<StatModule> m_module;

  std::string m_name;
  Stats::Ref m_stats;
};

#endif
