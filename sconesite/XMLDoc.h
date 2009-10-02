/* SconeServer (http://www.sconemad.com)

Sconesite XML document

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

#ifndef sconesiteXMLDoc_h
#define sconesiteXMLDoc_h

#include "sconex/FilePath.h"
#include "sconex/Date.h"
#include "sconex/ArgObject.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

typedef std::map<std::string,std::string> XMLAttrs;

class Context;

//=========================================================================
class XMLDoc : public scx::ArgObjectInterface {

public:

  XMLDoc(const std::string& name,
         const scx::FilePath& path);
  
  virtual ~XMLDoc();
  
  const std::string& get_name() const;
  const scx::FilePath& get_path() const;

  bool process(Context& context);

  const scx::Date& get_modtime() const;

  void parse_error(const std::string& msg);

  // ArgObject interface
  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const scx::Auth& auth,const std::string& name,scx::Arg* args);

protected:

  virtual void process_node(Context& context, xmlNode* node);
  
  virtual bool open();
  virtual void close();  

  std::string m_name;
  scx::FilePath m_path;
  scx::Date m_modtime;

  xmlDoc* m_xmldoc;
  std::string m_errors;
  
};

#endif