/* SconeServer (http://www.sconemad.com)

Sconesite file

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef sconesiteArgFile_h
#define sconesiteArgFile_h

#include "sconex/Arg.h"
#include "sconex/FilePath.h"

//=============================================================================
class ArgFile : public scx::Arg {

public:

  ArgFile(const scx::FilePath& path, const std::string& filename);
  ArgFile(const ArgFile& c);
  virtual ~ArgFile();
  virtual scx::Arg* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::Arg* op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right);

  const scx::FilePath& get_path() const;
  const std::string& get_filename() const;
  
protected:

  scx::FilePath m_path;
  std::string m_filename;

};

#endif
