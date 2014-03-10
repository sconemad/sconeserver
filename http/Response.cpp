/* SconeServer (http://www.sconemad.com)

HTTP Response

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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


#include <http/Response.h>

#include <sconex/StreamSocket.h>
#include <sconex/File.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
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
bool Response::parse_response(const std::string& str)
{
  std::string::size_type start = 0;
  std::string::size_type end = 0;

  // Parse HTTP
  
  end = str.find_first_of("/");
  if (end == std::string::npos) {
    DEBUG_LOG("Bad response header (missing HTTP version)");
    return false;
  }
  std::string http = str.substr(start,end);
  if (http != "HTTP") {
    DEBUG_LOG("Bad response header (missing HTTP string)");
    return false;
  }
  start = end + 1;

  // Parse version
  
  std::string sdig;
  int ver_major=1;
  int ver_minor=0;

  end = str.find_first_of(".",start);
  if (end == std::string::npos) {
    DEBUG_LOG("Bad response header (missing HTTP version digit 1)");
    return false;
  }
  sdig = str.substr(start,end-start);
  ver_major = atoi(sdig.c_str());
  start = end + 1;

  end = str.find_first_of(" ",start);
  if (end == std::string::npos) {
    DEBUG_LOG("Bad response header (missing HTTP version digit 2)");
    return false;
  }
  sdig = str.substr(start,end-start);
  ver_minor = atoi(sdig.c_str());
  start = end + 1;

  m_version = scx::VersionTag(ver_major,ver_minor);

  // Parse status code

  start = str.find_first_not_of(" ",start);
  end = str.find_first_of(" ",start);
  if (end == std::string::npos) {
    DEBUG_LOG("Bad response header (missing status code)");
    return false;
  }
  sdig = str.substr(start,end-start);
  m_status = Status((Status::Code)atoi(sdig.c_str()));
  if (!m_status.valid()) {
    DEBUG_LOG("Bad response header (invalid status code)");
    return false;
  }

  // Ignore the rest, its not meant for us

  return true;
}

//===========================================================================
bool Response::parse_header(const std::string& str)
{
  std::string name = m_headers.parse_line(str);
  return (!name.empty());
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
std::string Response::get_string() const
{
  return "Response";
}

//=========================================================================
scx::ScriptRef* Response::script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("set_header" == name ||
	"set_status" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
    
    if (name == "version") 
      return new scx::ScriptRef(m_version.new_copy());
    if (name == "status") return scx::ScriptString::new_ref(m_status.string());
    if (name == "statuscode") return scx::ScriptInt::new_ref(m_status.code());
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* Response::script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args)
{
  if (name == "set_header") {
    const scx::ScriptString* a_header = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_header) 
      return scx::ScriptError::new_ref("set_header() No name specified");

    const scx::ScriptObject* a_value = 
      scx::get_method_arg<scx::ScriptObject>(args,1,"value");
    if (!a_value) 
      return scx::ScriptError::new_ref("set_header() No value specified");

    set_header(a_header->get_string(),a_value->get_string());
    return 0;
  }

  if (name == "set_status") {
    const scx::ScriptInt* i_status = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"status");
    if (!i_status) 
      return scx::ScriptError::new_ref("set_status() No status specified");

    Status status((Status::Code)i_status->get_int());
    if (!status.valid()) 
      return scx::ScriptError::new_ref("set_status() Invalid status code");

    set_status(status);
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

};
