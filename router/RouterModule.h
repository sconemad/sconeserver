/* SconeServer (http://www.sconemad.com)

Router module

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

#ifndef scxRouterModule_h
#define scxRouterModule_h

#include "sconex/Module.h"

class RouterChain;

//#########################################################################
class RouterModule : public scx::Module {

public:

  RouterModule();
  virtual ~RouterModule();

  virtual std::string info() const;

  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

  void add(const std::string& name,RouterChain* c);
  RouterChain* find(const std::string& name);
  
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);

private:

  typedef HASH_TYPE<std::string,RouterChain*> RouterChainMap;
  RouterChainMap m_chains;
  
};
  
#endif
