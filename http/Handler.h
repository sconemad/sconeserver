/* SconeServer (http://www.sconemad.com)

HTTP message handler

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

#ifndef httpHandler_h
#define httpHandler_h

#include <http/http.h>
#include <sconex/Provider.h>
#include <sconex/IOBase.h>
namespace http {

class MessageStream;
  
//=============================================================================
// Handler - Base HTTP message handler
//
class HTTP_API Handler {
public:

  // Create a handler of the specified type
  static Handler* create(const std::string& type,
                         scx::ScriptRef* args);
  
  virtual ~Handler();

  // message handler interface

  virtual scx::Condition handle_message(MessageStream* message) = 0;
  
  // Interface for registering new handlers
  static void register_handler(const std::string& type,
                               scx::Provider<Handler>* factory);
  static void unregister_handler(const std::string& type,
                                 scx::Provider<Handler>* factory);
  
protected:

  // Create a handler
  Handler();
  
private:

  static void init();

  static scx::ProviderScheme<Handler>* s_providers;
};

  
//=============================================================================
// HandlerMap - Connects an incoming HTTP request to a handler.
//
class HandlerMap {
public:

  HandlerMap(const std::string& type,
             scx::ScriptRef* args);
  
  ~HandlerMap();

  const std::string& get_type() const;
  
  scx::Condition handle_message(MessageStream* message);
  
private:
  std::string m_type;
  scx::ScriptRef* m_args;
};
  
};
#endif
