/* SconeServer (http://www.sconemad.com)

Mime header

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

#ifndef scxMimeHeader_h
#define scxMimeHeader_h

#include "sconex/sconex.h"
namespace scx {

  
//=============================================================================
class SCONEX_API MimeHeaderValue {
public:
  MimeHeaderValue();

  const std::string& value() const;
  void set_value(const std::string& headervalue);

  bool get_parameter(const std::string& parname, std::string& parvalue) const;
  bool set_parameter(const std::string& parname, const std::string& parvalue);
  bool remove_parameter(const std::string& parname);

  std::string get_string() const;
  
private:
  std::string m_value;
  std::map<std::string,std::string> m_pars;
};

  
//=============================================================================
class SCONEX_API MimeHeader {
public:
  MimeHeader();
  MimeHeader(const std::string& name);

  bool parse_line(const std::string& header);
  bool parse_value(const std::string& str);
  
  const std::string& name() const;
  void set_name(const std::string& headername);
  
  int num_values() const;
  const MimeHeaderValue* get_value() const;
  const MimeHeaderValue* get_value(int index) const;
  const MimeHeaderValue* get_value(const std::string& value) const;
  bool add_value(const MimeHeaderValue& value);
  void remove_value(const std::string& value);
  void remove_all_values();
  
  std::string get_string() const;
  
private:
  std::string m_name;
  std::list<MimeHeaderValue> m_values;
};
  

//=============================================================================
class SCONEX_API MimeHeaderTable {
public:
  MimeHeaderTable();
  ~MimeHeaderTable();

  void set(const std::string& name,const std::string& value);
  bool erase(const std::string& name);
  std::string get(const std::string& name) const;
  MimeHeader get_parsed(const std::string& name) const;

  std::string get_all() const;

private:

  std::string normalize(const std::string& name) const;
  
  std::map<std::string,std::string> m_headers;

};

};
#endif
