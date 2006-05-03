/* SconeServer (http://www.sconemad.com)

Sconex file path

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

#include "sconex/FilePath.h"
namespace scx {

#ifdef WIN32
  std::string path_sep("\\");
  std::string bad_path_sep("/");
#else
  const char* path_sep = "/";
  const char* bad_path_sep = "\\";
#endif
  
//=============================================================================
FilePath::FilePath(const std::string& path)
  : m_path(path)
{
  DEBUG_COUNT_CONSTRUCTOR(FilePath);
  normalize(m_path);
}

//=============================================================================
FilePath::FilePath(const char* path)
  : m_path(path)
{
  DEBUG_COUNT_CONSTRUCTOR(FilePath);
  normalize(m_path);
}
 
//=============================================================================
FilePath::FilePath(const FilePath& c)
  : m_path(c.m_path)
{
  DEBUG_COUNT_CONSTRUCTOR(FilePath);
}

//=============================================================================
FilePath::~FilePath()
{
  DEBUG_COUNT_DESTRUCTOR(FilePath);
}

//=============================================================================
const std::string& FilePath::path() const
{
  return m_path;
}

//=============================================================================
FilePath& FilePath::operator=(const FilePath& a)
{
  if (&a != this) {
    m_path = a.m_path;
  }
  return *this;
}

//=============================================================================
FilePath FilePath::operator+(const FilePath& a) const
{
  if (is_root(a.path())) {
    return FilePath(a);
  }
  std::string path = m_path + path_sep + a.m_path;
  return FilePath(path);
}

//=============================================================================
bool FilePath::operator==(const FilePath& a) const
{
  return m_path == a.m_path;
}

//=============================================================================
void FilePath::operator+=(const FilePath& a)
{
  *this = *this + a;
}

//=============================================================================
void FilePath::normalize(std::string& pathstr)
{
  if (!pathstr.empty()) {
    //  path.replace(path.find(bad_path_sep),1,path_sep);
    
    if (!is_root(pathstr)) {
      // Remove trailing slash
      unsigned int len = pathstr.length();
      DEBUG_ASSERT(len > 0,"normalize() Invalid path length");
      if (pathstr[len-1] == path_sep[0]) {
        pathstr.erase(len-1,1);
      }
    }
  }
}

//=============================================================================
bool FilePath::is_root(const std::string& pathstr)
{
  if (pathstr.empty()) {
    return false;
  }
  
  if (pathstr[0] == path_sep[0]) {
    return true;
  }

#ifdef WIN32
  if (pathstr.length() >= 3 &&
      pathstr[1] == ':' &&
      pathstr[2] == '\\') {
    return true;
  }
#endif
  
  return false;
}

};
