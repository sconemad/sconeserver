/* SconeServer (http://www.sconemad.com)

Simple Rot13 text transform stream

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com> */

#include "Rot13Stream.h"

//=========================================================================
Rot13Stream::Rot13Stream(
  bool rot_input,
  bool rot_output
)
  : scx::Stream("rot13"),
    m_rot_input(rot_input),
    m_rot_output(rot_output)
{
}

//=========================================================================
Rot13Stream::~Rot13Stream()
{
}

//=========================================================================
scx::Condition Rot13Stream::read(void* buffer,int n,int& na)
{
  // Skip rot13 operation if not required.
  if (!m_rot_input) {
    return Stream::read(buffer,n,na);
  }

  // Read data into buffer
  scx::Condition c = Stream::read(buffer,n,na);

  // Apply rot13 transform to data in buffer
  char* out_ptr = (char*)buffer;
  for (int i=0; i<na; ++i, ++out_ptr) {
    *out_ptr = rot13(*out_ptr);
  }

  // Return condition from read operation
  return c;
}

//=========================================================================
scx::Condition Rot13Stream::write(const void* buffer,int n,int& na)
{
  // Skip rot13 operation if not required
  if (!m_rot_output) {
    return Stream::write(buffer,n,na);
  }

  // Create a new buffer to hold the transformed data to be written
  char* rot_buffer = new char[n];
  char* out_ptr = rot_buffer;
  char* in_ptr = (char*)buffer;

  // Apply rot13 transform, saving into new buffer
  for (int i=0; i<n; ++i, ++out_ptr, ++in_ptr) {
    *out_ptr = rot13(*in_ptr);
  }

  // Write the new buffer
  scx::Condition c = Stream::write(rot_buffer,n,na);

  // Cleanup buffer
  delete[] rot_buffer;

  // Return condition from write operation
  return c;
}

//=========================================================================
char Rot13Stream::rot13(char c)
{
  const int rot = 13;
  if (c >= 'a' && c <= 'z') {
    // Perform transform on lowercase ASCII text
    c = 'a' + ((c - 'a' + rot) % 26);
  } else if (c >= 'A' && c <= 'Z') {
    // Perform transform on uppercase ASCII text
    c = 'A' + ((c - 'A' + rot) % 26);
  }
  // Anything else (numbers, symbols, control chars, etc) are unaltered
  return c;
}
