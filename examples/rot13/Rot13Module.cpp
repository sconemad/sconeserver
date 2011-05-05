/* SconeServer example

Simple rot13 text transform module

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
Rot13Module::Rot13Module()
  : scx::Module("rot13", scx::version())
{
  scx::Stream::register_stream("rot13",this);
}

//=============================================================================
// Destructor
Rot13Module::~Rot13Module()
{
  scx::Stream::unregister_stream("rot13",this);
}

//=============================================================================
// Return an information string describing this module
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

  // Create the requested stream
  object = new Rot13Stream(a_input->get_int(),
					a_output->get_int());

  // Add a reference to this module, so the module cannot be get unloaded 
  // while this stream is in use.
  object->add_module_ref(this);
}
