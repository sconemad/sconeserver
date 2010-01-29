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
#include "sconex/FileDir.h"
#include "sconex/FileStat.h"
#include "sconex/File.h"
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
bool FilePath::rmdir(const FilePath& path, bool recursive)
{
  if (recursive) {
    FileDir dir(path);
    while (dir.next()) {
      if (dir.name() == "." || dir.name() == "..") {
        // nothing
      } else if (dir.stat().is_dir()) {
        rmdir(dir.path(),true);
      } else {
        rmfile(dir.path());
      }
    }
  }
  
  return (0 == ::rmdir(path.path().c_str()));
}

//=============================================================================
bool FilePath::rmfile(const FilePath& path)
{
  return (0 == ::unlink(path.path().c_str()));
}

//=============================================================================
bool FilePath::chown(const FilePath& path, const User& user)
{
  return (0 == ::chown(path.path().c_str(),
                       user.get_user_id(),
                       user.get_group_id()));
}

//=============================================================================
bool FilePath::copy(const FilePath& source, const FilePath& dest, mode_t mode)
{
  File fsource;
  if (scx::Ok != fsource.open(source,scx::File::Read)) {
    return false;
  }

  if (mode == 0) {
    // Copy source file's mode
    mode = fsource.stat().mode();
  }
  
  File fdest;
  if (scx::Ok != fdest.open(dest,scx::File::Write|File::Create|File::Truncate,mode)) {
    return false;
  }

  const unsigned int buffer_size = 1024;
  char buffer[buffer_size];
  int na = 0;
  Condition c;

  while (true) {
    if (scx::Ok != (c = fsource.read(buffer,buffer_size,na))) {
      break;
    }
    if (scx::Ok != (c = fdest.write(buffer,na,na))) {
      break;
    }
  }

  return (c == scx::End);
}

//=============================================================================
bool FilePath::move(const FilePath& source, const FilePath& dest)
{
  // Try a rename first, this will only work if the source and dest are on the
  // same device
  if (0 == ::rename(source.path().c_str(),dest.path().c_str())) {
    return true;
  }

  if (EXDEV == errno) {
    // Cross device, copy then delete source
    if (copy(source,dest)) {
      if (rmfile(source)) {
        return true;
      }
    }
  }

  DEBUG_LOG_ERRNO("FilePath::move failed");
  return false;
}


};
