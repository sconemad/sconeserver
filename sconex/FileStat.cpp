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

#include "sconex/FileStat.h"
#include "sconex/FilePath.h"
namespace scx {

static int StatTypeInvalid=0;
static int StatTypeFile=1;
static int StatTypeDir=2;
static int StatTypeSpecial=3;
  
//=============================================================================
FileStat::FileStat()
  : m_type(StatTypeInvalid)
{
  DEBUG_COUNT_CONSTRUCTOR(FileStat);
}
  
//=============================================================================
FileStat::FileStat(const FilePath& filepath)
  : m_type(StatTypeInvalid)
{
  DEBUG_COUNT_CONSTRUCTOR(FileStat);

  struct stat s;
  if (0==stat(filepath.path().c_str(),&s)) {
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
  return m_type != StatTypeInvalid;
}

//=============================================================================
bool FileStat::is_file() const
{
  return m_type == StatTypeFile;
}

//=============================================================================
bool FileStat::is_dir() const
{
  return m_type == StatTypeDir;
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
  if (0==fstat(filedes,&s)) {
    become(&s);
  }
}

//=============================================================================
void FileStat::become(struct stat* a)
{
  if (a==0) {
    m_type = StatTypeInvalid;
    return;
  }

  if (S_ISREG(a->st_mode)) {
    m_type = StatTypeFile;
    
  } else if (S_ISDIR(a->st_mode)) {
    m_type = StatTypeDir;
    
  } else {
    m_type = StatTypeSpecial;
  }
      
  m_size = (long)a->st_size;
  m_time = Date(a->st_mtime);
}

};
