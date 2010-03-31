/* SconeServer (http://www.sconemad.com)

Sconesite Profile

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

#ifndef sconesiteProfile_h
#define sconesiteProfile_h

#include "sconex/Stream.h"
#include "sconex/FilePath.h"
#include "sconex/ArgObject.h"

class SconesiteModule;
class Article;
class Template;

//=========================================================================
class Profile : public scx::ArgObjectInterface {

public:

  Profile(SconesiteModule& module,
          const std::string& name,
          const scx::FilePath& path);
  
  ~Profile();

  void refresh();

  SconesiteModule& get_module();
  scx::FilePath& get_path();

  Article* get_index();
  
  Template* lookup_template(const std::string& name);

  // ArgObject interface
  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);
  
private:

  SconesiteModule& m_module;

  std::string m_name;
  scx::FilePath m_path;

  Article* m_index;
  
  std::list<Template*> m_templates;

  scx::Time m_purge_threshold;
  
};



#endif
