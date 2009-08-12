/* SconeServer (http://www.sconemad.com)

http Response

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


#include "http/Response.h"

#include "sconex/ModuleRef.h"
#include "sconex/StreamSocket.h"
#include "sconex/File.h"
#include "sconex/Module.h"
namespace http {

//===========================================================================
Response::Response()
  : m_status(Status::Ok)
{
  
}

//===========================================================================
Response::Response(const Response& c)
  : m_version(c.m_version),
    m_status(c.m_status),
    m_headers(c.m_headers)
{

}
  
//===========================================================================
Response::~Response()
{

}

//===========================================================================
scx::Arg* Response::new_copy() const
{
  return new Response(*this);
}

//===========================================================================
void Response::set_version(const scx::VersionTag& ver)
{
  m_version = ver;
}

//===========================================================================
const scx::VersionTag& Response::get_version() const
{
  return m_version;
}

//===========================================================================
void Response::set_status(const Status& status)
{
  m_status = status;
}

//===========================================================================
const Status& Response::get_status() const
{
  return m_status;
}

//===========================================================================
void Response::set_header(const std::string& name, const std::string& value)
{
  m_headers.set(name,value);
}

//===========================================================================
bool Response::remove_header(const std::string& name)
{
  return m_headers.erase(name);
}

//===========================================================================
std::string Response::get_header(const std::string& name) const
{
  return m_headers.get(name);
}

//===========================================================================
std::string Response::build_header_string()
{
  std::string str = "HTTP/" + m_version.get_string() + " " + 
    m_status.string() + CRLF +
    m_headers.get_all() + CRLF;

  return str;
}

//=============================================================================
std::string Response::get_string() const
{
  return std::string("RESPONSE(") + m_status.string() + ")";
}

//=============================================================================
int Response::get_int() const
{
  return m_status.valid();
}

//=============================================================================
scx::Arg* Response::op(
  scx::Arg::OpType optype,
  const std::string& opname,
  scx::Arg* right
)
{
  return 0;
}

};
