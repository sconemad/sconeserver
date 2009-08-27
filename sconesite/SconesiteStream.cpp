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
#include "Context.h"

#include "http/HTTPModule.h"
#include "http/Request.h"
#include "http/MessageStream.h"
#include "http/MultipartStream.h"
#include "http/Status.h"

#include "sconex/Stream.h"
#include "sconex/FileDir.h"
#include "sconex/Date.h"
#include "sconex/LineBuffer.h"
#include "sconex/Buffer.h"
#include "sconex/MimeType.h"
#include "sconex/NullFile.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Kernel.h"

//=========================================================================
SconesiteStream::SconesiteStream(
  SconesiteModule& module,
  const std::string& profile
) : http::ResponseStream("sconesite"),
    m_module(module),
    m_profile(profile),
    m_seq(Start),
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
  const scx::Uri& uri = req.get_uri();

  bool auth = (req.get_auth_user() != "");
  
  if (req.get_method() == "POST" && req.get_auth_user() == "") {
    msg->get_response().set_status(http::Status::Forbidden);
    return scx::Close;
  }
  
  Profile* profile = m_module.lookup_profile(m_profile);
  
  switch (m_seq) {

    case Start: {

      std::string pathinfo = req.get_path_info();

      std::string::size_type is = pathinfo.find_first_of("/");
      std::string art_name = pathinfo;
      std::string art_file = "";
      if (is != std::string::npos) {
        art_name = pathinfo.substr(0,is);
        art_file = pathinfo.substr(is+1);
      }
      m_article = profile->lookup_article(art_name);

      m_module.log("art_name '" + art_name + "'");
      m_module.log("art_file '" + art_file + "'");
      
      if (pathinfo == "") {
        m_module.log("Sending index"); 
        m_seq = RunTemplate;

      } else if (is == std::string::npos) {
        scx::Uri new_uri = uri;
        new_uri.set_path(new_uri.get_path() + "/");
        m_module.log("Redirect '" + uri.get_string() + "' to '" + new_uri.get_string() + "'"); 
        msg->get_response().set_status(http::Status::Found);
        msg->get_response().set_header("Content-Type","text/html");
        msg->get_response().set_header("Location",new_uri.get_string());
        return scx::Close;

      } else if (art_file != "") {
        // Redirect to data location
        scx::Uri new_uri = uri;
        new_uri.set_path("data/" + art_name + "/" + art_file);
        m_module.log("Redirect data '" + uri.get_string() + "' to '" + new_uri.get_string() + "'"); 
        msg->get_response().set_status(http::Status::Found);
        msg->get_response().set_header("Content-Type","text/html");
        msg->get_response().set_header("Location",new_uri.get_string());
        return scx::Close;
        
      } else if (m_article) {
        m_module.log("Sending article '" + art_name + "'");
        msg->get_response().set_header("Content-Type","text/html");
        m_seq = RunTemplate;
        
      } else {
        m_module.log("Sending 404, pathinfo is '" + pathinfo + "'");
        msg->get_response().set_status(http::Status::NotFound);
        return scx::Close;
      }

    } break;
    
    case RunTemplate: {
      std::string tplname = "test.xml";
      if (m_article == 0) {
        tplname = "index.xml";
        
      } else if (auth) {
        if (uri.get_query() == "delete") {
        
        } else if (uri.get_query() == "edit") {
          tplname = "edit.xml";
          //          scx::Uri action = uri;
          //          action.set_query("submit");
          //          write("<form id='editform' name='editform' method='post' action='" + action.get_string() + "' enctype='multipart/form-data'>\n");
          //          write("<input type='submit' value='Save'/>\n");
        }
      }
      
      Profile* profile = m_module.lookup_profile(m_profile);
      Template* tpl = profile->lookup_template(tplname);
      if (tpl) {
        Context* ctx = new Context(*profile,endpoint());
        ctx->set_article(m_article);
        tpl->process(*ctx);
        delete ctx;
      }
      return scx::Close;
    } break;
  }
  
  return scx::Ok;
}

//=========================================================================
scx::Condition SconesiteStream::start_section(const scx::MimeHeaderTable& headers)
{
  ++m_section;
  STREAM_DEBUG_LOG("Start section " << m_section);

  http::MessageStream* msg = GET_HTTP_MESSAGE();

  const http::Request& req = msg->get_request();
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
    } else if (name == "testfile") {
      std::string filename;
      fdata->get_parameter("filename",filename);
      STREAM_DEBUG_LOG("Section filename is '" << filename << "'");
      if (filename != "") {
        discard = false;
        path += filename;
      }
    }
  }
  
  // Discard posted data from unauthorized user
  if (msg->get_request().get_auth_user() == "") {
    discard = true;
  }

  if (!discard) {
    scx::File* file = new scx::File();
    if (file->open(path.path(),scx::File::Write | scx::File::Create | scx::File::Truncate) == scx::Ok) {
      STREAM_DEBUG_LOG("Writing section to '" << path.path() << "'");
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
