/* SconeServer (http://www.sconemad.com)

Sconex directory enumerator

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/FileDir.h"
namespace scx {

//=============================================================================
FileDir::FileDir(const FilePath& root)
  : m_root(root),
    m_state(0)
{
  DEBUG_COUNT_CONSTRUCTOR(FileDir);
#ifdef WIN32
  m_hsearch = INVALID_HANDLE_VALUE;
#else
  m_dir = 0;
#endif
}

//=============================================================================
FileDir::~FileDir()
{
  reset();
  DEBUG_COUNT_DESTRUCTOR(FileDir);
}

//=============================================================================
void FileDir::reset()
{
#ifdef WIN32
  if (m_hsearch != INVALID_HANDLE_VALUE) {
    FindClose(m_hsearch);
  }
#else
  if (m_dir) {
    closedir(m_dir);
  }
#endif
  m_state = 0;
}

//=============================================================================
bool FileDir::next()
{
#ifdef WIN32
  WIN32_FIND_DATA data;

  if (m_state == 0) {
    FilePath search = m_root + "*.*";
    m_hsearch = FindFirstFile(search.path().c_str(),&data);
    int i=0;
    if (m_hsearch != INVALID_HANDLE_VALUE) {
      m_state = 1;
      m_current_name = data.cFileName;
      m_current_stat.become(&data);
      return true;
    }
  }
  
  if (m_state == 1) {
    if (FindNextFile(m_hsearch,&data)) {
      m_current_name = data.cFileName;
      m_current_stat.become(&data);
      return true;
    }
  }

  m_state = 2;
  return false;
#else
  if (m_state == 0) {
    m_dir = opendir(m_root.path().c_str());
    if (m_dir) {
      m_state = 1;
    }
  }

  if (m_state == 1) {
    struct dirent* dent = readdir(m_dir);
    if (dent) {
      m_current_name = dent->d_name;
      m_current_stat = FileStat(path());
      return true;
    }
  }

  m_state = 2;
  return false;
#endif
}

//=============================================================================
const FilePath& FileDir::root() const
{
  return m_root;
}

//=============================================================================
const FilePath& FileDir::name() const
{
  return m_current_name;
}

//=============================================================================
FilePath FileDir::path() const
{
  return FilePath(m_root + m_current_name);
}

//=============================================================================
const FileStat& FileDir::stat() const
{
  return m_current_stat;
}

};
