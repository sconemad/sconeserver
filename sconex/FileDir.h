/* SconeServer (http://www.sconemad.com)

Directory enumerator

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

#ifndef scxFileDir_h
#define scxFileDir_h

#include "sconex/sconex.h"
#include "sconex/FileStat.h"
#include "sconex/FilePath.h"

#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#endif

namespace scx {

//=============================================================================
class SCONEX_API FileDir {

public:

  FileDir(const FilePath& root);
  // Construct diretory enumerator for specified directory
  
  ~FileDir();
  // Destructor

  void reset();
  // Reset iteration
  
  bool next();
  // Iterate to the next directory entry
  
  const FilePath& root() const;
  // Get root path of directory being enumerated
  
  const FilePath& name() const;
  // Get name of current file

  FilePath path() const;
  // Get full path to current file
  
  const FileStat& stat() const;
  // Get stats for current file
   
private:

  FilePath m_root;
  // Root directory being enumerated
  
  FilePath m_current_name;
  FileStat m_current_stat;
  // Details for current file
  
  int m_state;
  DIR* m_dir;
};


};
#endif
