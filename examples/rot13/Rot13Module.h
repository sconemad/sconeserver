/* SconeServer example

Simple rot13 text transform module

Warning: The "encryption" provided by this module should not be considered
secure by any means. Its purpose is to demonstrate the basic principles for
creating modules that modify data streams within the SconeServer framework.

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com> */

#include "sconex/Module.h"
#include "sconex/Stream.h"

//=============================================================================
// Rot13Module - our module class, which must be derived from scx::Module
//
class Rot13Module : public scx::Module,
                    public scx::Provider<scx::Stream> {
public:

  Rot13Module();
  
  virtual ~Rot13Module();

  virtual std::string info() const;

  // Provider<scx::Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);
};
