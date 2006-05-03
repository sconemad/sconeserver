/* SconeServer (http://www.sconemad.com)

I/O Multiplexer and event dispatcher

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

#include "sconex/Multiplexer.h"
#include "sconex/Descriptor.h"
#include "sconex/Stream.h"
namespace scx {

//=============================================================================
Multiplexer::Multiplexer()
{
  DEBUG_COUNT_CONSTRUCTOR(Multiplexer);
}

//=============================================================================
Multiplexer::~Multiplexer()
{
  DEBUG_COUNT_DESTRUCTOR(Multiplexer);
  
  std::list<Descriptor*>::iterator it = m_descriptors.begin();
  while (it != m_descriptors.end()) {
    delete (*it);
    it++;
  }
}

//=============================================================================
void Multiplexer::add(Descriptor* d)
{
  m_descriptors.push_back(d);
/*
  fd_set fds_read; FD_ZERO(&fds_read);
  fd_set fds_write; FD_ZERO(&fds_write);
  fd_set fds_except; FD_ZERO(&fds_except);
  d->dispatch(&fds_read, &fds_write, &fds_except);
*/
}

//=============================================================================
int Multiplexer::spin()
{
  fd_set fds_read; FD_ZERO(&fds_read);
  fd_set fds_write; FD_ZERO(&fds_write);
  fd_set fds_except; FD_ZERO(&fds_except);

  int num_added=0;
  int maxfd=0;

  std::list<Descriptor*>::iterator it = m_descriptors.begin();
  while (it != m_descriptors.end()) {
    num_added += (*it)->setup_select(&maxfd,&fds_read,&fds_write,&fds_except);
    ++it;
  }

  if (num_added==0) {
    return -1;
  }

  // Using a timeval of 0,0 causes select to go non-blocking
  timeval time;
  time.tv_usec = 0; 
  time.tv_sec = 1;

  // Select
  int num = select(maxfd+1, &fds_read, &fds_write, &fds_except, &time);
  
  if (num < 0) {
    DEBUG_LOG("select failed, errno=" << errno);
    return 1;
  }

  // Dispatch socket events
  it = m_descriptors.begin();
  while (it != m_descriptors.end()) {
    Descriptor* d = *it;
    
    if (d->dispatch(&fds_read, &fds_write, &fds_except)) {
      // Close and destroy the socket
      d->close();
      std::list<Descriptor*>::iterator it_erase = it;
      ++it;
      m_descriptors.erase(it_erase);
      delete d;
    
    } else {
      ++it;
    }
    
  }

  return num;
}

//=============================================================================
std::string Multiplexer::describe() const
{
  std::ostringstream oss;
  std::list<Descriptor*>::const_iterator it = m_descriptors.begin();
  while (it != m_descriptors.end()) {
    Descriptor* d = *it;
    oss << "[" << d->uid() << "] " << d->describe() << "\n";

    std::list<Stream*>::const_iterator its = d->m_streams.begin();
    while (its != d->m_streams.end()) {
      oss << "      " << (*its)->stream_name() << "\n";
      its++;
    }
    
    it++;
  }
  return oss.str();
}

};
