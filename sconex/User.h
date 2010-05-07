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

#ifndef scxUser_h
#define scxUser_h

#include "sconex/sconex.h"

#include <pwd.h>

namespace scx {

//=============================================================================
class SCONEX_API User {

public:

  static User current();
  // Get the current user
  
  User();
  User(const std::string& user_name);
  User(uid_t user_id); 

  ~User();

  bool set_real();
  // Set real user ID for this process
  
  bool set_effective();
  // Set effective user ID for this process
  
  bool is_valid() const;
  
  bool set_user_name(const std::string& user_name);
  bool set_user_id(uid_t user_id);

  const std::string& get_user_name() const;

  bool verify_password(const std::string& password) const;
  
  uid_t get_user_id() const;
  gid_t get_group_id() const;
  
protected:

private:

  bool set_from_passwd(const struct passwd* pwent);
  
  std::string m_user_name;

  uid_t m_user_id;
  gid_t m_group_id;
};

};
#endif
