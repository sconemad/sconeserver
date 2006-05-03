/* SconeServer (http://www.sconemad.com)

Argument object

ArgObject is a transient Arg proxy class that can be used to refer to a more
longer-lived object (which must be of a class derived from ArgObjectInterface).
This mechanism can be used to allow an object to be accessed from SconeScript.

Also defined is ArgObjectFunction, which is an Arg class representing a
callable function on an object.

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

#ifndef scxArgObject_h
#define scxArgObject_h

#include "sconex/sconex.h"
#include "sconex/Arg.h"
#include "sconex/Logger.h"

namespace scx {

//=============================================================================
class SCONEX_API ArgObjectInterface {

public:

  ArgObjectInterface();

  virtual ~ArgObjectInterface();
  
  virtual std::string name() const =0;
  // Get the name of this object

  virtual void log(
    const std::string& message,
    Logger::Level level = Logger::Info
  );
  // Log message against this object
    
  virtual Arg* arg_lookup(const std::string& name);
  // Lookup sub element within this object

  virtual Arg* arg_resolve(const std::string& name);
  // Resolve name by recursing down the tree
  
  virtual Arg* arg_function(const std::string& name, Arg* args);
  // Call named arg function on this object

protected:

};

//=============================================================================
class SCONEX_API ArgObject : public Arg {

public:
  
  ArgObject(ArgObjectInterface* obj);
  ArgObject(const ArgObject& c);
  virtual ~ArgObject();
  virtual Arg* new_copy() const;
  
  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

  void log(
    const std::string& message,
    Logger::Level level = Logger::Info
  );
  
  ArgObjectInterface* get_object();
   
protected:

  ArgObjectInterface* m_obj;
  
};

//=============================================================================
class SCONEX_API ArgObjectFunction : public Arg {

public:
  
  ArgObjectFunction(
    ArgObject* obj,
    const std::string& name
  );
  ArgObjectFunction(const ArgObjectFunction& c);
  virtual ~ArgObjectFunction();
  virtual Arg* new_copy() const;

  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual Arg* op(OpType optype, const std::string& opname, Arg* right);

protected:

  ArgObject* m_obj;
  std::string m_name;
  
};
  
};
#endif
