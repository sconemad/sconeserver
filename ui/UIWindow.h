/* SconeServer (http://www.sconemad.com)

UI Window

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

#ifndef uiUIWindow_h
#define uiUIWindow_h

#include "UIDisplay.h"
#include "UIModule.h"

//=========================================================================
class UIWindow { //: public scx::ScriptObject {
public:

  UIWindow(UIDisplay* display);
  ~UIWindow();

  virtual void xevent(XEvent& xe);

  void plot(int x, int y, unsigned long value);

  void paint();

protected:

  UIDisplay* m_display;
  Window m_win;
  GC m_gc;
  XImage* m_image;

};

#endif
