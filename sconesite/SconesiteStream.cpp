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
#include "ThreadManager.h"

#include "http/HTTPModule.h"
#include "http/Request.h"
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
SconesiteStream::SconesiteStream(
  SconesiteModule& module,
  const std::string& profile
) : http::ResponseStream("sconesite"),
    m_module(module),
    m_profile(profile),
    m_article(0),
    m_prev_cond(scx::Ok),
    m_section(0),
    m_template(0)
{

}

//=========================================================================
SconesiteStream::~SconesiteStream()
{
  
}

//=========================================================================
scx::Condition SconesiteStream::send_response()
{
  http::MessageStream* msg = GET_HTTP_MESSAGE();
  const http::Request& req = msg->get_request();
  http::Response& resp = msg->get_response();
  const scx::Uri& uri = req.get_uri();

  if (req.get_method() == "POST" && req.get_auth_user() == "") {
    msg->get_response().set_status(http::Status::Forbidden);
    return scx::Close;
  }
  
  Profile* profile = m_module.lookup_profile(m_profile);
  
  std::string pathinfo = req.get_path_info();
  std::string::size_type is = pathinfo.find_first_of("/");
  std::string art_name = pathinfo;
  std::string art_file = "";
  if (is != std::string::npos) {
    art_name = pathinfo.substr(0,is);
    art_file = pathinfo.substr(is+1);
  }
  m_article = profile->lookup_article(art_name);
  
  if (pathinfo == "") {
    m_module.log("Sending index"); 
    
  } else if (is == std::string::npos) {
    scx::Uri new_uri = uri;
    new_uri.set_path(new_uri.get_path() + "/");
    m_module.log("Redirect '" + uri.get_string() + "' to '" + new_uri.get_string() + "'"); 
    resp.set_status(http::Status::Found);
    resp.set_header("Content-Type","text/html");
    resp.set_header("Location",new_uri.get_string());
    return scx::Close;
    
  } else if (!art_file.empty()) {
    // Update the path in the request (cast away const for this!)
    http::Request& reqmod = (http::Request&)req;
    reqmod.set_path(profile->get_path() + "art" + art_name + art_file);

    // Connect the getfile module and relinquish
    scx::ModuleRef getfile = msg->get_module().get_module("getfile");
    if (getfile.valid()) {
      m_module.log("Sending '" + art_name + "/" + art_file + "' with getfile"); 
      scx::ArgList args;
      getfile.module()->connect(&endpoint(),0);
      return scx::End;
    }

    //  } else if (m_article) {
  } else if (!art_name.empty()) {
    m_module.log("Sending article '" + art_name + "'");
    resp.set_header("Content-Type","text/html");
    
  } else {
    m_module.log("Sending NotFound, pathinfo is '" + pathinfo + "'");
    resp.set_status(http::Status::NotFound);
    return scx::Close;
  }
  
  // Create a socketpair to connect to the render job thread
  scx::StreamSocket* source = 0;
  scx::StreamSocket* bio = 0;
  scx::StreamSocket::pair(source,bio,"sconesite");
  
  // Create a transfer to transfer from source into this stream
  scx::StreamTransfer* xfer = new scx::StreamTransfer(source,1024);
  xfer->set_close_when_finished(true);
  endpoint().add_stream(xfer);
  scx::Kernel::get()->connect(source,0);
  
  // Create context for rendering HTML to the bio socket
  RenderMarkupContext* ctx = new RenderMarkupContext(*profile,bio,req);
  ctx->set_article(m_article);
  
  // Create and add a job to render this page
  RenderMarkupJob* job = new RenderMarkupJob(ctx);
  m_module.get_thread_manager().add(job);
  
  // Don't need us any more!
  return scx::End;
}

//=========================================================================
scx::Condition SconesiteStream::start_section(const scx::MimeHeaderTable& headers)
{
  ++m_section;
  STREAM_DEBUG_LOG("Start section " << m_section);

  http::MessageStream* msg = GET_HTTP_MESSAGE();

  const http::Request& req = msg->get_request();
  bool auth = (req.get_auth_user() != "");

  //  const scx::Uri& uri = req.get_uri();
  Profile* profile = m_module.lookup_profile(m_profile);
  std::string pathinfo = req.get_path_info();
  std::string::size_type is = pathinfo.find_first_of("/");
  std::string art_name = pathinfo;
  std::string art_file = "";
  if (is != std::string::npos) {
    art_name = pathinfo.substr(0,is);
    art_file = pathinfo.substr(is+1);
  }
  m_article = profile->lookup_article(art_name);

  if (0 == m_article && auth) {
    m_article = profile->create_article(art_name);
  }
  
  scx::MimeHeader disp = headers.get_parsed("Content-Disposition");
  bool discard = true;
  
  scx::FilePath path = m_article->get_root();
  const scx::MimeHeaderValue* fdata = disp.get_value("form-data");
  if (fdata) {
    std::string name;
    fdata->get_parameter("name",name);
    STREAM_DEBUG_LOG("Section name is '" << name << "'");
    if (name == "artbody") {
      discard = false;
      path += "article.xml";
    } else if (name == "file") {
      std::string filename;
      fdata->get_parameter("filename",filename);
      STREAM_DEBUG_LOG("Section filename is '" << filename << "'");
      if (filename != "") {
        discard = false;
	path += "files";
        path += filename;
      }
    }
  }
  
  // Discard posted data from unauthorized user
  if (!auth) {
    discard = true;
  }

  if (!discard) {
    scx::File* file = new scx::File();
    if (file->open(path.path(),scx::File::Write | scx::File::Create | scx::File::Truncate) == scx::Ok) {
      STREAM_DEBUG_LOG("Writing section to '" << path.path() << "'");
      endpoint().add_stream(new scx::StreamDebugger("https-file"));
      scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
      file->add_stream(xfer);
      // Add file to kernel
      scx::Kernel::get()->connect(file,0);
      file = 0;
    } else {
      STREAM_DEBUG_LOG("Error opening file '" << path.path() << "'");
      discard = true;
      delete file;
    }
  }
  
  if (discard) {
    // Transfer to a null file to discard the data
    scx::NullFile* file = new scx::NullFile();
    scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
    file->add_stream(xfer);
    scx::Kernel::get()->connect(file,0);
  }
  
  return scx::Ok;
}
