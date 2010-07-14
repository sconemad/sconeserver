/* SconeServer (http://www.sconemad.com)

Sconesite Template

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


#include "Template.h"
#include "Article.h"
#include "Profile.h"
#include "Context.h"

#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/FileDir.h"


//=========================================================================
Template::Template(
  Profile& profile,
  const std::string& name,
  const scx::FilePath& path
) : XMLDoc(name,path,name + ".xml"),
    m_profile(profile),
    m_headings(1,name,0)
{

}

//=========================================================================
Template::~Template()
{

}

//=========================================================================
const ArticleHeading& Template::get_headings() const
{
  return m_headings;
}
