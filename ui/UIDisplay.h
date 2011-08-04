/* SconeServer (http://www.sconemad.com)

UI Display

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

#ifndef uiUIDisplay_h
#define uiUIDisplay_h

#include "UIModule.h"
#include <sconex/Descriptor.h>
#include <sconex/Stream.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

//=========================================================================
class UIDisplay : public scx::Stream {
public:

  UIDisplay(UIModule* module);
  ~UIDisplay();

  virtual scx::Condition event(scx::Stream::Event e);

  Display* get_dpy();

  void register_window(Window xw, UIWindow* uiw);
  void unregister_window(Window xw);
 
  void close();

protected:

  scx::ScriptRefTo<UIModule> m_module;
  Display* m_dpy;

  typedef std::map<Window,UIWindow*> WindowMap;
  WindowMap m_windows;
};

#endif
