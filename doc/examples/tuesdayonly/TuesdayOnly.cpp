/* SconeServer example

A very simple module which only allows connections on Tuesdays!

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com> */

#include "sconex/Module.h"
#include "sconex/ModuleInterface.h"
#include "sconex/TimeDate.h"

//=============================================================================
// Our module class, which must be derived from scx::Module
class TuesdayOnlyModule : public scx::Module {

public:

  TuesdayOnlyModule();
  
  virtual ~TuesdayOnlyModule();

  virtual std::string info() const;
  
  virtual int init();

  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

};

// The following must be present to allow the resulting
// shared object to be used as a SconeServer module.
SCONESERVER_MODULE(TuesdayOnlyModule);

//=============================================================================
// Constructor
TuesdayOnlyModule::TuesdayOnlyModule()
  : scx::Module("tuesdayonly", scx::VersionTag(1,0,0))
{
  // Specify module name and version in base constructor
}

//=============================================================================
// Destructor
TuesdayOnlyModule::~TuesdayOnlyModule()
{
  // Nothing to do here
}

//=============================================================================
// Return an information string describing this module
std::string TuesdayOnlyModule::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
  "A very simple example module which only allows connections on Tuesdays!\n";
}
  
//=============================================================================
// Initialise the module
int TuesdayOnlyModule::init()
{
  // Everything is fine so return 0
  return 0;
}

//=============================================================================
// Notification of connection attempt
bool TuesdayOnlyModule::connect(
  scx::Descriptor* /* endpoint */,
  scx::ArgList* /* args */
)
{
  // Return 'true' only if today is a tuesday, otherwise return 'false' -
  // causing the connection to be dropped.
  return (scx::Date::now().day() == scx::Date::Tue);
}
