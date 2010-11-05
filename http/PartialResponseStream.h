/* SconeServer (http://www.sconemad.com)

HTTP Partial Response Stream

Manages the sending of an individual message within a HTTP connection,
prepending headers where appropriate.

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

#ifndef httpPartialResponseStream_h
#define httpPartialResponseStream_h

#include "http/HTTPModule.h"
#include "http/Status.h"
#include "http/Response.h"
#include "sconex/Stream.h"
#include "sconex/VersionTag.h"
#include "sconex/FilePath.h"
#include "sconex/MimeHeader.h"
namespace scx { class Buffer; };

namespace http {

class ConnectionStream;
class Request;
class FSNode;
class FSDirectory;
class DocRoot;
class Host;

//=============================================================================
class HTTP_API PartialResponseStream : public scx::Stream {
public:

  PartialResponseStream(
    HTTPModule& module
  );
  
  virtual ~PartialResponseStream();

  virtual scx::Condition write(const void* buffer,int n,int& na);

  virtual std::string stream_status() const;
 
private:

  HTTPModule& m_module;

  int m_range_start;
  int m_range_length;
  int m_position;
  int m_content_length;
  bool m_error;
};

};
#endif
