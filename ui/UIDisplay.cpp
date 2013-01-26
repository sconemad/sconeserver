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

#include "UIDisplay.h"
#include "UIWindow.h"
#include <sconex/Descriptor.h>
#include <sconex/Kernel.h>

//=========================================================================
class UIDisplaySocket : public scx::Descriptor {
public:

  UIDisplaySocket(int xfd)
    : m_xfd(xfd) 
  {
    m_state = scx::Descriptor::Connected;
  };
  ~UIDisplaySocket() {};

  virtual void close() { m_state = scx::Descriptor::Closed; };
  virtual int fd() { return m_xfd; };

protected:

  virtual scx::Condition endpoint_read(void* buffer,int n,int& na)
  { return scx::Error; }
  virtual scx::Condition endpoint_write(const void* buffer,int n,int& na)
  { return scx::Error; }

  int m_xfd;
};

//=========================================================================
UIDisplay::UIDisplay(UIModule* module)
  : scx::Stream("UI display"),
    m_module(module)
{
  m_dpy = XOpenDisplay(0);

  UIDisplaySocket* sock = new UIDisplaySocket(ConnectionNumber(m_dpy));
  sock->add_stream(this);
  scx::Kernel::get()->connect(sock);

  enable_event(scx::Stream::Readable,true);
}

//=========================================================================
UIDisplay::~UIDisplay()
{
  XCloseDisplay(m_dpy);
  m_module.object()->display_closing(this);
}

//=========================================================================
scx::Condition UIDisplay::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Readable) {
    while (XPending(m_dpy)) {
      XEvent xe;
      XNextEvent(m_dpy, &xe);

      WindowMap::iterator it = m_windows.find(xe.xany.window);
      if (it != m_windows.end()) {
	it->second->xevent(xe);
      } else {
        std::cerr << "XEvent type=" << xe.type << 
          " for unknown window " << xe.xany.window << "\n";      }
    }
  }

  return scx::Ok;
}

//=========================================================================
Display* UIDisplay::get_dpy()
{
  return m_dpy;
}

//=========================================================================
void UIDisplay::register_window(Window xw, UIWindow* uiw)
{
  m_windows[xw] = uiw;
}

//=========================================================================
void UIDisplay::unregister_window(Window xw)
{
  m_windows.erase(xw);
}

//=========================================================================
void UIDisplay::close()
{
  endpoint().close();
}
