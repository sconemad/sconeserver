/* SconeServer (http://www.sconemad.com)

ArgTest SconeScript test program

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/ArgObject.h"
#include "sconex/ArgScript.h"
#include "sconex/Multiplexer.h"
#include "sconex/Console.h"
#include "sconex/Logger.h"
#include "sconex/Debug.h"
using namespace scx;

Console* con = 0;

//=============================================================================
class ArgTest : public ArgObjectInterface {

public:

  ArgTest();

  virtual ~ArgTest();
  
  virtual std::string name() const;

  virtual Arg* arg_lookup(const std::string& name);
  virtual Arg* arg_resolve(const std::string& name);
  virtual Arg* arg_function(const std::string& name, Arg* args);

};


//=============================================================================
ArgTest::ArgTest()
{

}

//=============================================================================
ArgTest::~ArgTest()
{

}

//=============================================================================
std::string ArgTest::name() const
{
  return "argtest";
}

//=============================================================================
Arg* ArgTest::arg_lookup(const std::string& name)
{
  // See if its a function
  if ("exit" == name ||
      "print" == name ||
      "input" == name ||
      "test" == name) {
    return new ArgObjectFunction(new ArgObject(this),name);
  }

  if ("endl" == name) {
    return new ArgString("\n");
  }
  
  return ArgObjectInterface::arg_lookup(name);
}

//=============================================================================
Arg* ArgTest::arg_resolve(const std::string& name)
{
  return ArgObjectInterface::arg_resolve(name);
}

//=============================================================================
Arg* ArgTest::arg_function(
  const std::string& name,
  Arg* args
)
{
  ArgList* l = dynamic_cast<ArgList*>(args);

  if ("exit" == name) {
    const ArgInt* a_int = dynamic_cast<const ArgInt*>(l->get(0));
    exit(a_int ? a_int->get_int() : 0);
  }
  
  if ("print" == name) {
    // Print string representations of all the arguments
    for (int i=0; i<l->size(); ++i) {
      con->write(l->get(i)->get_string());
    }
    return 0;
  }
  
  if ("input" == name) {
    char buffer[1024];
    int na=0;
    con->read(buffer,1000,na);
    for (; na>0 && (buffer[na-1] == '\r' || buffer[na-1] == '\n'); --na) ;
    buffer[na] = '\0';
    return new ArgString(buffer);
  }

  if ("test" == name) {
    const ArgInt* cond = dynamic_cast<const ArgInt*>(l->get(0));
    if (cond==0 || cond->get_int() == 0) {
      const ArgString* a_msg =
        dynamic_cast<const ArgString*>(l->get(1));
      std::string msg = (a_msg ? a_msg->get_string() : "");
      std::cerr << "TEST FAILED: " << msg << "\n";
      exit(1);
    }
    return 0;
  }

  return ArgObjectInterface::arg_function(name,args);
}

//=============================================================================
int main(int argc,char* argv[])
{
  Logger* logger = new scx::Logger("argtest.log");
  Debug::get()->set_logger(logger);

  con = new Console();
  
  Descriptor* in = 0;
  if (argv[1]) {
    File* f = new File();
    f->open(argv[1],File::Read);
    in = f;
  } else {
    in = con;
  }
  
  ArgTest argtest;
  ArgObject* ctx = new ArgObject(&argtest);
  ArgScript* script = new ArgScript(ctx);
  //  script->set_error_des(con);
  in->add_stream(script);

  Multiplexer spinner;
  spinner.add(in);

  while (spinner.spin() >= 0) ;

  return 0;
  
}
