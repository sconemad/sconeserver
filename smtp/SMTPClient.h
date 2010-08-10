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
#include "sconex/MimeHeader.h"
namespace smtp {

//=============================================================================
class MessageHeader {

public:

  MessageHeader();
  
  std::string get_string() const;
  void get_all_rcpts(std::vector<std::string>& rcpts) const;

  std::string m_id;
  std::string m_from;
  std::vector<std::string> m_to_rcpts;
  std::vector<std::string> m_cc_rcpts;
  std::vector<std::string> m_bcc_rcpts;
  std::string m_subject;
  scx::MimeHeaderTable m_extra_headers;
};
  
//=============================================================================
class Client : public scx::Arg {

public:

  enum Result {
    Unknown,
    Success,
    Failure
  };
  
  Client(SMTPModule& module,const scx::ArgList* args);
  Client(const Client& c);
  Client(RefType ref, Client& c);
  virtual ~Client();
  virtual scx::Arg* new_copy() const;
  virtual scx::Arg* ref_copy(RefType ref);

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::Arg* op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right);

  scx::Arg* send(const std::string& message);

  void event_complete(Result result,const std::string& result_str);

private:

  SMTPModule& m_module;

  MessageHeader* m_header;
  
  scx::Mutex* m_mutex;
  scx::ConditionEvent* m_complete;

  Result* m_result;
  std::string* m_result_str;
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
    Client* client
  );
  
  virtual ~ClientStream();

  bool set_message(const MessageHeader& header,
                   const std::string& message);
  
  virtual scx::Condition event(scx::Stream::Event e);

  virtual std::string stream_status() const;

protected:

  void setup_state();
  scx::Condition read_smtp_resp(int& code,std::string& line);
  std::string format_email_address(const std::string& str);

private:

  SMTPModule& m_module;
  Client* m_client;
  Client::Result m_result;
  std::string m_result_str;
  
  std::string m_from;
  std::vector<std::string> m_rcpts;
  
  Sequence m_seq;
  unsigned int m_rcpt_seq;
  scx::Buffer* m_buffer;
};

};
#endif
