/* SconeServer (http://www.sconemad.com)

Stream base class

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

#ifndef scxStream_h
#define scxStream_h

#include <sconex/sconex.h>
#include <sconex/IOBase.h>
#include <sconex/Descriptor.h>
#include <sconex/Provider.h>
namespace scx {

class Module;
class ScriptRef;

//=============================================================================
class SCONEX_API Stream : public IOBase {

public:

  // Create a new stream of the specified type
  static Stream* create_new(const std::string& type,
			    const ScriptRef* args);

  explicit Stream(const std::string& stream_name);
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
  // Handle event notification
  //
  // Streams need to override this method in order to handle events.
  // 
  // Opening
  //   This event is sent to each stream in order proceeding up the chain when
  //   a connection is opened (or to new streams that are added to the chain).
  //   Return values:
  //     Ok    - Accept, opened will not be called again on this stream
  //     Wait  - Don't send opened event to subsequent streams yet
  //     End   - Remove this stream from the list, no further events
  //     Close - Initiate close sequence for this connection
  //     Error - Force the connection closed at earliest oppurtunity
  //
  // Closing
  //   This event is sent to each stream is order proceeding DOWN the chain
  //   when a connection is closing.
  //   Return values:
  //     Ok    - Remove this stream from the list, no further events
  //     Close - Remove this stream from the list, no further events
  //     Wait  - Suspend closing temporarily, don't send closing 
  //             event to preceeding streams yet.
  //     End   - Suspend closing, connection should remain open.
  //     Error - Force the connection closed at earliest oppurtunity
  //
  //
  // Readable and Writeable
  //   These are sent to streams that have enabled these events (using
  //   enable_event) when it is possible to read data from, or write data to,
  //   the endpoint, or any preceeding streams. Returning Ok will cause the
  //   event to be proporgated to the next interested stream UP the chain,
  //   while returning Wait will cause event processing to stop at this stream.
  //   Return values:
  //     Ok    - Accept
  //     Wait  - Don't send event to subsequent streams yet
  //     End   - Remove this stream from the list, no further events
  //     Close - Initiate close sequence for this connection
  //     Error - Force the connection closed at earliest oppurtunity  
  //

  void set_endpoint(Descriptor* endpoint);
  void set_chain(Stream* chain);
  // Get/set chain pointer

  //  void add_module_ref(Module* module);
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
  // Get a string indicating the current event status of this stream.

  // Stream provider interface
  static void register_stream(const std::string& type,
			      Provider<Stream>* factory);
  static void unregister_stream(const std::string& type,
				Provider<Stream>* factory);
  
protected:

  Stream(const Stream& c);
  Stream& operator=(const Stream& v);
  // Prohibit copy

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
  // Allow access to the endpoint

  std::string m_stream_name;
  // The name of the stream
      
private:

  // Event status
  int m_events;

  /*
  // List of modules used by this stream
  typedef std::list<ScriptRef*> ModuleRefList;
  ModuleRefList m_module_refs;
  */

  // Upstream pointer
  Stream* m_chain;

  // Endpoint (where the data is ultimately written to and read from)
  Descriptor* m_endpoint;

  static void init();

  // Stream providers
  static ProviderScheme<Stream>* s_providers;
  
};

};
#endif
