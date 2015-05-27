/* SconeServer (http://www.sconemad.com)

Transfer stream

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

#ifndef scxStreamTransfer_h
#define scxStreamTransfer_h

#include <sconex/sconex.h>
#include <sconex/Stream.h>
#include <sconex/Descriptor.h>
#include <sconex/Buffer.h>
namespace scx {

#define StreamTransfer_MAX_BUFFER (10*1048576)
#define StreamTransfer_DEFAULT_BUFFER 65536

class StreamTransferSource;
class StreamTransferManager;
  
//=============================================================================
class SCONEX_API StreamTransfer : public Stream {

public:

  enum Status {
    Transfer, 
    Finished, 
    Read_error,
    Write_error
  };

  StreamTransfer(
    Descriptor* source_des,
    int buffer_size=StreamTransfer_DEFAULT_BUFFER
  );
  // Create a stream pump to transfer FROM instream
  // using the specified read buffer size

  virtual ~StreamTransfer();

  virtual Condition read(void* buffer,int n,int& na);
  virtual Condition write(const void* buffer,int n,int& na);

  virtual Condition event(Event e);

  virtual std::string stream_status() const;
  
  Status transfer();

  void set_close_when_finished(bool onoff);

protected:

  StreamTransfer(const StreamTransfer& c);
  StreamTransfer& operator=(const StreamTransfer& v);
  // Prohibit copy

  friend class StreamTransferManager;
  void source_event(Event e);

  Status m_status;
  
  StreamTransferManager* m_manager;

  Buffer m_buffer;

  bool m_close_when_finished;
  
};

//=============================================================================
class SCONEX_API StreamTransferSource : public Stream {

public:

  StreamTransferSource(StreamTransferManager* manager);
  virtual ~StreamTransferSource();

  virtual Condition event(Event e);

  virtual std::string stream_status() const;
  
protected:

  StreamTransferSource(const StreamTransferSource& c);
  StreamTransferSource& operator=(const StreamTransferSource& v);
  // Prohibit copy

  friend class StreamTransferManager;
  void dest_event(Event e);

  StreamTransferManager* m_manager;
  bool m_close;

};

//=============================================================================
class SCONEX_API StreamTransferManager {

public:
  
  StreamTransferManager(StreamTransfer* dest);
  ~StreamTransferManager();
  
  void set_source(StreamTransferSource* source);
  StreamTransferSource* get_source();

  void dest_event(Stream::Event e);
  void source_event(Stream::Event e);
  
  bool dest_finished();
  bool source_finished();

  int get_uid() const;
  
private:

  StreamTransferManager(const StreamTransferManager& c);
  StreamTransferManager& operator=(const StreamTransferManager& v);
  // Prohibit copy

  StreamTransfer* m_dest;
  StreamTransferSource* m_source;
  Mutex m_mutex;

  int m_uid;
  // Unique ID for this transfer

  static int s_tra_count;
  // Transfer count, used to generate UIDs.
  
};

  
};
#endif
