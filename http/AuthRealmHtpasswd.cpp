/* SconeServer (http://www.sconemad.com)

HTTP authentication using htpasswd files

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

//=============================================================================
AuthRealmHtpasswd::AuthRealmHtpasswd(HTTPModule* module,
				     const scx::FilePath& path)
  : AuthRealm(module),
    m_path(path),
    m_modtime(),
    m_mutex(),
    m_users()
{
  DEBUG_COUNT_CONSTRUCTOR(AuthRealmHtpasswd);
}

//=============================================================================
AuthRealmHtpasswd::~AuthRealmHtpasswd()
{
  DEBUG_COUNT_DESTRUCTOR(AuthRealmHtpasswd);
}

//=============================================================================
std::string AuthRealmHtpasswd::lookup_hash(const std::string& username)
{
  refresh();

  scx::MutexLocker locker(m_mutex);
  UserMap::const_iterator it = m_users.find(username);
  if (it == m_users.end()) return "";
  return it->second;
}

//=============================================================================
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
