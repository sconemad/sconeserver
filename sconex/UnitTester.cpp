/* SconeServer (http://www.sconemad.com)

Sconex Unit Test 

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

#include "sconex/UnitTester.h"
namespace scx {

UnitTester* UnitTester::s_ut = 0;

//=============================================================================
UnitTester* UnitTester::get()
{
  if (!s_ut) {
    s_ut = new UnitTester();
  }
  return s_ut;
}
  
//=============================================================================
void UnitTester::msg(
  const std::string& m,
  int line
)
{
  std::cout << line << ": " << m << "\n";
}

//=============================================================================
void UnitTester::file(
  const std::string& f
)
{
  m_file = f;
  m_section = "";
  std::cout << ansi("1;33") << "### RUNNING TESTS FROM " << m_file << ansi("0")
            << "\n";
}

//=============================================================================
void UnitTester::sec(
  const std::string& m,
  int line
)
{
  m_section = m;
  std::cout << ansi("1;33") << line << ": ### "
            << m_file << ": " << m_section << ansi("0") << "\n";
}

//=============================================================================
void UnitTester::test(
  bool t
)
{
  if (!t) {
    std::cout << ansi("1;31") << "<<< FAILED >>>" << ansi("0") << "\n";
  }

  UnitTestResult* result = m_results.empty() ? 0 : m_results.back();
  if (!result ||
      result->m_file != m_file ||
      result->m_section != m_section) {
    result = new UnitTestResult(m_file,m_section);
    m_results.push_back(result);
  }
  ++(t ? result->m_pass : result->m_fail);
}

//=============================================================================
int UnitTester::print_results()
{
  int tpass=0;
  int tfail=0;

  std::cout << "\n"
            << " *** SUMMARY ***\n"
            << "\n"
            << " PASS FAIL Module: Section\n"
            << "----------------------------------------------------------\n";
  
  for (std::list<UnitTestResult*>::iterator it = m_results.begin();
       it != m_results.end();
       it++) {
    UnitTestResult* r = (*it);
    std::cout.width(4);
    std::cout << r->m_pass << " ";

    std::cout.width(4);
    std::cout << r->m_fail << "  ";
    
    std::cout << r->m_file << ": " << r->m_section << "\n";
    tpass += r->m_pass;
    tfail += r->m_fail;
  }

  std::cout << "----------------------------------------------------------\n";

  std::cout.width(4);
  std::cout << tpass << " ";

  std::cout.width(4);
  std::cout << tfail << "  ";

  std::cout << "TOTAL\n"
            << "----------------------------------------------------------\n"
            << "\n";


  return tfail > 0;
}

//=============================================================================
std::string UnitTester::ansi(const std::string& fmt)
{
  return std::string("\033[") + fmt + "m";
}


//=============================================================================
UnitTester::UnitTester()
{

}
 
//=============================================================================
UnitTester::~UnitTester()
{

}

};
