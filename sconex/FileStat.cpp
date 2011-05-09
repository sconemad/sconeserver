/* SconeServer (http://www.sconemad.com)

Sconex file stats

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

#include <sconex/FileStat.h>
#include <sconex/FilePath.h>
namespace scx {

//=============================================================================
FileStat::FileStat()
  : m_mode(0)
{
  DEBUG_COUNT_CONSTRUCTOR(FileStat);
}
  
//=============================================================================
FileStat::FileStat(const FilePath& filepath)
  : m_mode(0)
{
  DEBUG_COUNT_CONSTRUCTOR(FileStat);

  struct stat s;
  if (0 == ::stat(filepath.path().c_str(),&s)) {
    become(&s);
  }
}

//=============================================================================
FileStat::~FileStat()
{
  DEBUG_COUNT_DESTRUCTOR(FileStat);
}

//=============================================================================
bool FileStat::exists() const
{
  return m_mode != 0;  
}

//=============================================================================
bool FileStat::is_file() const
{
  return S_ISREG(m_mode);
}

//=============================================================================
bool FileStat::is_dir() const
{
  return S_ISDIR(m_mode);
}

//=============================================================================
mode_t FileStat::mode() const
{
  return m_mode;
}

//=============================================================================
long FileStat::size() const
{
  return m_size;
}

//=============================================================================
const Date& FileStat::time() const
{
  return m_time;
}

//=============================================================================
FileStat::FileStat(int filedes)
{
  DEBUG_COUNT_CONSTRUCTOR(FileStat);

  struct stat s;
  if (0 == ::fstat(filedes,&s)) {
    become(&s);
  }
}

//=============================================================================
void FileStat::become(struct stat* a)
{
  if (a==0) {
    m_mode = 0;
    return;
  }

  m_mode = a->st_mode;      
  m_size = (long)a->st_size;
  m_time = Date(a->st_mtime);
}

};
