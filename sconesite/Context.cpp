/* SconeServer (http://www.sconemad.com)

Sconesite Context

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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


#include "Context.h"
#include "Profile.h"
#include "Article.h"
#include "SconesiteModule.h"

#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/FileDir.h"

//=========================================================================
Context::Context(
  Profile& profile,
  scx::Descriptor& output
) : m_profile(profile),
    m_output(output),
    m_article(0),
    m_articles_mode(false),
    m_files_mode(false),
    m_files_it("")
{

}

//=========================================================================
Context::~Context()
{
  
}

//=========================================================================
void Context::set_article(Article* article)
{
  m_article = article;
}

//=========================================================================
bool Context::handle_start(const std::string& name, XMLAttrs& attrs, bool empty)
{
  bool descend = false;

  if (m_articles_mode && name == "name") {
    Article* article = (*m_articles_it);
    m_output.write(article->get_name());
    
  } else if (m_articles_mode && name == "link") {
    Article* article = (*m_articles_it);
    std::string artname = article->get_name();
    std::ostringstream oss;
    oss << "<a href=\"/art/" << artname << "/\">" << artname << "</a>";
    m_output.write(oss.str());


  } else if (m_files_mode && name == "link") {
    std::string filename = m_files_it.name();
    std::ostringstream oss;
    oss << "<a href=\"" << filename << "\">" << filename << "</a>";
    m_output.write(oss.str());

    
  } else if (name == "article") { // Current article processed
    if (m_article) {
      m_article->process(*this);
    }

  } else if (name == "editor") { // Article editor
    if (m_article) {
      scx::File* file = new scx::File();
      if (scx::Ok == file->open(m_article->get_path(),scx::File::Read)) {
        char* buffer[1024];
        while (true) {
          int na = 0;
          scx::Condition c = file->read(buffer,1024,na);
          if (scx::Ok != c) {
            break;
          }
          m_output.write(buffer,na,na);
        }
      }
      delete file;
    }
     
  } else if (name == "name") { // Current article name
    if (m_article) {
      m_output.write(m_article->get_name());
    }
    
  } else if (name == "articles") {
    m_articles_it = m_profile.articles().begin();
    if (m_articles_it != m_profile.articles().end()) {
      m_articles_mode = true;
      descend = true;
    }

  } else if (name == "files") {
    m_articles_it = m_profile.articles().begin();
    m_files_it = scx::FileDir(m_article->get_root());
    if (m_files_it.next()) {
      m_files_mode = true;
      descend = true;
    }

  } else {
    std::ostringstream oss;
    oss << "<" << name;
    for (XMLAttrs::const_iterator it = attrs.begin();
         it != attrs.end();
         ++it) {
      oss << " " << (*it).first << "=\"" << (*it).second << "\"";
    }
    
    if (!empty) {
      oss << ">";
      descend = true;
    } else {
      oss << "/>";
      descend = false;
    }
    m_output.write(oss.str());
  }
  return descend;
}

//=========================================================================
bool Context::handle_end(const std::string& name, XMLAttrs& attrs)
{
  bool repeat = false;

  if (name == "articles") {
    if (++m_articles_it != m_profile.articles().end()) {
      repeat = true;
    } else {
      m_articles_mode = false;
    }

  } else if (name == "files") {
    if (m_files_it.next()) {
      repeat = true;
    } else {
      m_files_mode = false;
    }
    
  } else {
    std::ostringstream oss;
    oss << "</" << name << ">";
    m_output.write(oss.str());
  }

  return repeat;
}

//=========================================================================
void Context::handle_process(const std::string& name, const char* data)
{

}

//=========================================================================
void Context::handle_text(const char* text)
{
  m_output.write(text);
}

//=========================================================================
void Context::handle_comment(const char* text)
{
  m_output.write(text);
}

//=========================================================================
void Context::handle_error()
{
  m_output.write("Parsing error");

}

//=========================================================================
std::string Context::name() const
{
  if (m_article) {
    return m_article->get_name();
  }
  return "";
}

//=========================================================================
scx::Arg* Context::arg_resolve(const std::string& name)
{
  return 0;
}

//=========================================================================
scx::Arg* Context::arg_lookup(const std::string& name)
{
  // Methods
  if ("map" == name ||
      "map_path" == name ||
      "add_realm" == name ||
      "map_realm" == name ||
      "set_param" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Sub-objects
  
  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=========================================================================
scx::Arg* Context::arg_function(const std::string& name,scx::Arg* args)
{
  return 0;
}
