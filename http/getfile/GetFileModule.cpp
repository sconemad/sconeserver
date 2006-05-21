/* SconeServer (http://www.sconemad.com)

HTTP Get file module

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
#include "http/MessageStream.h"
#include "http/Request.h"
#include "http/Status.h"
#include "http/FSNode.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/File.h"
#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"

//=========================================================================
class GetFileStream : public scx::Stream {

public:

  GetFileStream(
    scx::Module& module
  ) : Stream("http:getfile"),
      m_module(module)
  {

  }

  ~GetFileStream()
  {

  }
  
protected:

  virtual scx::Condition event(scx::Stream::Event e) 
  {
    if (e == scx::Stream::Opening) {
      http::MessageStream* msg = 
	dynamic_cast<http::MessageStream*>(find_stream("http:message"));
      const http::Request& req = msg->get_request();

      if (req.get_method() != "GET" && 
	  req.get_method() != "HEAD" ) {
	// Don't understand the method
	msg->set_status(http::Status::NotImplemented);
	return scx::Close;
      }

      // Open the file
      scx::FilePath path = msg->get_node()->path();
      scx::File* file = new scx::File();
      if (file->open(path,scx::File::Read) != scx::Ok) {
        m_module.log("Cannot open file '" + path.path() + "'"); 
        msg->set_status(http::Status::Forbidden);
        write("<html>\n");
        write("<head><title>Forbidden</title></head>\n");
        write("<body><h1>Forbidden</h1></body>\n");
        write("</html>\n");
        delete file;
	return scx::Close;
      } 

      // Find last modified date
      scx::Date lastmod = file->stat().time();
      msg->set_header("Last-Modified",lastmod.string());

      std::string mod = req.get_header("If-Modified-Since");
      if (!mod.empty()) {
	scx::Date dmod = scx::Date(mod);
	if (lastmod <= dmod) {
	  m_module.log("File is not modified"); 
	  msg->set_status(http::Status::NotModified);
	  delete file;
	  return scx::Close;
	}
      }

      msg->set_status(http::Status::Ok);

      // Set content length
      int clength = file->size();
      std::ostringstream oss;
      oss << clength;
      msg->set_header("Content-Length",oss.str());

      // Lookup MIME type for file
      std::string cmd = "mime.lookup(\"" + path.path() + "\")";
      scx::Arg* ret = m_module.arg_eval(cmd);
      if (ret) {
	msg->set_header("Content-Type",ret->get_string());
	delete ret;
      }

      if (req.get_method() == "HEAD") {
	// Don't actually send the file, just the headers
	delete file;
	return scx::Close;
      }

      m_module.log("Sending '" + path.path() + "'"); 

      const int MAX_BUFFER_SIZE = 65536;
            
      scx::StreamTransfer* xfer =
        new scx::StreamTransfer(file,std::min(clength,MAX_BUFFER_SIZE));
      xfer->set_close_when_finished(true);
      endpoint().add_stream(xfer);
      
      // Add file to kernel
      scx::Kernel::get()->connect(file,0);
    }
    
    return scx::Ok;
  };

private:

  scx::Module& m_module;

};


//=========================================================================
class GetFileModule : public scx::Module {
public:

  GetFileModule();
  virtual ~GetFileModule();

  virtual std::string info() const;

  virtual int init();
  
  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

protected:

private:

};

SCONESERVER_MODULE(GetFileModule);

//=========================================================================
GetFileModule::GetFileModule(
) : scx::Module("http:getfile",scx::version())
{

}

//=========================================================================
GetFileModule::~GetFileModule()
{

}

//=========================================================================
std::string GetFileModule::info() const
{
  return "Copyright (c) 2000-2005 Andrew Wedgbury\n"
         "HTTP file transfer\n";
}

//=========================================================================
int GetFileModule::init()
{
  return Module::init();
}

//=========================================================================
bool GetFileModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  GetFileStream* s = new GetFileStream(*this);
  s->add_module_ref(ref());
  
  endpoint->add_stream(s);
  return true;
}
