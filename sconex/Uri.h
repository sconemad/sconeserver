/* SconeServer (http://www.sconemad.com)

Uniform Resource Indentifier 

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

#ifndef scxUri_h
#define scxUri_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

//===========================================================================
// Uri - Uniform Resource Indicator in the form 
// "scheme://host:port/path?query" as defined in RFC2396.
//
// NOTE: The scheme and host parts are case-insensitive, and are always stored
// in lowercase.
//
class SCONEX_API Uri : public ScriptObject {
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
  Uri(const ScriptRef* args);
  Uri(const Uri& c);
  virtual ~Uri();

  ScriptObject* new_copy() const;

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

  std::string get_base() const;
  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);
  
  Uri& operator=(const Uri& v);

  bool operator==(const Uri& v) const;
  bool operator!=(const Uri& v) const;

  static short default_port(const std::string& scheme);

  static std::string encode(const std::string& str);
  static std::string decode(const std::string& str);
  
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
