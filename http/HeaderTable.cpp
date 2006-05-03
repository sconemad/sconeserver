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

#include "http/HeaderTable.h"
namespace http {

//===========================================================================
HeaderTable::HeaderTable(
)
{

}

//===========================================================================
HeaderTable::~HeaderTable()
{

}
  
//===========================================================================
void HeaderTable::set(
  const std::string& name,
  const std::string& value
)
{
  m_headers[normalize(name)]=value;
}

//===========================================================================
bool HeaderTable::erase(const std::string& name)
{
  std::map<std::string,std::string>::iterator it =
    m_headers.find(normalize(name));
  if (it == m_headers.end()) {
    return false;
  }
  m_headers.erase(it);
  return false;
}

//===========================================================================
std::string HeaderTable::get(const std::string& name) const
{
  std::map<std::string,std::string>::const_iterator it =
    m_headers.find(normalize(name));
  if (it == m_headers.end()) {
    return "";
  }
  return (*it).second;
}

//===========================================================================
std::string HeaderTable::get_all() const
{
  std::ostringstream oss;
  std::map<std::string,std::string>::const_iterator it = m_headers.begin();
  while (it != m_headers.end()) {
    oss << (*it).first << ": " << (*it).second << "\r\n";
    it++;
  }
  return oss.str();
}

//=============================================================================
std::string HeaderTable::normalize(const std::string& name) const
{
  std::string str = name;
  int max = str.length();
  int wi = 0;
  for (int i=0; i<max; ++i) {
    char c = str[i];
    if (isalpha(c)) {
      if (++wi==1) {
        // Uppercase first character in each word
        str[i] = toupper(c);
      } else {
        str[i] = tolower(c);
      }
    } else {
      wi = 0;
    }
  }
  return str;
}

};
