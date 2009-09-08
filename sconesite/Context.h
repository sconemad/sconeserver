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
#include "sconex/ArgObject.h"

//=========================================================================
class Context : public scx::ArgObjectInterface {

public:

  Context();
  ~Context();

  // XMLDoc interface
  virtual bool handle_start(const std::string& name, XMLAttrs& attrs, bool empty) =0;
  virtual bool handle_end(const std::string& name, XMLAttrs& attrs) =0;
  virtual void handle_process(const std::string& name, const char* data) =0;
  virtual void handle_text(const char* text) =0;
  virtual void handle_comment(const char* text) =0;
  virtual void handle_error() =0;

  // ArgObject interface
  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:
  
};

#endif
