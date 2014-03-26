/* SconeServer (http://www.sconemad.com)

SconeX abstract base stream

Copyright (c) 2000-2014 Andrew Wedgbury <wedge@sconemad.com>

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

#include <sconex/Stream.h>
#include <sconex/Module.h>
namespace scx {

ProviderScheme<Stream>* Stream::s_providers = 0;

//=========================================================================
Stream* Stream::create_new(const std::string& type,
			   const ScriptRef* args)
{
  init();
  return s_providers->provide(type,args);
}

//=============================================================================
Stream::Stream(const std::string& stream_name)
  : m_stream_name(stream_name),
    m_events(0),
    m_chain(0),
    m_endpoint(0)
{
  DEBUG_COUNT_CONSTRUCTOR(Stream);
}

//=============================================================================
Stream::~Stream()
{
  /*
  // Release the module refs
  for (ModuleRefList::iterator it = m_module_refs.begin();
       it != m_module_refs.end();
       ++it) {
    ScriptRef* r = (*it);
    delete r;
  }
  */

  DEBUG_COUNT_DESTRUCTOR(Stream);
}

//=============================================================================
Condition Stream::read(void* buffer,int n,int& na)
{
  DEBUG_ASSERT(m_chain || m_endpoint,"read() Unconnected stream");
  if (m_chain) {
    return m_chain->read(buffer,n,na);
  }
  return m_endpoint->endpoint_read(buffer,n,na);
}

//=============================================================================
Condition Stream::write(const void* buffer,int n,int& na)
{
  DEBUG_ASSERT(m_chain || m_endpoint,"write() Unconnected stream");
  if (m_chain) {
    return m_chain->write(buffer,n,na);
  }
  return m_endpoint->endpoint_write(buffer,n,na);
}

//=============================================================================
// Write adaptor for zero terminated string
//
int Stream::write(const char* string)
{
  int nw=0;
  int len = strlen(string);

  if (m_chain) {
    return m_chain->write(string,len,nw);
  }
  m_endpoint->endpoint_write(string,len,nw);

  DEBUG_ASSERT(nw==len,"write(const char*) Could not write complete string");
  return nw;
}

//=============================================================================
// Write adaptor for c++ string
//
int Stream::write(const std::string& string)
{
  int nw=0;
  int len = string.size();

  if (m_chain) {
    return m_chain->write(string.c_str(),len,nw);
  }
  m_endpoint->endpoint_write(string.c_str(),len,nw);

  DEBUG_ASSERT(nw==len,"write(const std::string&) Could not write complete string");
  return nw;
}

//=============================================================================
Condition Stream::event(Event e)
{
  return Ok;
}

//=============================================================================
void Stream::set_endpoint(Descriptor* endpoint)
{
  m_endpoint = endpoint;
}

//=============================================================================
void Stream::set_chain(Stream* chain)
{
  m_chain = chain;
}

/*
//=============================================================================
void Stream::add_module_ref(Module* module)
{
  m_module_refs.push_back(new ScriptRef(module));
}
*/

//=============================================================================
const std::string& Stream::stream_name() const
{
  return m_stream_name;
}

//=============================================================================
std::string Stream::stream_status() const
{
  return "";
}

//=============================================================================
std::string Stream::event_status() const
{
  std::string es = "------";
  if (m_events & (1<<Opening)) es[0]='o';
  if (m_events & (1<<Closing)) es[1]='c';
  if (m_events & (1<<Readable)) es[2]='r';
  if (m_events & (1<<Writeable)) es[3]='w';
  if (m_events & (1<<SendReadable)) es[4]='R';
  if (m_events & (1<<SendWriteable)) es[5]='W';
  return es;
}

//=============================================================================
void Stream::register_stream(const std::string& type,
			     Provider<Stream>* factory)
{
  init();
  s_providers->register_provider(type,factory);
}

//=============================================================================
void Stream::unregister_stream(const std::string& type,
			       Provider<Stream>* factory)
{
  init();
  s_providers->unregister_provider(type,factory);
}

//=============================================================================
bool Stream::event_enabled(Event e) const
{
  return 0 != (m_events & (1 << e));
}

//=============================================================================
void Stream::enable_event(Event e, bool onoff)
{
  if (onoff) {
    m_events |= (1 << e);
  } else {
    m_events &= ~(1 << e);
  }
}

//=============================================================================
Stream* Stream::find_stream(const std::string& stream_name)
{
  if (m_chain == 0) {
    return 0;

  } else if (m_chain->stream_name() == stream_name) {
    return m_chain;
  }

  return m_chain->find_stream(stream_name);
}

//=============================================================================
Descriptor& Stream::endpoint()
{
  DEBUG_ASSERT(0 != m_endpoint,"endpoint() No endpoint set");
  return *m_endpoint;
}

//=============================================================================
void Stream::init()
{
  if (!s_providers) s_providers = new ProviderScheme<Stream>();
}

};
