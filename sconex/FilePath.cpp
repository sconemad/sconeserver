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
#include "sconex/User.h"
namespace scx {

const char* path_sep = "/";
const char* bad_path_sep = "\\";
  
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

  return false;
}

//=============================================================================
bool FilePath::mkdir(const FilePath& path, bool recursive, mode_t mode)
{
  const std::string& str = path.path();

  if (recursive) {
    std::string::size_type i = 0;
    while (true) {
      i = str.find(path_sep[0],i);
      if (i == std::string::npos) {
        break;
      }
      std::string sub = std::string(str,0,i);
      ::mkdir(sub.c_str(),mode);
      ++i;
    }
  }
  
  return (0 == ::mkdir(str.c_str(),mode));
}

//=============================================================================
bool FilePath::chown(const FilePath& path, const User& user)
{
  return (0 == ::chown(path.path().c_str(),
                       user.get_user_id(),
                       user.get_group_id()));
}

};
