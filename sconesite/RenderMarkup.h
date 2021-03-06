/* SconeServer (http://www.sconemad.com)

Context for rendering XML based documents

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

#ifndef sconesiteRenderMarkup_h
#define sconesiteRenderMarkup_h

#include <sconesite/Context.h>
#include <sconesite/Article.h>
#include <http/Request.h>
#include <http/Response.h>
#include <sconex/ScriptBase.h>
#include <sconex/Stream.h>
#include <sconex/Descriptor.h>
#include <sconex/FilePath.h>
#include <sconex/FileDir.h>
#include <sconex/ScriptStatement.h>
namespace scs {

class Profile;
class SconesiteStream;

//=========================================================================
// RenderMarkupContext - Context for rendering XML based documents
// May also be used to generate other documents if you're careful!
//
class RenderMarkupContext : public Context {
public:

  RenderMarkupContext(
    Profile* profile,
    SconesiteStream& stream,
    scx::Descriptor& output,
    http::Request& request,
    http::Response& response
  );

  ~RenderMarkupContext();

  Profile* get_profile();
  const http::Request& get_request() const;

  void set_article(Article* article);
  const Article* get_article() const;

  bool in_template() const;
  
  // Context methods
  virtual bool handle_start(const std::string& name, 
			    NodeAttrs& attrs, 
			    bool empty, 
			    void* data);

  virtual bool handle_end(const std::string& name,
			  NodeAttrs& attrs,
			  void* data);

  virtual void handle_process(const std::string& name,
			      const char* data,
			      int line,
                              void* extra);

  virtual void handle_text(const char* text);

  virtual void handle_comment(const char* text);


  // ScriptObject methods

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<RenderMarkupContext> Ref;

protected:

  void log(const std::string message);
  
  Profile* m_profile;
  SconesiteStream& m_stream;
  scx::Descriptor& m_output;
  http::Request::Ref* m_request;
  http::Response::Ref* m_response;

  scx::ScriptMap m_scx_env;
  typedef std::vector<scx::ScriptStatement::Ref*> StatementGroupList;
  StatementGroupList m_old_groups;

  Article::Ref* m_article;

  bool m_auto_number;

  std::string m_section;
  bool m_inhibit;

  std::string m_base_url;
};

};
#endif
