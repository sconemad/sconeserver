/* SconeServer (http://www.sconemad.com)

DSO Module loader

Copyright (c) 2000-2004 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef scxModuleLoaderDLL_h
#define scxModuleLoaderDLL_h

#include <sconex/sconex.h>
#include <sconex/ModuleLoader.h>
namespace scx {

//=============================================================================
class SCONEX_API ModuleLoaderDLL : public ModuleLoader {

public:

  ModuleLoaderDLL(
    const std::string& name,
    Module* parent
  );
 
  virtual ~ModuleLoaderDLL();

protected:

  virtual bool load_module();
  virtual bool unload_module();

private:

  bool load_dll(const std::string& filename);
  bool unload_dll();
  // Load/unload the dll or shared object

  void* locate_symbol(const std::string& name) const;
  // Locate the named symbol, returning a pointer to
  // its entry point
  
  // Module handle
  void* m_dll;

};

};
#endif
