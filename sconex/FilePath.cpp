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

#include <sconex/FilePath.h>
#include <sconex/FileDir.h>
#include <sconex/FileStat.h>
#include <sconex/File.h>
#include <sconex/User.h>
#include <sconex/ScriptTypes.h>
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
std::string FilePath::pop()
{
  std::string popped;
  std::string::size_type i = m_path.rfind(path_sep[0]);
  if (i == std::string::npos) {
    popped = m_path;
    m_path = "";

  } else if (i == 0) {
    if (m_path.size() > 1) {
      popped = m_path.substr(1);
      m_path = path_sep;
    }

  } else {
    popped = m_path.substr(i+1);
    m_path = m_path.substr(0,i);
  }
    
  return popped;
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
  if (m_path.empty()) {
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
  // Remove any trailing slash
  unsigned int len = pathstr.length();
  if (len > 1 && pathstr[len-1] == path_sep[0]) {
    pathstr.erase(len-1,1);
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
bool FilePath::valid_filename(const std::string& name)
{
  // I suspect this can be improved
  if (name.empty() ||
      name == "." ||
      name == ".." ||
      std::string::npos != name.find("/")) return false;
  return true;
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


// ### ScriptFile ###

//=========================================================================
ScriptFile::ScriptFile(const scx::FilePath& path, const std::string& filename)
  : ScriptObject(),
    m_path(path),
    m_filename(filename)
{

}

//=========================================================================
ScriptFile::ScriptFile(const ScriptFile& c)
  : ScriptObject(),
    m_path(c.m_path),
    m_filename(c.m_filename)
{

}

//=========================================================================
ScriptFile::~ScriptFile()
{

}

//=========================================================================
scx::ScriptObject* ScriptFile::new_copy() const
{
  return new ScriptFile(*this);
}

//=========================================================================
std::string ScriptFile::get_string() const
{
  return m_filename;
}

//=========================================================================
int ScriptFile::get_int() const
{
  return !m_filename.empty();
}

//=========================================================================
ScriptRef* ScriptFile::script_op(const ScriptAuth& auth,
				 const ScriptRef& ref,
				 const ScriptOp& op,
				 const ScriptRef* right)
{
  if (op.type() == ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    if (name == "filename") 
      return ScriptString::new_ref(m_filename);
    if (name == "exists")
      return ScriptInt::new_ref(FileStat(m_path).exists());
    if (name == "is_file")
      return ScriptInt::new_ref(FileStat(m_path).is_file());
    if (name == "is_dir")
      return ScriptInt::new_ref(FileStat(m_path).is_dir());
    if (name == "size") 
      return ScriptInt::new_ref(FileStat(m_path).size());
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
const FilePath& ScriptFile::get_path() const
{
  return m_path;
}

//=========================================================================
const std::string& ScriptFile::get_filename() const
{
  return m_filename;
}


};
