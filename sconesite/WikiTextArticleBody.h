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

#ifndef sconesiteWikiTextArticleBody_h
#define sconesiteWikiTextArticleBody_h

#include "ArticleBody.h"

#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/Mutex.h>

class Context;

//=========================================================================
// WikiTextArticleBody - An article body implementation for WikiText 
// documents
//
class WikiTextArticleBody : public ArticleBody {
public:

  WikiTextArticleBody(const std::string& name,
		      const scx::FilePath& path);
  
  virtual ~WikiTextArticleBody();
  
  virtual const scx::FilePath& get_root() const;
  virtual const std::string& get_file() const;
  virtual scx::FilePath get_filepath() const;

  virtual bool process(Context& context);

  void parse_error(const std::string& msg);

  // Unload the article if it hasn't been accessed since purge_time
  virtual bool purge(const scx::Date& purge_time);
  
  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  typedef scx::ScriptRefTo<WikiTextArticleBody> Ref;

protected:

  scx::FilePath m_root;
  std::string m_file;

  std::string m_errors;
  
  scx::Date m_last_access;
  int m_clients;
  volatile bool m_opening;

  static scx::Mutex* m_clients_mutex;
};

#endif
