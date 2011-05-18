/* SconeServer (http://www.sconemad.com)

SMTP Client

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

#ifndef smtpClient_h
#define smtpClient_h

#include "smtp/SMTPModule.h"

#include <sconex/ScriptBase.h>
#include <sconex/LineBuffer.h>
#include <sconex/Mutex.h>
#include <sconex/MimeHeader.h>

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
class SMTPClient : public scx::ScriptObject {
public:

  // Result states
  enum Result {
    Unknown,
    Success, 
    Failure 
  };
  
  SMTPClient(SMTPModule* module,
	     const scx::ScriptRef* args);
  SMTPClient(const SMTPClient& c);
  virtual ~SMTPClient();

  scx::ScriptRef* send(const std::string& message);

  void event_complete(Result result,const std::string& result_str);

  // ScriptObject methods
  virtual scx::ScriptObject* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<SMTPClient> Ref;

private:

  scx::ScriptRefTo<SMTPModule> m_module;

  MessageHeader m_header;
  
  scx::Mutex m_mutex;
  scx::ConditionEvent m_complete;

  Result m_result;
  std::string m_result_str;
};


//=============================================================================
class SMTPClientStream : public scx::LineBuffer {
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

  SMTPClientStream(SMTPModule* module,
		   SMTPClient* client);
  
  virtual ~SMTPClientStream();

  bool set_message(const MessageHeader& header,
                   const std::string& message);
  
  virtual scx::Condition event(scx::Stream::Event e);

  virtual std::string stream_status() const;

protected:

  void setup_state();
  scx::Condition read_smtp_resp(int& code,std::string& line);
  std::string format_email_address(const std::string& str);

private:

  scx::ScriptRefTo<SMTPModule> m_module;
  SMTPClient* m_client;
  SMTPClient::Result m_result;
  std::string m_result_str;
  
  std::string m_from;
  std::vector<std::string> m_rcpts;
  
  Sequence m_seq;
  unsigned int m_rcpt_seq;
  scx::Buffer* m_buffer;
};

};
#endif
