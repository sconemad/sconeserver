/* SconeServer (http://www.sconemad.com)

Markup renderer

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
#include "sconex/FileDir.h"
#include "sconex/ArgProc.h"
#include "sconex/MemFile.h"
#include "sconex/ArgScript.h"
#include "sconex/utils.h"

//=========================================================================
RenderMarkupContext::RenderMarkupContext(
  Profile& profile,
  scx::Descriptor* output,
  const http::Request& request,
  http::Response& response

) : m_profile(profile),
    m_output(output),
    m_request(request),
    m_response_obj(new scx::ArgObject(&response)),
    m_article(0),
    m_processing(false)
{

}

//=========================================================================
RenderMarkupContext::~RenderMarkupContext()
{
  http::Response* response = dynamic_cast<http::Response*>(m_response_obj->get_object());
  delete m_response_obj;
  if (response->get_num_refs() == 0) {
    DEBUG_LOG("--- RMC deleting response")
    delete response;
  }
  delete m_output;
}

//=========================================================================
Profile& RenderMarkupContext::get_profile()
{
  return m_profile;
}

//=========================================================================
const http::Request& RenderMarkupContext::get_request() const
{
  return m_request;
}

//=========================================================================
void RenderMarkupContext::set_article(Article* article)
{
  m_article = article;
}

//=========================================================================
const Article* RenderMarkupContext::get_article() const
{
  return m_article;
}

//=========================================================================
bool RenderMarkupContext::handle_start(const std::string& name, XMLAttrs& attrs, bool empty)
{
  bool descend = false;

  if (name == "article") {
    descend = !empty;

  } else if (name == "template") {
    descend = !empty;

  } else if (name == "if") {
    std::string condition = attrs["condition"];
    scx::ArgObject ctx(this);
    scx::ArgProc proc(scx::Auth::Untrusted,&ctx);
    scx::Arg* arg = proc.evaluate(condition);
    bool result = (arg && arg->get_int() != 0);
    delete arg;
    return result;

  } else {
    std::ostringstream oss;
    oss << "<" << name;
    for (XMLAttrs::const_iterator it = attrs.begin();
         it != attrs.end();
         ++it) {
      oss << " " << (*it).first << "=\"" << (*it).second << "\"";
    }
    
    if (!empty) {
      oss << ">";
      descend = true;
    } else {
      oss << "/>";
      descend = false;
    }
    m_output->write(oss.str());
  }
  return descend;
}

//=========================================================================
bool RenderMarkupContext::handle_end(const std::string& name, XMLAttrs& attrs)
{
  bool repeat = false;

  if (name == "if" ||
      name == "article" ||
      name == "template") {
    // Ignore

  } else {
    std::ostringstream oss;
    oss << "</" << name << ">";
    m_output->write(oss.str());
  }

  return repeat;
}

//=========================================================================
void RenderMarkupContext::handle_process(const std::string& name, const char* data)
{
  //  scx::Auth auth(scx::Auth::Untrusted);
  scx::Auth auth(scx::Auth::Trusted);
  XMLDoc* doc = get_current_doc();
  const std::type_info& ti = typeid(*doc);
  if (ti == typeid(Template)) {
    // Trust templates
    auth = scx::Auth(scx::Auth::Trusted);
  }

  if (name == "scxp") { // Sconescript evaluate and print
    scx::ArgObject ctx(this);
    scx::ArgProc proc(auth,&ctx);
    scx::Arg* arg = 0;
    try {
      arg = proc.evaluate(data);
      if (arg) {
	std::string str = arg->get_string();
	if (!str.empty()) {
	  m_output->write(str);
	}
      }
    } catch (...) {
      delete arg;
      DEBUG_LOG("EXCEPTION in RenderMarkup::handle_process(scxp)");
      throw;
    }
    delete arg;

  } else if (name == "scx") { // Sconescript
    try {
      int len = strlen(data);
      scx::MemFileBuffer fbuf(len);
      fbuf.get_buffer()->push_from(data,len);
      scx::MemFile mfile(&fbuf);
      
      scx::ArgObject* ctx = new scx::ArgObject(this);
      scx::ArgScript* script = new scx::ArgScript(auth,ctx);
      mfile.add_stream(script);
      
      while (script->event(scx::Stream::Readable) == scx::Ok);
    } catch (...) { 
      DEBUG_LOG("EXCEPTION in RenderMarkup::handle_process(scx)"); 
      throw; 
    }
  }
}

//=========================================================================
void RenderMarkupContext::handle_text(const char* text)
{
  const char* p = text;
  while (*p == '\n' || *p == '\r') ++p;
  if (*p != '\0') {
    m_output->write(p);
  }
}

//=========================================================================
void RenderMarkupContext::handle_comment(const char* text)
{
  // m_output.write(text);
}

//=========================================================================
void RenderMarkupContext::handle_error(const std::string& msg)
{
  m_output->write("<html><body>");
  m_output->write("<p class='scxerror'>ERROR: Cannot process article</p>");
  m_output->write("<pre>");
  m_output->write(msg);
  m_output->write("</pre>");
  m_output->write("</body></html>");
}

//=========================================================================
scx::Arg* RenderMarkupContext::arg_resolve(const std::string& name)
{
  scx::Arg* a = arg_lookup(name);
  if (BAD_ARG(a)) {
    delete a;
    a = m_profile.get_module().arg_resolve(name);
  }
  return a;
}

//=========================================================================
scx::Arg* RenderMarkupContext::arg_lookup(const std::string& name)
{
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
    return new_method(name);
  }

  // Sub-objects
  if ("request" == name) return new scx::ArgObject(&m_request);
  if ("response" == name) return m_response_obj->new_copy();
  if ("session" == name) {
    if (m_request.get_session()) {
      return new scx::ArgObject(m_request.get_session());
    } else {
      return new scx::ArgError("No session");
    }
  }
  if ("realms" == name) {
    scx::ModuleRef http = scx::Kernel::get()->get_module("http");
    if (http.valid()) return http.module()->arg_lookup("realms");
  }

  // Article dependent sub-objects
  if ("article" == name) {
    if (m_article) {
      return new scx::ArgObject(m_article);
    } else {
      return new scx::ArgError("No article");
    }
  }
  if ("root" == name) return new scx::ArgObject(m_profile.get_index());

  return Context::arg_lookup(name);
}

//=========================================================================
scx::Arg* RenderMarkupContext::arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (name == "print") {
    int n = l->size();
    for (int i=0; i<n; ++i) { 
      m_output->write(l->get(i)->get_string());
    }
    return 0;
  }

  if (name == "print_esc") {
    int n = l->size();
    for (int i=0; i<n; ++i) { 
      m_output->write(scx::escape_html(l->get(i)->get_string()));
    }
    return 0;
  }
  
  if (name == "print_json") {
    scx::ArgStore::store_arg(*m_output,l->get(0));
    return 0;
  }

  if (name == "escape") {
    scx::Arg* a_str = l->get(0);
    if (!a_str) {
      return new scx::ArgString("");
    }
    return new scx::ArgString(scx::escape_html(a_str->get_string()));
  }

  if (name == "process_article") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    Article* art = m_article;
    scx::Arg* a_art = l->get(0);
    if (a_art) {
      scx::ArgObject* a_obj = dynamic_cast<scx::ArgObject*>(a_art);
      if (a_obj) {
        art = dynamic_cast<Article*>(a_obj->get_object());
      }
    }
    if (!art) return new scx::ArgError("No article to process");
    
    //      if (m_processing) {
    //	m_output->write("<p class='scxerror'>ERROR: Already processing article</p>");
    //      } else {
    m_processing = true;
    art->process(*this);
    m_processing = false;
    //      }
    return 0;
  }

  if (name == "edit_article") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    if (m_article) {
      scx::File* file = new scx::File();
      if (scx::Ok == file->open(m_article->get_filepath(),scx::File::Read)) {
        char buffer[1024];
	int na = 0;
        while (scx::Ok == file->read(buffer,1000,na)) {
          std::string str(buffer,na);
          m_output->write(scx::escape_html(str));
        }
      }
      delete file;
    }
    return 0;
  }

  if (name == "template") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    const scx::ArgString* a_tpl = dynamic_cast<const scx::ArgString*>(l->get(0));
    if (!a_tpl) {
      return new scx::ArgError("Template name not specified");
    }
    Template* tpl = m_profile.lookup_template(a_tpl->get_string());
    if (!tpl) {
      return new scx::ArgError("Unknown template");
    }
    tpl->process(*this);
    return 0;
  }

  if (name == "abort") {
    if (!auth.trusted()) return new scx::ArgError("Not permitted");

    m_output->close();
    return 0;
  }

  return Context::arg_method(auth,name,args);
}



//=========================================================================
RenderMarkupJob::RenderMarkupJob(RenderMarkupContext* ctx)
  : Job("sconesite::RenderMarkup"),
    m_context(ctx)
{

}

//=========================================================================
RenderMarkupJob::~RenderMarkupJob()
{
  delete m_context;
}

//=============================================================================
bool RenderMarkupJob::should_run()
{
  return true;
}

//=========================================================================
bool RenderMarkupJob::run()
{
  //  DEBUG_LOG("Running " << type() << " " << describe());

  std::string tplname = m_context->get_request().get_param("tpl");
  if (tplname.empty()) {
    tplname = "default";
  }

  Template* tpl = m_context->get_profile().lookup_template(tplname);

  if (!tpl) {
    m_context->handle_error("No template");

  } else {
    tpl->process(*m_context);
  }

  return true;
}

//=========================================================================
std::string RenderMarkupJob::describe() const
{
  return std::string(m_context->get_article() ? m_context->get_article()->get_name() : "(no article)");
}
