/* SconeServer (http://www.sconemad.com)

SconeScript standard context

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef ScriptContext_h
#define ScriptContext_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
#include <sconex/Provider.h>
namespace scx {

//===========================================================================
// StandardContext - This is the standard context used by default when 
// evaluating expressions.
//
class StandardContext : public ScriptObject, 
                        public ProviderScheme<ScriptObject> {
public:

  static ScriptRefTo<StandardContext>* get();

  // Register/unregister object types
  // Modules can use these methods to register object types that they 
  // provide, to make them available in the standard context.
  static void register_type(const std::string& type,
			    Provider<ScriptObject>* factory);
  static void unregister_type(const std::string& type,
			      Provider<ScriptObject>* factory);

  // Method to programatically create an object of one of the registered
  // standard types.
  static ScriptObject* create_object(const std::string& type,
				     const ScriptRef* args);
  
  virtual std::string get_string() const;

  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

private:

  StandardContext();
  ~StandardContext();

  // Singleton instance
  static ScriptRefTo<StandardContext>* s_inst;
};

//===========================================================================
// StandardTypeProvider - This registers the standard SconeScript types and
// provides constructors for them.
//
class StandardTypeProvider : public Provider<ScriptObject> {
public:

  static void init();
  
private:
  
  StandardTypeProvider();

  virtual void provide(const std::string& type,
		       const ScriptRef* args,
		       ScriptObject*& object);

  // Singleton instance
  static StandardTypeProvider* s_inst;
  
};

};
#endif
