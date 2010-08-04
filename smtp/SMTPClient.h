/* SconeServer (http://www.sconemad.com)

SMTP Client

Copyright (c) 2000-2010 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef smtpClient_h
#define smtpClient_h

#include "smtp/SMTPModule.h"

#include "sconex/Arg.h"
#include "sconex/LineBuffer.h"
#include "sconex/Mutex.h"
namespace smtp {

//=============================================================================
class Client : public scx::Arg {

public:

  Client(SMTPModule& module);
  Client(const Client& c);
  Client(RefType ref, Client& c);
  virtual ~Client();
  virtual scx::Arg* new_copy() const;
  virtual scx::Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::Arg* op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right);

  bool send(const std::string& message);

  void event_complete(bool error);

private:

  SMTPModule& m_module;

  scx::Mutex* m_mutex;
  scx::ConditionEvent* m_complete;
  bool* m_error;
};

//=============================================================================
class ClientStream : public scx::LineBuffer {
public:

  enum Sequence {
    Start,
    RecvGreeting,
    SendHelo,RecvHeloResp,
    SendFrom,RecvFromResp,
    SendRcpt,RecvRcptResp,
    SendData,RecvDataResp,
    SendBody,RecvBodyResp,
    SendQuit,RecvQuitResp,
    End
  };

  ClientStream(
    SMTPModule& module,
    Client* client,
    const std::string& from,
    const std::string& rcpt,
    const std::string& message
  );
  
  virtual ~ClientStream();

  scx::Condition next_state();

  virtual scx::Condition event(scx::Stream::Event e);

  virtual std::string stream_status() const;
  
private:

  SMTPModule& m_module;
  Client* m_client;

  const std::string& m_from;
  const std::string& m_rcpt;
  const std::string& m_message;

  Sequence m_seq;
  scx::Buffer* m_buffer;
};

};
#endif
