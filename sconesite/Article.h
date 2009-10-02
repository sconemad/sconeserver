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

#ifndef sconesiteArticle_h
#define sconesiteArticle_h

#include "XMLDoc.h"

#include "sconex/Stream.h"
#include "sconex/FilePath.h"
#include "sconex/Arg.h"
#include "sconex/ArgStore.h"

class SconesiteModule;
class Profile;
class Article;

// Sort predicates:
bool ArticleSortDate(const Article* a, const Article* b);
bool ArticleSortName(const Article* a, const Article* b);

//=========================================================================
class ArticleMetaSorter {
public:
  ArticleMetaSorter(const std::string& meta, bool reverse);
  bool operator()(const Article* a, const Article* b);

private:
  std::string m_meta;
  bool m_reverse;
};


//=========================================================================
class Article : public XMLDoc {

public:

  Article(Profile& profile,
          const std::string& name,
          const scx::FilePath& root);
  
  ~Article();

  const scx::FilePath& get_root() const;
  const scx::Arg* get_meta(const std::string& name) const;
  
  // ArgObject interface
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);

protected:

  Profile& m_profile;
  scx::FilePath m_root;

  scx::ArgStore m_metastore;

};

#endif
