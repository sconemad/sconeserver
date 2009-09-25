/* SconeServer (http://www.sconemad.com)

Stream base class

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxStream_h
#define scxStream_h

#include "sconex/sconex.h"
#include "sconex/Descriptor.h"
#include "sconex/ModuleRef.h"
namespace scx {

//=============================================================================
class SCONEX_API Stream {

public:

  Stream(const std::string& stream_name);
  virtual ~Stream();
    
  virtual Condition read(void* buffer,int n,int& na);
  // Read n bytes from stream into buffer

  virtual Condition write(const void* buffer,int n,int& na);
  // write n bytes from buffer to stream

  int write(const char* string);
  int write(const std::string& string);
  // write string from buffer to stream

  // Event types
  enum Event {
    Opening, Closing,
    Readable, Writeable,
    SendReadable, SendWriteable
  };
  
  virtual Condition event(Event e);
  // Event notification
  // 
  // Opening
  //   This event is sent to each stream in order proceeding up the chain when
  //   a connection is opened. Returning Wait will delay sending Opening to
  //   subsequent streams - Opening events will then continue to be sent until
  //   Ok is returned, when the Opening event will then proceed up the chain.
  //   Any new streams added to the chain while the connection is opened will
  //   recieve an Opening event, and will continue to do so until they
  //   return Ok.
  //
  // Closing
  //   This event is sent to each stream is order proceeding DOWN the chain
  //   when a connection is closing.
  //
  // Readable, Writeable
  //   These are sent to streams that have enabled these events (using
  //   enable_event) when it is possible to read data from, or write data to,
  //   the endpoint or any preceeding streams. Returning Ok will cause the
  //   event to be proporgated to the next interested stream UP the chain,
  //   while returning Wait will cause event processing to stop at this stream.
  //   

  void set_endpoint(Descriptor* endpoint);
  void set_chain(Stream* chain);
  // Get/set chain pointer

  void add_module_ref(ModuleRef ref);
  // Make this stream reference a module
  // This should prevent the module from being unloaded while this stream is
  // active.
  
  const std::string& stream_name() const;
  // Get the name of the stream

  virtual std::string stream_status() const;
  // Get current status of the stream (if any) for debugging or
  // informational purposes. Derived classes can override this if
  // they wish to supply status information which might be useful
  // for debugging.

  std::string event_status() const;
  
protected:

  friend class Descriptor;

  bool event_enabled(Event e) const;
  // Is the specified event enabled
  
  void enable_event(Event e, bool onoff);
  // Enable/disable specified event
  //
  // Readable, Writeable
  //   Enable these events. event() will be called with the specified event
  //   when it is possible to read or write from/to the endpoint or preceeding
  //   streams.
  // 
  // SendReadable, SendWriteable
  //   Enabling these will cause Readable/Writeable events to be generated
  //   by this stream and passed UP the chain from this stream.
  //

  Stream* find_stream(const std::string& stream_name);
  // Try and find named stream in the chain of preceeding streams

  Descriptor& endpoint();
  // Allow const access to the endpoint

  std::string m_stream_name;
      
private:

  int m_events;

  std::list<ModuleRef*> m_module_refs;
  
  Stream* m_chain;
  Descriptor* m_endpoint;
  
};

};
#endif
