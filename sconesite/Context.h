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

#ifndef sconesiteContext_h
#define sconesiteContext_h

#include "XMLDoc.h"

#include "sconex/Stream.h"
#include "sconex/Descriptor.h"
#include "sconex/FilePath.h"
#include "sconex/FileDir.h"

class Profile;
class Article;

//=========================================================================
class Context {

public:

  Context(Profile& profile,
          scx::Descriptor& output);

  virtual ~Context();

  void set_article(Article* article);

  // XMLDoc interface
  virtual bool handle_start(const std::string& name, XMLAttrs& attrs, bool empty);
  virtual bool handle_end(const std::string& name, XMLAttrs& attrs);
  virtual void handle_process(const std::string& name, const char* data);
  virtual void handle_text(const char* text);
  virtual void handle_comment(const char* text);
  virtual void handle_error();
  
protected:
  
  Profile& m_profile;
  scx::Descriptor& m_output;

  Article* m_article;

  bool m_articles_mode;
  std::list<Article*>::const_iterator m_articles_it;

  bool m_files_mode;
  scx::FileDir m_files_it;
  
};

#endif
