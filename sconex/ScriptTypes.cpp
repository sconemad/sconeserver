/* SconeServer (http://www.sconemad.com)

SconeScript basic types

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

#include <sconex/ScriptTypes.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptExpr.h>
#include <sconex/ScriptStatement.h>
#include <IOBase.h>
#include <sconex/utils.h>

namespace scx {

// ### ScriptObjectSorter ##

//=========================================================================
class ScriptObjectSorter {
public:

  ScriptObjectSorter(const std::string& predicate) 
    : m_predicate(predicate) { };

  bool operator()(const ScriptRef* a, 
		  const ScriptRef* b)
  {
    ScriptMap* ctx = new ScriptMap();
    ScriptRef ctx_ref(ctx);
    ctx->give("a",const_cast<ScriptRef*>(a)->ref_copy(ScriptRef::ConstRef));
    ctx->give("b",const_cast<ScriptRef*>(b)->ref_copy(ScriptRef::ConstRef));

    ScriptExpr proc(ScriptAuth::Untrusted,&ctx_ref);
    ScriptRef* result = proc.evaluate(m_predicate);

    bool val = BAD_SCRIPTREF(result) ? false : result->object()->get_int();
    delete result;
    return val;
  };

private:

  std::string m_predicate;

};

// ### ScriptString ###

//===========================================================================
ScriptRef* ScriptString::new_ref(const std::string& str)
{
  return new ScriptRef(new ScriptString(str));
}

//===========================================================================
ScriptString::ScriptString(const char* str)
  : m_string(str)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptString);
}

//===========================================================================
ScriptString::ScriptString(const std::string& str)
  : m_string(str)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptString);
}

//===========================================================================
ScriptString::ScriptString(const ScriptString& c)
  : ScriptObject(c),
    m_string(c.m_string)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptString);
}

//===========================================================================
ScriptString::~ScriptString()
{
  DEBUG_COUNT_DESTRUCTOR(ScriptString);
}

//===========================================================================
ScriptObject* ScriptString::create(const ScriptRef* args)
{
  const ScriptRef* value = get_method_arg_ref(args,0,"value");
  return new ScriptString(value ? value->object()->get_string() : "");
}

//===========================================================================
ScriptObject* ScriptString::new_copy() const
{
  return new ScriptString(*this);
}

//===========================================================================
std::string ScriptString::get_string() const
{
  return m_string;
}

//===========================================================================
int ScriptString::get_int() const
{
  return !m_string.empty();
}

//===========================================================================
ScriptRef* ScriptString::script_op(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const ScriptOp& op,
				   const ScriptRef* right)
{
  switch (op.type()) {
    
  case ScriptOp::Add: // Concatenate
    return ScriptString::new_ref(m_string + right->object()->get_string());
    
  case ScriptOp::Equality:
    return ScriptInt::new_ref(m_string == right->object()->get_string());
      
  case ScriptOp::Inequality:
    return ScriptInt::new_ref(m_string != right->object()->get_string());

  case ScriptOp::GreaterThan:
      return ScriptInt::new_ref(m_string > right->object()->get_string());

  case ScriptOp::LessThan:
    return ScriptInt::new_ref(m_string < right->object()->get_string());

  case ScriptOp::Assign:
    if (!ref.is_const()) {
      m_string = right->object()->get_string();
    }
    return ref.ref_copy();

  case ScriptOp::AddAssign: // Append
    if (!ref.is_const()) {
      m_string += right->object()->get_string();
    }
    return ref.ref_copy();

  case ScriptOp::Lookup: 
    {
      std::string name = right->object()->get_string();
      if ("length" == name) return ScriptInt::new_ref(m_string.size());
      if ("empty" == name) return ScriptInt::new_ref(m_string.empty());
      
      if ("clear" == name ||
	  "split" == name ||
	  "uc" == name ||
	  "lc" == name) {
	return new ScriptMethodRef(ref,name);
      }
    } break;
    
    default: break;
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//===========================================================================
ScriptRef* ScriptString::script_method(const ScriptAuth& auth,
				       const ScriptRef& ref,
				       const std::string& name,
				       const ScriptRef* args)
{
  if ("clear" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");
    m_string = "";
    return 0;
  }
  
  if ("split" == name) {
    const ScriptString* a_pat = 
      get_method_arg<ScriptString>(args,0,"pattern");
    if (!a_pat) 
      return ScriptError::new_ref("ScriptString::split() No pattern specified");   
    std::string pat = a_pat->get_string();
    if (pat.empty())
      return ScriptError::new_ref("ScriptString::split() Empty pattern");
    
    ScriptList* list = new ScriptList();
    std::string::size_type start = 0;
    while (true) {
      std::string::size_type end = m_string.find(pat,start);
      std::string sub;
      if (end == std::string::npos) {
	sub = m_string.substr(start);
      } else {
	sub = m_string.substr(start,end-start);
      }
      if (sub.length() > 0) {
	list->give(ScriptString::new_ref(sub));
      }
      start = end + pat.length();
      if (end == std::string::npos) {
	break;
      }
    }
    return new ScriptRef(list);
  }
  
  if ("uc" == name) {
    std::string uc = m_string;
    strup(uc);
    return ScriptString::new_ref(uc);
  }
  
  if ("lc" == name) {
    std::string lc = m_string;
    strlow(lc);
    return ScriptString::new_ref(lc);
  }
 
  return ScriptObject::script_method(auth,ref,name,args);
}

//===========================================================================
void ScriptString::serialize(IOBase& output) const
{
  output.write("\"" + escape_quotes(get_string()) + "\"");
}


// ### ScriptNum ###

//===========================================================================
double ScriptNum::get_real() const
{
  return (double)get_int();
}


// ### ScriptInt ###

//===========================================================================
ScriptRef* ScriptInt::new_ref(int value)
{
  return new ScriptRef(new ScriptInt(value));
}

//===========================================================================
ScriptInt::ScriptInt(long value)
  : m_value(value)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptInt);
}

//===========================================================================
ScriptInt::ScriptInt(const ScriptInt& c)
  : ScriptNum(c),
    m_value(c.m_value)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptInt);
}

//===========================================================================
ScriptInt::~ScriptInt()
{
  DEBUG_COUNT_DESTRUCTOR(ScriptInt);
}

//===========================================================================
ScriptObject* ScriptInt::from_string(const std::string& str, int base)
{
  const char* cs = str.c_str();
  if (str.length() >= 2 && str[0] == '0') {
    char t = str[1];
    if (isdigit(t)) {
      base = 8;
      cs += 1;
    } else {
      switch (t) {
      case 'b': base = 2; break;
      case 't': base = 3; break;
      case 'o': base = 8; break;
      case 'd': base = 10; break;
      case 'x': base = 16; break;
      default: 
	return new ScriptError("Unknown base");
      }
      cs += 2;
    }
  }

  char* end = 0;
  long l = strtol(cs, &end, base);
  if (*end != '\0') {
    return new ScriptError("Bad integer format '"+str+"'");
  }
  if (l == LONG_MAX || l == LONG_MIN) {
    return new ScriptError("Integer overflow");
  }

  return new ScriptInt(l);
}

//===========================================================================
ScriptObject* ScriptInt::create(const ScriptRef* args)
{
  const ScriptRef* value = get_method_arg_ref(args,0,"value");
  if (!value) {
    return new ScriptInt(0);
  }

  const ScriptString* str = dynamic_cast<const ScriptString*>(value->object());
  if (str) {
    return ScriptInt::from_string(str->get_string());
  }
  
  return new ScriptInt(value->object()->get_int());
}

//===========================================================================
ScriptObject* ScriptInt::new_copy() const
{
  return new ScriptInt(*this);
}

//===========================================================================
std::string ScriptInt::get_string() const
{
  std::ostringstream oss;
  oss << m_value;
  return oss.str();
}

//===========================================================================
int ScriptInt::get_int() const
{
  return m_value;
}

//===========================================================================
ScriptRef* ScriptInt::script_op(const ScriptAuth& auth,
				const ScriptRef& ref,
				const ScriptOp& op,
				const ScriptRef* right)
{
  if (right) { // binary ops

    const ScriptInt* rnum = dynamic_cast<const ScriptInt*> (right->object());
    if (rnum) { // Int x Int ops
      int rvalue = rnum->get_int();
      switch (op.type()) {
        
      case ScriptOp::Add:
	return ScriptInt::new_ref(m_value + rvalue);
	
      case ScriptOp::Subtract:
	return ScriptInt::new_ref(m_value - rvalue);
	
      case ScriptOp::Multiply:
	return ScriptInt::new_ref(m_value * rvalue);
	
      case ScriptOp::Divide:
	if (rvalue != 0) {
	  return ScriptInt::new_ref(m_value / rvalue);
	} else {
	  return ScriptError::new_ref("Divide by zero");
	}

      case ScriptOp::Power:
	return ScriptInt::new_ref((int)pow(m_value,rvalue));

      case ScriptOp::Modulus:
	if (rvalue != 0) {
	  return ScriptInt::new_ref(m_value % rvalue);
	} else {
	  return ScriptError::new_ref("Divide by zero");
	}
	
      case ScriptOp::Assign:
	if (!ref.is_const()) {
	  m_value = rvalue;
	}
	return ref.ref_copy();
          
      case ScriptOp::AddAssign:
	if (!ref.is_const()) {
	  m_value += rvalue;
	}
	return ref.ref_copy();
	
      case ScriptOp::SubtractAssign:
	if (!ref.is_const()) {
	  m_value -= rvalue;
	}
	return ref.ref_copy();

      case ScriptOp::MultiplyAssign:
	if (!ref.is_const()) {
	  m_value *= rvalue;
	}
	return ref.ref_copy();
	
      case ScriptOp::DivideAssign:
	if (!ref.is_const()) {
	  m_value /= rvalue;
	}
	return ref.ref_copy();

      default: break;
      }

    } else if (dynamic_cast<const ScriptReal*>(right->object())) {
      // Promote to real and use real ops
      ScriptReal lr((double)m_value);
      return lr.script_op(auth,ref,op,right);
    }

  } else { // prefix or postfix ops

    switch (op.type()) {
      
    case ScriptOp::Positive:
      return ScriptInt::new_ref(+m_value);
      
    case ScriptOp::Negative:
      return ScriptInt::new_ref(-m_value);
      
    case ScriptOp::PreIncrement:
      if (!ref.is_const()) {
	++m_value;
      }
      return ref.ref_copy();
      
    case ScriptOp::PreDecrement:
      if (!ref.is_const()) {
	--m_value;
      }
      return ref.ref_copy();
      
    case ScriptOp::Factorial: 
      {
	int a = 1;
	for (int i=abs((int)m_value); i>1; --i) a *= i;
	return ScriptInt::new_ref(a * (m_value<0 ? -1 : 1));
      }
      
    case ScriptOp::PostIncrement: 
      {
	int pre_value = m_value;
	if (!ref.is_const()) {
	  ++m_value;
	}
	return ScriptInt::new_ref(pre_value);
      }
      
    case ScriptOp::PostDecrement: 
      {
	int pre_value = m_value;
	if (!ref.is_const()) {
	  --m_value;
	}
	return ScriptInt::new_ref(pre_value);
      }

    default: break;
    }
  }
    
  return ScriptObject::script_op(auth,ref,op,right);
}

//===========================================================================
void ScriptInt::serialize(IOBase& output) const
{
  output.write(get_string());
}

// ### ScriptReal ###

//===========================================================================
ScriptRef* ScriptReal::new_ref(double value)
{
  return new ScriptRef(new ScriptReal(value));
}

//===========================================================================
ScriptReal::ScriptReal(double value)
  : m_value(value)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptReal);
}

//===========================================================================
ScriptReal::ScriptReal(const ScriptReal& c)
  : ScriptNum(c),
    m_value(c.m_value)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptReal);
}

//===========================================================================
ScriptReal::~ScriptReal()
{
  DEBUG_COUNT_DESTRUCTOR(ScriptReal);
}

//===========================================================================
ScriptObject* ScriptReal::create(const ScriptRef* args)
{
  const ScriptRef* value = get_method_arg_ref(args,0,"value");
  if (!value) {
    return new ScriptReal(0.0);
  }
  const ScriptString* str = dynamic_cast<const ScriptString*>(value->object());
  if (str) {
    char* end = 0;
    double d = strtod(str->get_string().c_str(),&end);
    if (*end == '\0') {
      return new ScriptReal(d);
    }
    return new ScriptError("Bad numeric format '"+str->get_string()+"'");
  }

  const ScriptReal* real = dynamic_cast<const ScriptReal*>(value->object());
  if (real) {
    return new ScriptReal(real->get_real());
  }

  return new ScriptReal((double)value->object()->get_int());
}

//===========================================================================
ScriptObject* ScriptReal::new_copy() const
{
  return new ScriptReal(*this);
}

//===========================================================================
std::string ScriptReal::get_string() const
{
  std::ostringstream oss;
  oss << m_value;
  return oss.str();
}

//===========================================================================
int ScriptReal::get_int() const
{
  return (int)m_value;
}

//===========================================================================
ScriptRef* ScriptReal::script_op(const ScriptAuth& auth,
				 const ScriptRef& ref,
				 const ScriptOp& op,
				 const ScriptRef* right)
{
  if (right) { // binary ops
    const ScriptReal* rnum = dynamic_cast<const ScriptReal*>(right->object());
    const ScriptInt* rint = dynamic_cast<const ScriptInt*>(right->object());

    if (rnum || rint) { // Real x Real (or converted int) ops
      double rvalue = rnum ? rnum->get_real() : (double)rint->get_int();
      switch (op.type()) {
	
      case ScriptOp::Add:
	return ScriptReal::new_ref(m_value + rvalue);
	
      case ScriptOp::Subtract:
	return ScriptReal::new_ref(m_value - rvalue);

      case ScriptOp::Multiply:
	return ScriptReal::new_ref(m_value * rvalue);
	
      case ScriptOp::Divide:
	return ScriptReal::new_ref(m_value / rvalue);

      case ScriptOp::Power:
	return ScriptReal::new_ref(pow(m_value,rvalue));

      case ScriptOp::GreaterThan:
	return ScriptInt::new_ref(m_value > rvalue);
	
      case ScriptOp::LessThan:
	return ScriptInt::new_ref(m_value < rvalue);
	
      case ScriptOp::GreaterThanOrEqualTo:
	return ScriptInt::new_ref(m_value >= rvalue);
	
      case ScriptOp::LessThanOrEqualTo:
	return ScriptInt::new_ref(m_value <= rvalue);
	
      case ScriptOp::Equality:
	return ScriptInt::new_ref(m_value == rvalue);
	
      case ScriptOp::Inequality:
	return ScriptInt::new_ref(m_value != rvalue);
	
      case ScriptOp::Assign:
	if (!ref.is_const()) {
	  m_value = rvalue;
	}
	return ref.ref_copy();
        
      case ScriptOp::AddAssign:
	if (!ref.is_const()) {
	  m_value += rvalue;
	}
	return ref.ref_copy();
	
      case ScriptOp::SubtractAssign:
	if (!ref.is_const()) {
	  m_value -= rvalue;
	}
	return ref.ref_copy();

      case ScriptOp::MultiplyAssign:
	if (!ref.is_const()) {
	  m_value *= rvalue;
	}
	return ref.ref_copy();

      case ScriptOp::DivideAssign:
	if (!ref.is_const()) {
	  m_value /= rvalue;
	}
	return ref.ref_copy();

      default: break;
      }
    }

  } else { // prefix or postfix ops

    switch (op.type()) {
    
    case ScriptOp::Positive:
      return ScriptReal::new_ref(+m_value);
      
    case ScriptOp::Negative:
      return ScriptReal::new_ref(-m_value);
      
    case ScriptOp::Not:
      return ScriptReal::new_ref(!m_value);

    case ScriptOp::Factorial:
      {
	int a = 1;
	for (int i=abs((int)m_value); i>1; --i) a *= i;
	return ScriptReal::new_ref(a * (m_value<0 ? -1 : 1));
      }

    default: break;
    }
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//===========================================================================
void ScriptReal::serialize(IOBase& output) const
{
  output.write(get_string());
}

//===========================================================================
double ScriptReal::get_real() const
{
  return m_value;
}


// ### ScriptList ###

//===========================================================================
ScriptList::ScriptList()
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptList);
}

//===========================================================================
ScriptList::ScriptList(const ScriptList& c)
  : ScriptObject(c)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptList);
  for (ScriptListData::const_iterator it = c.m_list.begin();
       it != c.m_list.end();
       ++it) {
    m_list.push_back((*it)->new_copy());
  }
}

//===========================================================================
ScriptList::~ScriptList()
{
  for (ScriptListData::iterator it = m_list.begin();
       it != m_list.end();
       ++it) {
    delete *it;
  }
  DEBUG_COUNT_DESTRUCTOR(ScriptList);
}

//===========================================================================
ScriptObject* ScriptList::new_copy() const
{
  return new ScriptList(*this);
}

//===========================================================================
std::string ScriptList::get_string() const
{
  std::ostringstream oss;
  oss << "[";
  for (ScriptListData::const_iterator it = m_list.begin();
       it != m_list.end();
       ++it) {
    const ScriptObject* cur = (*it)->object();
    oss << (it==m_list.begin() ? "" : ",") 
	<< cur->get_string();
  }
  oss << "]";
  return oss.str();
}

//===========================================================================
int ScriptList::get_int() const
{
  return m_list.size();
}

//===========================================================================
ScriptRef* ScriptList::script_op(const ScriptAuth& auth,
				 const ScriptRef& ref,
				 const ScriptOp& op,
				 const ScriptRef* right)
{
  switch (op.type()) {

  case ScriptOp::Subscript:
    {
      int idx = right->object()->get_int();
      if (idx < 0 || idx >= (int)m_list.size()) {
	return ScriptError::new_ref("Array index out of bounds");
      }
      ScriptRef* a = get(idx);
      if (!a) return ScriptError::new_ref("Not defined");
      return a->ref_copy(ref.reftype());
    }

  case ScriptOp::Lookup:
    {
      std::string name = right->object()->get_string();
      if ("size" == name) return ScriptInt::new_ref(size());
      
      if ("push" == name ||
          "splice" == name ||
          "reverse" == name ||
          "sort" == name ||
          "join" == name ||
	  "clear" == name) {
        return new ScriptMethodRef(ref,name);
      }
    } break;

  default: break;
  }
  
  return ScriptObject::script_op(auth,ref,op,right);
}

//===========================================================================
ScriptRef* ScriptList::script_method(const ScriptAuth& auth,
				     const ScriptRef& ref,
				     const std::string& name,
				     const ScriptRef* args)
{
  const ScriptList* argl = dynamic_cast<const ScriptList*>(args->object());
  
  if ("push" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");
    
    if (argl->size() == 0) 
      return ScriptError::new_ref("No value(s) specified");
    
    for (int i=0; i<argl->size(); ++i) {
      const ScriptRef* value = argl->get(i);
      give(value->ref_copy());
    }

    return 0;
  }

  if ("splice" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");

    const ScriptInt* a_offs = get_method_arg<ScriptInt>(args,0,"offset");
    if (!a_offs) 
      return ScriptError::new_ref("ScriptList::splice() No offset specified");
      
    ScriptRef* entry = take(a_offs->get_int());
    delete entry;
    return 0;
  }

  if ("reverse" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");

    m_list.reverse();
    return 0;
  }

  if ("sort" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");
    
    const ScriptString* a_pred = 
      get_method_arg<ScriptString>(args,0,"predicate");
    std::string pred = a_pred ? a_pred->get_string() : "a < b";
    
    m_list.sort(ScriptObjectSorter(pred));
    return 0;
  }

  if ("join" == name) {
    const ScriptString* a_glue = get_method_arg<ScriptString>(args,0,"glue");
    if (!a_glue) 
      return ScriptError::new_ref("ScriptList::join() No glue specified");
    std::string glue = a_glue->get_string();

    std::ostringstream oss;
    for (ScriptListData::const_iterator it = m_list.begin();
	 it != m_list.end();
	 ++it) {
      const ScriptObject* cur = (*it)->object();
      oss << (it==m_list.begin() ? "" : glue) << cur->get_string();
    }
    return ScriptString::new_ref(oss.str());
  }

  if ("clear" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");
    clear();
    return 0;
  }

  return ScriptObject::script_method(auth,ref,name,args);
}

//===========================================================================
void ScriptList::serialize(IOBase& output) const
{
  output.write("[");
  for (ScriptListData::const_iterator it = m_list.begin();
       it != m_list.end();
       ++it) {
    if (it!=m_list.begin()) output.write(",");
    (*it)->object()->serialize(output);
  }
  output.write("]");
}

//===========================================================================
int ScriptList::size() const
{
  return m_list.size();
}

//===========================================================================
const ScriptRef* ScriptList::get(int i) const
{
  int ic=0;
  for (ScriptListData::const_iterator it = m_list.begin();
       it != m_list.end();
       ++it) {
    if (ic++==i) return *it;
  }

  return 0;
}

//===========================================================================
ScriptRef* ScriptList::get(int i)
{
  int ic=0;
  for (ScriptListData::iterator it = m_list.begin();
       it != m_list.end();
       ++it) {
    if (ic++==i) return *it;
  }
  
  return 0;
}

//===========================================================================
void ScriptList::give(ScriptRef* item, int i)
{
  if (-1==i) {
    m_list.push_back(item);
  } else {
    int ic=0;
    ScriptListData::iterator it;
    for (it = m_list.begin();
	 it != m_list.end();
	 ++it) {
      if (ic++==i) break;
    }
    m_list.insert(it,item);
  }
}

//===========================================================================
ScriptRef* ScriptList::take(int i)
{
  int ic=0;
  for (ScriptListData::iterator it = m_list.begin();
       it != m_list.end();
       ++it) {
    if (ic++==i) {
      ScriptRef* cur = *it;
      m_list.erase(it);
      return cur;
    }
  }
  return 0;
}

//===========================================================================
void ScriptList::clear()
{
  for (ScriptListData::iterator it = m_list.begin();
       it != m_list.end();
       ++it) {
    delete *it;
  }
  m_list.clear();
}


// ### ScriptMap ###

//===========================================================================
ScriptMap::ScriptMap()
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptMap);
}

//===========================================================================
ScriptMap::ScriptMap(const ScriptMap& c)
  : ScriptObject(c)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptMap);
  for (ScriptMapData::const_iterator it = c.m_map.begin();
       it != c.m_map.end();
       ++it) {
    const std::string key = it->first;
    ScriptRef* value = it->second->new_copy();
    m_map.insert( std::pair<std::string,ScriptRef*>(key,value) );
  }
}

//===========================================================================
ScriptMap::~ScriptMap()
{
  for (ScriptMapData::iterator it = m_map.begin();
       it != m_map.end();
       ++it) {
    delete it->second;
  }
  DEBUG_COUNT_DESTRUCTOR(ScriptMap);
}

//===========================================================================
ScriptObject* ScriptMap::new_copy() const
{
  return new ScriptMap(*this);
}

//===========================================================================
std::string ScriptMap::get_string() const
{
  std::ostringstream oss;
  oss << "{";
  for (ScriptMapData::const_iterator it = m_map.begin();
       it != m_map.end();
       ++it) {
    const std::string key = it->first;
    const ScriptObject* value = it->second->object();
    oss << (it==m_map.begin() ? "" : ",") 
        << "\"" << key << "\":"
	<< (value ? value->get_string() : "NULL");
  }
  oss << "}";
  return oss.str();
}

//===========================================================================
int ScriptMap::get_int() const
{
  return m_map.size();
}

//===========================================================================
ScriptRef* ScriptMap::script_op(const ScriptAuth& auth,
				const ScriptRef& ref,
				const ScriptOp& op,
				const ScriptRef* right)
{
  switch (op.type()) {

  case ScriptOp::Subscript:
    {
      ScriptRef* a = lookup(right->object()->get_string());
      if (!a) return ScriptError::new_ref("Not defined");
      return a->ref_copy(ref.reftype());
    }
    
  case ScriptOp::Lookup:
  case ScriptOp::Resolve:
    {
      std::string name = right->object()->get_string();

      if ("set" == name ||
	  "add" == name ||
          "remove" == name ||
          "clear" == name) {
        return new ScriptMethodRef(ref,name);
      }

      if (name == "size") return ScriptInt::new_ref(size());
      if (name == "keys") {
	ScriptList* list = new ScriptList();
	for (ScriptMapData::const_iterator it = m_map.begin();
	     it != m_map.end();
	     ++it) {
	  list->give( ScriptString::new_ref(it->first) );
	}
	return new ScriptRef(list);
      }
      
      ScriptRef* a = lookup(name);
      if (a) {
	return a->ref_copy(ref.reftype());
      }
    } break;

  default: break;
  }

  return ScriptObject::script_op(auth,ref,op,right);
}

//===========================================================================
ScriptRef* ScriptMap::script_method(const ScriptAuth& auth,
				    const ScriptRef& ref,
				    const std::string& name,
				    const ScriptRef* args)
{
  if ("set" == name || "add" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");
    
    const ScriptString* a_name = get_method_arg<ScriptString>(args,0,"name");
    if (!a_name) 
      return ScriptError::new_ref("ScriptMap::set() No name specified");

    const ScriptRef* a_value = get_method_arg_ref(args,1,"value");
    if (!a_value) 
       return ScriptError::new_ref("ScriptMap::set() No value specified");

    give(a_name->get_string(),a_value->ref_copy());
    return 0;
  }

  if ("remove" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");

    const ScriptString* a_name = get_method_arg<ScriptString>(args,0,"name");
    if (!a_name) 
      return ScriptError::new_ref("ScriptMap::remove() No name specified");

    ScriptRef* value = take(a_name->get_string());
    if (!value)
      return ScriptError::new_ref("ScriptMap::remove() Item not found");

    delete value;
    return 0;
  }

  if ("clear" == name) {
    if (ref.is_const()) return ScriptError::new_ref("Not permitted");
    clear();
    return 0;
  }

  return ScriptObject::script_method(auth,ref,name,args);
}

//===========================================================================
void ScriptMap::serialize(IOBase& output) const
{
  output.write("{");
  for (ScriptMapData::const_iterator it = m_map.begin();
       it != m_map.end();
       ++it) {
    if (it!=m_map.begin()) output.write(",");
    output.write("\"" + it->first + "\":");
    it->second->object()->serialize(output);
  }
  output.write("}");
}

//===========================================================================
int ScriptMap::size() const
{
  return m_map.size();
}

//===========================================================================
void ScriptMap::keys(std::vector<std::string>& keyvec) const
{
  keyvec.clear();
  keyvec.reserve(size());
  for (ScriptMapData::const_iterator it = m_map.begin();
       it != m_map.end();
       ++it) {
    keyvec.push_back(it->first);
  }
}

//===========================================================================
const ScriptRef* ScriptMap::lookup(const std::string& key) const
{
  ScriptMapData::const_iterator it = m_map.find(key);
  if (it != m_map.end()) {
    return it->second;
  }

  return 0;
}

//===========================================================================
ScriptRef* ScriptMap::lookup(const std::string& key)
{
  ScriptMapData::iterator it = m_map.find(key);
  if (it != m_map.end()) {
    return it->second;
  }

  return 0;
}

//===========================================================================
void ScriptMap::give(const std::string& key, ScriptRef* value)
{
  delete lookup(key);
  m_map[key] = value;
}

//===========================================================================
ScriptRef* ScriptMap::take(const std::string& key)
{
  ScriptMapData::iterator it = m_map.find(key);
  if (it != m_map.end()) {
    ScriptRef* value = it->second;
    m_map.erase(it);
    return value;
  }
  return 0;
}

//===========================================================================
void ScriptMap::clear()
{
  for (ScriptMapData::iterator it = m_map.begin();
       it != m_map.end();
       ++it) {
    delete it->second;
  }
  m_map.clear();
}

// ### ScriptSub ###

//===========================================================================
ScriptSub::ScriptSub(const std::string& name,
		     const ScriptSubArgNames& args,
		     ScriptRefTo<ScriptStatement>* body,
		     ScriptTracer& tracer)
  : m_name(name),
    m_arg_names(args),
    m_body(body),
    m_tracer(tracer.new_copy())
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptSub);
}

//===========================================================================
ScriptSub::ScriptSub(const ScriptSub& c)
  : ScriptObject(c),
    m_name(c.m_name),
    m_arg_names(c.m_arg_names),
    m_body(0),
    m_tracer(c.m_tracer->new_copy())
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptSub);
  if (c.m_body) {
    m_body = c.m_body->new_copy();
  }
}

//===========================================================================
ScriptSub::~ScriptSub()
{
  delete m_body;
  delete m_tracer;
  DEBUG_COUNT_DESTRUCTOR(ScriptSub);
}

//===========================================================================
ScriptObject* ScriptSub::new_copy() const
{
  return new ScriptSub(*this);
}

//===========================================================================
std::string ScriptSub::get_string() const
{
  std::ostringstream oss;
  oss << "sub " << m_name << "()";
  return oss.str();
}

//===========================================================================
int ScriptSub::get_int() const
{
  return !m_name.empty();
}

//===========================================================================
ScriptRef* ScriptSub::script_op(const ScriptAuth& auth,
				const ScriptRef& ref,
				const ScriptOp& op,
				const ScriptRef* right)
{
  // Only allow list "()" operator - subroutine call
  if (op.type() == ScriptOp::List) return call(auth,right);
  
  return ScriptObject::script_op(auth,ref,op,right);
}

//=============================================================================
ScriptRef* ScriptSub::call(const ScriptAuth& auth, const ScriptRef* args)
{
  if (m_body) {

    ScriptRefTo<ScriptStatement>* body = m_body->new_copy();

    // Setup the arguments
    int pos = 0;
    for (ScriptSubArgNames::const_iterator it = m_arg_names.begin();
         it != m_arg_names.end();
         ++it) {

      // Predeclare vars for the subroutine arguments
      ScriptList* v_args = new ScriptList();
      ScriptRef v_args_ref(v_args);
      v_args->give(ScriptString::new_ref(*it));
      const ScriptRef* arg = get_method_arg_ref(args,pos++,*it);
      if (arg) {
        v_args->give(arg->ref_copy());
        body->object()->script_method(auth,*args,"var",&v_args_ref);
      }
    }

    // Execute the subroutine body
    ScriptRef* ret = body->object()->execute(*m_tracer);
    delete body;
    return ret;
  }
  return 0;
}

// ### ScriptError ###

//===========================================================================
ScriptRef* ScriptError::new_ref(const std::string& error)
{
  return new ScriptRef(new ScriptError(error));
}

//===========================================================================
ScriptError::ScriptError(const char* str)
  : m_string(str)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptError);
}

//===========================================================================
ScriptError::ScriptError(const std::string& str)
  : m_string(str)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptError);
}

//===========================================================================
ScriptError::ScriptError(const ScriptError& c)
  : ScriptObject(c),
    m_string(c.m_string)
{
  DEBUG_COUNT_CONSTRUCTOR(ScriptError);
}

//===========================================================================
ScriptError::~ScriptError()
{
  DEBUG_COUNT_DESTRUCTOR(ScriptError);
}

//===========================================================================
ScriptObject* ScriptError::create(const ScriptRef* args)
{
  const ScriptRef* value = get_method_arg_ref(args,0,"value");
  return new ScriptError(value ? value->object()->get_string() : "unknown");
}

//===========================================================================
ScriptObject* ScriptError::new_copy() const
{
  return new ScriptError(*this);
}

//===========================================================================
std::string ScriptError::get_string() const
{
  std::ostringstream oss;
  oss << "ERROR: " << m_string;
  return oss.str();
}

//===========================================================================
int ScriptError::get_int() const
{
  return 0;
}

//===========================================================================
void ScriptError::serialize(IOBase& output) const
{
  output.write("Error(\"" + m_string + "\")");
}

};
