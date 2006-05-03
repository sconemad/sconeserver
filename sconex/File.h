/* SconeServer (http://www.sconemad.com)

Disk file

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

#ifndef scxFile_h
#define scxFile_h

#include "sconex/sconex.h"
#include "sconex/Descriptor.h"
#include "sconex/FilePath.h"
#include "sconex/FileStat.h"

namespace scx {

//=============================================================================
class SCONEX_API File : public Descriptor {

public:

  File();
  virtual ~File();

  static const int Read;
  static const int Write;
  static const int Create;
  static const int Truncate;
  static const int Append;
  
  Condition open(
    const FilePath& filepath,
    int mode
  );
  // Open a file

  virtual void close();
  // Destruct the object, closing the file

  virtual std::string describe() const;
  // Describe the socket

  virtual int fd();
  
  Condition seek(
    long offset,
    int origin =SEEK_SET
  );
  // Move the file pointer to the specified offset

  long tell() const;
  // Get the current file position

  long size() const;
  // Get the file size

  bool is_open() const;
  // Is the file open

  FileStat stat() const;
  // Get file stats
  
protected:

  virtual int event_create();

  virtual Condition endpoint_read(void* buffer,int n,int& na);
  virtual Condition endpoint_write(const void* buffer,int n,int& na);

  int  m_file;

  FilePath m_filepath;

private:

};


};
#endif
