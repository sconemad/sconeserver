/* SconeServer (http://www.sconemad.com)

Sconex base Descriptor

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

#include "sconex/Descriptor.h"
#include "sconex/Stream.h"
namespace scx {

// Uncomment to enable debug logging of event dispatches
//#define EVENT_DEBUG_LOG(m) DESCRIPTOR_DEBUG_LOG(m)

#ifndef EVENT_DEBUG_LOG
#  define EVENT_DEBUG_LOG(m)
#endif
  
int Descriptor::s_des_count = 0;
  
//=============================================================================
Descriptor::Descriptor()
  : m_state(Closed),
    m_virtual_events(0),
    m_uid(++s_des_count)
{
  DEBUG_COUNT_CONSTRUCTOR(Descriptor);
  set_timeout(scx::Time(0));
}


//=============================================================================
Descriptor::~Descriptor()
{
  std::list<Stream*>::reverse_iterator it = m_streams.rbegin();
  while (it != m_streams.rend()) {
    delete (*it);
    ++it;
  }

  DEBUG_COUNT_DESTRUCTOR(Descriptor);
}

//=============================================================================
Condition Descriptor::read(void* buffer,int n,int& na)
{
  if (m_streams.empty()) {
    return endpoint_read(buffer,n,na);    
  }
  return m_streams.back()->read(buffer,n,na);
}

//=============================================================================
Condition Descriptor::write(const void* buffer,int n,int& na)
{
  if (m_streams.empty()) {
    return endpoint_write(buffer,n,na);    
  }
  return m_streams.back()->write(buffer,n,na);
}

//=============================================================================
// Write adaptor for zero terminated string
//
int Descriptor::write(const char* string)
{
  int nw=0;
  int len = strlen(string);
  write(string,len,nw);
  DEBUG_ASSERT(nw==len,"write(const char*) Did not write complete string");
  return nw;
}

//=============================================================================
// Write adaptor for c++ string
//
int Descriptor::write(const std::string& string)
{
  int nw=0;
  int len = string.size();
  write(string.c_str(),len,nw);
  DEBUG_ASSERT(nw==len,"write(const std::string&) Did not write string");
  if (nw!=len) throw std::exception();
  return nw;
}

//=============================================================================
Descriptor::State Descriptor::state() const
{
  return m_state;
}

//=============================================================================
std::string Descriptor::describe() const
{
  switch (m_state) {
    case Closed:     return "Closed";
    case Closing:    return "Closing";
    case Connected:  return "Connected";
    case Connecting: return "Connecting";
    case Listening:  return "Listening";
  }
  return "Unknown";
}

//=============================================================================
Descriptor::Error Descriptor::error() const
{
  switch (errno) {
    case 0:	return Ok;
    case EWOULDBLOCK: return Wait;
    case EINPROGRESS: return Wait;
    default: return Fatal;
  }
  return Ok;
}

//=============================================================================
bool Descriptor::dup(int d)
{
  DEBUG_ASSERT(d>=0,"dup() Invalid descriptor");
 
  if (dup2(fd(),d) < 0) {
    return false;
  }
  return true;
}

//=============================================================================
void Descriptor::add_stream(Stream* stream)
{
  DEBUG_ASSERT(stream>=0,"add_stream() Invalid stream");

  m_streams.push_back(stream);
  stream->set_endpoint(this);
  link_streams();
}

//=============================================================================
void Descriptor::add_stream_before(Stream* stream,const Stream* before)
{
  DEBUG_ASSERT(stream,"add_stream_before() Invalid stream");
  DEBUG_ASSERT(before,"add_stream_before() Invalid marker stream");
  
  std::list<Stream*>::iterator it = m_streams.end();
  while (it != m_streams.begin()) {
    if (before == (*it)) {
      --it;
      break;
    }
    --it;
  }
  DEBUG_ASSERT(it != m_streams.begin(),
               "add_stream_before() Marker stream not found - adding to start");

  m_streams.insert(it,stream);
  stream->set_endpoint(this);
  link_streams();
}

//=============================================================================
void Descriptor::add_stream_after(Stream* stream,const Stream* after)
{
  DEBUG_ASSERT(stream,"add_stream_after() Invalid stream");
  DEBUG_ASSERT(after,"add_stream_after() Invalid marker stream");
  
  std::list<Stream*>::iterator it = m_streams.begin();
  while (it != m_streams.end()) {
    if (after == (*it)) {
      ++it;
      break;
    }
    ++it;
  }
  DEBUG_ASSERT(it != m_streams.end(),
               "add_stream_after() Marker stream not found - adding to end");
  
  m_streams.insert(it,stream);
  stream->set_endpoint(this);
  link_streams();
}

//=============================================================================
bool Descriptor::remove_stream(Stream* stream)
{
  DEBUG_ASSERT(stream>=0,"remove_stream() Invalid stream");

  std::list<Stream*>::iterator it = m_streams.begin();
  while (it != m_streams.end()) {
    if (stream == (*it)) {
      m_streams.erase(it);
      link_streams();
      return true;
    }
    ++it;
  }
  // Didn't find stream
  return false;
}

//=============================================================================
void Descriptor::set_timeout(const scx::Time& t)
{
  m_timeout_interval = t;
  reset_timeout();
}

//=============================================================================
void Descriptor::reset_timeout()
{
  if (m_timeout_interval.seconds() > 0) {
    m_timeout = scx::Date::now() + m_timeout_interval;
  } else {
    m_timeout = scx::Date();
  }
}

//=============================================================================
int Descriptor::uid() const
{
  return m_uid;
}

//=============================================================================
int Descriptor::event_create()
{
  return 0;
}

//=============================================================================
int Descriptor::event_connecting()
{
  m_state = Connected;
  return 0;
}

//=============================================================================
void Descriptor::link_streams()
{
  Stream* prev=0;
  std::list<Stream*>::iterator it = m_streams.begin();
  while (it != m_streams.end()) {
    Stream* stream = (*it);
    stream->set_chain(prev);
    prev = stream;
    ++it;
  }
}

//=============================================================================
int Descriptor::get_event_mask()
{
  int event_mask = 0;

  if (m_state == Connecting) {
    // Non-blocking connect in progress, wait for writeable
    // as that will indicate a connection has ocurred
    event_mask = (1<<Stream::Writeable);
    
  } else if (m_state != Closed) {
    if (fd() < 0) {
      if (m_virtual_events & (1<<Stream::Readable)) event_mask |= Stream::SendReadable;
      if (m_virtual_events & (1<<Stream::Writeable)) event_mask |= Stream::SendWriteable;
    }
    std::list<Stream*>::const_iterator it = m_streams.begin();
    while (it != m_streams.end()) {
      const Stream* stream = (*it);
      event_mask |= stream->m_events;
      ++it;
    }
  }
  
  return event_mask;
}

//=============================================================================
int Descriptor::dispatch(int events)
{
  if (state() == Closed) {
    // Closed by some other means
    EVENT_DEBUG_LOG("Dispatch Closed");
    return 1;
  }

  if (check_timeout()) {
    // Close due to timeout
    EVENT_DEBUG_LOG("Dispatch Closing due to timeout");
    return 1;
  }

  // Add virtual events
  events |= m_virtual_events;

  // Decode individual events
  bool event_opened    = (state() == Connected);
  bool event_readable  = events & (1<<Stream::Readable); 
  bool event_writeable = events & (1<<Stream::Writeable);
//  bool event_except    = FD_ISSET(fd(),except_set);

  if (state() == Connecting) {
    // Result of a non-blocking connect
    if (event_writeable) {
      if (event_connecting() != 0) {
        return 1;
      }
    }
    return 0;
  }
  
  bool remove,error,close,open_wait;

  // OPEN/READ/WRITE SEQUENCE
  std::list<Stream*>::iterator it = m_streams.begin();
  while (it != m_streams.end()) {
    Stream* stream = (*it);
    remove = error = close = open_wait = false;
    std::string name = stream->stream_name();

    if (event_opened && 
	!stream->event_enabled(Stream::Opening)) {

      EVENT_DEBUG_LOG("Dispatch OPENED -> [" << name << "] -> ");
      switch(stream->event(Stream::Opening)) {
        case scx::Ok:
          EVENT_DEBUG_LOG("OK");
          stream->enable_event(Stream::Opening,true);
          break;
        case scx::Wait:
          EVENT_DEBUG_LOG("WAIT");
          open_wait = true; 
          break;
        case scx::End:
          EVENT_DEBUG_LOG("END");
          remove = true;
          break;
        case scx::Close:
          EVENT_DEBUG_LOG("CLOSE");
          stream->enable_event(Stream::Opening,true);
          close = true;
          break;
        case scx::Error:
          EVENT_DEBUG_LOG("ERROR");
          error = true;
          break;
      }
    }
    
    if (!remove && 
	!error &&
        event_readable && 
	stream->event_enabled(Stream::Readable)) {

      EVENT_DEBUG_LOG("Dispatch READABLE -> [" << name << "] -> ");
      switch(stream->event(Stream::Readable)) {
        case scx::Ok:
          EVENT_DEBUG_LOG("OK");
          break;
        case scx::Wait:
          EVENT_DEBUG_LOG("WAIT");
          event_readable = false;
          break;
        case scx::End:
          EVENT_DEBUG_LOG("END");
          remove = true;
          break;
        case scx::Close:
          EVENT_DEBUG_LOG("CLOSE");
          close = true;
          break;
        case scx::Error:
          EVENT_DEBUG_LOG("ERROR");
          error = true;
          break;
      }
    }

    if (!remove && 
	!error &&
        event_writeable && 
	stream->event_enabled(Stream::Writeable)) {

      EVENT_DEBUG_LOG("Dispatch WRITEABLE -> [" << name << "] -> ");
      switch(stream->event(Stream::Writeable)) {
        case scx::Ok:
          EVENT_DEBUG_LOG("OK");
          break;
        case scx::Wait:
          EVENT_DEBUG_LOG("WAIT");
          event_writeable = false;
          break;
        case scx::End:
          EVENT_DEBUG_LOG("END");
          remove = true;
          break;
        case scx::Close:
          EVENT_DEBUG_LOG("CLOSE");
          close = true;
          break;
        case scx::Error:
          EVENT_DEBUG_LOG("ERROR");
          error = true;
          break;
      }
    }

    if (close && state() != Closed) {
      m_state = Closing;
    }
        
    if (open_wait &&
	!stream->event_enabled(Stream::Opening)) {
      break;
    }

    if (error) {
      // Close socket due to error
      EVENT_DEBUG_LOG("Dispatch Closing descriptor");
      return 1;
    }

    if (remove) {
      // Remove stream from list and relink
      it = m_streams.erase(it);
      delete stream;
      link_streams();
      
    } else {
      // See if there any events to send
      if (!event_readable && 
	  stream->event_enabled(Stream::SendReadable)) {
	EVENT_DEBUG_LOG("SEND READABLE");
	event_readable = true;
      }
      if (!event_writeable 
	  && stream->event_enabled(Stream::SendWriteable)) {
	EVENT_DEBUG_LOG("SEND WRITEABLE");
	event_writeable = true;
      }

      ++it;    
    }

  }

  // CLOSE SEQUENCE    
  if (state() == Closing) {
    std::list<Stream*>::reverse_iterator it = m_streams.rbegin();
    while (it != m_streams.rend()) {
      Stream* stream = (*it);
      std::string name = stream->stream_name();
  
      EVENT_DEBUG_LOG("Dispatch CLOSING -> [" << name << "] -> ");
      Condition c = stream->event(Stream::Closing);
      
      if (c == scx::Wait) {
        // Suspend closing for now
        EVENT_DEBUG_LOG("WAIT");
        break;
        
      } else if (c == scx::End) {
	// Cancel closing
        EVENT_DEBUG_LOG("CANCELLED");
	m_state = Connected;
	break;

      } else if (c == scx::Error) {
        EVENT_DEBUG_LOG("ERROR - Closing descriptor");
        return 1;
      }

      EVENT_DEBUG_LOG("OK - Removing stream");
      // Remove stream from list and relink
      it = std::list<Stream*>::reverse_iterator(
        m_streams.erase(--(it.base())) );
      delete stream;
      link_streams();
      
      if (m_streams.size() == 0) break;
    }      
  }

  if (m_streams.size() == 0) {
    EVENT_DEBUG_LOG("Dispatch No streams left - Closing descriptor");
    return 1;
  }
  
  return 0;
}

//=============================================================================
bool Descriptor::check_timeout() const
{
  if (!m_timeout.valid()) {
    return false;
  }
  return (scx::Date::now() > m_timeout);
}

//=============================================================================
void Descriptor::set_blocking(bool onoff)
{
  // Set non-blocking mode
  int flags = fcntl(fd(),F_GETFL);
  if (onoff) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  if (fcntl(fd(),F_SETFL,flags) < 0) {
    DESCRIPTOR_DEBUG_LOG("set_blocking() could not change mode");
  }
}


//=============================================================================
DescriptorJob::DescriptorJob(Descriptor* d)
  : Job("DES"),
    m_descriptor(d),
    m_events(0)
{
  DEBUG_COUNT_CONSTRUCTOR(DescriptorJob);
}
	
//=============================================================================
DescriptorJob::~DescriptorJob()
{
  m_descriptor->close();
  delete m_descriptor;
  DEBUG_COUNT_DESTRUCTOR(DescriptorJob);
}

//=============================================================================
bool DescriptorJob::run()
{
  int retval = m_descriptor->dispatch(m_events);

  return (bool)retval;
}

//=============================================================================
std::string DescriptorJob::describe() const
{
  std::ostringstream oss;
  oss << "[" << m_descriptor->uid() << "] " << m_descriptor->describe() << "\n";

  std::list<Stream*>::const_iterator its = m_descriptor->m_streams.begin();
  while (its != m_descriptor->m_streams.end()) {
    oss << "   - " << (*its)->stream_name()
	<< " " << (*its)->stream_status() << "\n";
    its++;
  }

  return oss.str();
}

//=============================================================================
Descriptor* DescriptorJob::get_descriptor()
{
  return m_descriptor;
}

//=============================================================================
void DescriptorJob::set_events(int events)
{
  m_events = events;
}

};
