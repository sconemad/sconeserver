/* SconeServer (http://www.sconemad.com)

Sconex logger

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

#ifndef scxLogger_h
#define scxLogger_h

#include <sconex/sconex.h>
#include <sconex/Thread.h>
#include <deque>
namespace scx {

class File;
class Mutex;
class LogThread;

//=============================================================================
class SCONEX_API Logger {
public:

  // Construct logger
  Logger(const std::string& name);
  
  virtual ~Logger();

  enum Level { Error, Warning, Info, Debug };

  typedef std::map<std::string,std::string> LogData;

  
  // Write message to the log
  virtual void log(const std::string& message,
                   Level level);
  
protected:

  struct LogEntry {
    timeval time;
    std::string message;
    Level level;
    LogData* data;
  };
  
  friend class LogThread;
  void process_queue();
  void log_entry(LogEntry* entry);
  
  File* m_file;
  Mutex* m_mutex;
  LogThread* m_thread;

  typedef std::deque<LogEntry*> LogQueue;
  LogQueue m_queue;
	
};

//=============================================================================
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
