/* SconeServer (http://www.sconemad.com)

http Host

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpHost_h
#define httpHost_h

#include <http/http.h>
#include <http/DocRoot.h>
#include <sconex/ScriptBase.h>
#include <sconex/FilePath.h>

namespace scx { class Descriptor; }

namespace http {

class HTTPModule;
class HostMapper;
class Request;
class Response;
  
//=============================================================================
// Host - An HTTP host configuration
//
class HTTP_API Host : public scx::ScriptObject {
public:

  // Create a host with id and dir
  Host(
    HTTPModule& module,
    HostMapper& mapper,
    const std::string id,
    const std::string hostname,
    const scx::FilePath& dir
  );

  virtual ~Host();

  int init();

  // Process an incoming connection request
  bool connect_request(scx::Descriptor* endpoint,
		       Request& request,
		       Response& response);
  
  const std::string get_id() const;
  const std::string get_hostname() const;
  const scx::FilePath& get_path() const;

  DocRoot* get_docroot(const std::string& profile);

  DocRoot::Ref add_docroot(const std::string& profile,
                           const scx::FilePath& path);

  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);
  
  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

  typedef scx::ScriptRefTo<Host> Ref;
  
protected:

private:

  HTTPModule& m_module;
  HostMapper& m_mapper;
  std::string m_id;
  std::string m_hostname;
  scx::FilePath m_dir;

  typedef std::map<std::string,DocRoot::Ref*> DocRootMap;
  DocRootMap m_docroots;

};

};
#endif
