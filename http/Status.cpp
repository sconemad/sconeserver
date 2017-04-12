/* SconeServer (http://www.sconemad.com)

HTTP Status

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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


#include <http/Status.h>

namespace http {

//===========================================================================
Status::Status(Status::Code code)
  : m_code(code)
{
  
}

//===========================================================================
Status::~Status()
{

}

//===========================================================================
bool Status::valid() const
{
  return !desc().empty();
}

//===========================================================================
Status::Code Status::code() const
{
  return m_code;
}

//===========================================================================
std::string Status::desc() const
{
  switch ((int)m_code) {
    case 100: return "OK";
    case 101: return "Switching Protocols";
        
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authorative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authenitcation Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "Request-URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested Range Not Satisfiable";
    case 417: return "Expectation Failed";
    
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
  }

  return std::string();
}
  
//===========================================================================
std::string Status::string() const
{
  std::ostringstream oss;
  oss << ((int)m_code) << " " << desc();
  return oss.str();
}

//===========================================================================
bool Status::has_body() const
{
  return (m_code >= 200 && m_code != 204 && m_code != 304);
}
  
//const char* CRLF = "\r\n";
//const char* HTTP = "HTTP";
//const char* HEADER_CONTENT_LENGTH = "Content-Length";

};
