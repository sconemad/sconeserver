/* SconeServer (http://www.sconemad.com)

Sconex Configuration stream

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#include "sconex/ConfigStream.h"
#include "sconex/sconex.h"
#include "sconex/ArgProc.h"
#include "sconex/utils.h"

namespace scx {

//=========================================================================
ConfigStream::ConfigStream(
  ModuleRef root_ref,
  bool shutdown_on_exit
)
  : LineBuffer("config"),
    m_proc(Auth::Admin),
    m_shutdown_on_exit(shutdown_on_exit)
{
  m_argmod = new ArgModule(root_ref);
  m_proc.set_ctx(m_argmod);
  enable_event(Stream::Readable,true);
}

//=========================================================================
ConfigStream::~ConfigStream()
{
  delete m_argmod;
}

//=========================================================================
Condition ConfigStream::event(Stream::Event e)
{
  switch (e) {
    
    case Stream::Opening: { // OPENING

    } break;
   
    case Stream::Closing: { // CLOSING
      if (m_shutdown_on_exit) {
	Stream::write("Foreground console exiting - shutting down\n");
	m_proc.evaluate("shutdown()");
      }
    } return Close;
    
    case Stream::Readable: { // READABLE
      std::string buffer;
      Condition c;
      
      while ( (c = tokenize(buffer)) == Ok) {
	
	if (buffer == "exit") {
	  return Close;
	}
	
	Arg* result = 0;
	try {
	  result = m_proc.evaluate(buffer);
	  write_arg(result);
	  write("\n");
	} catch (...) {
	}
	delete result;
      }
    } break;
    
    default:
      break;
    
  }

  return Ok;
}

#define WRITE_INDENT(in) {for (int i=0; i<(in); ++i) write("  ");}

//=============================================================================
bool ConfigStream::write_arg(const Arg* arg, int indent)
{
  if (arg == 0) {
    write("NULL");
    return true;
  }

  const std::type_info& ti = typeid(*arg);
  
  write("(" + type_name(ti) + ") ");

  if (typeid(ArgList) == ti) {
    const ArgList* l = dynamic_cast<const ArgList*>(arg);
    write("[\n");
    int max = l->size();
    for (int i=0; i<max; ++i) {
      WRITE_INDENT(indent+1);
      write_arg(l->get(i),indent+1);
      if (i != max-1) write(",");
      write("\n");
    }
    WRITE_INDENT(indent);
    write("]");

  } else if (typeid(ArgMap) == ti) {
    const ArgMap* m = dynamic_cast<const ArgMap*>(arg);
    write("{\n");
    std::vector<std::string> keys;
    m->keys(keys);
    for (std::vector<std::string>::const_iterator it = keys.begin();
	 it != keys.end();
	 ++it) {
      WRITE_INDENT(indent+1);
      std::string key = *it;
      write("\"" + key + "\": ");
      write_arg(m->lookup(key),indent+1);
      if (it != keys.end()-1) write(",");
      write("\n");
    }
    WRITE_INDENT(indent);
    write("}");

  } else {
    write(arg->get_string());
  }

  return true;
}

};
