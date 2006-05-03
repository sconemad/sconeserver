/* SconeServer (http://www.sconemad.com)

SSL connection module

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

#ifndef sslModule_h
#define sslModule_h

#include "sconex/Module.h"
#include "sconex/Descriptor.h"

class SSLChannel;

//=============================================================================
class SSLModule : public scx::Module {

public:

  SSLModule();
  virtual ~SSLModule();

  virtual std::string info() const;

  virtual int init();

  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

  SSLChannel* find_channel(const std::string& name);
  
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:

private:

  std::map<std::string,SSLChannel*> m_channels;
  
};

#endif

