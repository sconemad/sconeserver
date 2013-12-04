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

// Convert string to uppercase
void SCONEX_API strup(std::string& s);

// Convert string to lowercase
void SCONEX_API strlow(std::string& s);

// Escape string to remove quotes
std::string SCONEX_API escape_quotes(const std::string& s);

// Escape string to remove html control chars
std::string SCONEX_API escape_html(const std::string& s);

// New up a c style string from a c++ string
char* SCONEX_API new_c_str(const std::string& str);

// Get demangled type name from a type_info
std::string SCONEX_API type_name(const std::type_info& ti);

// Get a random byte [0-255]
unsigned char SCONEX_API random_byte();

// Get a random integer [0-RAND_MAX]
unsigned int SCONEX_API random_int();

// Get a random string picked from a given set of chars
std::string random_string(int len, const char* chars);
  
// Get a random hex string (with characters [0-9a-f])
std::string random_hex_string(int len);

// Get a random base64 string (with characters [0-0a-zA-Z+/])
std::string random_b64_string(int len);

// Get a random base64url string (with characters [0-0a-zA-Z\-_])
std::string random_b64url_string(int len);

};

#endif
