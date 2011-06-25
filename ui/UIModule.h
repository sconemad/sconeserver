/* SconeServer (http://www.sconemad.com)

User interface module

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

#ifndef uiUIModule_h
#define uiUIModule_h

#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/ScriptTypes.h>
#include <sconex/ScriptExpr.h>

class UIDisplay;

//=========================================================================
class UIModule : public scx::Module,
                 public scx::Provider<scx::ScriptObject> {
public:

  UIModule();
  virtual ~UIModule();

  virtual std::string info() const;

  virtual int init();
  virtual bool close();

  // Provider<ScriptObject> method
  virtual void provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object);
private:

  UIDisplay* m_display;

};

#endif
