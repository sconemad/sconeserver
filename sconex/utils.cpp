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
std::string escape_quotes(const std::string& s)
{
  std::string r;
  int n = s.length();
  for (int i=0; i<n; ++i) {
    char c = s[i];
    if (isgraph(c) || ' '==c) {
      r += c;
    } else {
      char e = 0;
      switch (c) {
        case '\\': r += "\\\\"; break;
        case '\"': r += "\\\""; break;
        case '\'': r += "\\\'"; break;
        case '\n': r += "\\n"; break;
        case '\r': r += "\\r"; break;
        case '\b': r += "\\b"; break;
        case '\t': r += "\\t"; break;
        case '\f': r += "\\f"; break;
        case '\a': r += "\\a"; break;
        case '\v': r += "\\v"; break;
        case '\0': r += "\\0"; break;
        default: {
	  char x[5];
	  r += sprintf(x,"\\x%02x",c);
	} break;
      }
    }
  }
  return r;
}

//============================================================================
std::string escape_html(const std::string& s)
{
  std::string r;
  int n = s.length();
  for (int i=0; i<n; ++i) {
    char c = s[i];
    switch (c) {
      case '&': r += "&amp;"; break;
      case '<': r += "&lt;"; break;
      case '>': r += "&gt;"; break;
      case '\"': r += "&quot;"; break;
      default: r += c; break;
    }
  }
  return r;
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
