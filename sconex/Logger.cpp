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

#include <sconex/Logger.h>
#include <sconex/File.h>
#include <sconex/Mutex.h>
namespace scx {

// Uncomment this to force synchronous logging
//#define SYNC_LOGGING


//=============================================================================
Logger::Logger(const std::string& name)
  : m_file(new File()),
    m_mutex(new Mutex()),
    m_thread(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Logger);
  if (m_file->open(name.c_str(),File::Write | File::Append | File::Create) !=
      Ok) {
    std::cerr << "Unable to open log file '" << name
              << "' - will log to stdout\n";
    delete m_file; m_file = 0;
  }

#ifndef SYNC_LOGGING
  m_thread = new LogThread(*this);
  m_thread->start();
#endif

  if (m_file) {
    log("-----------------------------------------------------------------",
        Info);
  }
}

//=============================================================================
Logger::~Logger()
{
  if (m_thread) {
    m_thread->stop();
    delete m_thread;
  }
  if (m_file) {
    m_file->close();
    delete m_file;
  }
  delete m_mutex;
  DEBUG_COUNT_DESTRUCTOR(Logger);
}

//=============================================================================
void Logger::log(const std::string& message, Level level)
{
  LogEntry* entry = new LogEntry();
  ::gettimeofday(&entry->time,0);
  entry->message = message;
  entry->level = level;
  entry->data = 0;

  m_mutex->lock();
  m_queue.push_back(entry);
  m_mutex->unlock();

  if (m_thread) {
    m_thread->awaken();
  } else {
    process_queue();
  }
}

//=============================================================================
void Logger::process_queue()
{
  MutexLocker locker(*m_mutex, false);
  while (true) {
    locker.lock();
    if (m_queue.empty()) break;
    LogEntry* entry = m_queue.front();
    m_queue.pop_front();
    locker.unlock();

    log_entry(entry);
    delete entry;
  }
}

//=============================================================================
void Logger::log_entry(LogEntry* entry)
{
  int na=0;
  std::string lcode;
  switch (entry->level) {
    case Logger::Error:    lcode = "E"; break;
    case Logger::Warning:  lcode = "W"; break;
    case Logger::Info:     lcode = "i"; break;
    case Logger::Debug:    lcode = "d"; break;
  }
  Date date(entry->time, true);
  std::ostringstream oss;
  oss << date.code() << " " 
      << std::setfill('0') << std::setw(6) << date.microsecond() << " "
      << lcode << " "
      << entry->message << "\n";
  std::string str = oss.str();

  // We only need the lock if not in the logger thread (synchronous)
  MutexLocker locker(*m_mutex, !m_thread);
  if (m_file) {
    m_file->write(str.c_str(),str.size(),na);
  } else {
    std::cout << str;
  }
}


//=============================================================================
LogThread::LogThread(Logger& logger)
  : m_logger(logger)
{
}

//=============================================================================
LogThread::~LogThread()
{
}

//=============================================================================
void* LogThread::run()
{
  m_mutex.lock();
  while (true) {

    // Wait to be woken up
    m_wakeup.wait(m_mutex);

    if (should_exit()) {
      // Time for the thread to stop
      break;
    }

    try {
      m_logger.process_queue();
    } catch (...) { }

  }
  
  return 0;
}

void LogThread::awaken()
{
  m_wakeup.signal();
}
  
};
