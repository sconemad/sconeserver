/* SconeServer (http://www.sconemad.com)

Statistics Stream

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

#ifndef statStream_h
#define statStream_h

#include "StatModule.h"
#include "sconex/Stream.h"
#include "sconex/Module.h"

//=========================================================================
class StatStream : public scx::Stream {

public:

  StatStream(
    StatModule& mod,
    const std::string& channel
  );

  ~StatStream();
  
protected:

  //  virtual scx::Condition event(int type);

  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);
  
private:

  void add_stats(
    StatChannel::StatType type,
    long count
  );
  
  StatModule& m_mod;
  std::string m_channel;  
};

#endif
