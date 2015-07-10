/* SconeServer (http://www.sconemad.com)

Sconesite document interface

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconesite/Document.h>
#include <sconesite/Context.h>
#include <sconex/Mutex.h>
#include <sconex/ScriptTypes.h>
#include <sconex/FileStat.h>
#include <sconex/Log.h>
namespace scs {

scx::Mutex* Document::m_clients_mutex = 0;
scx::ProviderScheme<Document>* Document::s_document_providers = 0;

//=========================================================================
scx::ScriptRefTo<Document>* Document::find(const std::string& name,
					   const scx::FilePath& root)
{
  scx::ScriptMap::Ref args(new scx::ScriptMap());
  args.object()->give("name",scx::ScriptString::new_ref(name));
  args.object()->give("root",scx::ScriptString::new_ref(root.path()));
  
  for (scx::ProviderScheme<Document>::ProviderMap::const_iterator it = 
	 s_document_providers->providers().begin(); 
       it != s_document_providers->providers().end(); ++it) {
    std::string file = "article." + it->first;
    if (scx::FileStat(root + file).is_file()) {
      args.object()->give("file",scx::ScriptString::new_ref(file));
      Document* doc = s_document_providers->provide(it->first, &args);
      return new Document::Ref(doc);
    }
  }
  return 0;
}

//=========================================================================
Document::Document(const std::string& name,
		   const scx::FilePath& root,
		   const std::string& file)
  : m_name(name),
    m_root(root),
    m_file(file),
    m_clients(0),
    m_opening(false),
    m_headings(1,name,0)
{
  if (!m_clients_mutex) {
    m_clients_mutex = new scx::Mutex();
  }
}

//=========================================================================
Document::~Document()
{
}

//=========================================================================
const std::string& Document::get_name() const
{
  return m_name;
}

//=========================================================================
const scx::FilePath& Document::get_root() const
{
  return m_root;
}

//=========================================================================
const std::string& Document::get_file() const
{
  return m_file;
}

//=========================================================================
scx::FilePath Document::get_filepath() const
{
  return m_root + m_file;
}

//=========================================================================
const Heading& Document::get_headings() const
{
  const_cast<Document*>(this)->open();
  return m_headings; 
}

//=========================================================================
const Document::ErrorList& Document::get_errors() const
{
  return m_errors; 
}

//=========================================================================
void Document::log_errors() const
{
  for (ErrorList::const_iterator it = m_errors.begin();
       it != m_errors.end(); ++it) {
    scx::Log log("sconesite.doc");
    log.attach("file", get_filepath().path());
    log.submit(*it);
  }
}

//=========================================================================
bool Document::purge(const scx::Date& purge_time)
{
  if (!is_open()) return false;
  if (m_last_access > purge_time) return false;

  scx::MutexLocker locker(*m_clients_mutex);
  if (m_clients > 0) return false;
  locker.unlock();

  DEBUG_LOG("Purging " + get_filepath().path());
  handle_close();
  return true;
}

//=========================================================================
bool Document::process(Context& context)
{
  m_clients_mutex->lock();
  ++m_clients;
  m_clients_mutex->unlock();

  bool success = false;
  if (open()) success = handle_process(context);

  m_clients_mutex->lock();
  --m_clients;
  m_clients_mutex->unlock();
  
  return success;
}

//=========================================================================
void Document::report_error(const std::string& error)
{
  m_errors.push_back(error);
}

//=========================================================================
void Document::register_document_type(const std::string& type,
				      scx::Provider<Document>* factory)
{
  init();
  s_document_providers->register_provider(type,factory);
}

//=========================================================================
void Document::unregister_document_type(const std::string& type,
					scx::Provider<Document>* factory)
{
  init();
  s_document_providers->unregister_provider(type,factory);
}

//=========================================================================
std::string Document::get_string() const
{
  return m_name;
}

//=========================================================================
scx::ScriptRef* Document::script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("test" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Sub-objects
    if ("name" == name) {
      return scx::ScriptString::new_ref(m_name);
    }

    if ("modtime" == name) {
      return new scx::ScriptRef(scx::FileStat(get_filepath()).time().new_copy());
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
bool Document::open()
{
  m_last_access = scx::Date::now();

  m_clients_mutex->lock();
  bool open_wait = m_opening;
  if (!m_opening) m_opening = true;
  m_clients_mutex->unlock();
  
  if (open_wait) {
    // Doc is being opened in another thread, wait for it to complete
    DEBUG_LOG("Document::Open() Waiting for another thread to complete");
    while (m_opening) { };
    DEBUG_LOG("Document::Open() Other thread finished open, continuing");
    return is_open();
  }
 
  bool opened = is_open();
  scx::FilePath path = get_filepath();
  scx::FileStat stat(path);
  if (stat.is_file()) {
    if (!opened || m_modtime != stat.time()) {
      if (opened) handle_close();
      m_errors.clear();
      m_headings.clear();
      opened = handle_open();
      m_modtime = stat.time();
    }
  } else {
    handle_close();
  }

  m_clients_mutex->lock();
  m_opening = false;
  m_clients_mutex->unlock();

  return opened;
}

void Document::init()
{
  if (!s_document_providers) {
    s_document_providers = new scx::ProviderScheme<Document>();
  }
}

};
