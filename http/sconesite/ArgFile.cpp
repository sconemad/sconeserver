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


#include "ArgFile.h"
#include "sconex/FileStat.h"

//=========================================================================
ArgFile::ArgFile(const scx::FilePath& path, const std::string& filename)
  : m_path(path),
    m_filename(filename)
{

}

//=========================================================================
ArgFile::ArgFile(const ArgFile& c)
  : m_path(c.m_path),
    m_filename(c.m_filename)
{

}

//=========================================================================
ArgFile::~ArgFile()
{

}

//=========================================================================
scx::Arg* ArgFile::new_copy() const
{
  return new ArgFile(*this);
}

//=========================================================================
std::string ArgFile::get_string() const
{
  return m_filename;
}

//=========================================================================
int ArgFile::get_int() const
{
  return !m_filename.empty();
}

//=========================================================================
scx::Arg* ArgFile::op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right)
{
  if (scx::Arg::Binary == optype && "." == opname) {
    std::string name = right->get_string();
    if (name == "exists") return new scx::ArgInt(77);
    if (name == "filename") return new scx::ArgString(m_filename);
    if (name == "size") return new scx::ArgInt(scx::FileStat(m_path).size());
  }
  return SCXBASE Arg::op(auth,optype,opname,right);
}

//=========================================================================
const scx::FilePath& ArgFile::get_path() const
{
  return m_path;
}

//=========================================================================
const std::string& ArgFile::get_filename() const
{
  return m_filename;
}
