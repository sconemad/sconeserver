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

#include <sconex/MimeHeader.h>
namespace scx {

//===========================================================================
MimeHeaderValue::MimeHeaderValue()
{

}

//===========================================================================
const std::string& MimeHeaderValue::value() const
{
  return m_value;
}

//===========================================================================
void MimeHeaderValue::set_value(const std::string& headervalue)
{
  m_value = headervalue;
}

//===========================================================================
bool MimeHeaderValue::get_parameter(const std::string& parname, std::string& parvalue) const
{
  ParMap::const_iterator it = m_pars.find(parname);
  if (it == m_pars.end()) {
    return false;
  }

  parvalue = it->second;
  return true;
}

//===========================================================================
bool MimeHeaderValue::set_parameter(const std::string& parname, const std::string& parvalue)
{
  m_pars[parname] = parvalue;
  return true;
}

//===========================================================================
bool MimeHeaderValue::remove_parameter(const std::string& parname)
{
  ParMap::iterator it = m_pars.find(parname);
  if (it == m_pars.end()) {
    return false;
  }

  m_pars.erase(it);
  return true;
}

//===========================================================================
std::string MimeHeaderValue::get_string() const
{
  std::ostringstream oss;
  oss << m_value;

  for (ParMap::const_iterator it = m_pars.begin();
       it != m_pars.end();
       it++) {
    oss << "; " << it->first << "=\"" << it->second << "\"";
  }
  return oss.str();
}


//%%% MimeHeader %%%


//===========================================================================
MimeHeader::MimeHeader()
{

}

//===========================================================================
MimeHeader::MimeHeader(const std::string& name)
  : m_name(name)
{
  
}

//===========================================================================
bool MimeHeader::parse_line(const std::string& header)
{
  std::string::size_type i = header.find_first_of(":");
  if (i == std::string::npos) {
    return false;
  }
  
  m_name = header.substr(0,i);
  i = header.find_first_not_of(" ",i+1);

  return parse_value(header.substr(i));
}
  
//===========================================================================
bool MimeHeader::parse_value(const std::string& str)
{
  // Reset the values before parsing
  remove_all_values();
  
  int i = 0;
  int seq = 1; // 1:value, 2:parname, 3:parvalue
  bool comment = false;
  bool quote = false;
  bool space = true;
  std::string name;
  std::string value;
  std::string tok;

  int len = str.length();
  for (; i<=len; ++i) {
    const char c = (i==len ? '\0' : str[i]);
    //    std::cerr << "PARSER '" << c << "' \n";

    if (space && c != ' ') { space = false; }
    
    if (comment && c == ')') { comment = false; continue; }
    if (!comment && !quote && c == '(') { comment = true; continue; }

    if (quote && c == '"') { quote = false; continue; }
    if (!quote && !comment && c == '"') { quote = true; continue; }

    if (!quote && !comment) {
      switch (seq) {

        case 1:
          if (c==';' || c==',' || c=='\0') {
            //            std::cerr << "    TOKEN value '" << tok << "'\n";
            m_values.push_back(MimeHeaderValue());
            m_values.back().set_value(tok);
            tok = "";
            space = true;
            seq=2;
          }
          break;
            
        case 2:
          if (c=='=' || c=='\0') {
            //            std::cerr << "    TOKEN par_name '" << tok << "'\n";
            name = tok;
            tok = "";
            space = true;
            seq=3;
          }
          break;
          
        case 3:
          if (c==';' || c=='\0') {
            //            std::cerr << "    TOKEN par_value '" << tok << "'\n";
            value = tok;
            m_values.back().set_parameter(name,value);
            tok = "";
            space = true;
            seq=2;
          }
          break;
      }
    }

    if (!comment && !space) {
      tok.append(1,c);
    }
  }

  return true;
}

//===========================================================================
const std::string& MimeHeader::name() const
{
  return m_name;
}

//===========================================================================
void MimeHeader::set_name(const std::string& headername)
{
  m_name = headername;
}
  
//===========================================================================
int MimeHeader::num_values() const
{
  return m_values.size();
}

//===========================================================================
const MimeHeaderValue* MimeHeader::get_value() const
{
  if (m_values.size() > 0) {
    return &m_values.front();
  }
  return 0;
}

//===========================================================================
const MimeHeaderValue* MimeHeader::get_value(int index) const
{
  ValueList::const_iterator it = m_values.begin();
  for (int i=0; i<index; ++i) {
    it++;
  }
  if (it != m_values.end()) {
    return &(*it);
  }
  return 0;
}

//===========================================================================
const MimeHeaderValue* MimeHeader::get_value(const std::string& value) const
{
  for (ValueList::const_iterator it = m_values.begin();
       it != m_values.end();
       it++) {
    const MimeHeaderValue& mhv = *it;
    if (mhv.value() == value) {
      return &mhv;
    }
  }
  return 0;
}

//===========================================================================
bool MimeHeader::add_value(
  __attribute__((unused)) const MimeHeaderValue& value
)
{

  return true;
}

//===========================================================================
void MimeHeader::remove_value(const std::string& value)
{
  for (ValueList::iterator it = m_values.begin();
       it != m_values.end();
       it++) {
    MimeHeaderValue& mhv = *it;
    if (mhv.value() == value) {
      m_values.erase(it);
    }
  }
}

//===========================================================================
void MimeHeader::remove_all_values()
{
  for (ValueList::iterator it = m_values.begin();
       it != m_values.end();
       it++) {
    m_values.erase(it);
  }
}

//===========================================================================
std::string MimeHeader::get_string() const
{
  std::ostringstream oss;
  oss << m_name << ":";

  bool first = true;
  for (ValueList::const_iterator it = m_values.begin();
       it != m_values.end();
       it++) {
    if (!first) oss << ",";
    first=false;
    oss << " " << it->get_string();
  }
  return oss.str();
}


//%%% MimeHeaderTable %%%

  
//===========================================================================
MimeHeaderTable::MimeHeaderTable(
)
{

}

//===========================================================================
MimeHeaderTable::~MimeHeaderTable()
{

}
  
//===========================================================================
void MimeHeaderTable::set(
  const std::string& name,
  const std::string& value
)
{
  m_headers[normalize(name)]=value;
}

//===========================================================================
std::string MimeHeaderTable::parse_line(const std::string& line)
{
  std::string::size_type i = line.find_first_of(":");
  if (i == std::string::npos) {
    return "";
  }
  
  std::string name = std::string(line,0,i);
  std::string value;

  if (i < line.length()) {
    i = line.find_first_not_of(" ",i+1);
    if (i != std::string::npos) {
      value = std::string(line,i);
    }
  }
 
  m_headers[normalize(name)]=value;
  return name;
}

//===========================================================================
bool MimeHeaderTable::erase(const std::string& name)
{
  HeaderMap::iterator it = m_headers.find(normalize(name));
  if (it == m_headers.end()) {
    return false;
  }
  m_headers.erase(it);
  return false;
}

//===========================================================================
std::string MimeHeaderTable::get(const std::string& name) const
{
  HeaderMap::const_iterator it = m_headers.find(normalize(name));
  if (it == m_headers.end()) {
    return "";
  }
  return (*it).second;
}

//===========================================================================
MimeHeader MimeHeaderTable::get_parsed(const std::string& name) const
{
  MimeHeader header(name);
  
  HeaderMap::const_iterator it = m_headers.find(normalize(name));
  if (it == m_headers.end()) {
    return header;
  }

  header.parse_value(it->second);
  return header;
}

//===========================================================================
std::string MimeHeaderTable::get_all() const
{
  std::ostringstream oss;
  for (HeaderMap::const_iterator it = m_headers.begin();
       it != m_headers.end();
       ++it) {
    oss << (*it).first << ": " << (*it).second << "\r\n";
  }
  return oss.str();
}

//=============================================================================
std::string MimeHeaderTable::normalize(const std::string& name) const
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
