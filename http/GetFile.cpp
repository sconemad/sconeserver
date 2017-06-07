/* SconeServer (http://www.sconemad.com)

HTTP Get file

Copyright (c) 2000-2016 Andrew Wedgbury <wedge@sconemad.com>

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


#include <http/GetFile.h>
#include <http/MessageStream.h>
#include <http/Request.h>
#include <http/Status.h>

#include <sconex/File.h>
#include <sconex/Stream.h>
#include <sconex/StreamTransfer.h>
#include <sconex/Date.h>
#include <sconex/Kernel.h>
#include <sconex/MimeType.h>
#include <sconex/GzipStream.h>
namespace http {

//=========================================================================
scx::Condition GetFileHandler::handle_message(MessageStream* message)
{
  const Request& request = message->get_request();
  Response& response = message->get_response();

  if (request.get_method() != "GET" && 
      request.get_method() != "HEAD" ) {
    // Don't understand the method
    response.set_status(http::Status::NotImplemented);
    return scx::Close;
  }
  
  // Open the file
  scx::FilePath path = request.get_path();
  scx::File* file = new scx::File();
  if (file->open(path,scx::File::Read) != scx::Ok) {
    message->log("Cannot open file '" + path.path() + "'"); 
    response.set_status(http::Status::Forbidden);
    delete file;
    return scx::Close;
  } 
  
  // Find last modified date
  scx::Date lastmod = file->stat().time();
  response.set_header("Last-Modified",lastmod.string());
  
  std::string mod = request.get_header("If-Modified-Since");
  if (!mod.empty()) {
    scx::Date dmod = scx::Date(mod);
    if (lastmod <= dmod) {
      message->log("File is not modified"); 
      response.set_status(http::Status::NotModified);
      delete file;
      return scx::Close;
    }
  }
  
  response.set_status(http::Status::Ok);
  
  // Set content length
  int clength = file->size();
  std::ostringstream oss;
  oss << clength;
  response.set_header("Content-Length",oss.str());
  
  // Lookup MIME type for file
  bool compressible = false;
  scx::Module::Ref mime = scx::Kernel::get()->get_module("mime");
  if (mime.valid()) {
    scx::ScriptList::Ref args(new scx::ScriptList());
    args.object()->give( scx::ScriptString::new_ref(path.path()) );
    scx::ScriptMethodRef lookup_method(mime,"lookup");
    scx::MimeType* mimetype = 0;
    scx::ScriptRef* ret = 
      lookup_method.call(scx::ScriptAuth::Untrusted,&args);
    if (ret && (mimetype = dynamic_cast<scx::MimeType*>(ret->object()))) {
      response.set_header("Content-Type",mimetype->get_string());
    }
    // Decide if the file is compressible, this will be used to
    // determine whether to use gzip to send it.
    compressible = (mimetype->get_type() == "text");
    delete ret;
  }
  
  if (request.get_method() == "HEAD") {
    // Don't actually send the file, just the headers
    message->log("GetFile headers for '" + path.path() + "'"); 
    delete file;
    return scx::Close;
  }

  message->log("GetFile '" + path.path() + "'");

  // Decide whether to use gzip
  // This is based on whether the file is compressible (text)
  // and if it is over a certain size (i.e. worth compressing)
  if (compressible && clength > 1000) {
    // Also check if the client accepts gzip encoding
    bool gzip_accepted = false;
    scx::MimeHeader ae = request.get_header_parsed("Accept-Encoding");
    for (int i=0; i<ae.num_values(); ++i) {
      if (ae.get_value(i)->value() == "gzip") {
        gzip_accepted = true;
        break;
      }
    }
    if (gzip_accepted) {
      message->log("Using gzip");
      message->add_stream(new scx::GzipStream(0,16384));
      response.set_header("Content-Encoding","gzip");
      // Unfortunately we have to remove the content-length, so 
      // chunked encoding will be used for gzipped content.
      response.remove_header("Content-Length");
    }
  }
  
  const int MAX_BUFFER_SIZE = 65536;
  
  scx::StreamTransfer* xfer =
    new scx::StreamTransfer(file,std::min(clength,MAX_BUFFER_SIZE));
  xfer->set_close_when_finished(true);
  message->add_stream(xfer);
  
  // Add file to kernel
  scx::Kernel::get()->connect(file);
  return scx::Ok;
}

};
