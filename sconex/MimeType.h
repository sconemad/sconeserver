/* SconeServer (http://www.sconemad.com)

MIME Type

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

#ifndef scxMimeType_h
#define scxMimeType_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
namespace scx {

//===========================================================================
// MimeType - A MimeType in the form "type/subtype *(;name=(value | "value"))
// As defined in RFC 1521
//
// NOTE: The type, subtype and parameter names are case-insensitive, and are 
// always stored in lowercase.
//
class SCONEX_API MimeType : public ScriptObject {
public:

  MimeType();
  MimeType(const std::string& str);
  MimeType(
    const std::string& type,
    const std::string& subtype,
    const std::string& params = ""
  );
  MimeType(const ScriptRef* args);

  MimeType(const MimeType& c);
  virtual ~MimeType();

  ScriptObject* new_copy() const;

  void set_type(const std::string& type);
  void set_subtype(const std::string& subtype);
  void set_param(const std::string& name, const std::string& value);
  bool erase_param(const std::string& name);
  
  const std::string& get_type() const;
  const std::string& get_subtype() const;
  std::string get_param(const std::string& name) const;

  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);
  
  MimeType& operator=(const MimeType& v);

  bool operator==(const MimeType& v) const;
  bool operator!=(const MimeType& v) const;

protected:
  
  void from_string(const std::string& str);
  void params_from_string(const std::string& str);
  
  std::string m_type;
  std::string m_subtype;

  typedef std::map<std::string,std::string> ParamMap;
  ParamMap m_params;

};

};

#endif
