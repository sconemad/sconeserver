/* SconeServer (http://www.sconemad.com)

File statistics

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

#ifndef scxFileStat_h
#define scxFileStat_h

#include "sconex.h"
#include "TimeDate.h"

#include <sys/stat.h>
#include <sys/types.h>

namespace scx {

class File;
class FilePath;
  
//=============================================================================
class SCONEX_API FileStat {

public:

  FileStat();
  // Construct an invalid filestat
  
  FileStat(const FilePath& filepath);
  // Construct an object to access stats for the specified file
  // NOTE: Can also be constructed from a File object)
  
  ~FileStat();
  // Destructor

  bool exists() const;
  // Does the file exist
  
  bool is_file() const;
  // Is this a regular file
  
  bool is_dir() const;
  // Is this a directory
  
  long size() const;
  // Get size of file in bytes
  
  const Date& time() const;
  // Get last modification time
 
private:

  friend class File;
  friend class FileDir;
  FileStat(int filedes);

  void become(struct stat* a);
  
#ifdef WIN32
  void become(WIN32_FIND_DATA* a);
#endif

  int m_type;
  long m_size;
  Date m_time;
  
};


};
#endif
