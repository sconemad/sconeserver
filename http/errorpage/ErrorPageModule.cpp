/* SconeServer (http://www.sconemad.com)

Error Page module

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
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Stream.h"

//=========================================================================
class ErrorPageStream : public scx::Stream {

public:

  ErrorPageStream(
    scx::Module& module
  ) : Stream("http:errorpage"),
      m_module(module)
  {
    enable_event(scx::Stream::Writeable,true);
  }

  ~ErrorPageStream()
  {

  }

  
protected:

  virtual scx::Condition event(scx::Stream::Event e) 
  {
    if (e == scx::Stream::Writeable) {
      http::MessageStream* msg = 
	dynamic_cast<http::MessageStream*>(find_stream("http:message"));
      const http::Request& req = msg->get_request();

      if (req.get_method() != "GET" && 
	  req.get_method() != "HEAD" ) {
	// Don't understand the method
	msg->set_status(http::Status::NotImplemented);
	return scx::Close;
      }

      m_module.log("Not found '" + req.get_uri().get_string() + "'"); 

      msg->set_status(http::Status::NotFound);
      msg->set_header("Content-Type","text/html");

      if (req.get_method() == "GET") {
	// Only need to send the message body if method is GET
	write("<html>\n");
	write("<head><title>404 Not Found</title></head>\n");
	write("<body>\n");
	write("<h1>404 Not Found</h1>\n");
	write("<p>The requested page '<em>" +
	      req.get_uri().get_path() +
	      "</em>' could not be found on this server.</p>\n");
	write("</body>\n");
	write("</html>\n");
      }

      return scx::Close;
    }
    
    return scx::Ok;
  };

private:
    
  scx::Module& m_module;

};


//=========================================================================
class ErrorPageModule : public scx::Module {
public:

  ErrorPageModule();
  virtual ~ErrorPageModule();

  virtual std::string info() const;

  virtual int init();
  
  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

protected:

private:

};

SCONESERVER_MODULE(ErrorPageModule);

//=========================================================================
ErrorPageModule::ErrorPageModule(
) : scx::Module("http:errorpage",scx::version())
{

}

//=========================================================================
ErrorPageModule::~ErrorPageModule()
{

}

//=========================================================================
std::string ErrorPageModule::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
         "HTTP error page handler\n";
}

//=========================================================================
int ErrorPageModule::init()
{
  return Module::init();
}

//=========================================================================
bool ErrorPageModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  ErrorPageStream* s = new ErrorPageStream(*this);
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}
