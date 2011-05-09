/* SconeServer (http://www.sconemad.com)

Sconex Unit Tester

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

#ifndef scxUnitTester_h
#define scxUnitTester_h

#include <sconex/sconex.h>

//===========================================================================
// Run unit tests for the specified file
#define UTRUN(f) \
  scx::UnitTester::get()->file(#f); \
  void f##_ut(); \
  f##_ut();

//===========================================================================
// Start a new test section
#define UTSEC(section) \
  scx::UnitTester::get()->sec(section,__LINE__);

//===========================================================================
// Perform a test, printing the code first, and recording the result
#define UTEST(cond) \
  scx::UnitTester::get()->msg("TEST( "#cond" )",__LINE__); \
  scx::UnitTester::get()->test(cond);

//===========================================================================
// Print a message to the test log
#define UTMSG(message) \
  scx::UnitTester::get()->msg("# " message,__LINE__);

//===========================================================================
// Run a line of code, also recording it in the test log
#define UTCOD(code) \
  scx::UnitTester::get()->msg(#code";",__LINE__); \
  code;

//===========================================================================
// Print results and return status code for unit test
#define UTEND \
  return scx::UnitTester::get()->print_results();


namespace scx {

//===========================================================================
class UnitTestResult {
public:
  UnitTestResult(const std::string& file,const std::string& section)
    : m_file(file), m_section(section), m_pass(0), m_fail(0) { };

  std::string m_file;
  std::string m_section;
  int m_pass;
  int m_fail;
};

//===========================================================================
class SCONEX_API UnitTester {
public:

  static UnitTester* get();
  
  void msg(const std::string& m, int line);
  void file(const std::string& f);
  void sec(const std::string& m, int line);
  void test(bool t);

  int print_results();
  
private:

  std::string ansi(const std::string& fmt);
  
  UnitTester();
  ~UnitTester();
  
  std::string m_file;
  std::string m_section;

  std::list<UnitTestResult*> m_results;
  
  static UnitTester* s_ut;
};

};

#endif
