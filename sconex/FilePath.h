/* SconeServer (http://www.sconemad.com)

File path

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

#ifndef scxFilePath_h
#define scxFilePath_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

class User;
  
//=============================================================================
class SCONEX_API FilePath {
public:

  explicit FilePath(const std::string& path = "");
  explicit FilePath(const char* path);
  FilePath(const FilePath& c);
    
  ~FilePath();

  const std::string& path() const;

  // Pop and return the last directory, 
  // i.e. so "/one/two/three" becomes "/one/two" and returns "three"
  std::string pop();

  FilePath& operator=(const FilePath& a);
  
  FilePath operator+(const FilePath& a) const;
  bool operator==(const FilePath& a) const;

  void operator+=(const FilePath& a);
  
  static void normalize(std::string& path);

  // Is this an absolute path
  static bool is_root(const std::string& path);

  // Is this a valid file name
  static bool valid_filename(const std::string& name);

  static bool mkdir(const FilePath& path, bool recursive, mode_t mode);
  static bool rmdir(const FilePath& path, bool recursive=false);
  static bool rmfile(const FilePath& path);
  static bool chown(const FilePath& path, const User& user);

  static bool move(const FilePath& source, const FilePath& dest);
  static bool copy(const FilePath& source, const FilePath& dest, mode_t mode=0);
  
protected:

  std::string m_path;

private:

};

//=============================================================================
class ScriptFile : public scx::ScriptObject {
public:

  ScriptFile(const FilePath& path, const std::string& filename);
  ScriptFile(const ScriptFile& c);
  virtual ~ScriptFile();

  const FilePath& get_path() const;
  const std::string& get_filename() const;
  
  // ScriptObject methods
  virtual ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  typedef scx::ScriptRefTo<ScriptFile> Ref;

protected:

  FilePath m_path;
  std::string m_filename;

};

};
#endif
