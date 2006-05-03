/* SconeServer (http://www.sconemad.com)

HTTP Header Table

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

#ifndef httpHeaderTable_h
#define httpHeaderTable_h

#include "sconex/sconex.h"
#include "http/http.h"
namespace http {

//=============================================================================
class HTTP_API HeaderTable {

public:

  HeaderTable();
  ~HeaderTable();

  void set(const std::string& name,const std::string& value);
  bool erase(const std::string& name);
  std::string get(const std::string& name) const;

  std::string get_all() const;

private:

  std::string normalize(const std::string& name) const;
  
  std::map<std::string,std::string> m_headers;

};

};
#endif
