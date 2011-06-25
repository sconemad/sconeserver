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

#include "smtp/SMTPClient.h"

#include <sconex/Logger.h>
#include <sconex/VersionTag.h>
#include <sconex/utils.h>
#include <sconex/Kernel.h>
#include <sconex/Date.h>
#include <sconex/StreamSocket.h>
#include <sconex/StreamDebugger.h>
#include <sconex/ScriptTypes.h>
namespace smtp {

//=============================================================================
MessageHeader::MessageHeader()
{
  // Generate a globally unique id for this message
  timeval tv;
  gettimeofday(&tv,0);
  scx::Date now(tv.tv_sec);

  std::ostringstream oss;
  oss << "SconeServer." << scx::version().get_string();
  oss << "." << scx::Date::now().dcode();
  oss << "." << std::setw(6) << std::setfill('0') << tv.tv_usec;
  oss << "." << pthread_self();
  oss << "@" << scx::Kernel::get()->get_system_nodename();

  m_id = oss.str();   
}
  
//=============================================================================
std::string MessageHeader::get_string() const
{
  std::string str;

  str += "Date: " + scx::Date::now().string() + CRLF;
  str += "From: " + m_from + CRLF;
  
  std::vector<std::string>::const_iterator it;

  if (m_to_rcpts.size()) {
    str += "To: ";
    for (it=m_to_rcpts.begin(); it!=m_to_rcpts.end(); ++it) {
      if (it != m_to_rcpts.begin()) str += ", ";
      str += *it;
    }
    str += CRLF;
  }
  
  if (m_cc_rcpts.size()) {
    str += "Cc: ";
    for (it=m_cc_rcpts.begin(); it!=m_cc_rcpts.end(); ++it) {
      if (it != m_cc_rcpts.begin()) str += ", ";
      str += *it;
    }
    str += CRLF;
  }

  str += "Subject: " + m_subject + CRLF;
  str += "Message-ID: <" + m_id + ">" CRLF;
  str += "User-Agent: SconeServer/" + scx::version().get_string() + CRLF;
  str += "MIME-Version: 1.0" CRLF;
  str += "Content-Type: text/plain; charset=us-ascii" CRLF;
  
  m_extra_headers.get_all();
  return str;
}

//=============================================================================
void MessageHeader::get_all_rcpts(std::vector<std::string>& rcpts) const
{
  std::vector<std::string>::const_iterator it;
  
  for (it=m_to_rcpts.begin(); it!=m_to_rcpts.end(); ++it) {
    rcpts.push_back(*it);
  }
  
  for (it=m_cc_rcpts.begin(); it!=m_cc_rcpts.end(); ++it) {
    rcpts.push_back(*it);
  }

  for (it=m_bcc_rcpts.begin(); it!=m_bcc_rcpts.end(); ++it) {
    rcpts.push_back(*it);
  }
}
  
  
//=============================================================================
SMTPClient::SMTPClient(SMTPModule* module, 
		       const scx::ScriptRef* args)
  : m_module(module),
    m_result(Unknown)
{
  DEBUG_COUNT_CONSTRUCTOR(SMTPSMTPClient);

  const scx::ScriptString* a_subject = 
    scx::get_method_arg<scx::ScriptString>(args,0,"subject");
  if (a_subject) 
    m_header.m_subject = a_subject->get_string();

  const scx::ScriptString* a_from = 
    scx::get_method_arg<scx::ScriptString>(args,1,"from");
  if (a_from) 
    m_header.m_from = a_from->get_string();

  const scx::ScriptString* a_to_rcpt = 
    scx::get_method_arg<scx::ScriptString>(args,2,"to");
  if (a_to_rcpt) 
    m_header.m_to_rcpts.push_back(a_to_rcpt->get_string());
}

//=============================================================================
SMTPClient::SMTPClient(const SMTPClient& c)
  : ScriptObject(c),
    m_module(c.m_module),
    m_header(c.m_header),
    m_result(c.m_result),
    m_result_str(c.m_result_str)
{
  DEBUG_COUNT_CONSTRUCTOR(SMTPSMTPClient);
}

//=============================================================================
SMTPClient::~SMTPClient()
{
  DEBUG_COUNT_DESTRUCTOR(SMTPSMTPClient);
}

//=============================================================================
scx::ScriptObject* SMTPClient::new_copy() const
{
  return new SMTPClient(*this);
}

//=============================================================================
std::string SMTPClient::get_string() const
{
  return m_header.m_subject;
}
  
//=============================================================================
int SMTPClient::get_int() const
{
  return (m_result == Success);
}

//=============================================================================
scx::ScriptRef* SMTPClient::send(const std::string& message)
{
  scx::StreamSocket* sock = m_module.object()->new_server_connection();

  if (!sock) {
    // Could not create the socket
    return scx::ScriptError::new_ref("Configuration error");
  }
  
  // Set idle timeout
  sock->set_timeout(15);

  // Add debug stream
  sock->add_stream(new scx::StreamDebugger("smtp-client"));
  
  // Add client stream
  SMTPClientStream* cs = new SMTPClientStream(m_module.object(),this);
  sock->add_stream(cs);
  
  if (!cs->set_message(m_header,message)) {
    // Something is badly wrong with the message,
    // don't attempt to send it
    delete sock;
    m_result_str = "Missing sender or recipient";
    return scx::ScriptError::new_ref(m_result_str);
  }    

  m_result = SMTPClient::Unknown;
  
  // Lock mutex
  m_mutex.lock();
  
  // Give to the kernel for async processing
  if (!scx::Kernel::get()->connect(sock)) {
    m_mutex.unlock();
    delete sock;
    DEBUG_LOG("System failure");
    return scx::ScriptError::new_ref("System failure");
  }
  
  // Now wait for the completion signal
  m_complete.wait(m_mutex);
  
  // Clean up
  m_mutex.unlock();

  // Log result
  if (m_result != Success) {
    m_module.object()->log("SMTP session completed with error:" + 
			   m_result_str,
			   scx::Logger::Error);
  } else {
    m_module.object()->log("SMTP session completed succesfully: " + 
			   m_result_str);
  }
  
  return scx::ScriptInt::new_ref(m_result == Success); 
}

//=============================================================================
void SMTPClient::event_complete(Result result,
				const std::string& result_str)
{
  m_mutex.lock();
  
  m_result = result;
  m_result_str = result_str;
  m_complete.signal();
    
  m_mutex.unlock();
}

//=============================================================================
scx::ScriptRef* SMTPClient::script_op(const scx::ScriptAuth& auth,
				      const scx::ScriptRef& ref,
				      const scx::ScriptOp& op,
				      const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if (name == "send" ||
	name == "add_recipient" ||
	name == "add_cc_recipient" ||
	name == "add_bcc_recipient") {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Properties
    if (name == "result") 
      return scx::ScriptString::new_ref(m_result_str);
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* SMTPClient::script_method(const scx::ScriptAuth& auth,
					  const scx::ScriptRef& ref,
					  const std::string& name,
					  const scx::ScriptRef* args)
{
  if ("send" == name) {
    const scx::ScriptObject* a_data = 
      scx::get_method_arg<scx::ScriptObject>(args,0,"data");
    std::string data = (a_data ? a_data->get_string() : "");
    
    return send(data);
  }
  
  if ("add_recipient" == name) {
    const scx::ScriptString* a_rcpt = 
      scx::get_method_arg<scx::ScriptString>(args,0,"address");
    if (!a_rcpt) 
      return scx::ScriptError::new_ref("Must specify recipient");
    m_header.m_to_rcpts.push_back(a_rcpt->get_string());
    return 0;
  }
  
  if ("add_cc_recipient" == name) {
    const scx::ScriptString* a_rcpt = 
      scx::get_method_arg<scx::ScriptString>(args,0,"address");
    if (!a_rcpt) 
      return scx::ScriptError::new_ref("Must specify recipient");
    m_header.m_cc_rcpts.push_back(a_rcpt->get_string());
    return 0;
  }
  
  if ("add_bcc_recipient" == name) {
    const scx::ScriptString* a_rcpt = 
      scx::get_method_arg<scx::ScriptString>(args,0,"address");
    if (!a_rcpt) 
      return scx::ScriptError::new_ref("Must specify recipient");
    m_header.m_bcc_rcpts.push_back(a_rcpt->get_string());
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=============================================================================
SMTPClientStream::SMTPClientStream(SMTPModule* module,
				   SMTPClient* client)
  : scx::LineBuffer("smtp:client",1024),
    m_module(module),
    m_client(client),
    m_result(SMTPClient::Unknown),
    m_seq(Start),
    m_rcpt_seq(0),
    m_buffer(0)
{

}

//=============================================================================
SMTPClientStream::~SMTPClientStream()
{
  if (m_seq != End && m_buffer != 0) {
    // If we're being deleted during a session,
    // inform client of a connection error
    m_client->event_complete(SMTPClient::Failure,"Network error");
  }
  delete m_buffer;
}

//=============================================================================
bool SMTPClientStream::set_message(const MessageHeader& header,
				   const std::string& message)
{
  DEBUG_ASSERT(m_seq == Start,"SMTP SMTPClient already running");
  
  std::string hdr_string = header.get_string();
  std::string end_string = CRLF "." CRLF;

  // Sanitise the message string by replacing \n. with \n..
  std::string m;
  std::string::size_type start = 0;
  while (true) {
    std::string::size_type end = message.find("\n.",start);
    if (end == std::string::npos) {
      m.append(message.substr(start));
      break;
    } else {
      m.append(message.substr(start,end-start));
      m.append("\n..");
    }
    start = end + 2;
  }

  delete m_buffer;
  m_buffer = new scx::Buffer(hdr_string.length() +
                             m.length() +
                             end_string.length());

  m_buffer->push_string(hdr_string);
  m_buffer->push_string(m);
  m_buffer->push_string(end_string);

  m_from = header.m_from;
  if (m_from.empty()) {
    // No "from" address
    return false;
  }

  m_rcpts.clear();
  header.get_all_rcpts(m_rcpts);
  if (m_rcpts.size() == 0) {
    // No recipients
    return false;
  }

  return true;
}

//=============================================================================
scx::Condition SMTPClientStream::event(scx::Stream::Event e)
{
  scx::Condition c = scx::Ok;
  int na = 0;
  int code = 0;
  std::string line;

  switch (m_seq) {

    case Start:
      if (m_buffer == 0) {
        DEBUG_LOG("Message has not been setup");
        return scx::Error;
      }
      m_seq = RecvGreeting;
      break;
    
    case RecvGreeting:
      c = read_smtp_resp(code,line);
      if (c != scx::Ok) return c;
      if (code != 220) {
        m_seq = SendQuit;
        m_result = SMTPClient::Failure;
        m_result_str = line;
      } else {
        m_seq = SendHelo;
      }
      break;
      
    case SendHelo:
      write("HELO "+scx::Kernel::get()->get_system_nodename()+CRLF);
      m_seq = RecvHeloResp;
      break;
      
    case RecvHeloResp:
      c = read_smtp_resp(code,line);
      if (c != scx::Ok) return c;
      if (code != 250) {
        m_seq = SendQuit;
        m_result = SMTPClient::Failure;
        m_result_str = line;
      } else {
        m_seq = SendFrom;
      }
      break;
      
    case SendFrom:
      write("MAIL FROM:"+format_email_address(m_from)+CRLF);
      m_seq = RecvFromResp;
      break;
      
    case RecvFromResp:
      c = read_smtp_resp(code,line);
      if (c != scx::Ok) return c;
      if (code != 250) {
        m_seq = SendQuit;
        m_result = SMTPClient::Failure;
        m_result_str = line;
      } else {
        m_seq = SendRcpt;
      }
      break;
      
    case SendRcpt:
      if (m_rcpt_seq >= m_rcpts.size()) {
        DEBUG_LOG("Missing recipient");
        return scx::Error;
      }
      write("RCPT TO:"+format_email_address(m_rcpts[m_rcpt_seq])+CRLF);
      m_seq = RecvRcptResp;
      break;
      
    case RecvRcptResp:
      c = read_smtp_resp(code,line);
      if (c != scx::Ok) return c;
      if (code != 250 && code != 251) {
        m_seq = SendQuit;
        m_result = SMTPClient::Failure;
        m_result_str = line;
      } else if (++m_rcpt_seq < m_rcpts.size()) {
        // Another recipient
        m_seq = SendRcpt;
      } else {
        // Last recipient, continue to send data
        m_seq = SendData;
      }
      break;
      
    case SendData:
      write("DATA" CRLF);
      m_seq = RecvDataResp;
      break;
      
    case RecvDataResp:
      c = read_smtp_resp(code,line);
      if (c != scx::Ok) return c;
      if (code != 354) {
        m_seq = SendQuit;
        m_result = SMTPClient::Failure;
        m_result_str = line;
      } else {
        m_seq = SendBody;
      }
      break;
      
    case SendBody: 
      c = Stream::write(m_buffer->head(),m_buffer->used(),na);
      if (c != scx::Ok && c != scx::Wait) {
        return c;
      }
      m_buffer->pop(na);
      if (m_buffer->used() != 0) {
        // More data to be sent
        return scx::Ok;
      }
      m_seq = RecvBodyResp;
      break;

    case RecvBodyResp:
      c = read_smtp_resp(code,line);
      if (c != scx::Ok) return c;
      m_result_str = line;
      if (code != 250) {
        m_seq = SendQuit;
        m_result = SMTPClient::Failure;
      } else {
        m_seq = SendQuit;
        m_result = SMTPClient::Success;
      }
      break;
      
    case SendQuit:
      write("QUIT" CRLF);
      m_seq = RecvQuitResp;
      break;

    case RecvQuitResp:
      c = read_smtp_resp(code,line);
      if (c != scx::Ok) return c;
      if (code >= 400) {
        m_result = SMTPClient::Failure;
        m_result_str = line;
      }
      m_seq = End;
      m_client->event_complete(m_result,m_result_str);
      return scx::End;
      
    default:
      break;
  }

  setup_state();
  return scx::Ok;
}

//=============================================================================
std::string SMTPClientStream::stream_status() const
{
  return scx::StreamTokenizer::stream_status();
}

//=============================================================================
void SMTPClientStream::setup_state()
{
  // Enable read/write events according to state
  switch (m_seq) {

    case RecvGreeting: 
    case RecvHeloResp:
    case RecvFromResp:
    case RecvRcptResp:
    case RecvDataResp:
    case RecvBodyResp:
    case RecvQuitResp:
      enable_event(scx::Stream::Readable,true);
      enable_event(scx::Stream::Writeable,false);
      break;
    
    case SendHelo:
    case SendFrom:
    case SendRcpt:
    case SendData:
    case SendBody:
    case SendQuit:
      enable_event(scx::Stream::Readable,false);
      enable_event(scx::Stream::Writeable,true);
      break;
      
    default:
      enable_event(scx::Stream::Readable,false);
      enable_event(scx::Stream::Writeable,false);
      break;
  }
}

//=============================================================================
  scx::Condition SMTPClientStream::read_smtp_resp(int& code,std::string& line)
{
  code = 0;
  scx::Condition c = tokenize(line);
  //DEBUG_LOG("SMTP server: " << line);
  if (c == scx::Ok) {
    std::string::size_type end = line.find_first_of(" \r\n");
    std::string token = line.substr(0,end);
    code = atoi(token.c_str());
  }
  return c;
}
  
//=============================================================================
std::string SMTPClientStream::format_email_address(const std::string& str)
{
  std::string::size_type start = str.find_first_of("<");
  if (start == std::string::npos) {
    return "<"+str+">";
  } 
  std::string::size_type end = str.find_last_of(">");
  if (end == std::string::npos) {
    return "<"+str.substr(start)+">";
  }
  return str.substr(start,end-start+1);
}

};
