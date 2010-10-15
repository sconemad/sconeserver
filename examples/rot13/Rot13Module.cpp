/* SconeServer example

Simple rot13 text transform module

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com> */

#include "Rot13Module.h"
#include "Rot13Stream.h"

#include "sconex/ModuleInterface.h"

// The following must be present to allow the resulting
// shared object to be used as a SconeServer module.
SCONESERVER_MODULE(Rot13Module);

//=============================================================================
// Constructor
Rot13Module::Rot13Module()
  : scx::Module("rot13", scx::VersionTag(1,0,0))
{
  // Specify module name and version in base constructor
}

//=============================================================================
// Destructor
Rot13Module::~Rot13Module()
{
  // Nothing to do here
}

//=============================================================================
// Return an information string describing this module
std::string Rot13Module::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
  "Simple rot13 text transform module\n";
}
  
//=============================================================================
// Initialise the module
int Rot13Module::init()
{
  // Everything is fine so return 0
  return 0;
}

//=============================================================================
// Notification of connection attempt
bool Rot13Module::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  const scx::ArgInt* a_input =
    dynamic_cast<const scx::ArgInt*>(args->get(0));
  const scx::ArgInt* a_output =
    dynamic_cast<const scx::ArgInt*>(args->get(1));

  if (args->size() != 2 ||
      !a_input ||
      !a_output) {
    log("Invalid options specified");
    return false;
  }
  
  Rot13Stream* s = new Rot13Stream( a_input->get_int(), a_output->get_int() );
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}
