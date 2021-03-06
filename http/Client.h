/* SconeServer (http://www.sconemad.com)

HTTP Client

Copyright (c) 2000-2014 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpClient_h
#define httpClient_h

#include <http/HTTPModule.h>
#include <http/Request.h>
#include <http/Response.h>

#include <sconex/ScriptBase.h>
#include <sconex/LineBuffer.h>
#include <sconex/Uri.h>
#include <sconex/Mutex.h>
namespace http {

class ClientStream;

//=============================================================================
class HTTP_API Client : public scx::ScriptObject {
public:

  Client(HTTPModule* module,
         const std::string& method,
         const scx::Uri& url);
  Client(const Client& c);
  virtual ~Client();

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

  typedef scx::ScriptRefTo<Client> Ref;

  // Client methods
  bool run(const std::string& request_data = "");

  void set_header(const std::string& name, const std::string& value);

  const Response& get_response() const;
  const std::string& get_response_data() const;

  void event_complete(bool error);

private:

  HTTPModule::Ref m_module;

  Request::Ref m_request;
  Response::Ref m_response;
  std::string m_response_data;

  scx::Mutex m_mutex;
  scx::ConditionEvent m_complete;
  bool m_error;
};

//=============================================================================
class HTTP_API ClientStream : public scx::LineBuffer {
public:

  enum Sequence {
    Send,
    ReceiveResponse,
    ReceiveHeaders,
    ReceiveBody,
    End
  };

  ClientStream(HTTPModule* module,
	       Client* client,
	       Request& request,
	       const std::string& request_data,
	       Response& response,
	       std::string& response_data);
  
  virtual ~ClientStream();

  virtual scx::Condition event(scx::Stream::Event e);

  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);

  virtual std::string stream_status() const;
  
private:

  void build_request(const std::string& method, const scx::Uri& url);
  scx::Condition send_request();
  scx::Condition receive_response();
  scx::Condition receive_headers();
  scx::Condition receive_body();
  scx::Condition receive_body_chunked();
  scx::Condition receive_data();
  void signal_complete(bool error);
  
  HTTPModule::Ref m_module;
  Client* m_client;

  Request& m_request;
  const std::string& m_request_data;

  Response& m_response;
  std::string& m_response_data;

  bool m_chunked;
  int m_remaining;

  Sequence m_seq;
  scx::Buffer* m_buffer;
};

//=============================================================================
class HTTP_API ProxyConnectStream : public scx::LineBuffer {
public:

  enum Sequence {
    Send,
    ReceiveResponse,
    ReceiveHeaders,
    Established
  };

  ProxyConnectStream(HTTPModule* module,
		     Request& request);
  
  virtual ~ProxyConnectStream();

  virtual scx::Condition event(scx::Stream::Event e);

  virtual std::string stream_status() const;
  
private:

  HTTPModule::Ref m_module;

  Request& m_request;
  Response m_response;

  Sequence m_seq;
};

};
#endif
