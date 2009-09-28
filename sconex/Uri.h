/* SconeServer (http://www.sconemad.com)

Uniform Resource Indentifier 

In the form "scheme://host:port/path&query"
As defined in RFC2396

NOTE: The scheme and host parts are case-insensitive, and are always stored
in lowercase.

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

#ifndef scxUri_h
#define scxUri_h

#include "sconex/sconex.h"
#include "sconex/Arg.h"
namespace scx {

//===========================================================================
class SCONEX_API Uri : public Arg {

public:

  Uri();
  Uri(const std::string& str);
  Uri(
    const std::string& scheme,
    const std::string& host,
    short port,
    const std::string& path,
    const std::string& query = ""
  );
  Uri(Arg* args);
  Uri(const Uri& c);
  virtual ~Uri();
  Arg* new_copy() const;

  void set_scheme(const std::string& scheme);
  void set_host(const std::string& host);
  void set_port(short port);
  void set_path(const std::string& path);
  void set_query(const std::string& query);
  
  const std::string& get_scheme() const;
  const std::string& get_host() const;
  short get_port() const;
  const std::string& get_path() const;
  const std::string& get_query() const;

  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual Arg* op(const Auth& auth, OpType optype, const std::string& opname, Arg* right);
  
  bool operator==(const Uri& v) const;
  bool operator!=(const Uri& v) const;

  static short default_port(const std::string& scheme);

protected:
  
  void from_string(const std::string& str);
  
  std::string m_scheme;
  std::string m_host;
  short m_port;
  std::string m_path;
  std::string m_query;
};

};

#endif
