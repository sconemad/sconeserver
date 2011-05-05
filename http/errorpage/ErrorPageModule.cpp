/* SconeServer (http://www.sconemad.com)

Error Page module

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
#include "http/Request.h"
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/Stream.h"
#include "sconex/StreamTransfer.h"
#include "sconex/Kernel.h"
#include "sconex/LineBuffer.h"

//=========================================================================
class VarSubstStream : public scx::LineBuffer {

public:
  VarSubstStream(http::MessageStream* msg)
    : scx::LineBuffer("http:errorpage:varsubst"),
      m_pos(0),
      m_msg(msg)
  {
    m_vars["status"] = m_msg->get_response().get_status().string();
    m_vars["method"] = m_msg->get_request().get_method();
    m_vars["uri"] = m_msg->get_request().get_uri().get_string();
    m_vars["httpversion"] = m_msg->get_request().get_version().get_string();
  };
  
  virtual ~VarSubstStream()
  {
  };

  virtual scx::Condition read(void* buffer,int n,int& na)
  {
    scx::Condition c = scx::Ok;
    if (m_pos == 0) {
      c = tokenize(m_line);
      if (c == scx::Ok) m_line += "\n";
      std::string::size_type start=0;
      std::string::size_type end=0;
      while (std::string::npos != (start = m_line.find("<?",end))) {
        end = m_line.find("?>",start);
        int toklen = end-start-2;
        std::string tok = std::string(m_line,start+2,toklen);
        std::string rep;
        std::map<std::string,std::string>::const_iterator iter = m_vars.find(tok);
        if (iter!=m_vars.end()) {
          rep = (*iter).second;
        }
        m_line.erase(start,toklen+4);
        m_line.insert(start,rep);
        end = start - toklen - 4 + rep.size();
      }
    }

    na = std::min(n,(int)m_line.size() - m_pos);
    memcpy(buffer,m_line.c_str()+m_pos,na);
    m_pos += n;
    if (m_pos >= (int)m_line.size()) m_pos = 0;
    
    return c;
  };
  
private:
  
  std::string m_line;
  int m_pos;
  http::MessageStream* m_msg;
  std::map<std::string,std::string> m_vars;
  
};

//=========================================================================
class ErrorPageStream : public scx::Stream {

public:

  ErrorPageStream(
    scx::Module& module
  ) : Stream("errorpage"),
      m_module(module),
      m_file_mode(false)
  {

  }

  ~ErrorPageStream()
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
      const http::Status& status = msg->get_response().get_status();
      
      if (req.get_method() != "GET" && 
	  req.get_method() != "HEAD" &&
	  req.get_method() != "POST" ) {
	// Don't understand the method
	return scx::Close;
      }
 
      bool body_allowed = (status.code() >= 200 &&
                           status.code() != 204 &&
                           status.code() != 304);

      if (body_allowed) {
        msg->get_response().set_header("Content-Type","text/html");
      }
      if (body_allowed &&
          (req.get_method() == "GET" || req.get_method() == "POST")) {
	// Only need to send the message body if method is GET

        /*
        const http::FSDirectory* dir = msg->get_dir_node();
        const scx::Arg* a_error_page = dir->get_param("error_page");
        if (a_error_page) {
          std::string s_error_page = a_error_page->get_string();
          scx::FilePath path = dir->path() + s_error_page;
          
          scx::File* file = new scx::File();
          if (file->open(path,scx::File::Read) == scx::Ok) {
            m_file_mode = true;

            m_module.log("Sending '" + status.string() +
                         "' response using template file mode"); 
            
            VarSubstStream* varsubst = new VarSubstStream(msg);
            file->add_stream(varsubst);
            
            int clength = file->size();
            const int MAX_BUFFER_SIZE = 65536;
            
            scx::StreamTransfer* xfer =
              new scx::StreamTransfer(file,std::min(clength,MAX_BUFFER_SIZE));
            xfer->set_close_when_finished(true);
            endpoint().add_stream(xfer);
            
            // Add file to kernel
            scx::Kernel::get()->connect(file,0);
          } else {
            delete file;
          }
        }
        */
        if (m_file_mode == false) {
          log("Sending '" + status.string() + "' response using basic mode"); 
          enable_event(scx::Stream::Writeable,true);
        }
        
      } else {
        log("Sending '" + status.string() + "' header-only response");
        return scx::Close;
      }
    }
    
    if (e == scx::Stream::Writeable) {

      if (!m_file_mode) {
        send_basic_page();
        return scx::Close;
      }
    }
  
    return scx::Ok;
  };

  void send_basic_page()
  {
    http::MessageStream* msg = GET_HTTP_MESSAGE();
    const http::Request& req = msg->get_request();
    const http::Status& status = msg->get_response().get_status();

    std::ostringstream oss;
    oss << "<html>"
        << "<head>"
        << "<title>"
        << status.string()
        << "</title>"
        << "</head>"
        << "<body>"
        << "<h1>"
        << status.string()
        << "</h1>";
    
    switch (status.code()) {
      
      case http::Status::Found:
        oss << "<p>"
            << "Redirecting to '<em>"
            << msg->get_response().get_header("Location")
            << "</em>'"
            << "</p>\n";
        break;
        
      case http::Status::NotFound:
        oss << "<p>"
            << "The requested page '<em>"
            << req.get_uri().get_path()
            << "</em>' could not be found on this server."
            << "</p>\n";
        break;
        
      default:
        break;
    }
    
    oss << "</body>"
        << "</html>";
    
    Stream::write(oss.str());
  };
  
private:
    
  scx::Module& m_module;

  bool m_file_mode;
  
};


//=========================================================================
class ErrorPageModule : public scx::Module,
			public scx::Provider<scx::Stream> {
public:

  ErrorPageModule();
  virtual ~ErrorPageModule();

  virtual std::string info() const;

  virtual int init();
  
  // Provider<Stream> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::Stream*& object);

protected:

private:

};

SCONESERVER_MODULE(ErrorPageModule);

//=========================================================================
ErrorPageModule::ErrorPageModule(
) : scx::Module("http:errorpage",scx::version())
{
  scx::Stream::register_stream("errorpage",this);
}

//=========================================================================
ErrorPageModule::~ErrorPageModule()
{
  scx::Stream::unregister_stream("errorpage",this);
}

//=========================================================================
std::string ErrorPageModule::info() const
{
  return "HTTP error page handler";
}

//=========================================================================
int ErrorPageModule::init()
{
  return Module::init();
}

//=========================================================================
void ErrorPageModule::provide(const std::string& type,
			      const scx::ScriptRef* args,
			      scx::Stream*& object)
{
  object = new ErrorPageStream(*this);
  object->add_module_ref(this);
}
