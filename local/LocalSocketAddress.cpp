/* SconeServer (http://www.sconemad.com)

Socket address for Local (UNIX domain) protocol

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

#include "LocalSocketAddress.h"
#include "sconex/FileStat.h"
#include "sconex/FilePath.h"
#include "sconex/ScriptTypes.h"

// Some platforms still use UNIX instead of LOCAL
#ifndef PF_LOCAL
#  define PF_LOCAL PF_UNIX
#endif
#ifndef AF_LOCAL
#  define AF_LOCAL AF_UNIX
#endif
    
//=============================================================================
LocalSocketAddress::LocalSocketAddress(const scx::ScriptRef* args)
  : scx::SocketAddress(PF_LOCAL,SOCK_STREAM)
{
  DEBUG_COUNT_CONSTRUCTOR(LocalSocketAddress);
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.sun_family = AF_LOCAL;

  const scx::ScriptString* a_path =
    scx::get_method_arg<scx::ScriptString>(args,0,"path");
  if (a_path) {
    set_path(a_path->get_string());
  }

}

//=============================================================================
LocalSocketAddress::LocalSocketAddress(const LocalSocketAddress& c)
  : scx::SocketAddress(c)
{
  DEBUG_COUNT_CONSTRUCTOR(LocalSocketAddress);
  memcpy(&m_addr,&c.m_addr,sizeof(m_addr));
}

//=============================================================================
LocalSocketAddress::~LocalSocketAddress()
{
  DEBUG_COUNT_DESTRUCTOR(LocalSocketAddress);
}

//=============================================================================
scx::ScriptObject* LocalSocketAddress::new_copy() const
{
  return new LocalSocketAddress(*this);
}

//=============================================================================
bool LocalSocketAddress::valid_for_bind() const
{
  return true;
}

//=============================================================================
bool LocalSocketAddress::valid_for_connect() const
{
  return true;
}

//=============================================================================
void LocalSocketAddress::set_sockaddr(const struct sockaddr* sa)
{
  DEBUG_ASSERT(sa!=0,"Invalid sockaddr pointer");
  DEBUG_ASSERT(sa->sa_family == m_domain,"Socket domain mismatch");

  memcpy(&m_addr,sa,sizeof(m_addr));
}

//=============================================================================
const struct sockaddr* LocalSocketAddress::get_sockaddr() const
{
  return (const struct sockaddr*)&m_addr;
}

//=============================================================================
socklen_t LocalSocketAddress::get_sockaddr_size() const
{
  return sizeof(m_addr);
}

//=============================================================================
int LocalSocketAddress::socket_bind(SOCKET s) const
{
  scx::FileStat fs(get_path());
  if (fs.exists()) {
    if (fs.is_file() || fs.is_dir()) {
      return 1;
    }
    unlink(get_path().c_str());
  }
  
  return SCXBASE SocketAddress::socket_bind(s);
}

//=============================================================================
std::string LocalSocketAddress::get_string() const
{
  return std::string("local:") + get_path();
}

//=============================================================================
scx::ScriptRef* LocalSocketAddress::script_op(const scx::ScriptAuth& auth,
					      const scx::ScriptRef& ref,
					      const scx::ScriptOp& op,
					      scx::ScriptRef* right)
{
  if (scx::ScriptOp::Lookup == op.type()) {
    std::string name = right->object()->get_string();
    if (name == "family") return scx::ScriptString::new_ref("local");
    if (name == "path") return scx::ScriptString::new_ref(get_path());
  }

  return scx::SocketAddress::script_op(auth,ref,op,right);
}

//=============================================================================
bool LocalSocketAddress::set_path(const std::string& path)
{
  unsigned int max = sizeof(m_addr.sun_path);
  if (path.length() >= max) {
    DEBUG_LOG("set_path() Path too long for sockaddr_un structure");
    return false;
  }
  strncpy(m_addr.sun_path,path.c_str(),max-1);
  return true;
}

//=============================================================================
std::string LocalSocketAddress::get_path() const
{
  return std::string(m_addr.sun_path);
}
