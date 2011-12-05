/* SconeServer (http://www.sconemad.com)

WikiText Article Body

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


#include "WikiTextArticleBody.h"
#include "Article.h"
#include "Context.h"

#include <sconex/Stream.h>
#include <sconex/Date.h>
#include <sconex/File.h>
#include <sconex/FileStat.h>
#include <sconex/LineBuffer.h>
#include <sconex/utils.h>
#include <sconex/ScriptTypes.h>

scx::Mutex* WikiTextArticleBody::m_clients_mutex = 0;

//=========================================================================
WikiTextArticleBody::WikiTextArticleBody(const std::string& name,
					 const scx::FilePath& path)
  : ArticleBody(name),
    m_root(path),
    m_file("article.wtx"),
    m_clients(0),
    m_opening(false)
{
  if (!m_clients_mutex) {
    m_clients_mutex = new scx::Mutex();
  }
}

//=========================================================================
WikiTextArticleBody::~WikiTextArticleBody()
{

}

//=========================================================================
const scx::FilePath& WikiTextArticleBody::get_root() const
{
  return m_root;
}

//=========================================================================
const std::string& WikiTextArticleBody::get_file() const
{
  return m_file;
}

//=========================================================================
scx::FilePath WikiTextArticleBody::get_filepath() const
{
  return m_root + m_file;
}

//=========================================================================
bool WikiTextArticleBody::process(Context& context)
{
  m_clients_mutex->lock();
  ++m_clients;
  m_clients_mutex->unlock();

  if (context.handle_doc_start(this)) {
    do {
      //TODO: Implement an actual WikiText parser.
      // As a test, we just output lines within html p tags.
      scx::File file;
      file.open(get_filepath(),scx::File::Read);
      scx::LineBuffer* parser = new scx::LineBuffer("wikitext");
      file.add_stream(parser);

      std::string line;
      while (scx::Ok == parser->tokenize(line)) {
	NodeAttrs attrs;
	context.handle_start("p",attrs,false,0);
	context.handle_text(line.c_str());
	context.handle_end("p",attrs,0);
      }

    } while (context.handle_doc_end(this));
  }

  m_clients_mutex->lock();
  --m_clients;
  m_clients_mutex->unlock();
  
  return true;
}

//=========================================================================
const scx::Date& WikiTextArticleBody::get_modtime() const
{
  return m_modtime;
}

//=========================================================================
void WikiTextArticleBody::parse_error(const std::string& msg)
{
  m_errors += msg;
}

//=========================================================================
bool WikiTextArticleBody::purge(const scx::Date& purge_time)
{
  if (m_last_access > purge_time) return false;

  scx::MutexLocker locker(*m_clients_mutex);
  if (m_clients > 0) return false;

  DEBUG_LOG("Purging " + get_filepath().path());
  return true;
}

//=========================================================================
std::string WikiTextArticleBody::get_string() const
{
  return m_name;
}

//=========================================================================
scx::ScriptRef* WikiTextArticleBody::script_op(const scx::ScriptAuth& auth,
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
      return new scx::ScriptRef(m_modtime.new_copy());
    }
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

