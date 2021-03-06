/* SconeServer (http://www.sconemad.com)

Trivial File Transfer Protocol (TFTP) Stream

Copyright (c) 2000-2011 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef tftpStream_h
#define tftpStream_h

#include <sconex/Stream.h>
#include <sconex/File.h>

class TFTPModule;

//=============================================================================
class TFTPStream : public scx::Stream {
public:

  TFTPStream(TFTPModule* module,
	     const std::string& profile);

  virtual ~TFTPStream();

  virtual scx::Condition event(scx::Stream::Event e);

  virtual std::string stream_status() const;
  
private:

  bool open_file(const scx::FilePath& path,int mode);
  bool write_ack(unsigned short block);
  bool write_data(unsigned short block);
  bool write_error(unsigned short code, const std::string& message);

  void log(const std::string& message);
 
  scx::ScriptRefTo<TFTPModule> m_module;
  std::string m_profile;

  enum TFTPState {
    tftp_Request,
    tftp_ReadData,
    tftp_WriteData
  };

  TFTPState m_state;
  scx::FilePath m_filename;
  scx::File* m_file;
  unsigned short m_block;
  bool m_finished;
  
};

#endif
