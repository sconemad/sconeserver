/* SconeServer (http://www.sconemad.com)

User

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

#include "sconex/User.h"
#include "sconex/Process.h"

namespace scx {

//=============================================================================
User User::current()
{
  return User(::getuid());
}
  
//=============================================================================
User::User()
  : m_user_id(0),
    m_group_id(0)
{
  DEBUG_COUNT_CONSTRUCTOR(User);
}

//=============================================================================
User::User(const std::string& user_name)
{
  DEBUG_COUNT_CONSTRUCTOR(User);
  set_user_name(user_name);
}

//=============================================================================
User::User(uid_t user_id)
{
  DEBUG_COUNT_CONSTRUCTOR(User);
  set_user_id(user_id);
}

//=============================================================================
User::~User()
{
  DEBUG_COUNT_DESTRUCTOR(User);
}

//=============================================================================
bool User::set_real()
{
  if (0 != ::setgid(m_group_id)) {
    return false;
  }
  if (0 != ::setuid(m_user_id)) {
    return false;
  }
  ::umask(0);
  return true;
}

//=============================================================================
bool User::set_effective()
{
  if (0 != ::setegid(m_group_id)) {
    return false;
  }
  if (0 != ::seteuid(m_user_id)) {
    return false;
  }
  ::umask(0);
  return true;
}

//=============================================================================
bool User::is_valid() const
{
  return (m_user_id > 0);
}
  
//=============================================================================
bool User::set_user_name(const std::string& user_name)
{
  struct passwd* pwent = getpwnam(user_name.c_str());
  if (!pwent) {
    return false;
  }
  return set_from_passwd(pwent);
}

//=============================================================================
bool User::set_user_id(uid_t user_id)
{
  struct passwd* pwent = getpwuid(user_id);
  if (!pwent) {
    return false;
  }
  return set_from_passwd(pwent); 
}

//=============================================================================
const std::string& User::get_user_name() const
{
  return m_user_name;
}
  
//=============================================================================
bool User::verify_password(const std::string& password) const
{
  return Process::verify_system_password(m_user_name,password);
}
  
//=============================================================================
uid_t User::get_user_id() const
{
  return m_user_id;
}

//=============================================================================
gid_t User::get_group_id() const
{
  return m_group_id;
}

//=============================================================================
bool User::set_from_passwd(const struct passwd* pwent)
{
  if (!pwent) {
    return false;
  }

  m_user_id = pwent->pw_uid;
  m_group_id = pwent->pw_gid;
  m_user_name = pwent->pw_name;

  return true;
}

};
