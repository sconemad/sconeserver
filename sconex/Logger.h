/* SconeServer (http://www.sconemad.com)

Sconex logger

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxLogger_h
#define scxLogger_h

#include <sconex/sconex.h>
#include <sconex/Log.h>
#include <sconex/Thread.h>
#include <sconex/ScriptBase.h>
#include <sconex/Provider.h>
#include <sconex/File.h>
#include <deque>
namespace scx {

class Mutex;
class Logger;
class LogThread;

//=============================================================================
// LogEntry - Stores data for a single log entry
//
class SCONEX_API LogEntry {
public:
  LogEntry();
  ~LogEntry();
  
  timeval m_time;
  std::string m_category;
  std::string m_message;
  LogData* m_data;
};

  
//=============================================================================
// LogChannel - Base class for ouputting log entries
//
class SCONEX_API LogChannel : public ScriptObject {
public:

  typedef ScriptRefTo<LogChannel> Ref;

  // Create a new LogChannel of the requested type
  static LogChannel::Ref* create(const std::string& type,
                                 const ScriptRef* args);
  
  LogChannel(const std::string& name);
  ~LogChannel();

  // LogChannel interface
  virtual void log_entry(LogEntry* entry) = 0;
  
  // ScriptObject methods
  virtual std::string get_string() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
                               const ScriptRef& ref,
                               const ScriptOp& op,
                               const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
                                   const ScriptRef& ref,
                                   const std::string& name,
                                   const ScriptRef* args);

  static void register_provider(const std::string& type,
                                Provider<LogChannel::Ref>* factory);

  static void unregister_provider(const std::string& type,
                                  Provider<LogChannel::Ref>* factory);

private:

  static void init();

  static ProviderScheme<LogChannel::Ref>* s_providers;

  std::string m_name;
  
};

  
//=============================================================================
// FileLogChannel - Writes log entries to a file
//
class SCONEX_API FileLogChannel : public LogChannel {
public:

  FileLogChannel(const std::string& name,
                 const FilePath& path,
                 bool fallback = false);
  ~FileLogChannel();

  virtual void log_entry(LogEntry* entry);

protected:  

  File m_file;
  bool m_fallback;
  
};


//=============================================================================
// CacheLogChannel - Caches recent log entries for inspection
//
class SCONEX_API CacheLogChannel : public LogChannel {
public:

  CacheLogChannel(const std::string& name, int max=100);
  ~CacheLogChannel();

  virtual void log_entry(LogEntry* entry);

  virtual ScriptRef* script_op(const ScriptAuth& auth,
                               const ScriptRef& ref,
                               const ScriptOp& op,
                               const ScriptRef* right=0);

protected:  

  std::list<std::string> m_cache;
  int m_max;
  
};

  
//=============================================================================
// Logger - Manages queuing of log entries and sending to channels for output
//
class SCONEX_API Logger : public ScriptObject,
                          public Provider<LogChannel::Ref> {
public:

  // Initialise logger, specifying a default log file path
  static void init(const scx::FilePath& path);
  
  // Get the logger singleton
  static Logger* get();
  
  virtual ~Logger();

  // Submit a log entry
  void log(const std::string& category,
           const std::string& message,
           LogData* data);

  // Enable asynchronous logging
  void set_async(bool async);
  
  // Find log channel by name
  LogChannel* find_channel(const std::string& name);
  
  // ScriptObject methods
  virtual std::string get_string() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
                               const ScriptRef& ref,
                               const ScriptOp& op,
                               const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
                                   const ScriptRef& ref,
                                   const std::string& name,
                                   const ScriptRef* args);

  // Provider<LogChannel::Ref> methods
  virtual void provide(const std::string& type,
		       const ScriptRef* args,
		       LogChannel::Ref*& object);
  
  typedef ScriptRefTo<Logger> Ref;

protected:

  Logger(const FilePath& path);

  friend class LogThread;
  void process_queue();
  void log_entry(LogEntry* entry);

private:

  FilePath m_path;
  Mutex* m_mutex;
  LogThread* m_thread;

  typedef std::deque<LogEntry*> LogQueue;
  LogQueue m_queue;

  typedef std::map<std::string,LogChannel::Ref*> ChannelMap;
  ChannelMap m_channels;

  static ScriptRefTo<Logger>* s_logger;
  
};


//=============================================================================
// LogThread - Separate thread to processes queued log entries
//
class SCONEX_API LogThread : public Thread {
public:

  LogThread(Logger& logger);
  virtual ~LogThread();

  virtual void* run();

  void awaken();

protected:

  Logger& m_logger;
  
};
  
};
#endif
