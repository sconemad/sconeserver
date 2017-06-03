/* SconeServer (http://www.sconemad.com)

HTTP Status

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

#ifndef httpStatus_h
#define httpStatus_h

#include <http/http.h>

#include <sconex/sconex.h>
namespace http {
  
//===========================================================================
class HTTP_API Status {

public:

  enum Code {
    Continue = 100,
    SwitchingProtocols,

    Ok = 200,
    Created,
    Accepted,
    NonAuthorativeInformation,
    NoContent,
    ResetContent,
    PartialContent,
    
    MultipleChoices = 300,
    MovedPermanently,
    Found,
    SeeOther,
    NotModified,
    UseProxy = 305,
    TemporaryRedirect = 307,

    BadRequest = 400,
    Unauthorized,
    PaymentRequired,
    Forbidden,
    NotFound,
    MethodNotAllowed,
    NotAcceptable,
    ProxyAuthenitcationRequired,
    RequestTimeout,
    Conflict,
    Gone,
    LengthRequired,
    PreconditionFailed,
    RequestEntityTooLarge,
    RequestURITooLong,
    UnsupportedMediaType,
    RequestedRangeNotSatisfiable,
    ExpectationFailed,

    InternalServerError = 500,
    NotImplemented,
    BadGateway,
    ServiceUnavailable,
    GatewayTimeout,
    HTTPVersionNotSupported
  };
  
  Status(Code code);
  ~Status();

  bool valid() const;
  // Is this status code valid?

  Code code() const;
  // Get the status code

  std::string desc() const;
  // Get description i.e. "OK" or "Not Found"
  
  std::string string() const;
  // Get complete string including code and description
  // i.e. "200 OK" or "404 Not Found"

  // Should a response with this status code include a message body?
  bool has_body() const;
  
private:

  Code m_code;
  
};
  
};

#endif
