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
Response::~Response()
{

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

//=========================================================================
scx::Arg* Response::arg_lookup(const std::string& name)
{
  // Methods
  if ("set_header" == name ||
      "set_status" == name) {
    return new_method(name);
  }
  
  if (name == "version") return m_version.new_copy();
  if (name == "status") return new scx::ArgString(m_status.string());
  if (name == "statuscode") return new scx::ArgInt(m_status.code());

  return SCXBASE ArgObjectInterface::arg_lookup(name);
}

//=========================================================================
scx::Arg* Response::arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (name == "set_header") {
    const scx::ArgString* a_header =  dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_header) return new scx::ArgError("set_header() No name specified");

    const scx::ArgString* a_value = dynamic_cast<const scx::ArgString*>(l->get(1));
    if (!a_value) return new scx::ArgError("set_header() No value specified");

    set_header(a_header->get_string(),a_value->get_string());
    return 0;
  }

  if (name == "set_status") {
    const scx::ArgInt* i_status =  dynamic_cast<const scx::ArgInt*>(l->get(0));
    if (!i_status) return new scx::ArgError("set_status() No status code specified");

    Status status((Status::Code)i_status->get_int());
    if (!status.valid()) return new scx::ArgError("set_status() Invalid status code specified");

    set_status(status);
    return 0;
  }

  return SCXBASE ArgObjectInterface::arg_method(auth,name,args);
}

};
