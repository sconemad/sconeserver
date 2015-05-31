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

#include <sconex/Logger.h>
#include <sconex/Mutex.h>
#include <sconex/ScriptTypes.h>
namespace scx {

//=============================================================================
LogEntry::LogEntry()
  : m_time(),
    m_category(),
    m_message(),
    m_data(0)
{
}

//=============================================================================
LogEntry::~LogEntry()
{
  if (m_data) {
    for (LogData::iterator it = m_data->begin(); it != m_data->end(); ++it) {
      delete it->second;
    }
    delete m_data;
  }
}


ProviderScheme<LogChannel::Ref>* LogChannel::s_providers = 0;

//=============================================================================
LogChannel::Ref* LogChannel::create(const std::string& type,
                                    const ScriptRef* args)
{
  init();
  return s_providers->provide(type, args);
}

//=============================================================================
LogChannel::LogChannel(const std::string& name)
  : m_name(name)
{
  DEBUG_COUNT_CONSTRUCTOR(LogChannel);
}

//=============================================================================
LogChannel::~LogChannel()
{
  DEBUG_COUNT_DESTRUCTOR(LogChannel);
}

//=============================================================================
std::string LogChannel::get_string() const
{
  return m_name;
}

//=============================================================================
ScriptRef* LogChannel::script_op(const ScriptAuth& auth,
                                 const ScriptRef& ref,
                                 const ScriptOp& op,
                                 const ScriptRef* right)
{
  if (op.type() == ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("test" == name) {
      return new ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("test" == name) {
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* LogChannel::script_method(const ScriptAuth& auth,
                                     const ScriptRef& ref,
                                     const std::string& name,
                                     const ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");

  return ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
void LogChannel::register_provider(const std::string& type,
                                   Provider<LogChannel::Ref>* factory)
{
  init();
  s_providers->register_provider(type,factory);
}
  
//=========================================================================
void LogChannel::unregister_provider(const std::string& type,
                                     Provider<LogChannel::Ref>* factory)
{
  init();
  s_providers->unregister_provider(type,factory);
}
  
//=========================================================================
void LogChannel::init()
{
  if (!s_providers) {
    s_providers = new ProviderScheme<LogChannel::Ref>();
  }
}


//=============================================================================
FileLogChannel::FileLogChannel(const std::string& name,
                               const FilePath& path,
                               bool fallback)
  : LogChannel(name),
    m_file(),
    m_fallback(fallback)
{
  if (Ok != m_file.open(path, File::Write | File::Append | File::Create,
                        0640)) {
    if (m_fallback) {
      std::cerr << "Unable to open log file '"
                << path.path() << "' - will log to stdout\n";
    }
  } else {
    m_file.write("\n----------\n\n");
  }
}

//=============================================================================
FileLogChannel::~FileLogChannel()
{

}

//=============================================================================
void FileLogChannel::log_entry(LogEntry* entry)
{
  if (!m_file.is_open() && !m_fallback) return;

  Date date(entry->m_time, true);
  std::ostringstream oss;
  oss << date.code() << " " 
      << std::setfill('0') << std::setw(6) << date.microsecond() << " "
      << "[" << entry->m_category;

  if (entry->m_data) {
    LogData::const_iterator it = entry->m_data->find("id");
    if (it != entry->m_data->end()) {
      oss << "/" << it->second->object()->get_string();
    }
  }
  
  oss << "] " << entry->m_message;

  if (entry->m_data) {
    for (LogData::const_iterator it = entry->m_data->begin();
         it != entry->m_data->end(); ++it) {
      if (it->first != "id") {
        oss << "\n  " << it->first << ": "
            << it->second->object()->get_string() << "";
      }
    }
  }

  oss << "\n";

  if (m_file.is_open()) {
    int na=0;
    std::string str = oss.str();
    m_file.write(str.c_str(),str.size(),na);
  } else if (m_fallback) {
    std::cout << oss.str();
  }
}


//=============================================================================
CacheLogChannel::CacheLogChannel(const std::string& name, int max)
  : LogChannel(name),
    m_cache(),
    m_max(max)
{
}

//=============================================================================
CacheLogChannel::~CacheLogChannel()
{
}

//=============================================================================
void CacheLogChannel::log_entry(LogEntry* entry)
{
  Date date(entry->m_time, true);
  std::ostringstream oss;
  oss << date.code() << " " 
      << std::setfill('0') << std::setw(6) << date.microsecond() << " "
      << "[" << entry->m_category;

  if (entry->m_data) {
    LogData::const_iterator it = entry->m_data->find("id");
    if (it != entry->m_data->end()) {
      oss << "/" << it->second->object()->get_string();
    }
  }
  
  oss << "] " << entry->m_message;

  if (entry->m_data) {
    oss << " {";
    bool first = true;
    for (LogData::const_iterator it = entry->m_data->begin();
         it != entry->m_data->end(); ++it) {
      oss << (first ? "" : ", ")
          << it->first << ":\""
          << it->second->object()->get_string() << "\"";
      first = false;
    }
    oss << "}";
  }

  if ((int)m_cache.size() == m_max) {
    m_cache.erase(m_cache.begin());
  }

  m_cache.push_back(oss.str());
}

//=============================================================================
ScriptRef* CacheLogChannel::script_op(const ScriptAuth& auth,
                                      const ScriptRef& ref,
                                      const ScriptOp& op,
                                      const ScriptRef* right)
{
  if (op.type() == ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Properties
    if ("entries" == name) {
      ScriptList* list = new ScriptList();
      for (std::list<std::string>::const_iterator it = m_cache.begin();
	   it != m_cache.end();
	   ++it) {
	list->give(ScriptString::new_ref(*it));
      }
      return new ScriptRef(list);
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
}


ScriptRefTo<Logger>* Logger::s_logger = 0;

//=============================================================================
void Logger::init(const FilePath& path)
{
  delete s_logger;
  s_logger = new ScriptRefTo<Logger>( new Logger(path) );
}
  
//=============================================================================
Logger* Logger::get()
{
  return s_logger->object();
}
  
//=============================================================================
Logger::~Logger()
{
  if (m_thread) {
    m_thread->stop();
    delete m_thread;
  }
  for (ChannelMap::const_iterator it = m_channels.begin();
       it != m_channels.end(); ++it) {
    delete it->second;
  }
  delete m_mutex;
}

//=============================================================================
void Logger::log(const std::string& category,
                 const std::string& message,
                 LogData* data)
{
  LogEntry* entry = new LogEntry();
  ::gettimeofday(&entry->m_time,0);
  entry->m_category = category;
  entry->m_message = message;
  entry->m_data = data;

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
void Logger::set_async(bool async)
{
  if (async && !m_thread) {
    m_thread = new LogThread(*this);
    m_thread->start();
    log("logger", "Enabled asynchronous logging", 0);

  } else if (!async && m_thread) {
    m_thread->stop();
    delete m_thread; m_thread = 0;
    log("logger", "Disabled asynchronous logging", 0);
  }
}

//=============================================================================
LogChannel* Logger::find_channel(const std::string& name)
{
  ChannelMap::const_iterator it = m_channels.find(name);
  
  if (it != m_channels.end()) {
    return it->second->object();
  }
  
  return 0;
}

//=============================================================================
std::string Logger::get_string() const
{
  return "Logger";
}

//=============================================================================
ScriptRef* Logger::script_op(const ScriptAuth& auth,
                             const ScriptRef& ref,
                             const ScriptOp& op,
                             const ScriptRef* right)
{
  if (op.type() == ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("add" == name ||
	"remove" == name ||
        "set_async" == name) {
      return new ScriptMethodRef(ref,name);
    }      

    // Properties
    if ("channels" == name) {
      ScriptList* list = new ScriptList();
      for (ChannelMap::const_iterator it = m_channels.begin();
	   it != m_channels.end();
	   ++it) {
	list->give(it->second->ref_copy(ref.reftype()));
      }
      return new ScriptRef(list);
    }

    if ("async" == name) {
      return ScriptInt::new_ref(m_thread != 0);
    }
    
    // Sub-objects
    LogChannel* channel = find_channel(name);
    if (channel) {
      return new ScriptRef(channel);
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* Logger::script_method(const ScriptAuth& auth,
                                 const ScriptRef& ref,
                                 const std::string& name,
                                 const ScriptRef* args)
{
  if (!auth.admin()) return scx::ScriptError::new_ref("Not permitted");
  
  if ("add" == name) {
    const ScriptString* a_name = 
      get_method_arg<ScriptString>(args,0,"name");
    if (!a_name) 
      return ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    // Check channel doesn't already exist
    if (find_channel(s_name))
      return ScriptError::new_ref("Channel '" + s_name + "' exists");

    const ScriptString* a_type = 
      get_method_arg<ScriptString>(args,1,"type");
    if (!a_type) 
      return ScriptError::new_ref("Type must be specified");
    std::string s_type = a_type->get_string();

    LogChannel::Ref* channel = LogChannel::create(s_type,args);
    if (!channel) 
      return ScriptError::new_ref("Failed to create log channel");

    m_channels[s_name] = channel;
    return channel->ref_copy();
  }
  
  if ("remove" == name) {
    const ScriptString* a_name = 
      get_method_arg<ScriptString>(args,0,"name");
    if (!a_name) 
      return ScriptError::new_ref("Name must be specified");
    std::string s_name = a_name->get_string();

    // Find and remove channel
    ChannelMap::iterator it = m_channels.find(s_name);
    if (it == m_channels.end())
      return ScriptError::new_ref("Channel '" + s_name + 
                                  "' does not exist");

    delete it->second;
    m_channels.erase(it);

    return 0;
  }

  if ("set_async" == name) {
    const ScriptInt* a_async = 
      get_method_arg<ScriptInt>(args,0,"async");
    bool async = true;
    if (a_async)
      async = a_async->get_int();
    set_async(async);
    return 0;
  }
  
  return ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
void Logger::provide(const std::string& type,
                     const ScriptRef* args,
                     LogChannel::Ref*& object)
{
  if ("file" == type) {
    const ScriptString* a_name = 
      get_method_arg<ScriptString>(args,0,"name");
    if (a_name) {
      std::string s_name = a_name->get_string();
      FilePath path = m_path;
      const ScriptString* a_file = 
        get_method_arg<ScriptString>(args,2,"file");
      if (a_file) {
        path += FilePath(a_file->get_string());
      } else {
        path += FilePath(s_name+std::string(".log"));
      }
      object = new LogChannel::Ref(new FileLogChannel(s_name, path));
    }
    
  } else if ("cache" == type) {
    const ScriptString* a_name = 
      get_method_arg<ScriptString>(args,0,"name");
    if (a_name) {
      std::string s_name = a_name->get_string();
      int max = 100;
      const ScriptInt* a_max = 
        get_method_arg<ScriptInt>(args,2,"max");
      if (a_max) max = a_max->get_int();
      object = new LogChannel::Ref(new CacheLogChannel(s_name, max));
    }
  }
}

//=============================================================================
Logger::Logger(const FilePath& path)
  : m_path(path),
    m_mutex(new Mutex()),
    m_thread(0),
    m_queue(),
    m_channels()
{
  LogChannel::register_provider("file", this);
  LogChannel::register_provider("cache", this);

  FilePath::mkdir(path,false,0755);
  FilePath defaultPath = path + FilePath("sconeserver.log");
  m_channels["default"] =
    new LogChannel::Ref(new FileLogChannel("default",defaultPath,true));
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
  for (ChannelMap::const_iterator it = m_channels.begin();
       it != m_channels.end(); ++it) {
    it->second->object()->log_entry(entry);
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
