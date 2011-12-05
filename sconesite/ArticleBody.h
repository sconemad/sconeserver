/* SconeServer (http://www.sconemad.com)

Sconesite Article Body

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

#ifndef sconesiteArticleBody_h
#define sconesiteArticleBody_h

#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/ScriptBase.h>
#include "ArticleHeading.h"

class Context;

typedef std::map<std::string,std::string> NodeAttrs;

//=========================================================================
// ArticleBody - Sconesite article body interface
//
class ArticleBody : public scx::ScriptObject {
public:

  ArticleBody(const std::string& name)
    : m_name(name), m_headings(1,name,0) { }
  
  virtual const std::string& get_name() const
  {
    return m_name; 
  }

  virtual const scx::FilePath& get_root() const =0;
  virtual const std::string& get_file() const =0;
  virtual scx::FilePath get_filepath() const =0;

  virtual bool process(Context& context) =0;
  virtual bool purge(const scx::Date& purge_time) =0;

  virtual const ArticleHeading& get_headings() const
  {
    return m_headings; 
  }

  typedef scx::ScriptRefTo<ArticleBody> Ref;

protected:

  std::string m_name;
  ArticleHeading m_headings;
  
};

#endif
