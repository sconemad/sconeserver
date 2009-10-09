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

#include "sconex/ArgStore.h"
#include "sconex/ArgProc.h"
#include "sconex/File.h"
#include "sconex/utils.h"
namespace scx {

// Uncomment to enable debug info
//#define ArgStore_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef ArgStore_DEBUG_LOG
#  define ArgStore_DEBUG_LOG(m)
#endif

//=============================================================================
ArgStore::ArgStore(const FilePath& path)
  : m_path(path),
    m_data(0)
{
  DEBUG_COUNT_CONSTRUCTOR(ArgStore);
  reset();
}
 
//=============================================================================
ArgStore::~ArgStore()
{
  delete m_data;
  DEBUG_COUNT_DESTRUCTOR(ArgStore);
}

//=============================================================================
bool ArgStore::load()
{
  scx::File file;
  if (file.open(m_path,scx::File::Read) == scx::Ok) {
    ArgStoreStream* s = new ArgStoreStream(ArgStoreStream::Read);
    file.add_stream(s);
    Condition c = Ok;
    while (c == Ok) {
      c = s->event(Stream::Readable);
    }
    Arg* res = s->take_arg_read();
    ArgMap* res_map = dynamic_cast<ArgMap*>(res);
    if (res_map) {
      delete m_data;
      m_data = res_map;
    } else {
      if (res) {
	ArgStore_DEBUG_LOG("Read bad data: " << res->get_string());
	delete res;
      } else {
	ArgStore_DEBUG_LOG("Read NULL data");
      }
    }
    file.close();
    return (c == End);
  }
  
  return false;
}

//=============================================================================
bool ArgStore::save()
{
  scx::File file;
  if (file.open(m_path,File::Write | File::Truncate | File::Create) == scx::Ok) {
    ArgStoreStream* s = new ArgStoreStream(ArgStoreStream::Write, m_data);
    file.add_stream(s);
    Condition c = Ok;
    while (c == Ok) {
      c = s->event(Stream::Writeable);
    }
    file.close();
    return (c == End);
  }
  return false;
}

//=========================================================================
void ArgStore::reset()
{
  delete m_data;
  m_data = new ArgMap();
}

//=========================================================================
std::string ArgStore::name() const
{
  return "ArgStore";
}

//=========================================================================
Arg* ArgStore::arg_lookup(const std::string& name)
{
  // Methods
  if ("save" == name ||
      "load" == name ||
      "add" == name ||
      "set" == name ||
      "remove" == name ||
      "reset" == name) {
    return new_method(name);
  }

  // Sub-objects
  Arg* a = m_data->lookup(name);
  if (a) return a->ref_copy(Arg::Ref);

  return ArgObjectInterface::arg_lookup(name);
}

//=========================================================================
Arg* ArgStore::arg_function(const Auth& auth, const std::string& name,Arg* args)
{
  ArgList* l = dynamic_cast<ArgList*>(args);

  if (name == "save") {
    save();
    return 0;
  }

  if (name == "load") {
    load();
    return 0;
  }

  if ("add" == name ||
      "set" == name) {
    Arg* n = l->get(0);
    if (n == 0) {
      return new ArgError("add_meta: No name specified");
    }
    Arg* v = l->take(1);
    if (v == 0) {
      return new ArgError("add_meta: No value specified");
    }
    m_data->give(n->get_string(),v);
    return 0;
  }
  
  if ("remove" == name) {
    Arg* n = l->get(0);
    if (n == 0) {
      return new ArgError("remove_meta: No name specified");
    }
    Arg* v = m_data->take(n->get_string());
    if (v == 0) {
      return new ArgError("remove_meta: Does not exist");
    }
    delete v;
    return 0;
  }

  if ("reset" == name) {
    reset();
    return 0;
  }

  return ArgObjectInterface::arg_function(auth,name,args);
}

//=============================================================================
void ArgStore::store_arg(Descriptor& out, const Arg* arg)
{
  if (arg == 0) {
    out.write("0");
    return;
  }

  const std::type_info& ti = typeid(*arg);
  
  if (typeid(ArgString) == ti) {
    out.write("\"" + escape_quotes(arg->get_string()) + "\"");

  } else if (typeid(ArgInt) == ti) {
    out.write(arg->get_string());
    
  } else if (typeid(ArgReal) == ti) {
    out.write(arg->get_string());

  } else if (typeid(ArgList) == ti) {
    const ArgList* l = dynamic_cast<const ArgList*>(arg);
    out.write("[");
    int max = l->size();
    for (int i=0; i<max; ++i) {
      if (i>0) out.write(",");
      store_arg(out,l->get(i));
    }
    out.write("]");
    
  } else if (typeid(ArgMap) == ti) {
    const ArgMap* m = dynamic_cast<const ArgMap*>(arg);
    out.write("{");
    std::vector<std::string> keys;
    m->keys(keys);
    for (std::vector<std::string>::const_iterator it = keys.begin();
	 it != keys.end();
	 ++it) {
      if (it != keys.begin()) out.write(",");
      std::string key = *it;
      out.write("\"" + key + "\":");
      store_arg(out,m->lookup(key));
    }
    out.write("}");

  } else {
    out.write("");
    //    write("\"" + arg->get_string() + "\"");
  }
}


//=============================================================================
ArgStoreStream::ArgStoreStream(Mode mode, Arg* arg_write)
  : Stream("ArgStore"),
    m_mode(mode),
    m_arg(arg_write)
{
  if (m_mode == Read) {
    enable_event(Stream::Readable,true);
  } else if (m_mode == Write) {
    enable_event(Stream::Writeable,true);
  }
}

//=============================================================================
ArgStoreStream::~ArgStoreStream()
{
  if (m_mode == Read) {
    delete m_arg;
  }
}

//=============================================================================
Condition ArgStoreStream::event(Stream::Event e)
{
  if (e == Stream::Readable) {
    char buffer[1024];
    int na = 0;
    Condition c = Ok;
    c = End;
    while ((c = read(buffer,1024,na)) == Ok) {
      m_data += std::string(buffer,na);
    }
    if (c == End) {
      scx::ArgProc proc(Auth::Untrusted,0);
      delete m_arg;
      m_arg = 0;
      m_arg = proc.evaluate(m_data);
      m_data.clear();
    }
    return c;
  }

  if (e == Stream::Writeable) {
    ArgStore::store_arg(endpoint(),m_arg);
    return End;
  }

  return Ok;
}

//=============================================================================
Arg* ArgStoreStream::take_arg_read()
{
  Arg* ret = m_arg;
  m_arg = 0;
  return ret;
}

};
