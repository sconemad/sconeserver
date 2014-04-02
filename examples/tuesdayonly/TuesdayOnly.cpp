/* SconeServer example

A very simple module which only allows connections on Tuesdays!

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com> */

#include <sconex/Module.h>
#include <sconex/ModuleInterface.h>
#include <sconex/Date.h>
#include <sconex/Log.h>
#include <sconex/Stream.h>

//=============================================================================
// Our module class, which must be derived from scx::Module
//
class TuesdayOnlyModule : public scx::Module,
			  public scx::Provider<scx::Stream> {
public:

  TuesdayOnlyModule();
  
  virtual ~TuesdayOnlyModule();

  virtual std::string info() const;
  
  // Provider<scx::Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);

};

// The following must be present to allow the resulting
// shared object to be used as a sconex module.
SCONEX_MODULE(TuesdayOnlyModule);

//=============================================================================
// Constructor
TuesdayOnlyModule::TuesdayOnlyModule()
  : scx::Module("tuesdayonly", scx::version())
{
  // Specify module name and version in base constructor

  // Register our stream type
  scx::Stream::register_stream("tuesdayonly", this);
}

//=============================================================================
// Destructor
TuesdayOnlyModule::~TuesdayOnlyModule()
{
  // Unregister our stream type
  scx::Stream::unregister_stream("tuesdayonly", this);
}

//=============================================================================
// Return an information string describing this module
std::string TuesdayOnlyModule::info() const
{
  return "A very simple example module which only allows connections on "
         "Tuesdays!";
}
  
//=============================================================================
// Request to provide a stream
//
void TuesdayOnlyModule::provide(const std::string& type,
				const scx::ScriptRef* args,
				scx::Stream*& object)
{
  // Check the type is "tuesdayonly" - this should always be the case since
  // this is the only stream type we have registered
  if ("tuesdayonly" == type) {

    // Check that it is Tuesday
    if (scx::Date::Tue != scx::Date::now().day()) {
      scx::Log("tuesdayonly").submit("It is not Tuesday");
      return;
    }

    // Create the requested stream and assign to the object reference
    object = new scx::Stream("tuesdayonly");
  }
}
