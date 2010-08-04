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

#include "smtp/SMTPClient.h"

#include "sconex/Logger.h"
#include "sconex/VersionTag.h"
#include "sconex/utils.h"
#include "sconex/StreamSocket.h"
#include "sconex/Kernel.h"
#include "sconex/Date.h"
namespace smtp {

//=============================================================================
Client::Client(SMTPModule& module)
  : m_module(module),
    m_mutex(new scx::Mutex()),
    m_complete(new scx::ConditionEvent()),
    m_error(new bool(false))
{
  DEBUG_COUNT_CONSTRUCTOR(SMTPClient);
}

//=============================================================================
Client::Client(const Client& c)
  : Arg(c),
    m_module(c.m_module),
    m_mutex(new scx::Mutex()),
    m_complete(new scx::ConditionEvent()),
    m_error(new bool(*c.m_error))
{
  DEBUG_COUNT_CONSTRUCTOR(SMTPClient);
}

//=============================================================================
Client::Client(RefType ref, Client& c)
  : Arg(ref,c),
    m_module(c.m_module),
    m_mutex(c.m_mutex),
    m_complete(c.m_complete),
    m_error(c.m_error)
{
  DEBUG_COUNT_CONSTRUCTOR(SMTPClient);
  
}

//=============================================================================
Client::~Client()
{
  if (last_ref()) {
    delete m_mutex;
    delete m_complete;
    delete m_error;
  }
  DEBUG_COUNT_DESTRUCTOR(SMTPClient);
}

//=============================================================================
scx::Arg* Client::new_copy() const
{
  return new Client(*this);
}

//=============================================================================
scx::Arg* Client::ref_copy(RefType ref)
{
  return new Client(ref,*this);
}

//=============================================================================
std::string Client::get_string() const
{
  return "SMTP Client";
}
  
//=============================================================================
int Client::get_int() const
{
  return !(*m_error);
}

//=============================================================================
scx::Arg* Client::op(const scx::Auth& auth,scx::Arg::OpType optype, const std::string& opname, scx::Arg* right)
{
  if (is_method_call(optype,opname)) {
    
    scx::ArgList* l = dynamic_cast<scx::ArgList*>(right);

    if ("send" == m_method) {
      std::string data;
      const scx::Arg* adata = l->get(0);
      if (adata) data = adata->get_string();
      
      bool success = send(data);
      return new scx::ArgInt(success);
    }
    
  } else if (scx::Arg::Binary == optype) {

    if ("." == opname) {
      std::string name = right->get_string();
      
      if (name == "send") return new_method(name);
    }
    
  }
  return SCXBASE Arg::op(auth,optype,opname,right);
}

//=============================================================================
bool Client::send(const std::string& message)
{
  scx::Uri addr_url = scx::Uri("smtp://localhost");
  
  // Lookup the router.ip module and use to create an IP4 socket address
  scx::SocketAddress* addr = 0;    
  scx::ModuleRef router = scx::Kernel::get()->get_module("router");
  if (router.valid()) {
    scx::ModuleRef ip = router.module()->get_module("ip");
    if (ip.valid()) {
      scx::ArgList args;
      args.give( new scx::ArgString(addr_url.get_host()) );
      args.give( new scx::ArgInt(addr_url.get_port()) );
      scx::Arg* ret = ip.module()->arg_method(scx::Auth::Trusted,"addr",&args);
      addr = dynamic_cast<scx::SocketAddress*>(ret);
    }
  }

  if (addr == 0) {
    DEBUG_LOG("Unable to create socket address");
    return false;
  }
  
  // Create the socket  
  scx::StreamSocket* sock = new scx::StreamSocket();

  // Set idle timeout
  sock->set_timeout(15);
  
  std::string from = "wedge@sconemad.com";
  std::string rcpt = "wedge@sconemad.com";

  // Add client stream
  ClientStream* cs = new ClientStream(m_module,this,from,rcpt,message);
  sock->add_stream(cs);

  // Start the socket connection
  scx::Condition err = sock->connect(addr);
  if (err != scx::Ok && err != scx::Wait) {
    delete sock;
    delete addr;
    DEBUG_LOG("Unable to initiate connection");
    return false;
  }
  delete addr;

  // Give to the kernel for async processing
  if (!scx::Kernel::get()->connect(sock,0)) {
    delete sock;
    DEBUG_LOG("System failure");
    return false;
  }

  // Now wait for the completion signal
  m_mutex->lock();
  m_complete->wait(*m_mutex);
  m_mutex->unlock();

  return !(*m_error);
}

//=============================================================================
void Client::event_complete(bool error)
{
  m_mutex->lock();

  *m_error = error;
  m_complete->signal();

  m_mutex->unlock();
}


//=============================================================================
ClientStream::ClientStream(
  SMTPModule& module,
  Client* client,
  const std::string& from,
  const std::string& rcpt,
  const std::string& message
) : scx::LineBuffer("smtp:client",1024),
    m_module(module),
    m_client(client),
    m_from(from),
    m_rcpt(rcpt),
    m_message(message),
    m_seq(Start),
    m_buffer(0)
{
  next_state();
}

//=============================================================================
ClientStream::~ClientStream()
{
  delete m_buffer;
  if (m_seq != End) {
    m_client->event_complete(true);
  }
}

//=============================================================================
scx::Condition ClientStream::next_state()
{
  m_seq = (Sequence)((int)m_seq + 1);
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

  case End:
    enable_event(scx::Stream::Readable,false);
    enable_event(scx::Stream::Writeable,false);
    m_client->event_complete(false);
    return scx::End;
  }

  return scx::Ok;
}

//=============================================================================
scx::Condition ClientStream::event(scx::Stream::Event e)
{
  scx::Condition c = scx::Ok;
  std::string line;

  switch (m_seq) {

  case RecvGreeting:
    c = tokenize(line);
    if (c != scx::Ok) return c;
    DEBUG_LOG("SMTP greeting: " << line);
    break;

  case SendHelo:
    write("HELO localhost\r\n");
    break;

  case RecvHeloResp:
    c = tokenize(line);
    if (c != scx::Ok) return c;
    DEBUG_LOG("SMTP helo resp: " << line);
    break;

  case SendFrom:
    write("MAIL FROM:<"+m_from+">\r\n");
    break;

  case RecvFromResp:
    c = tokenize(line);
    if (c != scx::Ok) return c;
    DEBUG_LOG("SMTP from resp: " << line);
    break;

  case SendRcpt:
    write("RCPT TO:<"+m_rcpt+">\r\n");
    break;

  case RecvRcptResp:
    c = tokenize(line);
    if (c != scx::Ok) return c;
    DEBUG_LOG("SMTP rcpt resp: " << line);
    break;

  case SendData:
    write("DATA\r\n");
    break;

  case RecvDataResp:
    c = tokenize(line);
    if (c != scx::Ok) return c;
    DEBUG_LOG("SMTP data resp: " << line);
    break;

  case SendBody:
    write("Date: "+scx::Date::now().string()+"\r\n");
    write("Subject: Test message\r\n");
    write("From: "+m_from+"\r\n");
    write("To: "+m_rcpt+"\r\n");
    write("\r\n");
    write(m_message);
    write("\r\n.\r\n");
    break;

  case RecvBodyResp:
    c = tokenize(line);
    if (c != scx::Ok) return c;
    DEBUG_LOG("SMTP body resp: " << line);
    break;

  case SendQuit:
    write("QUIT\r\n");
    break;

  case RecvQuitResp:
    c = tokenize(line);
    if (c != scx::Ok) return c;
    DEBUG_LOG("SMTP quit resp: " << line);
    break;

  default:
    break;
  }

  return next_state();
}

//=============================================================================
std::string ClientStream::stream_status() const
{
  std::ostringstream oss;
  oss << scx::StreamTokenizer::stream_status()
      << " seq:";
  switch (m_seq) {
    // Cannot be bothered!
    default: oss << "UNKNOWN!"; break;
  }
  return oss.str();
}

};
