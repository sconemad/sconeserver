/* SconeServer (http://www.sconemad.com)

http Document root

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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


#include "http/DocRoot.h"
#include "http/Host.h"
namespace http {

//=========================================================================
DocRoot::DocRoot(
  Host& host,
  const std::string profile,
  const scx::FilePath& path
) : FSLink(profile,path),
    m_host(host)
{
  // A bit of a hack really, but set up default module mappings so it will
  // do something sensible if there is no host config file present.
  m_mods["."] = "dirindex";
  m_mods["!"] = "errorpage";
  m_mods["*"] = "getfile";

  // Note: this can override the above
  build();
}

//=========================================================================
DocRoot::~DocRoot()
{

}

//=========================================================================
const std::string DocRoot::get_profile() const
{
  return FSNode::name();
}

//=========================================================================
std::string DocRoot::name() const
{
  std::ostringstream oss;
  oss << "DOCROOT:" << get_profile();
  return oss.str();
}

//=============================================================================
scx::Arg* DocRoot::arg_resolve(const std::string& name)
{
  scx::Arg* a = SCXBASE ArgObjectInterface::arg_resolve(name);
  if (a==0 || (dynamic_cast<scx::ArgError*>(a)!=0)) {
    delete a;
    return m_host.arg_resolve(name);
  }
  return a;
}

};
