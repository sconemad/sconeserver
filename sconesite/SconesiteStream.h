/* SconeServer (http://www.sconemad.com)

Sconesite Stream

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

#ifndef sconesiteStream_h
#define sconesiteStream_h

#include "RenderMarkup.h"
#include <sconex/Stream.h>
#include <http/ResponseStream.h>

class Profile;
class Article;

//=========================================================================
class SconesiteStream : public http::ResponseStream {

public:

  SconesiteStream(
    SconesiteModule& module,
    Profile& profile
  );
  
  ~SconesiteStream();

  virtual std::string stream_status() const;
  
  void log(const std::string message,
	   scx::Logger::Level level = scx::Logger::Info);

protected:

  virtual scx::Condition event(scx::Stream::Event e);
  virtual scx::Condition start_section(const scx::MimeHeaderTable& headers);
  virtual scx::Condition send_response();

private:
  
  SconesiteModule& m_module;

  Profile& m_profile;
  Article* m_article;

  bool m_accept;
  RenderMarkupContext::Ref* m_context;
  std::string m_file;

  long int m_start_time;
};

#endif
