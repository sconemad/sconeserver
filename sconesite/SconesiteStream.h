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

#include <sconesite/RenderMarkup.h>
#include <sconesite/Article.h>
#include <http/ResponseStream.h>
#include <sconex/Stream.h>
namespace scs {
  
class Profile;

//=========================================================================
class SconesiteHandler : public http::Handler {
public:
  SconesiteHandler(SconesiteModule* module, const scx::ScriptRef* args);
  virtual ~SconesiteHandler();
  
  virtual scx::Condition handle_message(http::MessageStream* message);

  void log(http::MessageStream* message, const std::string str);
  
private:
 
  scx::ScriptRefTo<SconesiteModule> m_module;
  Profile* m_profile;
  Article::Ref* m_article;
};

  
//=========================================================================
class SconesiteStream : public http::ResponseStream {
public:

  SconesiteStream(SconesiteModule* module,
                  Profile* profile,
                  Article* article,
                  http::MessageStream* message);
  
  virtual ~SconesiteStream();

  virtual std::string stream_status() const;
  
  void log(const std::string str);

protected:

  virtual scx::Condition event(scx::Stream::Event e);
  virtual bool handle_section(const scx::MimeHeaderTable& headers,
                              const std::string& name);
  virtual bool handle_file(const scx::MimeHeaderTable& headers,
                           const std::string& name,
                           const std::string& filename);
  virtual scx::Condition send_response();

private:
  
  scx::ScriptRefTo<SconesiteModule> m_module;
  
  Profile* m_profile;
  http::MessageStream* m_message;
  Article::Ref* m_article;

  RenderMarkupContext::Ref* m_context;
  std::string m_file;

  long int m_start_time;
};

};
#endif
