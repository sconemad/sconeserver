/* SconeServer (http://www.sconemad.com)

HTTP request handler

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


#include <http/Handler.h>
namespace http {

scx::ProviderScheme<Handler>* Handler::s_providers = 0;

//=========================================================================
Handler* Handler::create(const std::string& type,
                         scx::ScriptRef* args)
{
  init();
  return s_providers->provide(type, args);
}

//=========================================================================
Handler::~Handler()
{
  DEBUG_COUNT_DESTRUCTOR(Handler);
}

//=============================================================================
void Handler::register_handler(const std::string& type,
                               scx::Provider<Handler>* factory)
{
  init();
  s_providers->register_provider(type,factory);
}

//=============================================================================
void Handler::unregister_handler(const std::string& type,
                                 scx::Provider<Handler>* factory)
{
  init();
  s_providers->unregister_provider(type,factory);
}

//=============================================================================
Handler::Handler()
{
}

//=============================================================================
void Handler::init()
{
  if (!s_providers) {
    s_providers = new scx::ProviderScheme<Handler>();
  }
}

  
//=========================================================================
HandlerMap::HandlerMap(
  const std::string& type,
  scx::ScriptRef* args
) : m_type(type),
    m_args(args)
{
}
  
//=========================================================================
HandlerMap::~HandlerMap()
{
  delete m_args;
}

//=========================================================================
const std::string& HandlerMap::get_type() const
{
  return m_type;
}
  
//=========================================================================
bool HandlerMap::handle_message(MessageStream* message)
{
  Handler* handler = Handler::create(m_type, m_args);
  if (!handler) {
    DEBUG_LOG("Failed to create handler of type " << m_type <<
	      " args: " <<
              (m_args ? m_args->object()->get_string() : "NULL"));
    return false;
  }

  handler->handle_message(message);
  delete handler;
  return true;
}
  
};
