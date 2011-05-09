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

#ifndef scxUtils_h
#define scxUtils_h

#include <sconex/sconex.h>
namespace scx {

void SCONEX_API strup(std::string& s);
// Convert string to uppercase

void SCONEX_API strlow(std::string& s);
// Convert string to lowercase

std::string SCONEX_API escape_quotes(const std::string& s);
// Escape string to remove quotes

std::string SCONEX_API escape_html(const std::string& s);
// Escape string to remove html control chars

char* SCONEX_API new_c_str(const std::string& str);
// New up a c style string from a c++ string

std::string SCONEX_API type_name(const std::type_info& ti);
// Get demangled type name from a type_info

};

#endif
