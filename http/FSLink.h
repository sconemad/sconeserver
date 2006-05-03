/* SconeServer (http://www.sconemad.com)

HTTP filesystem link node

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

#ifndef httpFSLink_h
#define httpFSLink_h

#include "http/FSDirectory.h"
#include "sconex/sconex.h"
#include "sconex/FilePath.h"
namespace http {

//=============================================================================
class HTTP_API FSLink : public FSDirectory {

public:

  FSLink(
    const std::string& name,
    const scx::FilePath& target
  );
  
  virtual ~FSLink();

  virtual std::string url() const;
  // Get the url path to this file system node.
  
  virtual scx::FilePath path() const;
  // Get the local path to this file system node.

protected:

  scx::FilePath m_target;
  // The target of the link

private:

};

};
#endif
