/* SconeServer (http://www.sconemad.com)

SconeX stream with debug output

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxStreamDebugger_h
#define scxStreamDebugger_h

#include <sconex/sconex.h>
#include <sconex/Stream.h>
#include <sconex/File.h>
namespace scx {

//=============================================================================
class SCONEX_API StreamDebugger : public Stream {

public:

  StreamDebugger(const std::string& name = "");
  virtual ~StreamDebugger();

  virtual Condition read(void* buffer,int n,int& na);
  virtual Condition write(const void* buffer,int n,int& na);

  virtual std::string stream_status() const;
  
  std::string get_cond_str(Condition c);

protected:

  void write_header(const std::string& message);

  File m_file;

private:

};

};
#endif
