/* SconeServer (http://www.sconemad.com)

HTTP filesystem node

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

#ifndef httpFSNode_h
#define httpFSNode_h

#include "http/http.h"
#include "sconex/sconex.h"
#include "sconex/ArgObject.h"
#include "sconex/TimeDate.h"
#include "sconex/FilePath.h"
namespace http {

class FSDirectory;

//=============================================================================
class HTTP_API FSNode : public scx::ArgObjectInterface {

public:

  enum Type { Directory, File };

  FSNode(Type nodetype, const std::string& name);
  virtual ~FSNode();

  virtual Type type() const;
  // Get the node type

  virtual std::string name() const;
  // Get the name of this file system node.

  virtual std::string url() const;
  // Get the url path to this file system node.
  
  virtual scx::FilePath path() const;
  // Get the local path to this file system node.

  const FSDirectory* parent() const;
  FSDirectory* parent();
  // Get the parent node

  virtual bool build() =0;
  // Recursively build or update the node tree.

  bool modified() const;
  // See if the file associated with this node has been modified since the
  // last call to set_unmodified. Nodes are also reported as modified if
  // set_unmodified has never been called on the node.
  
  void set_unmodified();
  // Set the node as being unmodified.
  
  // ArgObject interface:
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:

private:

  Type m_type;
  std::string m_name;

  friend class FSDirectory;
  FSDirectory* m_parent;

  scx::Date m_modtime;

};

};
#endif
