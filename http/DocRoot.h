/* SconeServer (http://www.sconemad.com)

http Document root

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef httpDocRoot_h
#define httpDocRoot_h

#include "http/HTTPModule.h"
#include "sconex/ArgObject.h"
namespace http {

class HostMapper;
class MessageStream;
  
//=============================================================================
class HTTP_API DocRoot : public scx::ArgObjectInterface {
public:

  DocRoot(
    HTTPModule& module,
    Host& host,
    const std::string profile,
    const scx::FilePath& path
  );
  
  // Create a docroot for specified profile and root path

  virtual ~DocRoot();

  const std::string get_profile() const;

  bool connect_request(scx::Descriptor* endpoint, MessageStream& message);

  std::string lookup_mod(const std::string& name) const;
  
  const scx::Arg* get_param(const std::string& name) const;
  void set_param(const std::string& name, scx::Arg* value);

  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_function(const std::string& name,scx::Arg* args);
  
protected:

private:

  HTTPModule& m_module;
  Host& m_host;
  std::string m_profile;
  
  scx::FilePath m_path;

  std::map<std::string,std::string> m_mods;
  
  std::map<std::string,scx::Arg*> m_params;
};

};
#endif
