/* SconeServer (http://www.sconemad.com)
Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com> */

#ifndef rot13Stream_h
#define rot13Stream_h

#include <sconex/Stream.h>

class Rot13Module;

//=========================================================================
// Rot13Stream - Simple Rot13 text transform stream
//
class Rot13Stream : public scx::Stream {
public:

  Rot13Stream(Rot13Module& module,
	      bool rot_input,
	      bool rot_output);

  ~Rot13Stream();
  
protected:

  // From scx::Stream:
  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);

  // Helper method to perform a rot13 transformation on a character
  char rot13(char c);
  
private:

  // Store a reference to the module, this prevents it from being unloaded
  // while any Rot13Streams are in use
  scx::ScriptRefTo<Rot13Module> m_module;

  // Flags indicating whether input and/or output should be transformed
  bool m_rot_input;
  bool m_rot_output;
  
};

#endif
