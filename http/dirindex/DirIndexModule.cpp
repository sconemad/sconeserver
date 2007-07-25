/* SconeServer (http://www.sconemad.com)

HTTP Directory index module

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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


#include "http/HTTPModule.h"
#include "http/Request.h"
#include "http/FSDirectory.h"
#include "http/MessageStream.h"
#include "http/Status.h"
#include "http/FSNode.h"
#include "http/ResponseStream.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"

//=========================================================================
class DirIndexStream : public http::ResponseStream {

public:

  DirIndexStream(
    scx::Module& module
  ) : http::ResponseStream("http:dirindex"),
      m_module(module)
  {

  }

  ~DirIndexStream()
  {

  }

  
protected:

  void do_dir(const http::FSDirectory* fsdir)
  {
    const std::list<http::FSNode*>& dir = fsdir->dir();
    std::list<http::FSNode*>::const_iterator it = dir.begin();
    while (it != dir.end()) {
      const http::FSNode* cur = (*it);
      std::string name = cur->name();
      std::string link = cur->url();
      std::string type = "file";
      if (cur->type() == http::FSNode::Directory) {
        name += "/";
        link += "/";
        type = "dir";
      }
      
      write("<li class='" + type +
            "'><a href='" + link +
            "'>" + name + "</a></li>\n");
      
      //      if (cur->type() == http::FSNode::Directory) {
      //        write("<ul>\n");
      //        do_dir((const http::FSDirectory*)cur);      
      //        write("</ul>\n");
      //      }
      ++it;
    }
  
  
  };

  virtual scx::Condition send(http::MessageStream& msg) 
  {
    const http::Request& req = msg.get_request();
    const scx::Uri& uri = req.get_uri();

    if (req.get_method() != "GET" && 
        req.get_method() != "HEAD" ) {
      // Don't understand the method
      msg.set_status(http::Status::NotImplemented);
      return scx::Close;
    }
      
    const http::FSNode* node = msg.get_node();
    if (!node) {
      return scx::Close;
    }

    if (node->type() == http::FSNode::Directory) {
      const http::FSDirectory* fsdir = (const http::FSDirectory*)node;
      
      const scx::Arg* a_default_page = fsdir->get_param("default_page");
      std::string s_default_page =
        (a_default_page ? a_default_page->get_string() : "index.html");
      
      std::string url = uri.get_string();
      std::string path = uri.get_path();
      
      if (fsdir->lookup(s_default_page)) {
        // Redirect to default page
        if (url[url.size()-1] != '/') url += "/";
        m_module.log("Redirect '" + url + "' to '" +
                     url + s_default_page + "'"); 
        url += s_default_page;
        
        msg.set_status(http::Status::Found);
        msg.set_header("Content-Type","text/html");
        msg.set_header("Location",url);
        return scx::Close;
      }
      
      if (!path.empty() && path[path.size()-1] != '/') {
        // Redirect to directory URL ending in '/'
        m_module.log("Redirect '" + url + "' to '" + url + "/'"); 
        url += "/";
        
        msg.set_status(http::Status::Found);
        msg.set_header("Content-Type","text/html");
        msg.set_header("Location",url);
        return scx::Close;
      }
      
      const scx::Arg* a_allow_list = fsdir->get_param("allow_list");
      bool allow_list = (a_allow_list ? a_allow_list->get_int() : false);
      
      if (allow_list) {
        // Send directory listing if allowed
        m_module.log("Listing directory '" + url + "'"); 
        
        msg.set_status(http::Status::Ok);
        msg.set_header("Content-Type","text/html");
        
        if (req.get_method() == "GET") {
          write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
                "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
                "<html>\n"
                "<head>\n"
                "<title>Directory listing</title>\n"
                "<link rel='stylesheet' href='/dir.css' type='text/css' />"
                "</head>\n"
                "<body>\n"
                "<h1>Listing of " + node->url() + "</h1>\n"
                "<div class='box'>\n"
                "<ul>\n");
          
          if (fsdir->parent()) {
            write("<li class='parent'><a href='" +
                  fsdir->parent()->url() +
                  "'>../ (parent)</a></li>\n");
          }
          do_dir(fsdir);
          
          write("</ul>\n"
                "</div>\n"
                "</body>\n"
                "</html>\n");
        }
        return scx::Close;
      }
      
      // Otherwise respond unauthorised
      msg.set_status(http::Status::Unauthorized);
      return scx::Close;
      
    }

    return scx::Close;
  };

private:
    
  scx::Module& m_module;

};


//=========================================================================
class DirIndexModule : public scx::Module {
public:

  DirIndexModule();
  virtual ~DirIndexModule();

  virtual std::string info() const;

  virtual int init();
  
  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

protected:

private:

};

SCONESERVER_MODULE(DirIndexModule);

//=========================================================================
DirIndexModule::DirIndexModule(
) : scx::Module("http:dirindex",scx::version())
{

}

//=========================================================================
DirIndexModule::~DirIndexModule()
{

}

//=========================================================================
std::string DirIndexModule::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
         "HTTP directory and index file handler\n";
}

//=========================================================================
int DirIndexModule::init()
{
  return Module::init();
}

//=========================================================================
bool DirIndexModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  DirIndexStream* s = new DirIndexStream(*this);
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}
