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
#include "SconesiteArticle.h"

#include "http/HTTPModule.h"
#include "http/Request.h"
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/Stream.h"
#include "sconex/FileDir.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/LineBuffer.h"
#include "sconex/Buffer.h"
#include "sconex/MimeType.h"
#include "sconex/NullFile.h"

//=========================================================================
class MimeHeaderStream : public scx::LineBuffer {

public:

  MimeHeaderStream()
    : scx::LineBuffer("sconesite:mimeheader"),
      m_done(false),
      m_request("")
  {
    enable_event(scx::Stream::Readable,true);
  };

  virtual scx::Condition event(scx::Stream::Event e)
  {
    if (e == scx::Stream::Readable && !m_done) {
      scx::Condition c;
      std::string line;
    
      while ( (c=tokenize(line)) == scx::Ok ) {
        if (line.empty()) {
          m_done = true;
          enable_event(scx::Stream::Readable,false);
          return start_body();
        } else {
          m_request.parse_header(line);
        }
      }
    }
    
    return scx::Ok;
  };

  scx::Condition start_body()
  {
    http::MessageStream* m_message = GET_HTTP_MESSAGE();

    scx::MimeHeader disp = m_request.get_header_parsed("Content-Disposition");
    bool discard = true;

    scx::FilePath path = m_message->get_request().get_path();
    const scx::MimeHeaderValue* fdata = disp.get_value("form-data");
    if (fdata) {
      std::string name;
      fdata->get_parameter("name",name);
      if (name == "artbody") {
        path += "article.xml";
        discard = false;
      } else if (name == "testfile") {
        std::string filename;
        fdata->get_parameter("filename",filename);
        if (filename != "") {
          discard = false;
          path += filename;
        }
      }
    }

    // Discard posted data from unauthorized user
    if (m_message->get_request().get_auth_user() == "") {
      discard = true;
    }
    
    //m_message->send_continue();
    
    if (discard) {
      // Transfer to a null file to discard the data
      scx::NullFile* file = new scx::NullFile();
      scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
      file->add_stream(xfer);
      scx::Kernel::get()->connect(file,0);

    } else {
      scx::File* file = new scx::File();
      if (file->open(path.path(),scx::File::Write | scx::File::Create | scx::File::Truncate) == scx::Ok) {
        scx::StreamTransfer* xfer = new scx::StreamTransfer(&endpoint());
        file->add_stream(xfer);
        // Add file to kernel
        scx::Kernel::get()->connect(file,0);
        file = 0;
      } else {
        delete file;
        return scx::Close;
      }
    }
    
    return scx::Ok;
  };

protected:
  
  bool m_done;
  http::Request m_request;
  
};



//=========================================================================
SconesiteStream::SconesiteStream(
  SconesiteModule& module,
  const std::string& profile
) : http::ResponseStream("sconesite"),
    m_module(module),
    m_profile(profile),
    m_seq(Start),
    m_prev_cond(scx::Ok),
    m_section(0)
{

}

//=========================================================================
SconesiteStream::~SconesiteStream()
{
  
}

//=========================================================================
scx::Condition SconesiteStream::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Opening) {
    STREAM_DEBUG_LOG("Opening SconesiteStream");
  }
  
  if (e == scx::Stream::Closing) {
    STREAM_DEBUG_LOG("Closing SconesiteStream");
  }

  if (e == scx::Stream::Closing && m_seq == ArticleFile) {
    m_seq = ArticleFooter;
    return scx::End;
  }
  
  return http::ResponseStream::event(e);
}

//=========================================================================
scx::Condition SconesiteStream::send(http::MessageStream& msg)
{
  const http::Request& req = msg.get_request();
  const scx::Uri& uri = req.get_uri();

  if (req.get_method() == "POST" && req.get_auth_user() == "") {
    msg.get_response().set_status(http::Status::Forbidden);
    return scx::Close;
  }
  
  SconesiteArticleManager* profile = m_module.lookup_profile(m_profile);
  const std::list<SconesiteArticle*>& articles = profile->articles();
  
  switch (m_seq) {

    case Start: {

      scx::FileStat stat(req.get_path());
      if (stat.is_dir()) {
        std::string uripath = uri.get_path();
        if (!uripath.empty() && uripath[uripath.size()-1] != '/') {
          scx::Uri new_uri = uri;
          new_uri.set_path(uripath + "/");
          m_module.log("Redirect '" + uri.get_string() + "' to '" + new_uri.get_string() + "'"); 
          
          msg.get_response().set_status(http::Status::Found);
          msg.get_response().set_header("Content-Type","text/html");
          msg.get_response().set_header("Location",new_uri.get_string());
          return scx::Close;
        }

        m_file = new scx::File();
        scx::FilePath path = req.get_path();
        path += "article.xml";
        
        if (m_file->open(path.path(),scx::File::Read) == scx::Ok) {
          m_module.log("Processing file " + path.path()); 
          m_seq = ArticleHeader;
        } else {
          delete m_file;
          m_file = 0;
          m_module.log("Sending index"); 
          m_seq = IndexHeader;
        }
      } else {
        msg.get_response().set_status(http::Status::NotFound);
        return scx::Close;
      }
    } break;
    
    case IndexHeader: {
      write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
            "<html>\n"
            "<head>\n"
            "<link rel='shortcut icon' href='http://dichotomy.co.uk/favicon.ico' />\n"
            "<link rel='stylesheet' title='Dark' type='text/css' href='http://dichotomy.co.uk/style/main.css' />\n"
            "<link rel='alternate' type='text/xml' title='XML' href='http://dichotomy.co.uk/index.xml' />\n"
            "<title>Sconesite</title>\n"
            "</head>\n");
      
      write("<body>\n");
      
      write("<h1>" + uri.get_path() + "</h1>\n");

      write("\n\n");
      write("<ul>\n");
      if (articles.size() > 0) {
        m_seq = IndexList;
        m_list_it = articles.begin();
      } else {
        m_seq = IndexFooter;
      }
    } break;
    
    case IndexList: {
      const SconesiteArticle* article = (*m_list_it);
      std::string href = article->get_name() + "/";
      write("<li><a href='" + href + "'>" + article->get_name() + "</a></li>\n");
      
      ++m_list_it;
      if (m_list_it == articles.end()) {
        m_seq = IndexFooter;
      }
    } break;
    
    case IndexFooter: {
      write("</ul>\n");
      write("\n\n"
            "<div>\n"
            "<address>\n"
            "Copyright (c) 2009 Andrew Wedgbury / wedge (at) sconemad (dot) com\n"
            "</address>\n"
            "</body>\n"
            "</html>\n");
      return scx::Close;
    } break;
    
    case ArticleHeader: {
      write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
            "<html>\n"
            "<head>\n"
            "<link rel='shortcut icon' href='http://dichotomy.co.uk/favicon.ico' />\n"
            "<link rel='stylesheet' title='Dark' type='text/css' href='http://dichotomy.co.uk/style/main.css' />\n"
            "<link rel='alternate' type='text/xml' title='XML' href='http://dichotomy.co.uk/index.xml' />\n"
            "<title>Sconesite</title>\n"
            "</head>\n");
      
      write("<body>\n");
      
      write("<h1>" + uri.get_path() + "</h1>\n");

      if (uri.get_query() == "delete") {

        
        
      } else if (uri.get_query() == "edit") {
        scx::Uri action = uri;
        action.set_query("submit");
        write("<form id='editform' name='editform' method='post' action='" + action.get_string() + "' enctype='multipart/form-data'>\n");
        write("<input type='submit' value='Save'/>\n");
      } else {
        scx::Uri action = uri;
        action.set_query("edit");
        write("<p><a href='" + action.get_string() + "'>Edit this article</a></p>\n");
      }
      
      write("<div id='main'/>\n");
      write("\n\n");
      
      if (uri.get_query() == "edit") {
        write("<textarea name='artbody' id='artbody' cols='80' rows='25' tabindex='1' accesskey=','>\n");
      }
      
      scx::StreamTransfer* xfer = new scx::StreamTransfer(m_file,1024);
      xfer->set_close_when_finished(true);
      endpoint().add_stream(xfer);
      
      // Add file to kernel
      scx::Kernel::get()->connect(m_file,0);
      m_file = 0;
      m_seq = ArticleFile;
    } break;

    case ArticleFooter: {
      
      if (uri.get_query() == "edit") {
        write("</textarea>\n");
        write("<br/>\n");
        write("<input type='file' name='testfile'/>\n");
        write("</form>");

        scx::ModuleRef mime = scx::Kernel::get()->get_module("mime");
        
        write("<h2>Files:</h2>\n");
        write("<ul>\n");
        scx::FileDir dir(req.get_path());
        while (dir.next()) {
          std::string name = dir.name();
          std::string type = "";
          std::string subtype = "";
          
          // Lookup MIME type for file
          if (mime.valid()) {
            scx::ArgList args;
            args.give( new scx::ArgString(name) );
            scx::Arg* ret = mime.module()->arg_function("lookup",&args);
            scx::MimeType* mimetype = 0;
            if (ret) {
              mimetype = dynamic_cast<scx::MimeType*>(ret);
              if (mimetype) {
                type = mimetype->get_type();
                subtype = mimetype->get_subtype();
              }
            }
            delete ret;
          }
          
          if (name != "." && name != "..") {
            write("<li><a href='" + name + "'>" + name + " (" + type + "/" + subtype + ")</a></li>\n");
          }
        }
        write("</ul>\n");
              
      }

      write("\n\n"
            "<div>\n"
            "<address>\n"
            "Copyright (c) 2009 Andrew Wedgbury / wedge (at) sconemad (dot) com\n"
            "</address>\n"
            "</body>\n"
            "</html>\n");
      return scx::Close;
    } break;
  }
  
  return scx::Ok;
}

//=========================================================================
scx::Condition SconesiteStream::start_section()
{
  ++m_section;
  STREAM_DEBUG_LOG("Start section " << m_section);

  MimeHeaderStream* mh = new MimeHeaderStream();
  endpoint().add_stream(mh);

  return scx::Ok;
}
