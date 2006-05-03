/* SconeServer (http://www.sconemad.com)

HTTP Message Stream

Manages the sending of an individual message within a HTTP connection,
prepending headers where appropriate.

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpMessageStream_h
#define httpMessageStream_h

#include "http/Status.h"
#include "http/HeaderTable.h"
#include "sconex/Stream.h"
#include "sconex/VersionTag.h"
namespace scx { class Buffer; };

namespace http {

class ConnectionStream;
class Request;
class FSNode;

//=============================================================================
class HTTP_API MessageStream : public scx::Stream {
public:

  MessageStream(
    ConnectionStream& httpstream,
    Request* request
  );
  
  virtual ~MessageStream();

  virtual scx::Condition event(scx::Stream::Event e);

  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual scx::Condition write(const void* buffer,int n,int& na);

  void set_version(const scx::VersionTag& ver);
  const scx::VersionTag& get_version() const;
  
  void set_status(const Status& status);
  const Status& get_status() const;

  void set_header(
    const std::string& name,
    const std::string& value
  );
  std::string get_header(const std::string& name) const;

  const Request& get_request() const;

  void set_node(const FSNode* node);
  const FSNode* get_node() const;

private:

  bool build_header();
  scx::Condition write_header();
  
  ConnectionStream& m_httpstream;
  Request* m_request;

  scx::VersionTag m_version;
  Status m_status;
  HeaderTable m_headers;
  bool m_headers_sent;
  scx::Buffer* m_buffer;
  const FSNode* m_node;

  // Chunked
  bool m_write_chunked;
  int m_write_remaining;

  // Totals
  bool m_finished;

  int m_bytes_written;
  int m_bytes_read;

  int m_bytes_readable;
};

};
#endif
