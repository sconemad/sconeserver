/* SconeServer (http://www.sconemad.com)

HTTP Get file module

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


#include "http/HTTPModule.h"
#include "http/MessageStream.h"
#include "http/Request.h"
#include "http/Status.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/File.h"
#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"
#include "sconex/MimeType.h"

//=========================================================================
class GetFileStream : public scx::Stream {
public:

  GetFileStream(
    scx::Module& module
  ) : Stream("getfile"),
      m_module(module)
  {

  }

  ~GetFileStream()
  {

  }
  
protected:

  void log(const std::string message,
	   scx::Logger::Level level = scx::Logger::Info)
  {
    http::MessageStream* msg = GET_HTTP_MESSAGE();
    if (msg) {
      http::Request& req = const_cast<http::Request&>(msg->get_request());
      m_module.log(req.get_id() + " " + message,level);
    }
  };
  
  virtual scx::Condition event(scx::Stream::Event e) 
  {
    if (e == scx::Stream::Opening) {
      http::MessageStream* msg = GET_HTTP_MESSAGE();
      const http::Request& req = msg->get_request();
      http::Response& resp = msg->get_response();

      if (req.get_method() != "GET" && 
	  req.get_method() != "HEAD" ) {
	// Don't understand the method
	resp.set_status(http::Status::NotImplemented);
	return scx::Close;
      }

      // Open the file
      scx::FilePath path = req.get_path();
      scx::File* file = new scx::File();
      if (file->open(path,scx::File::Read) != scx::Ok) {
        log("Cannot open file '" + path.path() + "'"); 
        resp.set_status(http::Status::Forbidden);
        delete file;
	return scx::Close;
      } 

      // Find last modified date
      scx::Date lastmod = file->stat().time();
      resp.set_header("Last-Modified",lastmod.string());

      std::string mod = req.get_header("If-Modified-Since");
      if (!mod.empty()) {
	scx::Date dmod = scx::Date(mod);
	if (lastmod <= dmod) {
	  log("File is not modified"); 
	  resp.set_status(http::Status::NotModified);
	  delete file;
	  return scx::Close;
	}
      }

      resp.set_status(http::Status::Ok);

      // Set content length
      int clength = file->size();
      std::ostringstream oss;
      oss << clength;
      resp.set_header("Content-Length",oss.str());

      // Lookup MIME type for file
      scx::Module::Ref mime = scx::Kernel::get()->get_module("mime");
      if (mime.valid()) {
        scx::ScriptList::Ref args(new scx::ScriptList());
        args.object()->give( scx::ScriptString::new_ref(path.path()) );
	scx::ScriptMethodRef lookup_method(mime,"lookup");
        scx::ScriptRef* ret = 
	  lookup_method.call(scx::ScriptAuth::Untrusted,&args);
        scx::MimeType* mimetype = 0;
        if (ret && (mimetype = dynamic_cast<scx::MimeType*>(ret->object()))) {
          resp.set_header("Content-Type",mimetype->get_string());
        }
        delete ret;
      }

      if (req.get_method() == "HEAD") {
	// Don't actually send the file, just the headers
        log("Sending headers for '" + path.path() + "'"); 
	delete file;
	return scx::Close;
      }

      log("Sending '" + path.path() + "'"); 

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
class GetFileModule : public scx::Module,
		      public scx::Provider<scx::Stream> {
public:

  GetFileModule();
  virtual ~GetFileModule();

  virtual std::string info() const;

  virtual int init();
  
  // Provider<Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);

protected:

private:

};

SCONESERVER_MODULE(GetFileModule);

//=========================================================================
GetFileModule::GetFileModule(
) : scx::Module("http:getfile",scx::version())
{
  scx::Stream::register_stream("getfile",this);
}

//=========================================================================
GetFileModule::~GetFileModule()
{
  scx::Stream::unregister_stream("getfile",this);
}

//=========================================================================
std::string GetFileModule::info() const
{
  return "HTTP file transfer";
}

//=========================================================================
int GetFileModule::init()
{
  return Module::init();
}

//=========================================================================
void GetFileModule::provide(const std::string& type,
			    const scx::ScriptRef* args,
			    scx::Stream*& object)
{
  object = new GetFileStream(*this);
  object->add_module_ref(this);
}
