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

#include <sconesite/SconesiteModule.h>
#include <sconesite/SconesiteStream.h>
#include <sconesite/Article.h>
#include <sconesite/Profile.h>
#include <sconesite/Template.h>
#include <sconesite/RenderMarkup.h>
#include <http/HTTPModule.h>
#include <http/Request.h>
#include <http/Session.h>
#include <http/MessageStream.h>
#include <http/Status.h>
#include <sconex/Stream.h>
#include <sconex/Date.h>
#include <sconex/StreamTransfer.h>
#include <sconex/StreamSocket.h>
#include <sconex/Kernel.h>
#include <sconex/StreamDebugger.h>
#include <sconex/FilePath.h>
#include <sconex/Log.h>
namespace scs {

//=========================================================================
SconesiteHandler::SconesiteHandler(SconesiteModule* module,
                                   const scx::ScriptRef* args)
  : m_module(module),
    m_profile(0),
    m_article(0)
{
  const scx::ScriptString* profile_name =
    scx::get_method_arg<scx::ScriptString>(args,0,"profile");
  if (profile_name) {
    m_profile = module->lookup_profile(profile_name->get_string());
  }
}

//=========================================================================
SconesiteHandler::~SconesiteHandler()
{
  delete m_article;
}
  
//=========================================================================
scx::Condition SconesiteHandler::handle_message(http::MessageStream* message)
{
  http::Request& req = const_cast<http::Request&>(message->get_request());
  http::Response& resp = message->get_response();

  if (!m_profile) {
    log(message, "No profile found");
    resp.set_status(http::Status::NotFound);
    return scx::Close;
  }

  std::string pathinfo = req.get_path_info();
  if (pathinfo.find("//") != std::string::npos ||
      pathinfo.find("..") != std::string::npos) {
    log(message, "Request for '" + pathinfo +
        "' - Forbidden (path contains forbidden chars)");
    resp.set_status(http::Status::Forbidden);
    return scx::Close;
  }
  
  // Lookup the article and get remaining file path
  std::string file;
  m_article = m_profile->lookup_article(pathinfo, file);
  
  // Check article exists
  if (!m_article) {
    log(message, "Request for '" + pathinfo + "' - NotFound (no article)");
    resp.set_status(http::Status::NotFound);
    return scx::Close;
  }

  const scx::Uri& uri = req.get_uri();
  if (file.empty()) {
    // Article request, check if we need to redirect to correct the path
    std::string href = m_article->object()->get_href_path();
    if (pathinfo != href) {
      scx::Uri new_uri = uri;
      new_uri.set_path(href);
      log(message, "Redirect '"+uri.get_string()+"' to '"+new_uri.get_string()+"'"); 
      resp.set_header("Location",new_uri.get_string());
      resp.set_status(http::Status::Found);
      return scx::Close;
    }
    
  } else {
    // File request, update the path in the request
    scx::FilePath path = m_article->object()->get_root() + file;
    req.set_path(path);
    if (file.find("article.") == 0) {
      // Don't allow any article source to be sent
      log(message, "Request for '" + pathinfo + "' - Forbidden (article source)");
      resp.set_status(http::Status::Forbidden);
      return scx::Close;
      
    } else if (!scx::FileStat(path).is_file()) {
      log(message, "Request for '" + pathinfo + "' - NotFound");
      resp.set_status(http::Status::NotFound);
      // Note that we don't return here, since we want to go ahead
      // and process the article in order to display the 404.
      // XXX does this make sense?
      
    } else {
      log(message, "Request for '" + pathinfo +"' - Using getfile");

      // Pass on to the getfile handler
      http::Handler* getfile = http::Handler::create("getfile",0);
      if (!getfile) {
        // getfile not available
        resp.set_status(http::Status::InternalServerError);
        return scx::Close;
      }
      scx::Condition c = getfile->handle_message(message);
      delete getfile;
      return c;
    }
  }

  message->add_stream(new SconesiteStream(m_module.object(),
                                          m_profile,
                                          m_article->object(),
                                          message));
                                          
  return scx::Ok;
}

//=========================================================================
void SconesiteHandler::log(http::MessageStream* message, const std::string str)
{
  scx::Log l("sconesite");
  const http::Request& req = message->get_request();
  l.attach("id",req.get_id());
  if (m_article) l.attach("article",m_article->object()->get_href_path());
  l.submit(str);
}

//=========================================================================
SconesiteStream::SconesiteStream(SconesiteModule* module,
				 Profile* profile,
                                 Article* article,
                                 http::MessageStream* message)
  : http::ResponseStream("sconesite"),
    m_module(module),
    m_profile(profile),
    m_message(message),
    m_article(new Article::Ref(article)),
    m_context(0)
{

}

//=========================================================================
SconesiteStream::~SconesiteStream()
{
  delete m_context;
  delete m_article;
}

//=========================================================================
std::string SconesiteStream::stream_status() const
{
  std::ostringstream oss;
  oss << http::ResponseStream::stream_status()
      << " prf:" << m_profile->get_string()
      << " art:" 
      << (m_article ? m_article->object()->get_href_path() : "NULL");
  return oss.str();
}

//=========================================================================
void SconesiteStream::log(const std::string str)
{
  scx::Log l("sconesite-stream");
  const http::Request& req = m_message->get_request();
  l.attach("id",req.get_id());
  l.submit(str);
}

//=========================================================================
scx::Condition SconesiteStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Opening) {
    http::Request& req = const_cast<http::Request&>(m_message->get_request());
    http::Response& resp = m_message->get_response();
    // Create context for rendering article
    RenderMarkupContext* rmc = new RenderMarkupContext(m_profile,
						       *this,
						       endpoint(),
						       req,
						       resp);
    m_context = new RenderMarkupContext::Ref(rmc);
    rmc->set_article(m_article->object());
  }
  return http::ResponseStream::event(e);
}

//=========================================================================
bool SconesiteStream::handle_section(const scx::MimeHeaderTable& headers,
                                     const std::string& name)
{
  // If the section name starts with "file_", treat as a file 
  const std::string file_pattern = "file_";
  if (0 == name.find(file_pattern)) {
    return handle_file(headers, name, "");
  }
  
  return ResponseStream::handle_section(headers, name);  
}

//=========================================================================
bool SconesiteStream::handle_file(const scx::MimeHeaderTable& headers,
                                  const std::string& name,
                                  const std::string& filename)
{
  http::Request& req = const_cast<http::Request&>(m_message->get_request());
  http::Response& resp = m_message->get_response();
  const http::Session* session = req.get_session();

  if (!session || !session->has_permission("upload")) {
    resp.set_status(http::Status::Unauthorized);
    return false;
  }

  scx::FilePath path = "/tmp";
  path += std::string(m_module.object()->name() + "-" +
		      session->get_id() + "-" + name);
  req.set_param(name,
                new scx::ScriptRef(new scx::ScriptFile(path,filename)));
  
  scx::File* file = new scx::File();
  if (file->open(path.path(),
                 scx::File::Write | scx::File::Create | 
                 scx::File::Truncate,
                 00660) == scx::Ok) {
    //	endpoint().add_stream(new scx::StreamDebugger("https-file"));
    scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
    file->add_stream(xfer);
    scx::Kernel::get()->connect(file);
    file = 0;
    return true;
  }
      
  log("Error opening file '" + path.path() + "'");
  delete file;
  return false;
}

//=========================================================================
scx::Condition SconesiteStream::send_response()
{
  http::Request& req = const_cast<http::Request&>(m_message->get_request());
  http::Response& resp = m_message->get_response();
  http::Session* session = req.get_session();
  std::string pathinfo = req.get_path_info();

  log("Sending article '" + m_article->object()->get_string() + "'");
  
  // Find the template to use
  Template* tpl = m_profile->lookup_template("start");
  if (!tpl) {
    log("start template not found");
    resp.set_status(http::Status::InternalServerError);
    return scx::Close;
  }

  // If there is a session then we need to lock it before going any further
  if (session && !session->lock()) {
    log("Session [" + session->get_id() + "] is LOCKED, will try again later");
    return scx::Wait;
  }

  // Set the endpoint blocking, saving previous state
  bool prev_block = endpoint().set_blocking(true);
  scx::Date start_time = scx::Date::now();
  
  // Render the page
  try {
    if (!tpl->process(*m_context->object())) {
      tpl->log_errors();
      resp.set_status(http::Status::InternalServerError);
    }
   } catch (...) {
    //XXX this happens too often to log!
    //DEBUG_LOG("EXCEPTION caught in SconesiteStream");
  }

  // Unlock the session
  if (session) session->unlock();

  scx::Time elapsed = scx::Date::now() - start_time;
  std::ostringstream oss;
  oss << "Sent article in " << elapsed.to_microseconds() << " us";
  log(oss.str());

  // Restore endpoint blocking state and reset timeout
  endpoint().set_blocking(prev_block);
  endpoint().reset_timeout();

  // Finished
  return scx::Close;
}

};
