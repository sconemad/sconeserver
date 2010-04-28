/* SconeServer (http://www.sconemad.com)

Sconesite Stream

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


#include "SconesiteModule.h"
#include "SconesiteStream.h"
#include "Article.h"
#include "Profile.h"
#include "Template.h"
#include "RenderMarkup.h"
#include "ArgFile.h"

#include "http/HTTPModule.h"
#include "http/Request.h"
#include "http/Session.h"
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/Stream.h"
#include "sconex/Date.h"
#include "sconex/NullFile.h"
#include "sconex/StreamTransfer.h"
#include "sconex/StreamSocket.h"
#include "sconex/Kernel.h"
#include "sconex/StreamDebugger.h"

//=========================================================================
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
SconesiteStream::SconesiteStream(
  SconesiteModule& module,
  Profile* profile
) : http::ResponseStream("sconesite"),
    m_module(module),
    m_profile(profile),
    m_article(0),
    m_accept(false),
    m_context(0)
{

}

//=========================================================================
SconesiteStream::~SconesiteStream()
{
  delete m_context;
}

//=========================================================================
std::string SconesiteStream::stream_status() const
{
  std::ostringstream oss;
  oss << http::ResponseStream::stream_status()
      << " prf:" << m_profile->name()
      << " art:" << (m_article ? m_article->get_href_path() : "NULL");
  return oss.str();
}

//=========================================================================
scx::Condition SconesiteStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Opening) {
    http::MessageStream* msg = GET_HTTP_MESSAGE();
    http::Request& req = const_cast<http::Request&>(msg->get_request());
    http::Response& resp = msg->get_response();
    const scx::Uri& uri = req.get_uri();
    std::string pathinfo = req.get_path_info();

    if (pathinfo.find("//") != std::string::npos ||
        pathinfo.find("..") != std::string::npos) {
      msg->log("[sconesite] Dodgy pathinfo '" + pathinfo + "' - rejecting");
      resp.set_status(http::Status::Forbidden);
      return scx::Close;
    }

    // Lookup the article and get remaining file path
    m_article = m_profile->get_index()->find_article(pathinfo,m_file);

    // Check article exists
    if (!m_article) {
      msg->log("[sconesite] No article - sending NotFound, pathinfo is '" + pathinfo + "'");
      resp.set_status(http::Status::NotFound);
      return scx::Close;
    }

    // Create context for rendering article
    m_context = new RenderMarkupContext(*m_profile,endpoint(),req,resp);
    m_context->set_article(m_article);

    // Check access is allowed
    if (!m_article->allow_access(*m_context)) {
      resp.set_status(http::Status::Unauthorized);
      if (!m_file.empty()) {
        return scx::Close;
      }
    }
    
    if (m_file.empty()) {
      // Article request, check if we need to redirect to correct the path
      std::string href = m_article->get_href_path();
      if (pathinfo != href) {
        scx::Uri new_uri = uri;
        new_uri.set_path(href);
        msg->log("[sconesite] Redirect '"+uri.get_string()+"' to '"+new_uri.get_string()+"'"); 
        resp.set_status(http::Status::Found);
        resp.set_header("Location",new_uri.get_string());
        return scx::Close;
      }
      
    } else {
      // File request, update the path in the request
      scx::FilePath path = m_article->get_root() + m_file;
      req.set_path(path);
      
      if (!scx::FileStat(path).is_file()) {
        resp.set_status(http::Status::NotFound);
        //        return scx::Close;
      }
    }
    
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
  http::MessageStream* msg = GET_HTTP_MESSAGE();
  http::Request& req = const_cast<http::Request&>(msg->get_request());
  http::Response& resp = msg->get_response();
  const http::Session* session = req.get_session();

  if (m_article->allow_upload(*m_context)) {

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
    std::string::size_type ip = name.find(file_pattern);
    if (ip == 0) {
      // Name starts with "file_" so stream it into a file
      scx::FilePath path = "/tmp";
      std::string filename = "sconesite-" + session->get_id() + "-" + name;
      path += filename;
      //      STREAM_DEBUG_LOG("Streaming section to file '" << path.path() << "'");
      req.set_param(name,new ArgFile(path,fname));
      
      scx::File* file = new scx::File();
      if (file->open(path.path(),
                     scx::File::Write | scx::File::Create | scx::File::Truncate,
                     00660) == scx::Ok) {
        //	endpoint().add_stream(new scx::StreamDebugger("https-file"));
	scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
	file->add_stream(xfer);
	// Add file to kernel
	scx::Kernel::get()->connect(file,0);
	file = 0;
	return scx::Ok;
      }
      
      msg->log("[sconesite] Error opening file '" + path.path() + "'");
      delete file;
      
    } else {
      //      STREAM_DEBUG_LOG("Writing section to parameter '" << name << "'");
      endpoint().add_stream(new ParamReaderStream(name,req));
      return scx::Ok;
    }
  }
    
  // Transfer to a null file to discard the data
  scx::NullFile* file = new scx::NullFile();
  scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
  file->add_stream(xfer);
  scx::Kernel::get()->connect(file,0);
  resp.set_status(http::Status::Unauthorized);
  return scx::Ok;  
}

//=========================================================================
scx::Condition SconesiteStream::send_response()
{
  http::MessageStream* msg = GET_HTTP_MESSAGE();
  http::Request& req = const_cast<http::Request&>(msg->get_request());
  http::Response& resp = msg->get_response();
  std::string pathinfo = req.get_path_info();

  if (resp.get_status().code() == http::Status::Ok && !m_file.empty()) {
    // Request for a file
    
    // Connect the getfile module and relinquish
    scx::ModuleRef getfile = msg->get_module().get_module("getfile");
    if (getfile.valid()) {
      msg->log("[sconesite] article '" + m_article->name() + "'");
      msg->log("[sconesite] Sending '" + pathinfo + "' with getfile"); 
      scx::ArgList args;
      getfile.module()->connect(&endpoint(),0);
      return scx::End;
    }
    // Something went wrong
    resp.set_status(http::Status::InternalServerError);
    return scx::Close;
    
  } else {
    // Request for the article itself
    msg->log("[sconesite] Sending article '" + m_article->name() + "'");
  }
  
  // Set the endpoint blocking, saving previous state
  bool prev_block = endpoint().set_blocking(true);
  
  // Find the template to use, if one was specified, otherwise use the default
  std::string tplname = req.get_param("tpl");
  if (tplname.empty()) tplname = "default";
  Template* tpl = m_profile->lookup_template(tplname);

  // Render the page
  try {
    if (!tpl) {
      m_context->handle_error("No template");
    } else {
      tpl->process(*m_context);
    }
  } catch (...) {
    DEBUG_LOG("EXCEPTION caught in SconesiteStream")
  }

  // Restore endpoint blocking state and reset timeout
  endpoint().set_blocking(prev_block);
  endpoint().reset_timeout();

  // Finished
  return scx::Close;
}
