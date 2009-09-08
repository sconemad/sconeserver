/* SconeServer (http://www.sconemad.com)

Sconesite Article

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


#include "Article.h"
#include "Profile.h"
#include "Context.h"

#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/FileDir.h"
#include "sconex/LineBuffer.h"

//=========================================================================
bool ArticleSortDate(const Article* a, const Article* b)
{
  return (a->get_modtime() > b->get_modtime());
}

//=========================================================================
bool ArticleSortName(const Article* a, const Article* b)
{
  return (a->get_name() < b->get_name());
}

//=========================================================================
Article::Article(
  Profile& profile,
  const std::string& name,
  const scx::FilePath& root
) : XMLDoc(name,root + "article.xml"),
    m_profile(profile),
    m_root(root)
{

}

//=========================================================================
Article::~Article()
{
  
}

//=========================================================================
const scx::FilePath& Article::get_root() const
{
  return m_root;
}

//=========================================================================
std::string Article::name() const
{
  std::ostringstream oss;
  oss << "Article:" << m_name;
  return oss.str();
}

//=========================================================================
scx::Arg* Article::arg_resolve(const std::string& name)
{
  return XMLDoc::arg_resolve(name);
}

//=========================================================================
scx::Arg* Article::arg_lookup(const std::string& name)
{
  // Methods
  if ("test" == name) {
    return new scx::ArgObjectFunction(new scx::ArgObject(this),name);
  }

  // Sub-objects
  refresh();
  std::string metaval = m_metadata.get(name);
  if (!metaval.empty()) {
    return new scx::ArgString(metaval);
  }

  return XMLDoc::arg_lookup(name);
}

//=========================================================================
scx::Arg* Article::arg_function(const std::string& name,scx::Arg* args)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  return XMLDoc::arg_function(name,args);
}

//=========================================================================
void Article::refresh()
{
  m_metadata = scx::MimeHeaderTable();

  scx::File file;
  if (file.open(m_root + "meta.txt",scx::File::Read) == scx::Ok) {
    scx::LineBuffer* parser = new scx::LineBuffer("meta parser");
    file.add_stream(parser);

    std::string line;
    while (parser->tokenize(line) == scx::Ok) {
      m_metadata.parse_line(line);
    }
  }
}
