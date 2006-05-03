/* SconeServer (http://www.sconemad.com)

SconeX disk file stream

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

#include "sconex/File.h"
namespace scx {

const int File::Read = 1;
const int File::Write = 2;
const int File::Create = 4;
const int File::Truncate = 8;
const int File::Append = 16;
  
//=============================================================================
File::File()
  : m_file(-1)
{
  DEBUG_COUNT_CONSTRUCTOR(File);
}

//=============================================================================
File::~File()
{
  close();
  DEBUG_COUNT_DESTRUCTOR(File);
}

//=============================================================================
// Open file specified by filename
//
Condition File::open(
  const FilePath& filepath,
  int flags
)
{
  // Check file is not already open
  if (is_open()) {
    return scx::Error;
  }

  // Translate flags
  int f_flags = 0;
  int f_mode = 0;
  
  if ((flags & Read) && (flags & Write)) {
    f_flags = O_RDWR;
  } else if (flags & Read) {
    f_flags = O_RDONLY;
  } else if (flags & Write) {
    f_flags = O_WRONLY;
  } else {
    DESCRIPTOR_DEBUG_LOG("File::open() Must specify read or write flags");
  }
  
  if (flags & Create) {
    f_flags |= O_CREAT;
#ifdef WIN32
    f_mode = S_IREAD | S_IWRITE;
#else
    f_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#endif
  }

  if (flags & Truncate) {
    f_flags |= O_TRUNC;
  }

  if (flags & Append) {
    f_flags |= O_APPEND;
  }

  // Try to open the file
  if ((m_file = ::open(filepath.path().c_str(),f_flags,f_mode)) < 0) {
    return scx::Error;
  }
  
  m_filepath = filepath;
  m_state = Connected;
  return scx::Ok;
}

//=============================================================================
// Close the file
//
void File::close()
{
  // Check file is open
  if (!is_open()) {
    return;
  }

  if (::close(m_file) != 0) {
    return;
  }

  m_file = -1;
  m_state = Closed;
  return;
}

//=============================================================================
std::string File::describe() const
{
  return std::string("File (") + m_filepath.path() + ")";
}

//=============================================================================
int File::fd()
{
  return m_file;
}

//=============================================================================
// Seek to a specified file position
//
Condition File::seek(long offset,int origin)
{
  // Check file is open
  if (!is_open()) {
    return scx::Error;
  }

  // Perform the seek
  if (lseek(m_file,offset,origin) < 0) {
    return scx::Error;
  }
  
  return scx::Ok;
}

//=============================================================================
// Get the current file position
//
long File::tell() const
{
  // Check file is open
  if (!is_open()) {
    return 0;
  }

  // Seeking to the current position will return the current offset
  return lseek(m_file,0,SEEK_CUR);
}

//=============================================================================
// Get the file size
//
long File::size() const
{
  // Check file is open
  if (!is_open()) {
    return 0;
  }

  File* uc = (File*)this;

  // Seeking to the end will give the file length
  long save = tell();
  uc->seek(0,SEEK_END);

  // Seek back to where we were
  long size = tell();
  uc->seek(save);

  return size;
}

//=============================================================================
bool File::is_open() const
{
  return (m_file >= 0);
}

//=============================================================================
FileStat File::stat() const
{
  return FileStat(m_file);
}

//=============================================================================
int File::event_create()
{
  return 0;
}

//=============================================================================
// Read n bytes from file into buffer
//
Condition File::endpoint_read(void* buffer,int n,int& na)
{
  // Check file is open
  if (!is_open()) {
    return scx::Error;
  }

  // Perform the read
  na = ::read(m_file,buffer,n);

  if (na > 0) {
    // Some, if not all requested, bytes were read
    return scx::Ok;
    
  } else if (na == 0) {
    // End of file reached
    return scx::End;

  } else if (error() == Wait) {
    // No data available right now, but not an error as such
    na=0;
    return scx::Wait; 
  }

  // Fatal error occured
  na = 0;
  DESCRIPTOR_DEBUG_LOG("File::endpoint_read() error");
  return scx::Error;
}

//=============================================================================
// Write n bytes from file into buffer
//
Condition File::endpoint_write(const void* buffer,int n,int& na)
{
  // Check file is open
  if (!is_open()) {
    return scx::Error;
  }

  na = ::write(m_file,buffer,n);
  
  if (na > 0) {
    // Written some or all of the data ok
    return scx::Ok;

  } else if (error() == Wait) {
    // Cannot write right now
    na=0;
    return scx::Wait;
  }

  // Fatal error occured
  na = 0;
  DESCRIPTOR_DEBUG_LOG("File::endpoint_write() error");
  return scx::Error;
}

};
