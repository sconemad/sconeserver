/* SconeServer (http://www.sconemad.com)

Base64 utils

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (see the file COPYING); if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA */

#include <Base64.h>
namespace scx {

static char basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char index_64[128] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

#define char64(c)  (((c) < 0 || (c) > 127) ? -1 : index_64[(c)])

//===========================================================================
void output_chunk(int c1, int c2, int c3, int pads, std::ostream& out)
{
  out.put(basis_64[c1>>2]);
  out.put(basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)]);
  if (pads == 2) {
    out.put('=');
    out.put('=');
  } else if (pads) {
    out.put(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)]);
    out.put('=');
  } else {
    out.put(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)]);
    out.put(basis_64[c3 & 0x3F]);
  }
}

//===========================================================================
void Base64::encode(std::istream& in, std::ostream& out) 
{
  int c1, c2, c3, ct=0;
  
  while (!in.eof()) {
    c1 = in.get();
    c2 = in.get();
    if (in.eof()) {
      output_chunk(c1, 0, 0, 2, out);
    } else {
      c3 = in.get();
      if (in.eof()) {
        output_chunk(c1, c2, 0, 1, out);
      } else {
        output_chunk(c1, c2, c3, 0, out);
      }
    }
    ct += 4;
    if (ct > 71) {
      out.put('\n');
      ct = 0;
    }
  }
  if (ct) {
    out.put('\n');
  }
}

//===========================================================================
void Base64::decode(std::istream& in, std::ostream& out)
{
  int c1, c2, c3, c4;
  bool done = false;

  while (!in.eof()) {
    c1 = in.get();
    if (isspace(c1)) {
      continue;
    }
    if (done) continue;
    do {
      c2 = in.get();
    } while (!in.eof() && isspace(c2));
    do {
      c3 = in.get();
    } while (!in.eof() && isspace(c3));
    do {
      c4 = in.get();
    } while (!in.eof() && isspace(c4));
    if (c2 == EOF || c3 == EOF || c4 == EOF) {
      return;
    }
    if (c1 == '=' || c2 == '=') {
      done = true;
      continue;
    }
    c1 = char64(c1);
    c2 = char64(c2);
    out.put(((c1<<2) | ((c2&0x30)>>4)));
    if (c3 == '=') {
      done = true;
    } else {
      c3 = char64(c3);
      out.put((((c2&0XF) << 4) | ((c3&0x3C) >> 2)));
      if (c4 == '=') {
        done = true;
      } else {
        c4 = char64(c4);
        out.put((((c3&0x03) <<6) | c4));
      }
    }
  }
}

};
