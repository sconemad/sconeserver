/* SconeServer (http://www.sconemad.com)

SconeScript test program

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/ScriptBase.h"
#include "sconex/ScriptTypes.h"
#include "sconex/ScriptEngine.h"
#include "sconex/Multiplexer.h"
#include "sconex/Console.h"
#include "sconex/File.h"
#include "sconex/Logger.h"
#include "sconex/Debug.h"
using namespace scx;

Console* con = 0;

//=============================================================================
class ArgTest : public ScriptObject {

public:

  ArgTest();
  virtual ~ArgTest();
  
  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  virtual ScriptObject* new_copy() const { return 0; };

  virtual std::string get_string() const { return ""; };
  virtual int get_int() const { return 1; };

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
ScriptRef* ArgTest::script_op(const ScriptAuth& auth,
			      const ScriptRef& ref,
			      const ScriptOp& op,
			      const ScriptRef* right)
{
  if (ScriptOp::Lookup == op.type()) {
    const std::string name = right->object()->get_string();
    // See if its a function
    if ("exit" == name ||
	"print" == name ||
	"input" == name ||
	"test" == name) {
      return new ScriptMethodRef(ref,name);
    }

    if ("endl" == name) {
      return ScriptString::new_ref("\n");
    }

  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* ArgTest::script_method(const ScriptAuth& auth,
				  const ScriptRef& ref,
				  const std::string& name,
				  const ScriptRef* args)
{
  if ("exit" == name) {
    const ScriptInt* a_int = get_method_arg<ScriptInt>(args,0,"code");
    exit(a_int ? a_int->get_int() : 0);
  }
  
  if ("print" == name) {
    // Print string representations of all the arguments
    const ScriptList* l = dynamic_cast<const ScriptList*>(args->object());
    for (int i=0; i<l->size(); ++i) {
      con->write(l->get(i)->object()->get_string());
    }
    return 0;
  }
  
  if ("input" == name) {
    char buffer[1024];
    int na=0;
    con->read(buffer,1000,na);
    for (; na>0 && (buffer[na-1] == '\r' || buffer[na-1] == '\n'); --na) ;
    buffer[na] = '\0';
    return ScriptString::new_ref(buffer);
  }

  if ("test" == name) {
    const ScriptObject* result = 
      get_method_arg<ScriptObject>(args,0,"condition");
    
    if (!result || result->get_int() == 0) {
      const ScriptString* a_msg = 
	get_method_arg<ScriptString>(args,1,"message");
      std::string msg = (a_msg ? a_msg->get_string() : "");
      std::cerr << "TEST FAILED: " << msg << "\n";
      exit(1);
    }
    return 0;
  }

  return ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
int main(int argc,char* argv[])
{
  Logger* logger = new scx::Logger("scones.log");
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
  
  ScriptRef* ctx = new ScriptRef(new ArgTest());
  ScriptEngineExec* script = 
    new ScriptEngineExec(ScriptAuth(ScriptAuth::Admin),ctx);
  script->set_error_des(con);
  in->add_stream(script);

  Multiplexer spinner;
  spinner.add_job(new DescriptorJob(in));

  while (spinner.spin() >= 0) ;

  return 0;
  
}
