/* SconeServer (http://www.sconemad.com)

Trivial File Transfer Protocol (TFTP) Stream

Copyright (c) 2000-2007 Andrew Wedgbury <wedge@sconemad.com>

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

#include "TFTPStream.h"
#include "TFTPModule.h"
#include "TFTPProfile.h"

#include "sconex/DatagramSocket.h"
#include "sconex/DatagramChannel.h"
#include "sconex/Kernel.h"

// Uncomment to enable debug info
//#define TFTPStream_DEBUG_LOG(m) DEBUG_LOG(m)

#ifndef TFTPStream_DEBUG_LOG
#  define TFTPStream_DEBUG_LOG(m)
#endif

enum TFTPOpCode {
  RRQ = 1,
  WRQ,
  DATA,
  ACK,
  ERROR
};

#define TFTP_PACKET_SIZE 516

//=============================================================================
TFTPStream::TFTPStream(
  TFTPModule& mod,
  const std::string& profile
) : Stream("tftp"),
    m_mod(mod),
    m_profile(profile),
    m_state(tftp_Request),
    m_file(0),
    m_block(0),
    m_finished(false)
{
  enable_event(scx::Stream::Readable,true);
}

//=============================================================================
TFTPStream::~TFTPStream()
{
  delete m_file;
}

//=============================================================================
scx::Condition TFTPStream::event(scx::Stream::Event e)
{
  endpoint().reset_timeout();

  if (e == scx::Stream::Readable) {

    char packet[TFTP_PACKET_SIZE];
    int na = 0;
    read(packet,TFTP_PACKET_SIZE,na);
    
    unsigned short* p_op = (unsigned short*)&packet[0];

    TFTPOpCode op = (TFTPOpCode)ntohs(*p_op);

    if (m_state == tftp_Request) {

      if (op != RRQ && op != WRQ) {
	return scx::Error;
      }

      int start = 2;
      int end;
      for (end=start; packet[end] != 0; ++end) {
	if (end >= TFTP_PACKET_SIZE) return scx::Error;
      }
      std::string file(&packet[start],end-start);
      scx::FilePath path = "/tmp";
      path += file;
      std::cerr << "FILE: '" << path.path() << "'\n";
      
      start = end+1;
      for (end=start; packet[end] != 0; ++end) {
	if (end >= TFTP_PACKET_SIZE) return scx::Error;;
      }
      std::string mode(&packet[start],end-start);
      std::cerr << "MODE: '" << mode << "'\n";
      
      if (op == RRQ) {
	m_state = tftp_ReadData;
	std::cerr << "TFTP: Starting RRQ\n";

	open_file(path,scx::File::Read);

	m_finished = false;
	m_block = 1;
	write_data(m_block++);

      } else if (op == WRQ) {
	std::cerr << "TFTP: Starting WRQ\n";
	m_state = tftp_WriteData;

	open_file(path,
		  scx::File::Write | scx::File::Create | scx::File::Truncate);

	m_finished = false;
	m_block = 0;
       	write_ack(m_block++);
      }

    } else if (m_state == tftp_ReadData) {

      if (op != ACK) {
	std::cerr << "TFTP:   EXPECTED ACK packet\n";
	return scx::Error;
      }

      if (m_finished) {
	m_state = tftp_Request;
      }

      write_data(m_block++);

    } else if (m_state == tftp_WriteData) {
      
      if (op != DATA) {
	std::cerr << "TFTP:   EXPECTED DATA packet\n";
	return scx::Error;
      }

      unsigned short* p_block = (unsigned short*)&packet[2];
      unsigned short block = ntohs(*p_block);
      std::cerr << "TFTP:   WriteData block=" << block << "\n";
      if (block == m_block) {

	char* p_data = &packet[4];
	int data_size = na - 4;

	int nfa = 0;
	m_file->write(p_data,data_size,nfa);
	std::cerr << "TFTP:   WROTE " << nfa << " of " << data_size << "\n";

       	write_ack(m_block++);

	if (na < TFTP_PACKET_SIZE) {
	  // This must be the final packet, reset state
	  m_state = tftp_Request;
	}

      } else {
	std::cerr << "TFTP:   EXPECTED block=" << m_block << "\n";
      }
    }
    
  }  
    
  std::cerr << "\n";
  return scx::Ok;
}

//=============================================================================
std::string TFTPStream::stream_status() const
{
  std::ostringstream oss;
  oss << m_profile;
  return oss.str();
}

//=============================================================================
bool TFTPStream::open_file(const scx::FilePath& path,int mode)
{
  delete m_file;
  m_file = new scx::File();

  if (m_file->open(path,mode) != scx::Ok) {
    delete m_file;
    m_file = 0;
    return false;
  } 

  return true;
}

//=============================================================================
bool TFTPStream::write_ack(unsigned short block)
{
  const int packet_size = 4;
  char packet[packet_size];

  unsigned short* p_op = (unsigned short*)&packet[0];
  *p_op = htons(ACK);

  unsigned short* p_block = (unsigned short*)&packet[2];
  *p_block = htons(block);

  int na = 0;

  write(packet,packet_size,na);
  std::cerr << "TFTP: SENT ACK ("
	    << " block:" <<  block 
	    << " na:" << na
	    << ")\n";

  return (na == packet_size);
}

//=============================================================================
bool TFTPStream::write_data(unsigned short block)
{
  const int packet_size = TFTP_PACKET_SIZE;
  char packet[packet_size];

  unsigned short* p_op = (unsigned short*)&packet[0];
  *p_op = htons(DATA);

  unsigned short* p_block = (unsigned short*)&packet[2];
  *p_block = htons(block);

  char* p_data = &packet[4];
  int data_size = packet_size - 4;

  int na = 0;
  m_file->read(p_data,data_size,na);

  if (na < data_size) {
    m_finished = true;
  }

  write(packet,packet_size,na);
  std::cerr << "TFTP: SENT DATA ("
	    << " block:" <<  block 
	    << " na:" << na
	    << ")\n";

  return !m_finished;
}

