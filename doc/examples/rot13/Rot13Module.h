/* SconeServer example

Simple rot13 text transform module

Warning: The "encryption" provided by this module should not be considered
secure by any means. Its purpose is to demonstrate the basic principles for
creating modules that modify data streams within the SconeServer framework.

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com> */

#include "sconex/Module.h"

//=============================================================================
// Our module class, which must be derived from scx::Module
class Rot13Module : public scx::Module {

public:

  Rot13Module();
  
  virtual ~Rot13Module();

  virtual std::string info() const;
  
  virtual int init();

  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

};
