/* SconeServer (http://www.sconemad.com)

Descriptor

This is the base class for readable/writable endpoints, which may represent
files, network connections, terminals, etc.

Descriptors can contain streams, which are arranged in a chain and act as
filters on any data read to, or written from, the descriptor. Streams can also
elect to recieve event notifications from the descriptor, indicating the
presence of data to be read, or free buffer space to write data. To recieve
events, the descriptor must be added to the SconeServer Kernel using connect();

For example, here is a descriptor containing two streams:

          +-------------------------------------------------------+
          | DESCRIPTOR                                            |
          |                   ###########   ###########           |
          |                   # STREAM1 #   # STREAM2 #           |
          |                   #         #   #         #           |
  LOW  <--|- endpoint_read <--# read <--#---# read <--#-- read <--|-  EXTERNAL
 LEVEL    |                   #         #   #         #           |  DESCRIPTOR
  I/O  <--|- endpoint_write <-# write <-#---# write <-#-- write <-|-   CALLS
          |                   #         #   #         #           |
 SCONEX --|--> dispatch       #  event  #   #  event  #           |
 KERNEL   |       |           #### | ####   #### | ####           |
          |       o--------------->o------------>o                |
          |                                                       |
          +-------------------------------------------------------+

As the above diagram indicates, any calls to the desriptor's public read() or
write() methods will result in a read or write through the chain of streams,
until the endpoint methods are reached. If there are no streams present, then
the public read() and write() calls will call the endpoint methods directly.

Events are sent out to streams individually, with most events being passed in
turn along the chain of streams. The only exception to this is the Closing
event, which is passed starting at the end of the stream chain, to allow a
sensible shutdown sequence to occur.

See the Stream class for a more details on events.
          
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

#ifndef scxDescriptor_h
#define scxDescriptor_h

#include "sconex/sconex.h"
#include "sconex/Date.h"
#include "sconex/Job.h"
namespace scx {

class Stream;
class DescriptorThread;
  
enum Condition { Ok, Wait, End, Close, Error };

//=============================================================================
class SCONEX_API Descriptor {

public:

  Descriptor();
  virtual ~Descriptor();

  Condition read(void* buffer,int n,int& na);
  Condition write(const void* buffer,int n,int& na);

  int write(const char* string);
  int write(const std::string& string);
  // write string from buffer to descriptor
  
  virtual void close() =0;
    
  enum State { Closed, Closing, Connected, Connecting, Listening };
  State state() const;

  virtual std::string describe() const;
  // Get a string describing this descriptor -
  // i.e. current state, address(es) of endpoints, etc.
  
  enum Error { Ok, Wait, Fatal };
  Error error() const;

  bool dup(int d);
  // Duplicate the descriptor
  
  void add_stream(Stream* stream);
  // Add new stream to the end of the chain
  
  void add_stream_before(Stream* stream,const Stream* before);
  void add_stream_after(Stream* stream,const Stream* after);
  // Add new stream before/after another stream in the chain
  
  bool remove_stream(Stream* stream);
  // Remove stream from the chain
    
  void set_timeout(const Time& t);
  void reset_timeout();
  // Reset watchdog timeout
  // Specifying 0 will switch off the watchdog

  int uid() const;
  // Get the descriptor's unique ID

  virtual int fd() =0;
  // Get the file descriptor
 
protected:

  virtual int event_create();
  // Called after descriptor is created to allow any options to
  // be set
  
  virtual int event_connecting();
  // Called when events are recieved while in connecting state
  // Should return nonzero to indicate an error has occured.
  
  virtual Condition endpoint_read(void* buffer,int n,int& na) =0;
  virtual Condition endpoint_write(const void* buffer,int n,int& na) =0;
  // Implement in derived classes to provide I/O direct to the
  // underlying stream.

  void set_blocking(bool onoff);
  // Set the descriptor blocking option

  //----------------------------
  // Data  
  
  State m_state;

  int m_virtual_events;

private:

  friend class Stream;
  friend class DescriptorJob;
  friend class Multiplexer;
  friend class DatagramHandler;
 
  std::list<Stream*> m_streams;
  // Stream list

  void link_streams();
  // Link up the stream list

  int get_event_mask();
  // Get a bitmask representing the enabled events

  int dispatch(int events);
  // Dispatch events to this descriptor
  // Return value indicates whether the socket is to remain open

  Time m_timeout_interval;
  Date m_timeout;
  bool check_timeout() const;
  // Check if the timeout has expired

  int m_uid;
  // Unique ID for this descriptor

  static int s_des_count;
  // Descriptor count, used to generate UIDs.
  
};

//=============================================================================
class SCONEX_API DescriptorJob : public Job {

public:

  DescriptorJob(Descriptor* d);
  virtual ~DescriptorJob();

  virtual bool should_run();
  virtual bool run();
  virtual std::string describe() const;

  Descriptor* get_descriptor();
  int get_event_mask();
  void set_events(int events);
  
private:

  Descriptor* m_descriptor;
  int m_events;

};

};
#endif
