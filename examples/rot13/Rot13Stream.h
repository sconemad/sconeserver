/* SconeServer (http://www.sconemad.com)

Simple Rot13 text transform stream

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com> */

#ifndef rot13Stream_h
#define rot13Stream_h

#include "sconex/Stream.h"

//=========================================================================
class Rot13Stream : public scx::Stream {

public:

  Rot13Stream(
    bool rot_input,
    bool rot_output
  );

  ~Rot13Stream();
  
protected:

  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);

  char rot13(char c);
  
private:

  bool m_rot_input;
  bool m_rot_output;
  
};

#endif
