/* SconeServer (http://www.sconemad.com)

Compression/decompression stream using Gzip

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxGzipStream_h
#define scxGzipStream_h

#include <sconex/sconex.h>
#include <sconex/Stream.h>
#include <sconex/Buffer.h>
#include <zlib.h>
namespace scx {

#define GzipStream_MAX_BUFFER (10*1048576)
#define GzipStream_DEFAULT_BUFFER 1024

//=============================================================================
class SCONEX_API GzipStream : public Stream {

public:

  GzipStream(
    int read_buffer_size = GzipStream_DEFAULT_BUFFER,
    int write_buffer_size = GzipStream_DEFAULT_BUFFER
  );

  virtual ~GzipStream();

  virtual Condition read(void* buffer,int n,int& na);
  virtual Condition write(const void* buffer,int n,int& na);

  virtual Condition event(Event e);

  virtual bool has_readable() const;

  virtual std::string stream_status() const;
  
protected:

  int inflate_buffer(int flush);
  int deflate_buffer(int flush);

  Buffer m_read_in;
  Buffer m_read_out;
  z_streamp m_read_zs;

  Buffer m_write_in;
  Buffer m_write_out;
  z_streamp m_write_zs;

private:

};

};
#endif
