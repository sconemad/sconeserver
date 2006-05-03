/* SconeServer (http://www.sconemad.com)

Router chain

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef RouterChain_h
#define RouterChain_h

#include "sconex/ArgObject.h"

namespace scx { class Descriptor; class Module; };
class RouterNode;

//#############################################################################
class RouterChain : public scx::ArgObjectInterface {

public:

  RouterChain(
    const std::string& name,
    scx::Module& module
  );

  virtual ~RouterChain();

  bool connect(
    scx::Descriptor* d
  );
  
  void add(RouterNode* n);

  virtual std::string name() const;
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  // ArgObjectInterface
  
protected:

  std::string m_name;
  
  std::list<RouterNode*> m_nodes;

  scx::Module& m_module;

private:

};

//#############################################################################
class RouterNode {

public:
  
  RouterNode(
    const std::string& name,
    scx::ArgList* args,
    scx::Module& module
  );
  
  ~RouterNode();

  bool connect(
    scx::Descriptor* d
  );

  std::string get_string() const;
  
protected:

  std::string m_name;
  scx::ArgList* m_args;

  scx::Module& m_module;
  
};

#endif
