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


#include <http/AuthRealmHtpasswd.h>
#include <http/HTTPModule.h>
#include <sconex/ScriptTypes.h>
#include <sconex/LineBuffer.h>
#include <sconex/User.h>
#include <sconex/File.h>

namespace http {

//=========================================================================
AuthRealmHtpasswd::AuthRealmHtpasswd(HTTPModule* module,
				     const scx::FilePath& path)
  : AuthRealm(""),
    m_module(module),
    m_path(path)
{
  DEBUG_COUNT_CONSTRUCTOR(AuthRealmHtpasswd);
  m_parent = module;
}

//=========================================================================
AuthRealmHtpasswd::~AuthRealmHtpasswd()
{
  DEBUG_COUNT_DESTRUCTOR(AuthRealmHtpasswd);
}

//=========================================================================
scx::ScriptRef* AuthRealmHtpasswd::authorised(const std::string& username,
					      const std::string& password)
{
  refresh();

  scx::MutexLocker locker(m_mutex);
  UserMap::const_iterator it = m_users.find(username);
  if (it == m_users.end())
    return 0;
  std::string pwentry = it->second;
  locker.unlock();

  bool auth = false;
  if (pwentry == "!system") { 
    // Use system method
    auth = scx::User(username).verify_password(password);
    
  } else {
    // Use crypt method
#ifdef HAVE_CRYPT_R
    struct crypt_data data;
    memset(&data,0,sizeof(data));
    data.initialized = 0;
    std::string check = crypt_r(password.c_str(),
				pwentry.c_str(),
				&data);
#else
    //NOTE: crypt_r not available:
    std::string check = crypt(password.c_str(), pwentry.c_str());
#endif
    auth = (check == pwentry);
  }
  
  if (auth) {
    return scx::ScriptInt::new_ref(1);
  }
  return 0;
}

//=========================================================================
void AuthRealmHtpasswd::refresh()
{
  scx::FilePath path(m_path);
  scx::FileStat stat(path);
  if (stat.is_file()) {
    if (m_modtime != stat.time()) {
      scx::MutexLocker locker(m_mutex);

      m_users.clear();
      scx::File file;
      if (scx::Ok == file.open(path,scx::File::Read)) {
	scx::LineBuffer* tok = new scx::LineBuffer("");
	file.add_stream(tok);
	std::string line;
	while (scx::Ok == tok->tokenize(line)) {
	  std::string::size_type i = line.find_first_of(":");
	  std::string username;
	  std::string password;
	  username = line.substr(0,i);
          ++i;
	  password = line.substr(i);
	  if (!username.empty() && !password.empty()) {
	    m_users[username] = password;
	  }
	}
	file.close();
	m_modtime = stat.time();
      }
    }
  }
}

};
