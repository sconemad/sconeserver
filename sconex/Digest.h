/* SconeServer (http://www.sconemad.com)

Message Digest calculation

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

#ifndef scxDigest_h
#define scxDigest_h

#include <sconex/sconex.h>
#include <sconex/ScriptBase.h>
#include <sconex/Provider.h>
#include <sconex/Buffer.h>
namespace scx {

//=============================================================================
// Digest - A message digest calculator
//
class SCONEX_API Digest : public ScriptObject {
public:

  // Create a message digest calculator of the specified type
  // Type specifies the digest method, e.g. md5, sha1
  static Digest* create(const std::string& type,
                        const ScriptRef* args);
  
  virtual ~Digest();

  virtual void update(const void* buffer, int n) = 0;
  virtual void finish() = 0;

  // Get the digest as a byte buffer
  const Buffer& get_digest() const;

  // Get the digest as a hex string
  std::string get_digest_string() const;
  
  // Interface for registering new digest methods
  static void register_method(const std::string& type,
			      scx::Provider<Digest>* factory);
  static void unregister_method(const std::string& type,
				scx::Provider<Digest>* factory);

  virtual std::string get_string() const;
  virtual int get_int() const;
  
  virtual ScriptRef* script_op(const ScriptAuth& auth,
			       const ScriptRef& ref,
			       const ScriptOp& op,
			       const ScriptRef* right=0);

  virtual ScriptRef* script_method(const ScriptAuth& auth,
				   const ScriptRef& ref,
				   const std::string& name,
				   const ScriptRef* args);

  typedef ScriptRefTo<Digest> Ref;

protected:

  Digest(const std::string& method);
  Digest(const Digest& c);

  std::string m_method;
  Buffer m_digest;

private:

  static void init();

  static ProviderScheme<Digest>* s_providers;
};

};

#endif
