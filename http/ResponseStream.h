/* SconeServer (http://www.sconemad.com)

HTTP Response stream

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpResponseStream_h
#define httpResponseStream_h

#include <sconex/Stream.h>
#include <sconex/LineBuffer.h>
#include <sconex/Buffer.h>
#include <sconex/MimeHeader.h>
#include <http/http.h>
#include <http/MessageStream.h>

namespace http {

class MimeHeaderStream;
  
//=========================================================================
class HTTP_API ResponseStream : public scx::Stream {

public:

  ResponseStream(const std::string& stream_name);
  
  ~ResponseStream();

  virtual std::string stream_status() const;

protected:

  // from scx::Stream:
  virtual scx::Condition event(scx::Stream::Event e);
  virtual scx::Condition read(void* buffer,int n,int& na);
  virtual bool has_readable() const;

  // This is called when the request consists of a single-part message
  // body. By default we add a stream to read this, setting any
  // parameters on the request object.
  virtual bool handle_body();

  // This is called at the start of each (non-file) section within a
  // multipart message body. By default we read the content of the section
  // and add it to the named parameter in the request object.
  // Override and return true to handle the section, or false to discard
  // the data.
  virtual bool handle_section(const scx::MimeHeaderTable& headers,
                              const std::string& name);

  // This is called at the start of each file section within a multipart
  // message body.
  // Override and return true to handle the file section, or false to
  // discard the data (default implementation returns false).
  virtual bool handle_file(const scx::MimeHeaderTable& headers,
                           const std::string& name,
                           const std::string& filename);

  // This is called when ready to send a response.
  virtual scx::Condition send_response();

  bool send_file(const scx::FilePath& path);
  
private:

  enum ResponseSequence {
    resp_Start,
    resp_ReadSingle,
    resp_ReadMultiStart,
    resp_ReadMultiBoundary,
    resp_ReadMultiHeader,
    resp_ReadMultiBody,
    resp_ReadEnd,
    resp_Write,
    resp_WriteWait,
    resp_End
  };

  enum MimeBoundaryType {
    bound_Initial,
    bound_Intermediate,
    bound_Final
  };
  
  bool find_mime_boundary();
  
  friend class MimeHeaderStream;

  void mimeheader_line(const std::string& line);
  void mimeheader_end();

  ResponseSequence m_resp_seq;

  scx::Buffer m_buffer;
  std::string m_mime_boundary;
  int m_mime_boundary_pos;
  int m_mime_boundary_len;
  MimeBoundaryType m_mime_boundary_type;
  int m_mime_boundary_num;

  scx::MimeHeaderTable m_section_headers;
  
  scx::Condition m_source_cond;
};

};
#endif
