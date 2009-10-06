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
#include "sconex/ArgProc.h"

//=========================================================================
bool ArticleSortDate(const Article* a, const Article* b)
{
  const scx::Arg* a_time = a->get_meta("time");
  const scx::Arg* b_time = b->get_meta("time");

  int a_int = a_time ? a_time->get_int() : 0;
  int b_int = b_time ? b_time->get_int() : 0;

  return (a_int > b_int);
}

//=========================================================================
bool ArticleSortName(const Article* a, const Article* b)
{
  return (a->get_name() < b->get_name());
}

//=========================================================================
ArticleMetaSorter::ArticleMetaSorter(const std::string& meta, bool reverse)
  : m_meta(meta),
    m_reverse(reverse)
{

}

//=========================================================================
bool ArticleMetaSorter::operator()(const Article* a, const Article* b)
{
  bool ret = !m_reverse;
  
  const scx::Arg* a_arg = a->get_meta(m_meta);
  const scx::Arg* b_arg = b->get_meta(m_meta);

  if (a_arg) {
    ret = m_reverse;
    if (b_arg) {
      if (dynamic_cast<const scx::ArgInt*>(a_arg) != 0) {
	int a_int = a_arg ? a_arg->get_int() : 0;
	int b_int = b_arg ? b_arg->get_int() : 0;
	ret = (m_reverse ? (a_int >= b_int) : (a_int < b_int));
      } else {
	ret = (m_reverse ? (a_arg->get_string() >= b_arg->get_string()) : (a_arg->get_string() < b_arg->get_string()));
      }
    }
  }

  delete a_arg;
  delete b_arg;

  return ret;
}

//=========================================================================
Article::Article(
  Profile& profile,
  const std::string& name,
  const scx::FilePath& root
) : XMLDoc(name,root + "article.xml"),
    m_profile(profile),
    m_root(root),
    m_metastore(root + "meta.txt")
{
  m_metastore.load();
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
const scx::Arg* Article::get_meta(const std::string& name) const
{
  Article* uc = const_cast<Article*>(this);
  return uc->m_metastore.arg_lookup(name);
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
  if ("meta" == name) return new scx::ArgObject(&m_metastore);

  return XMLDoc::arg_lookup(name);
}

//=========================================================================
scx::Arg* Article::arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  return XMLDoc::arg_function(auth,name,args);
}
