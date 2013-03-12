/* SconeServer (http://www.sconemad.com)

HTTP Directory index stream

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

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

#include <http/DirIndexStream.h>
#include <http/HTTPModule.h>
#include <http/Request.h>
#include <http/MessageStream.h>
#include <http/Status.h>
#include <http/DocRoot.h>

#include <sconex/FileDir.h>
namespace http {
  
//=========================================================================
scx::Condition DirIndexStream::send_response()
{
  http::MessageStream* msg = GET_HTTP_MESSAGE();
  const http::Request& req = msg->get_request();
  const scx::Uri& uri = req.get_uri();
  const http::DocRoot* docroot = req.get_docroot();
  
  if (req.get_method() != "GET" && 
      req.get_method() != "HEAD" ) {
      // Don't understand the method
    msg->get_response().set_status(http::Status::NotImplemented);
    return scx::Close;
  }
  
  const scx::FilePath& path = req.get_path();
  scx::FileStat stat(path);
  
  if (stat.is_dir()) {
    const scx::ScriptRef* a_default_page = 
      docroot->get_param("default_page");
    std::string s_default_page = (BAD_SCRIPTREF(a_default_page) ? 
				  "index.html" : 
				  a_default_page->object()->get_string());
    
    std::string url = uri.get_string();
    std::string uripath = uri.get_path();
    
    if (scx::FileStat(path + s_default_page).exists()) {
      // Redirect to default page
      if (url[url.size()-1] != '/') url += "/";
      msg->log("Redirect '" + url + "' to '" + url + s_default_page + "'"); 
      url += s_default_page;
      
      msg->get_response().set_status(http::Status::Found);
      msg->get_response().set_header("Content-Type","text/html");
      msg->get_response().set_header("Location",url);
      return scx::Close;
    }
    
    if (!uripath.empty() && uripath[uripath.size()-1] != '/') {
      // Redirect to directory URL ending in '/'
      scx::Uri new_uri = uri;
      new_uri.set_path(uripath + "/");
      msg->log("Redirect '" + uri.get_string() + 
               "' to '" + new_uri.get_string() + "'"); 
      
      msg->get_response().set_status(http::Status::Found);
      msg->get_response().set_header("Content-Type","text/html");
      msg->get_response().set_header("Location",new_uri.get_string());
      return scx::Close;
    }
    
    const scx::ScriptRef* a_allow_list = docroot->get_param("allow_list");
    bool allow_list = (a_allow_list ? 
		       a_allow_list->object()->get_int() : 
		       false);
    
    if (allow_list) {
      // Send directory listing if allowed
      msg->log("Listing directory '" + url + "'"); 
      
      msg->get_response().set_status(http::Status::Ok);
      msg->get_response().set_header("Content-Type","text/html");
      
      if (req.get_method() == "GET") {
	write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
	      "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
	      "<html>\n"
	      "<head>\n"
	      "<title>Directory listing</title>\n"
	      "<link rel='stylesheet' href='/dir.css' type='text/css' />"
	      "</head>\n"
	      "<body>\n"
	      "<h1>Listing of " + url + "</h1>\n"
	      "<div class='box'>\n"
	      "<ul>\n");
	
	scx::FileDir dir(req.get_path());
	while (dir.next()) {
	  std::string name = dir.name();
	  if (name != ".") {
	    write("<li><a href='" + name + "'>" + name + "</a></li>\n");
	  }
	}
        
	write("</ul>\n"
	      "</div>\n"
	      "</body>\n"
	      "</html>\n");
      }
      return scx::Close;
    }
    
    // Otherwise respond unauthorised
    msg->get_response().set_status(http::Status::Unauthorized);
    return scx::Close;
    
  }
  
  return scx::Close;
}

};
