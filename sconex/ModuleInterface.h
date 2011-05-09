/* SconeServer (http://www.sconemad.com)

SconeServer dynamically loaded module interface

This defines the SCONESERVER_MODULE macro, which expands to provide the
standard entry point function which must be present in all SconeServer
modules. You should include this macro in the source file for your module
class, passing the class name as an argument, eg:

--- MyModule.cpp ---
#include "MyModule.h
#include <sconex/ModuleInterface.h>

SCONESERVER_MODULE(MyModule);

... source for MyModule ...


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


#define SCONESERVER_MODULE(MODULE) \
   extern "C" scx::Module* SconeServer_module() \
   { \
     return new MODULE(); \
   }
