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

#include "RenderMarkup.h"
#include "Profile.h"
#include "Article.h"
#include "Template.h"
#include "SconesiteModule.h"
#include "SconesiteStream.h"

#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/File.h"
#include "sconex/FileDir.h"
#include "sconex/ScriptEngine.h"
#include "sconex/ScriptExpr.h"
#include "sconex/MemFile.h"
#include "sconex/Socket.h"
#include "sconex/StreamSocket.h"
#include "sconex/utils.h"

const char* XHTML_DOCTYPE = "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>";

const char* XHTML_NAMESPACE = "http://www.w3.org/1999/xhtml";

//=========================================================================
RenderMarkupContext::RenderMarkupContext(
  Profile& profile,
  SconesiteStream& stream,
  scx::Descriptor& output,
  http::Request& request,
  http::Response& response

) : m_profile(profile),
    m_stream(stream),
    m_output(output),
    m_request(new http::Request::Ref(&request)),
    m_response(new http::Response::Ref(&response)),
    m_article(0),
    m_auto_number(false),
    m_inhibit(false)
{
  m_parent = &m_profile; // used to be module?
}

//=========================================================================
RenderMarkupContext::~RenderMarkupContext()
{
  for (StatementGroupList::iterator it = m_old_groups.begin();
       it != m_old_groups.end();
       ++it) {
    delete *it;
  }
  delete m_article;
  delete m_request;
  delete m_response;
}

//=========================================================================
Profile& RenderMarkupContext::get_profile()
{
  return m_profile;
}
  
//=========================================================================
const http::Request& RenderMarkupContext::get_request() const
{
  return *(m_request->object());
}

//=========================================================================
void RenderMarkupContext::set_article(Article* article)
{
  delete m_article;
  m_article = new Article::Ref(article);
}

//=========================================================================
const Article* RenderMarkupContext::get_article() const
{
  if (m_article) return m_article->object();
  return 0;
}

//=========================================================================
bool RenderMarkupContext::handle_start(const std::string& name,
				       XMLAttrs& attrs,
				       bool empty,
				       void* data)
{
  bool descend = false;
  std::string pre;

  if (name == "article") {
    descend = !empty;
    // Load settings for this article
    m_auto_number = XMLAttr_bool(attrs,"autonumber");

  } else if (name == "template") {
    descend = !empty;

  } else if (name == "if") {
    std::string condition = attrs["condition"];
    scx::ScriptRef ctx(this);
    scx::ScriptExpr proc(scx::ScriptAuth::Untrusted,&ctx);
    scx::ScriptRef* ret = proc.evaluate(condition);
    bool result = (ret && ret->object()->get_int() != 0);
    delete ret;
    return result;

  } else if (name == "section") {
    if (!m_section.empty()) {
      m_inhibit = (attrs["name"] != m_section);
    } else {
      m_inhibit = !XMLAttr_bool(attrs,"default");
    }
    descend = !empty;
    
  } else {
    
    std::ostringstream oss;

    // Monkey about with some of the standard HTML tags!
    
    if (name == "a" || name == "area" || name == "img") {
      std::string link_attr = (name == "img" ? "src" : "href");
      std::string link = attrs[link_attr];
      if (link.find(":") == std::string::npos) {
        if (link[0] != '/') {
          // Expand relative hrefs into full paths so they work anywhere
          attrs[link_attr] = m_base_url + "/" + 
	    m_article->object()->get_href_path() + link;
        } else {
	  attrs[link_attr] = m_base_url + link;
	}
      } else {
        // Add class 'external' to offsite links
        std::string& c = attrs["class"];
        if (c.empty()) {
          c = "external";
        } else {
          c += " external";
        }
      }

    } else if (name == "h1" || name == "h2" || name == "h3" ||
               name == "h4" || name == "h5" || name == "h6") {
      // Automatically insert anchors before headings
      if (data && m_article) { 
        const ArticleHeading* h = (const ArticleHeading*)(data);
        if (h) {
          int index = h->index();
          std::string anchor = 
	    m_article->object()->get_headings().lookup_anchor(index);
          if (m_auto_number) {
            std::string href = "/" + m_article->object()->get_href_path() + 
	                       "#" + anchor;
            pre += "<span class='section'><a href='" +
              href + "' title='Link to this section'>" +
              m_article->object()->get_headings().lookup_section(index) +
              ".</a></span> ";
          }
          
          oss << "<a name='" << anchor << "'></a>";
        }
      }
      
    } else if (name == "html") {
      // Setup for valid XHTML output
      oss << XHTML_DOCTYPE << "\n";
      attrs["xmlns"] = XHTML_NAMESPACE;
    }

    oss << "<" << name;
    for (XMLAttrs::const_iterator it = attrs.begin();
         it != attrs.end();
         ++it) {
      oss << " " << (*it).first << "=\"" << (*it).second << "\"";
    }
    
    if (!empty) {
      oss << ">" << pre;
      descend = true;
    } else {
      oss << "/>";
      descend = false;
    }
    if (!m_inhibit) m_output.write(oss.str());
  }
  return descend;
}

//=========================================================================
bool RenderMarkupContext::handle_end(const std::string& name,
				     XMLAttrs& attrs,
				     void* data)
{
  bool repeat = false;

  if (name == "if" ||
      name == "article" ||
      name == "template") {
    // Ignore

  } else if (name == "section") {
    if (!m_section.empty()) {
      if (!m_inhibit) {
        m_inhibit = (attrs["name"] == m_section);
      }
    } else {
      if (m_inhibit) {
        m_inhibit = XMLAttr_bool(attrs,"default");
      }
    }
  
  } else {
    std::ostringstream oss;
    oss << "</" << name << ">";
    if (!m_inhibit) m_output.write(oss.str());
  }

  return repeat;
}

//=========================================================================
void RenderMarkupContext::handle_process(const std::string& name,
					 const char* data)
{
  if (m_inhibit) return;
    
  scx::ScriptAuth auth(scx::ScriptAuth::Untrusted);
  XMLDoc* doc = get_current_doc();
  const std::type_info& ti = typeid(*doc);
  if (ti == typeid(Template)) {
    // Trust templates
    auth = scx::ScriptAuth(scx::ScriptAuth::Trusted);
  }

  if (name == "scxp") { // Sconescript evaluate and print
    // Create root statement using our environment
    scx::ScriptStatementGroup::Ref root(
      new scx::ScriptStatementGroup(&m_scx_env));
    root.object()->set_parent(this);
    scx::ScriptExpr proc(auth,&root);
    scx::ScriptRef* result = 0;
    try {
      result = proc.evaluate(data);
      if (result) {
	std::string str = result->object()->get_string();
	if (!str.empty()) {
	  m_output.write(str);
	}
      }
    } catch (...) {
      delete result;
      log("EXCEPTION in RenderMarkup::handle_process(scxp)",
	  scx::Logger::Error);
      throw;
    }
    delete result;

  } else if (name == "scx") { // Sconescript
    scx::ScriptRef* ret = 0;
    try {
      int len = strlen(data);
      scx::MemFileBuffer fbuf(len);
      fbuf.get_buffer()->push_from(data,len);
      scx::MemFile mfile(&fbuf);

      // Create root statement using our environment
      scx::ScriptStatement::Ref* root = 
	new scx::ScriptStatement::Ref(
          new scx::ScriptStatementGroup(&m_scx_env));
      m_old_groups.push_back(root);
      root->object()->set_parent(this);
      
      // Create a script parser
      scx::ScriptEngine* script = new scx::ScriptEngine(root);
      mfile.add_stream(script);

      // Parse statements
      if (script->parse() != scx::End) {
        std::ostringstream oss;
        oss << "Script ";
	switch (script->get_error_type()) {
  	  case scx::ScriptEngine::Tokenization: oss << "tokenization"; break;
	  case scx::ScriptEngine::Syntax: oss << "syntax"; break;
	  case scx::ScriptEngine::Underflow: oss << "underflow"; break;
	  default: oss << "unknown"; break;
	}
        oss << " error on line " << script->get_error_line() << ":\n" << data;
        log(oss.str(),scx::Logger::Error);
      }

      // Run statements
      scx::ScriptExpr proc(auth);
      ret = root->object()->execute(proc);
      if (ret && BAD_SCRIPTREF(ret)) {
        std::ostringstream oss;
        oss << "Script execution returned " 
	    << ret->object()->get_string() << "\n" 
	    << data;
        log(oss.str(),scx::Logger::Error);
      }
      delete ret;
      ((scx::ScriptStatementGroup*)root->object())->clear();

    } catch (...) { 
      delete ret;
      log("EXCEPTION in RenderMarkup::handle_process(scx)",
	  scx::Logger::Error); 
      throw; 
    }
  }
}

//=========================================================================
void RenderMarkupContext::handle_text(const char* text)
{
  if (m_inhibit) return;

  const char* p = text;
  while (*p == '\n' || *p == '\r') ++p;
  if (*p != '\0') {
    m_output.write(p);
  }
}

//=========================================================================
void RenderMarkupContext::handle_comment(const char* text)
{
  // m_output.write(text); // Is this supposed to be funny?
}

//=========================================================================
void RenderMarkupContext::handle_error(const std::string& msg)
{
  m_output.write("<html><body>");
  m_output.write("<p class='scxerror'>ERROR: Cannot process article</p>");
  m_output.write("<pre>");
  m_output.write(msg);
  m_output.write("</pre>");
  m_output.write("</body></html>");
}

//=========================================================================
scx::ScriptRef* RenderMarkupContext::script_op(const scx::ScriptAuth& auth,
					       const scx::ScriptRef& ref,
					       const scx::ScriptOp& op,
					       const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();

    // Methods
    if ("print" == name ||
	"print_esc" == name ||
	"print_json" == name ||
	"escape" == name ||
	"get_articles" == name ||
	"process_article" == name ||
	"edit_article" == name ||
	"template" == name ||
	"get_files" == name ||
	"abort" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }

    // Sub-objects

    if ("request" == name) 
      return m_request->ref_copy(ref.reftype());

    if ("response" == name) 
      return m_response->ref_copy(ref.reftype());

    if ("session" == name)
      return new scx::ScriptRef(m_request->object()->get_session());

    if ("local_addr" == name) {
      scx::Socket* socket = dynamic_cast<scx::Socket*>(&m_output);
      if (!socket) return 0;
      return new scx::ScriptRef(socket->get_local_addr()->new_copy());
    }

    if ("remote_addr" == name) {
      //NOTE: At the moment this is only ever likely to be a StreamSocket,
      // but this may change in the future, who knows?
      scx::StreamSocket* socket = dynamic_cast<scx::StreamSocket*>(&m_output);
      if (!socket) return 0;
      return new scx::ScriptRef(socket->get_remote_addr()->new_copy());
    }

    if ("realms" == name) { // for convenience access to http.realms
      scx::Module::Ref http = scx::Kernel::get()->get_module("http");
      DEBUG_ASSERT(http.valid(),"http module should be loaded");
      scx::ScriptList::Ref largs(new scx::ScriptList());
      largs.object()->give(scx::ScriptString::new_ref("realms"));
      scx::ScriptRef* realms = 
	http.script_op(auth,scx::ScriptOp::Lookup,&largs);
      return realms;
    }
    
    if ("article" == name) {
      if (!m_article)
	return scx::ScriptError::new_ref("No article");
      return m_article->ref_copy(ref.reftype());
    }

    if ("profile" == name) 
      return new scx::ScriptRef(&m_profile);
  }

  return scx::ScriptObject::script_op(auth,ref,op,right);
}

//=========================================================================
scx::ScriptRef* RenderMarkupContext::script_method(const scx::ScriptAuth& auth,
						   const scx::ScriptRef& ref,
						   const std::string& name,
						   const scx::ScriptRef* args)
{
  const scx::ScriptList* argsl = 
    dynamic_cast<const scx::ScriptList*>(args->object());

  if (name == "print") {
    int n = argsl->size();
    for (int i=0; i<n; ++i) {
      std::string str = argsl->get(i)->object()->get_string();
      if (str.size() > 0) {
        m_output.write(str);
      }
    }
    return 0;
  }

  if (name == "print_esc") {
    int n = argsl->size();
    for (int i=0; i<n; ++i) {
      std::string str = argsl->get(i)->object()->get_string();
      if (str.size() > 0) {
        m_output.write(scx::escape_html(str));
      }
    }
    return 0;
  }
  
  if (name == "print_json") {
    const scx::ScriptObject* value = 
      scx::get_method_arg<scx::ScriptObject>(args,0,"value");
    if (value) value->serialize(m_output);
    return 0;
  }

  if (name == "escape") {
    const scx::ScriptString* value = 
      scx::get_method_arg<scx::ScriptString>(args,0,"value");
    if (!value)
      return scx::ScriptString::new_ref(scx::escape_html(value->get_string()));
    return scx::ScriptString::new_ref("");
  }

  if (name == "process_article") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    Article::Ref* art = 0;
    const Article* a_art = scx::get_method_arg<Article>(args,0,"article");
    if (a_art) {
      // Use the specififed article
      art = new Article::Ref( const_cast<Article*>(a_art) );
    } else if (m_article) {
      // Otherwise use the current article
      art = m_article->ref_copy(ref.reftype());
    } else {
      return scx::ScriptError::new_ref("No article to process");
    }

    // Save current state and setup to process new article
    Article::Ref* orig_art = m_article; 
    std::string orig_section = m_section; 
    bool orig_inhibit = m_inhibit; 
    bool orig_auto_number = m_auto_number;
    std::string orig_base_url = m_base_url; 

    // Setup defaults for processing new article, reading any options
    m_article = art;
    m_section = "";
    m_base_url = "";
    const scx::ScriptMap* opts = 
      scx::get_method_arg<scx::ScriptMap>(args,1,"opts");
    if (opts) {
      const scx::ScriptRef* opt = 0;
      if ((opt = opts->lookup("section"))) 
	m_section = opt->object()->get_string();
      if ((opt = opts->lookup("base_url"))) 
	m_base_url = opt->object()->get_string();
    }
    m_inhibit = (!m_section.empty());

    // Process the article
    art->object()->process(*this);

    // Restore previous state
    m_article = orig_art;
    m_section = orig_section;
    m_inhibit = orig_inhibit;
    m_auto_number = orig_auto_number;
    m_base_url = orig_base_url;

    delete art;
    return 0;
  }

  if (name == "edit_article") {
    // This sends the source of the article, for editing
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    if (m_article) {
      scx::File* file = new scx::File();
      if (scx::Ok == file->open(m_article->object()->get_filepath(),
				scx::File::Read)) {
        char buffer[1024];
	int na = 0;
        while (scx::Ok == file->read(buffer,1000,na)) {
          std::string str(buffer,na);
          m_output.write(scx::escape_html(str));
        }
      }
      delete file;
    }
    return 0;
  }

  if (name == "template") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    const scx::ScriptString* a_tpl = 
      scx::get_method_arg<scx::ScriptString>(args,0,"name");
    if (!a_tpl) {
      return scx::ScriptError::new_ref("Template name not specified");
    }
    Template* tpl = m_profile.lookup_template(a_tpl->get_string());
    if (!tpl) {
      return scx::ScriptError::new_ref("Unknown template");
    }
    tpl->process(*this);
    return 0;
  }

  if (name == "abort") {
    if (!auth.trusted()) return scx::ScriptError::new_ref("Not permitted");

    //    m_output.close();
    throw std::exception();
    return 0;
  }

  return scx::ScriptObject::script_method(auth,ref,name,args);
}

//=========================================================================
void RenderMarkupContext::log(const std::string message,
			      scx::Logger::Level level)
{
  m_stream.log(message,level);
}
