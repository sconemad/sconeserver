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

#include <sconex/utils.h>
#include <sconex/File.h>
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
    if ((isgraph(c) || ' '==c) && ('\"' != c && '\'' != c)) {
      r += c;
    } else {
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


//============================================================================
static File* get_random()
{
  static File* s_random = 0;
  if (!s_random) {
    s_random = new File();
    if (Ok != s_random->open("/dev/urandom", File::Read)) {
      DEBUG_LOG("/dev/random not available, falling back to rand()");
    }
  }
  return s_random;
}
  
//============================================================================
unsigned char random_byte()
{
  File* random = get_random();

  unsigned char c;
  if (random->is_open()) {
    int na = 0;
    random->read(&c, 1, na);
  } else {
    c = rand() % 256;
  }
  return c;
}

//============================================================================
unsigned int random_int()
{
  File* random = get_random();

  unsigned int v;
  if (random->is_open()) {
    int na = 0;
    random->read(&v, sizeof(unsigned int), na);
    v = v % RAND_MAX;
  } else {
    v = rand();
  }
  return v;
}
  
//============================================================================
std::string random_string(int len, const char* chars)
{
  int max = strlen(chars);
  std::ostringstream oss;
  for (int i=0; i<len; ++i) {
    int c = random_int() % max; // Assume RAND_MAX >> max
    oss << chars[c];
  }
  return oss.str();
}

//============================================================================
std::string random_hex_string(int len)
{
  std::ostringstream oss;
  for (int i=0; i<(len+1)/2; ++i) {
    int c = random_byte();
    oss << std::setw(2) << std::setfill('0') << std::hex << c;
  }
  return oss.str().substr(0,len);
}

//============================================================================
std::string random_b64_string(int len)
{
  return random_string(len,
		       "ABCDEFGHIJKLMNOP"
		       "QRSTUVWXYZabcdef"
		       "ghijklmnopqrstuv"
		       "wxyz0123456789+/");
}

//============================================================================
std::string random_b64url_string(int len)
{
  return random_string(len,
		       "ABCDEFGHIJKLMNOP"
		       "QRSTUVWXYZabcdef"
		       "ghijklmnopqrstuv"
		       "wxyz0123456789-_");
}

};
