/* SconeServer (http://www.sconemad.com)

http Request

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

#ifndef httpRequest_h
#define httpRequest_h

#include "http/HeaderTable.h"
#include "sconex/Arg.h"
#include "sconex/VersionTag.h"
#include "sconex/Uri.h"
namespace http {

//=============================================================================
class HTTP_API Request : public scx::Arg {

public:

  Request();
  Request(const Request& c);
  
  virtual ~Request();

  scx::Arg* new_copy() const;

  const std::string& get_method() const;
  const scx::Uri& get_uri() const;
  const scx::VersionTag& get_version() const;
  bool parse_request(const std::string& str, bool secure);

  std::string get_header(const std::string& name) const;
  bool parse_header(const std::string& str);

  // Arg methods
  virtual std::string get_string() const;
  virtual int get_int() const;
  virtual scx::Arg* op(
    scx::Arg::OpType optype,
    const std::string& opname,
    scx::Arg* right
  );
  
protected:

  std::string m_method;
  scx::Uri m_uri;
  scx::VersionTag m_version;
  HeaderTable m_headers;
  
private:

};

};
#endif
