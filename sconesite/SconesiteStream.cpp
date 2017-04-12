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
#include <sconex/NullFile.h>
#include <sconex/StreamTransfer.h>
#include <sconex/StreamSocket.h>
#include <sconex/Kernel.h>
#include <sconex/StreamDebugger.h>
#include <sconex/FilePath.h>
#include <sconex/Log.h>
namespace scs {

//=========================================================================
// ParamReaderStream - Stream for reading parameters in mime headers
// TODO: probably should live in http
//
class ParamReaderStream : public scx::Stream {
public:

  ParamReaderStream(const std::string& name, http::Request& request)
    : scx::Stream("ParamReader"),
      m_name(name),
      m_request(request)
  {
    enable_event(scx::Stream::Readable,true);
  };

  virtual scx::Condition event(scx::Stream::Event e)
  {
    if (e == scx::Stream::Readable) {
      char buffer[1024];
      scx::Condition c;
      int na = 0;
      do {
	c = read(buffer,1024,na);
	m_value += std::string(buffer,na);
      } while (c == scx::Ok);

      if (c == scx::End) {
        m_request.set_param(m_name,m_value);
	return scx::Close;
      }
    
      return c;
    }
    
    return scx::Ok;
  };

protected:
  
  std::string m_name;
  std::string m_value;
  http::Request& m_request;
  
};


//=========================================================================
SconesiteHandler::SconesiteHandler(SconesiteModule* module,
                                   const scx::ScriptRef* args)
  : m_module(module),
    m_profile(0)
{
  const scx::ScriptString* profile_name =
    scx::get_method_arg<scx::ScriptString>(args,0,"profile");
  if (profile_name) {
    m_profile = module->lookup_profile(profile_name->get_string());
  }
}
  
//=========================================================================
scx::Condition SconesiteHandler::handle_message(http::MessageStream* message)
{
  if (!m_profile) {
    message->send_simple_response(http::Status::NotFound);
    return scx::Close;
  }

  http::Request& req = const_cast<http::Request&>(message->get_request());
  http::Response& resp = message->get_response();
  const scx::Uri& uri = req.get_uri();
  std::string pathinfo = req.get_path_info();
  
  if (pathinfo.find("//") != std::string::npos ||
      pathinfo.find("..") != std::string::npos) {
    //log("Request for '" + pathinfo +
    //    "' - Forbidden (path contains forbidden chars)");
    message->send_simple_response(http::Status::Forbidden);
    return scx::Close;
  }
  
  // Lookup the article and get remaining file path
  std::string file;
  Article::Ref* article = m_profile->lookup_article(pathinfo, file);
  
  // Check article exists
  if (!article) {
    //log("Request for '" + pathinfo + "' - NotFound (no article)");
    message->send_simple_response(http::Status::NotFound);
    return scx::Close;
  }

  if (file.empty()) {
    // Article request, check if we need to redirect to correct the path
    std::string href = article->object()->get_href_path();
    if (pathinfo != href) {
      scx::Uri new_uri = uri;
      new_uri.set_path(href);
      //log("Redirect '"+uri.get_string()+"' to '"+new_uri.get_string()+"'"); 
      resp.set_header("Location",new_uri.get_string());
      message->send_simple_response(http::Status::Found);
      return scx::Close;
    }
    
  } else {
    // File request, update the path in the request
    scx::FilePath path = article->object()->get_root() + file;
    req.set_path(path);
    if (!scx::FileStat(path).is_file()) {
      //log("File request for '" + path.path() + "' - NotFound");
      message->send_simple_response(http::Status::NotFound);
      
    } else if (file.find("article.") == 0) {
      // Don't allow any article source to be sent
      //log("File request for '" + path.path() + "' - Forbidden (article source)");
      message->send_simple_response(http::Status::Forbidden);
      
    } else {
      //log("File request for '" + path.path() + "'");
    }
  }

  scx::Log("sconesite").attach("file",file).submit("handle message");
  
  message->add_stream(new SconesiteStream(m_module.object(),
                                          m_profile, message));
  return scx::Ok;
}
  

//=========================================================================
SconesiteStream::SconesiteStream(SconesiteModule* module,
				 Profile* profile,
                                 http::MessageStream* message)
  : http::ResponseStream("sconesite"),
    m_module(module),
    m_profile(profile),
    m_message(message),
    m_article(0),
    m_accept(false),
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
  http::Request& req = const_cast<http::Request&>(m_message->get_request());
  scx::Log("sconesite").attach("id",req.get_id()).submit(str);
}

//=========================================================================
scx::Condition SconesiteStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Opening) {
    http::Request& req = const_cast<http::Request&>(m_message->get_request());
    http::Response& resp = m_message->get_response();
    const scx::Uri& uri = req.get_uri();
    std::string pathinfo = req.get_path_info();

    if (pathinfo.find("//") != std::string::npos ||
        pathinfo.find("..") != std::string::npos) {
      log("Request for '" + pathinfo + "' - Forbidden (path contains forbidden chars)");
      resp.set_status(http::Status::Forbidden);
      return scx::Close;
    }

    // Lookup the article and get remaining file path
    m_article = m_profile->lookup_article(pathinfo,m_file);

    // Check article exists
    if (!m_article) {
      log("Request for '" + pathinfo + "' - NotFound (no article)");
      resp.set_status(http::Status::NotFound);
      return scx::Close;
    }

    // Create context for rendering article
    RenderMarkupContext* rmc = new RenderMarkupContext(m_profile,
						       *this,
						       endpoint(),
						       req,
						       resp);
    m_context = new RenderMarkupContext::Ref(rmc);
    rmc->set_article(m_article->object());

   m_accept = true;
  }

  if (m_accept) {
    return http::ResponseStream::event(e);
  } else {
    return scx::Ok;
  }
}

//=========================================================================
scx::Condition SconesiteStream::start_section(const scx::MimeHeaderTable& headers)
{
  http::Request& req = const_cast<http::Request&>(m_message->get_request());
  http::Response& resp = m_message->get_response();
  const http::Session* session = req.get_session();

  std::string name;
  scx::MimeHeader disp = headers.get_parsed("Content-Disposition");
  const scx::MimeHeaderValue* fdata = disp.get_value("form-data");
  if (!fdata) {
    return scx::Close;
  }
  
  fdata->get_parameter("name",name);
  //    STREAM_DEBUG_LOG("Section name is '" << name << "'");
  
  std::string fname;
  fdata->get_parameter("filename",fname);
  
  const std::string file_pattern = "file_";
  if (0 == name.find(file_pattern)) {
    // Name starts with "file_" so stream it into a file, provided we
    // have a session with the upload permission.
    
    if (!session || !session->has_permission("upload")) {
      resp.set_status(http::Status::Unauthorized);
      
    } else {
      scx::FilePath path = "/tmp";
      std::string filename = 
	m_module.object()->name() + "-" + session->get_id() + "-" + name;
      path += filename;
      //      STREAM_DEBUG_LOG("Streaming section to file '" << path.path() << "'");
      req.set_param(name,
		    new scx::ScriptRef(new scx::ScriptFile(path,fname)));
      
      scx::File* file = new scx::File();
      if (file->open(path.path(),
		     scx::File::Write | scx::File::Create | 
		     scx::File::Truncate,
		     00660) == scx::Ok) {
	//	endpoint().add_stream(new scx::StreamDebugger("https-file"));
	scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
	file->add_stream(xfer);
	// Add file to kernel
	scx::Kernel::get()->connect(file);
	file = 0;
	return scx::Ok;
      }
      
      log("Error opening file '" + path.path() + "'");
      delete file;
    }
  } else {
    //      STREAM_DEBUG_LOG("Writing section to parameter '" << name << "'");
    endpoint().add_stream(new ParamReaderStream(name,req));
    return scx::Ok;
  }
  
  // Transfer to a null file to discard the data
  scx::NullFile* file = new scx::NullFile();
  scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
  file->add_stream(xfer);
  scx::Kernel::get()->connect(file);
  return scx::Ok;  
}

//=========================================================================
scx::Condition SconesiteStream::send_response()
{
  http::Request& req = const_cast<http::Request&>(m_message->get_request());
  http::Response& resp = m_message->get_response();
  http::Session* session = req.get_session();
  std::string pathinfo = req.get_path_info();

  if (resp.get_status().code() == http::Status::Ok && !m_file.empty()) {
    // Request for a file
    
    // Connect the getfile module and relinquish
    http::Handler* getfile = http::Handler::create("getfile",0);
    if (getfile) {
      log("Article '" + m_article->object()->get_string() + 
	  "' sending '" + pathinfo + "' with getfile"); 
      scx::Condition c = getfile->handle_message(m_message);
      delete getfile;
      if (c == scx::Ok) return scx::End;
      return c;
    }
    // Something went wrong
    resp.set_status(http::Status::InternalServerError);
    return scx::Close;
    
  } else {
    // Request for the article itself
    log("Sending article '" + m_article->object()->get_string() + "'");
  }
  
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
    DEBUG_LOG("EXCEPTION caught in SconesiteStream");
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
