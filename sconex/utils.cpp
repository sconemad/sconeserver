/* SconeServer (http://www.sconemad.com)

Sconex utils

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

#include "sconex/utils.h"
namespace scx {

//===========================================================================
void strup(std::string& s) 
{
  int n = s.length();
  for (int i=0; i<n; ++i) {
    s[i] = toupper(s[i]);
  }
}

//===========================================================================
void strlow(std::string& s) 
{
  int n = s.length();
  for (int i=0; i<n; ++i) {
    s[i] = tolower(s[i]);
  }
}

//============================================================================
char* new_c_str(const std::string& str)
{
  int len = str.size()+1;
  char* c_str = new char[len];
  memcpy(c_str,str.c_str(),len);
  return c_str;
}

//============================================================================
std::string type_name(const std::type_info& ti)
{
  int status;
  char* realname = abi::__cxa_demangle(ti.name(),0,0,&status);
  std::string ret(realname);
  free(realname);
  return ret;
}

};
