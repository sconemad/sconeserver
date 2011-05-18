/* SconeServer example
Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com> */

#include "Rot13Module.h"
#include "Rot13Stream.h"

#include "sconex/ModuleInterface.h"
#include "sconex/ScriptTypes.h"

// The following must be present to allow the resulting
// shared object to be used as a SconeServer module.
SCONESERVER_MODULE(Rot13Module);

//=============================================================================
// Constructor
//
Rot13Module::Rot13Module()
  : scx::Module("rot13", scx::version())
{
  // Register our stream type
  scx::Stream::register_stream("rot13",this);
}

//=============================================================================
// Destructor
//
Rot13Module::~Rot13Module()
{
  // Unregister our stream type
  scx::Stream::unregister_stream("rot13",this);
}

//=============================================================================
// Return an information string describing this module
//
std::string Rot13Module::info() const
{
  return "Simple rot13 text transform module";
}
  
//=============================================================================
// Request to provide a stream
//
void Rot13Module::provide(const std::string& type,
			  const scx::ScriptRef* args,
			  scx::Stream*& object)
{
  // Check the type is "rot13" - this should always be the case since this is
  // the only stream type we have registered
  if (type == "rot13") {

    // Get arguments
    const scx::ScriptInt* a_input = 
      scx::get_method_arg<scx::ScriptInt>(args,0,"input");
    const scx::ScriptInt* a_output =
      scx::get_method_arg<scx::ScriptInt>(args,1,"output");
    
    // Check arguments
    if (!a_input || !a_output) {
      log("Invalid options specified");
      return;
    }
    
    // Create the requested stream and assign to the object reference
    object = new Rot13Stream(*this, a_input->get_int(), a_output->get_int());
  }
}
