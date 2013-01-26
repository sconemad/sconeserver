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

#include "UIWindow.h"

//=========================================================================
UIWindow::UIWindow(UIDisplay* display)
  : m_display(display),
    m_width(400),
    m_height(400)
{
  Display* dpy = m_display->get_dpy();
  int screen_num = DefaultScreen(dpy);
  int black_color = BlackPixel(dpy,screen_num);
  int white_color = WhitePixel(dpy,screen_num);
  
  m_win = XCreateSimpleWindow(dpy,
			      DefaultRootWindow(dpy),
			      0,0,
			      m_width,m_height,
			      0,
			      black_color,white_color);

  m_display->register_window(m_win,this);
  
  XStoreName(dpy,m_win,"Hello!");
  XMapWindow(dpy,m_win);
  
  m_gc = XCreateGC(dpy,m_win,0,NULL);
  XSetForeground(dpy,m_gc,black_color);
  
  XSelectInput(dpy,
	       m_win,
	       ExposureMask | KeyPressMask | StructureNotifyMask |
	       ButtonPressMask | ButtonReleaseMask | Button1MotionMask);
  
  std::cerr << "created window \n" << (int)m_win;
  XFlush(dpy);
  
  Visual* visual = DefaultVisual(dpy,screen_num);
  int depth = DefaultDepth(dpy, screen_num);
  
  m_image = XCreateImage(dpy, visual, depth, ZPixmap,
			 0, 0, m_width, m_height, BitmapPad(dpy), 0);
  m_image->data = (char*)malloc(m_image->bytes_per_line * m_image->height);
}

//=========================================================================
UIWindow::~UIWindow()
{
  m_display->unregister_window(m_win);
  XDestroyImage(m_image);
}

//=========================================================================
void UIWindow::xevent(XEvent& xe)
{
  std::cerr << "UIWindow XEvent type=" << xe.type << "\n";
  switch (xe.type) {
  case Expose:
    paint();
    break;
  }
}

//=========================================================================
void UIWindow::plot(int x, int y, unsigned long value)
{
  Display* dpy = m_display->get_dpy();
  //  XLockDisplay(m_dpy);
  //  XDrawPoint(dpy, m_win, m_gc, x, y);
  unsigned long p = XGetPixel(m_image,x,y);
  XPutPixel(m_image,x,y,value);
  //  XUnlockDisplay(m_dpy);
  //  paint();
}

//=========================================================================
void UIWindow::paint()
{
  Display* dpy = m_display->get_dpy();
  XPutImage(dpy, m_win, m_gc, m_image, 0, 0, 0, 0, m_width, m_height);
}
