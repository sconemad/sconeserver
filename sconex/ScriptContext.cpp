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

#include <sconex/ScriptContext.h>
#include <sconex/ScriptTypes.h>
#include <sconex/VersionTag.h>
#include <sconex/Uri.h>
#include <sconex/Date.h>
#include <sconex/MimeType.h>
#include <sconex/Digest.h>
#include <sconex/RegExp.h>
#include <sconex/utils.h>
namespace scx {

ScriptRefTo<StandardContext>* StandardContext::s_inst = 0;
StandardTypeProvider* StandardTypeProvider::s_inst = 0;

//===========================================================================
ScriptRefTo<StandardContext>* StandardContext::get()
{
  if (!s_inst) {
    s_inst = new ScriptRefTo<StandardContext>(new StandardContext());
    StandardTypeProvider::init();
  }
  return s_inst;
}
  
//===========================================================================
void StandardContext::register_type(const std::string& type,
				    Provider<ScriptObject>* factory)
{
  get()->object()->register_provider(type,factory);
}

//===========================================================================
void StandardContext::unregister_type(const std::string& type,
				      Provider<ScriptObject>* factory)
{
  get()->object()->unregister_provider(type,factory);
}

//===========================================================================
ScriptObject* StandardContext::create_object(const std::string& type,
					     const ScriptRef* args)
{
  return get()->object()->provide(type,args);
}
  
//===========================================================================
std::string StandardContext::get_string() const
{
  return "Standard";
}

//===========================================================================
ScriptRef* StandardContext::script_op(const ScriptAuth& auth,
				      const ScriptRef& ref,
				      const ScriptOp& op,
				      const ScriptRef* right)
{
  if (ScriptOp::Lookup == op.type() || 
      ScriptOp::Resolve == op.type()) {
    std::string name = right->object()->get_string();

    // Constants
    if ("true" == name) return ScriptBool::new_ref(true);
    if ("false" == name) return ScriptBool::new_ref(false);
    
    // Standard functions
    if ("defined" == name ||
	"ref" == name ||
	"constref" == name) {
      return new ScriptMethodRef(ref,name);
    }

    // Registered type constructors
    ProviderMap::iterator it = m_providers.find(name);
    if (it != m_providers.end()) {
      return new ScriptMethodRef(ref,name);
    }    
  }
  return ScriptObject::script_op(auth,ref,op,right);
}

//===========================================================================
ScriptRef* StandardContext::script_method(const ScriptAuth& auth,
					  const ScriptRef& ref,
					  const std::string& name,
					  const ScriptRef* args)
{
  const ScriptList* argl = dynamic_cast<const ScriptList*>(args->object());
  const ScriptRef* ar = argl->get(0);

  if ("defined" == name) {
    // Is the argument defined (i.e. its not NULL or an error)
    return ScriptBool::new_ref(!BAD_SCRIPTREF(ar));
  }

  if ("ref" == name) {
    // Return a reference to the argument
    if (ar) return ar->ref_copy(ScriptRef::Ref);
    return 0;
  }

  if ("constref" == name) {
    // Return a const reference to the argument
    if (ar) return ar->ref_copy(ScriptRef::ConstRef);
    return 0;
  }

  // Registered type constructor, call provide to create the object
  return new ScriptRef(provide(name,args));
}

//===========================================================================
StandardContext::StandardContext()
{
}

//===========================================================================
StandardContext::~StandardContext()
{
}

  
// --- StandardTypeProvider ---

  
//===========================================================================
void StandardTypeProvider::init()
{
  if (!s_inst) s_inst = new StandardTypeProvider();
}
  
//===========================================================================
StandardTypeProvider::StandardTypeProvider() 
{
  StandardContext::register_type("String",this);
  StandardContext::register_type("Bool",this);
  StandardContext::register_type("Int",this);
  StandardContext::register_type("Real",this);
  StandardContext::register_type("Error",this);
  StandardContext::register_type("VersionTag",this);
  StandardContext::register_type("Date",this);
  StandardContext::register_type("Time",this);
  StandardContext::register_type("TimeZone",this);
  StandardContext::register_type("Uri",this);
  StandardContext::register_type("MimeType",this);
  StandardContext::register_type("Digest",this);
#ifdef HAVE_LIBPCRE
  StandardContext::register_type("RegExp",this);
#endif
};

//===========================================================================
void StandardTypeProvider::provide(const std::string& type,
				   const ScriptRef* args,
				   ScriptObject*& object)
{
  if ("String" == type) {
    object = ScriptString::create(args);

  } else if ("Bool" == type) {
    object = ScriptBool::create(args);

  } else if ("Int" == type) {
    object = ScriptInt::create(args);

  } else if ("Real" == type) {
    object = ScriptReal::create(args);

  } else if ("Error" == type) {
    object = ScriptError::create(args);

  } else if ("VersionTag" == type) {
    object = new VersionTag(args);

  } else if ("Date" == type) {
    object = new Date(args);

  } else if ("Time" == type) {
    object = new Time(args);

  } else if ("TimeZone" == type) {
    object = new TimeZone(args);

  } else if ("Uri" == type) {
    object = new Uri(args);

  } else if ("MimeType" == type) {
    object = new MimeType(args);

  } else if ("Digest" == type) {
    const ScriptString* a_method = 
      get_method_arg<ScriptString>(args,0,"method");
    object = Digest::create(a_method ? a_method->get_string() : "",
                            args);

#ifdef HAVE_LIBPCRE
  } else if ("RegExp" == type) {
    object = new RegExp(args);
#endif
  }
};

};
