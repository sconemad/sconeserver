/* SconeServer (http://www.sconemad.com)

HTTP Connection Stream

Builds requests by parsing the headers, invoking the appropriate module to 
handle each request.

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpConnectionStream_h
#define httpConnectionStream_h

#include <http/HTTPModule.h>
#include <sconex/LineBuffer.h>
namespace http {

class Request;

//=============================================================================
class HTTP_API ConnectionStream : public scx::LineBuffer {
public:

  enum Sequence {
    http_Request,
    http_Headers,
    http_Body
  };

  ConnectionStream(HTTPModule* module,
		   const std::string& profile);
  
  virtual ~ConnectionStream();

  virtual scx::Condition event(scx::Stream::Event e);

  virtual std::string stream_status() const;
  
  scx::Condition process_input();

  bool process_request(Request*& request);

  void set_persist(bool persist);
  bool get_persist() const;

  int get_num_connection() const;
  int get_num_request() const;
  
private:

  HTTPModule::Ref m_module;

  Request* m_request;
  // Stores the request currently being built

  std::string m_profile;
  
  Sequence m_seq;
  // Current state in sequence
  
  bool m_persist;
  // Should the connection persist following the current message
 
  int m_num_connection;
  // Connection number
  
  int m_num_request;
  // Current request number

};

};
#endif
