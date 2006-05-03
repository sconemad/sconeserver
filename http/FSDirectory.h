/* SconeServer (http://www.sconemad.com)

HTTP filesystem directory node

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpFSDirectory_h
#define httpFSDirectory_h

#include "http/FSNode.h"
#include "sconex/sconex.h"
namespace http {

//=============================================================================
class HTTP_API FSDirectory : public FSNode {

public:

  FSDirectory(const std::string& name);
  virtual ~FSDirectory();

  void add(FSNode* entry);
  void remove(FSNode* entry);
  const FSNode* find(const std::string& name) const;

  const std::list<FSNode*>& dir() const;

  virtual const FSNode* lookup(const std::string& name) const;
  // Locate a node in the tree
    
  virtual std::string lookup_mod(const std::string& name) const;
  // Lookup a module to handle this file name

  virtual bool build();

  // ArgObject interface:
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:

  std::list<FSNode*> m_dir;
  
  std::map<std::string,std::string> m_mods;
  
};

};
#endif
