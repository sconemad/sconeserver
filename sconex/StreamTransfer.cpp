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

#include <sconex/StreamTransfer.h>
namespace scx {

// Uncomment to enable debug logging
//#define TRANSFER_DEBUG_LOG(m) STREAM_DEBUG_LOG(m)

#ifndef TRANSFER_DEBUG_LOG
#  define TRANSFER_DEBUG_LOG(m)
#endif


//=============================================================================
StreamTransfer::StreamTransfer(
  Descriptor* source_des,
  int buffer_size
) : Stream("transfer"),
    m_status(StreamTransfer::Transfer),
    m_buffer(buffer_size),
    m_close_when_finished(false)
{
  DEBUG_COUNT_CONSTRUCTOR(StreamTransfer);

  m_manager = new StreamTransferManager(this);
  
  StreamTransferSource* source = new StreamTransferSource(m_manager);
  m_manager->set_source(source);
  if (source_des) {
    source_des->add_stream(source);
  }
}

//=============================================================================
StreamTransfer::~StreamTransfer()
{
  if (m_manager->dest_finished()) {
    delete m_manager;
  }
  
  DEBUG_COUNT_DESTRUCTOR(StreamTransfer);
}

//=============================================================================
Condition StreamTransfer::read(void* buffer,int n,int& na)
{
  return Stream::read(buffer,n,na);
}

//=============================================================================
Condition StreamTransfer::write(const void* buffer,int n,int& na)
{
  return Stream::write(buffer,n,na);
}

//=============================================================================
Condition StreamTransfer::event(Event e)
{
  switch (e) {
  
    case Stream::Opening:
      return scx::Ok;

    case Stream::Writeable:
      switch (transfer()) {
        
        case Transfer:
          return scx::Wait;
          
        case Finished:
          enable_event(Stream::Writeable,false);
          // Close source stream
          m_manager->dest_event(Stream::Closing); 
          if (m_close_when_finished) {
            return scx::Close;
          }
          return scx::End;

        case Read_error:
        case Write_error:
          return scx::Error;
      }
      break;

    default:
      break;
  }
  
  return scx::Ok;
}

//=============================================================================
std::string StreamTransfer::stream_status() const
{
  std::ostringstream oss;
  oss << "<-[" << m_manager->get_uid() << "] buf:" << m_buffer.status_string();
  if (m_close_when_finished) oss << " AUTOCLOSE";
  return oss.str();
}

//=============================================================================
StreamTransfer::Status StreamTransfer::transfer()
{
  if (m_status != StreamTransfer::Transfer) {
    return m_status;
  }

  int bytes_buffered = m_buffer.used();

  StreamTransferSource* source = m_manager->get_source();
  
  if (bytes_buffered==0 && source) {
    Condition c_in = source->read(
      m_buffer.tail(),m_buffer.free(),bytes_buffered);
    m_buffer.push(bytes_buffered);
    if (c_in==scx::End && bytes_buffered==0) {
      m_status = StreamTransfer::Finished;
      TRANSFER_DEBUG_LOG("read END c_in=" << c_in);
      return m_status;

    } else if (c_in==scx::Error) {
      m_status = StreamTransfer::Read_error;
      STREAM_DEBUG_LOG("read ERROR c_in=" << c_in);
      return m_status;
    }

    TRANSFER_DEBUG_LOG("read " << bytes_buffered << " bytes");
  }

  if (bytes_buffered==0) {
    enable_event(Stream::Writeable,false);
    if (!source) {
      m_status = StreamTransfer::Finished;
      TRANSFER_DEBUG_LOG("read END (source gone)");
      return m_status;      
    }
    m_manager->dest_event(Stream::Writeable);
    TRANSFER_DEBUG_LOG("wait (buffer empty)");
    return m_status;
  }

  int bytes_written=0;
  Condition c_out = Stream::write(
    m_buffer.head(),bytes_buffered,bytes_written);
  if (c_out==scx::Error) {
    m_status = StreamTransfer::Write_error;
    STREAM_DEBUG_LOG("write ERROR c_out=" << c_out);
    return m_status;
  }
  m_buffer.pop(bytes_written);

  endpoint().reset_timeout();
  TRANSFER_DEBUG_LOG("write " << bytes_written << " bytes");
  return m_status;
}

//=============================================================================
void StreamTransfer::set_close_when_finished(bool onoff)
{
  m_close_when_finished = onoff;
}

//=============================================================================
void StreamTransfer::source_event(Event e)
{
  switch (e) {
    
    case Stream::Readable: {
      enable_event(Stream::Writeable,true);
    } break;
    
    case Stream::Closing: {
      // Source has closed
    } break;

    default:
      break;
  }
}

//=============================================================================
StreamTransferSource::StreamTransferSource(
  StreamTransferManager* manager
) : Stream("transfer-src"),
    m_manager(manager),
    m_close(false)
{
  DEBUG_COUNT_CONSTRUCTOR(StreamTransferSource);

  enable_event(Stream::Readable,true);
}

//=============================================================================
StreamTransferSource::~StreamTransferSource()
{
  if (m_manager->source_finished()) {
    delete m_manager;
  }
  
  DEBUG_COUNT_DESTRUCTOR(StreamTransferSource);
}

//=============================================================================
Condition StreamTransferSource::event(Stream::Event e)
{
  if (m_close) {
    return scx::Close;
  }

  switch (e) {
    
    case Stream::Readable:
      enable_event(Stream::Readable,false);
      
    case Stream::Closing: {
      m_manager->source_event(e);
    } break;

    default:
      break;
  }

  return scx::Ok;
}

//=============================================================================
std::string StreamTransferSource::stream_status() const
{
  std::ostringstream oss;
  oss << "->[" << m_manager->get_uid() << "]";
  return oss.str();
}

//=============================================================================
void StreamTransferSource::dest_event(Event e)
{
  switch (e) {

    case Stream::Writeable: {
      endpoint().reset_timeout();
      enable_event(Stream::Readable,true);
    } break;
    
    case Stream::Closing: {
      // Dest has finished reading
      m_close = true;
      enable_event(Stream::Readable,true);
    } break;

    default:
      break;
  }
}


int StreamTransferManager::s_tra_count = 0;
  
//=============================================================================
StreamTransferManager::StreamTransferManager(StreamTransfer* dest)
  : m_dest(dest),
    m_source(0),
    m_uid(++s_tra_count)
{
  DEBUG_COUNT_CONSTRUCTOR(StreamTransferManager);
}

//=============================================================================
StreamTransferManager::~StreamTransferManager()
{
  DEBUG_COUNT_DESTRUCTOR(StreamTransferManager);
}

//=============================================================================
void StreamTransferManager::set_source(StreamTransferSource* source)
{
  m_source = source;
}

//=============================================================================
StreamTransferSource* StreamTransferManager::get_source()
{
  return m_source;
}

//=============================================================================
void StreamTransferManager::dest_event(Stream::Event e)
{
  m_mutex.lock();
  if (m_source) {
    m_source->dest_event(e);
  }
  m_mutex.unlock();
}

//=============================================================================
void StreamTransferManager::source_event(Stream::Event e)
{
  m_mutex.lock();
  if (m_dest) {
    m_dest->source_event(e);
  }
  m_mutex.unlock();
}

//=============================================================================
bool StreamTransferManager::dest_finished()
{
  m_mutex.lock();

  m_dest = 0;
  
  if (m_source) {
    m_source->dest_event(Stream::Closing);
  }
  
  bool killme = (m_source == 0);

  m_mutex.unlock();
  return killme;
}

//=============================================================================
bool StreamTransferManager::source_finished()
{
  m_mutex.lock();
  
  m_source = 0;

  if (m_dest) {
    m_dest->source_event(Stream::Closing); 
  }

  bool killme = (m_dest == 0);

  m_mutex.unlock();
  return killme;
}

//=============================================================================
int StreamTransferManager::get_uid() const
{
  return m_uid;
}


};
