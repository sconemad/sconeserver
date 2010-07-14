/* SconeServer (http://www.sconemad.com)

Sconesite Article Body

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/FilePath.h"
#include "sconex/Date.h"
#include "sconex/ArgObject.h"

class Context;

//=========================================================================
class ArticleHeading {
public:
  ArticleHeading(int level, const std::string& name, int index);
  ~ArticleHeading();

  int level() const;
  const std::string& name() const;
  int index() const;
  
  void clear();
  void add(int level, const std::string& name, int index);

  const ArticleHeading* lookup_index(int index) const;
  std::string lookup_anchor(int index) const;
  std::string lookup_section(int index) const;
  scx::Arg* get_arg(
    const std::string& anchor_prefix = "",
    const std::string& section_prefix = ""
  ) const;

private:
  
  int m_level;
  std::string m_name;
  int m_index;
  
  typedef std::vector<ArticleHeading*> ArticleHeadingList;
  ArticleHeadingList m_subs;
};


//=========================================================================
class ArticleBody : public scx::ArgObjectInterface {

public:
  
  virtual const std::string& get_name() const =0;
  virtual const scx::FilePath& get_root() const =0;
  virtual const std::string& get_file() const =0;
  virtual scx::FilePath get_filepath() const =0;

  virtual bool process(Context& context) =0;
  virtual bool purge(const scx::Date& purge_time) =0;

  virtual const ArticleHeading& get_headings() const =0;
  
};

#endif
