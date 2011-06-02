/* SconeServer (http://www.sconemad.com)

HTTP Authorisation using htpasswd files

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

#ifndef httpAuthRealmHtpasswd_h
#define httpAuthRealmHtpasswd_h

#include <http/AuthRealm.h>
namespace http {

//=============================================================================
// AuthRealmHtpasswd - An authorisation realm using htpasswd files.
//
class HTTP_API AuthRealmHtpasswd : public AuthRealm {
public:

  AuthRealmHtpasswd(HTTPModule* module,
		    const scx::FilePath& path);

  virtual ~AuthRealmHtpasswd();

  // AuthRealm methods  
  virtual scx::ScriptRef* authorised(const std::string& username,
				     const std::string& password);

protected:

  void refresh();

private:

  scx::ScriptRefTo<HTTPModule> m_module;
  scx::FilePath m_path;
  scx::Date m_modtime;
  scx::Mutex m_mutex;

  typedef std::map<std::string,std::string> UserMap;
  UserMap m_users;
};

};
#endif
