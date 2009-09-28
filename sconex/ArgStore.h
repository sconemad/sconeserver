/* SconeServer (http://www.sconemad.com)

Arg storage

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

#ifndef scxArgStore_h
#define scxArgStore_h

#include "sconex/sconex.h"
#include "sconex/ArgObject.h"
#include "sconex/Stream.h"
#include "sconex/FilePath.h"

#include "sconex/Date.h"
namespace scx {

class ArgObject;
class ArgStatement;
class ArgStatementGroup;

//=============================================================================
class SCONEX_API ArgStore : public ArgObjectInterface {

public:

  ArgStore(const FilePath& path);

  virtual ~ArgStore();

  bool load();
  bool save();
  void reset();

  // ArgObject interface
  virtual std::string name() const;
  virtual Arg* arg_lookup(const std::string& name);
  virtual Arg* arg_function(const Auth& auth, const std::string& name,Arg* args);
  
protected:

  FilePath m_path;
  Date m_modtime;
  ArgMap* m_data;

};

  
//=============================================================================
class SCONEX_API ArgStoreStream : public Stream {

public:

  enum Mode { Read, Write };

  ArgStoreStream(Mode mode, Arg* arg_write=0);

  virtual ~ArgStoreStream();

  virtual Condition event(Stream::Event e);
  // Handle event

  Arg* take_arg_read();

protected:

  bool write_arg(const Arg* arg);

  Mode m_mode;
  Arg* m_arg;
  std::string m_data;

private:
  
};

};
#endif
