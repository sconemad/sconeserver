/* SconeServer (http://www.sconemad.com)

Sconex Configuration stream

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/ConfigStream.h>
#include <sconex/ScriptTypes.h>
#include <sconex/sconex.h>
#include <sconex/utils.h>

namespace scx {

//=========================================================================
ConfigStream::ConfigStream(
  ScriptRef* ctx,
  bool shutdown_on_exit
)
  : LineBuffer("config"),
    m_proc(ScriptAuth::Admin),
    m_ctx(ctx),
    m_shutdown_on_exit(shutdown_on_exit),
    m_output_mode(Formatted)
{
  m_proc.set_ctx(m_ctx);
  enable_event(Stream::Readable,true);
}

//=========================================================================
ConfigStream::~ConfigStream()
{
  delete m_ctx;
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

	if (buffer == "output none") {
	  m_output_mode = None;
	  continue;
	}
	if (buffer == "output formatted") {
	  m_output_mode = Formatted;
	  continue;
	}
	if (buffer == "output serialized") {
	  m_output_mode = Serialized;
	  continue;
	}

	if (buffer.find("set_int_type") == 0) {
	  std::string::size_type i = buffer.find(" ");
	  i = buffer.find_first_not_of(" ",i);
	  m_proc.set_int_type(buffer.substr(i));
	  continue;
	}
	
	if (buffer.find("set_real_type") == 0) {
	  std::string::size_type i = buffer.find(" ");
	  i = buffer.find_first_not_of(" ",i);
	  m_proc.set_real_type(buffer.substr(i));
	  continue;
	}
	
	ScriptRef* result = 0;
	try {
	  result = m_proc.evaluate(buffer);
	  switch (m_output_mode) {
	  case None: 
	    break;
	  case Formatted: 
	    write_object(result); 
	    write("\n"); 
	    break;
	  case Serialized: 
	    if (result) result->object()->serialize(*this); 
	    write("\n"); 
	    break;
	  }
	} catch (...) {
	  write("EXCEPTION!\n");
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
bool ConfigStream::write_object(const ScriptRef* ref, int indent)
{
  if (ref == 0) {
    write("NULL");
    return true;
  }
  const ScriptMethodRef* mref = dynamic_cast<const ScriptMethodRef*>(ref);

  const ScriptObject* object = ref->object();
  const std::type_info& ti = typeid(*object);
  
  write("(" + type_name(ti) + (mref ? " method":"") + ") ");

  if (typeid(ScriptList) == ti) {
    const ScriptList* l = dynamic_cast<const ScriptList*>(object);
    write("[\n");
    int max = l->size();
    for (int i=0; i<max; ++i) {
      WRITE_INDENT(indent+1);
      write_object(l->get(i),indent+1);
      if (i != max-1) write(",");
      write("\n");
    }
    WRITE_INDENT(indent);
    write("]");

  } else if (typeid(ScriptMap) == ti) {
    const ScriptMap* m = dynamic_cast<const ScriptMap*>(object);
    write("{\n");
    std::vector<std::string> keys;
    m->keys(keys);
    for (std::vector<std::string>::const_iterator it = keys.begin();
	 it != keys.end();
	 ++it) {
      WRITE_INDENT(indent+1);
      std::string key = *it;
      write("\"" + key + "\": ");
      write_object(m->lookup(key),indent+1);
      if (it != keys.end()-1) write(",");
      write("\n");
    }
    WRITE_INDENT(indent);
    write("}");

  } else {
    write(object->get_string());
  }

  if (mref) {
    write("::" + mref->method());
  }

  if (ref->is_const()) {
    write(" [const]");
  }

  return true;
}

};
