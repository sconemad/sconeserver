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


#include <sconex/ModuleInterface.h>
#include <sconex/Module.h>
#include <sconex/Kernel.h>
#include <sconex/Descriptor.h>
#include <sconex/Stream.h>

#include "UIModule.h"

#include <X11/Xlib.h>

//=========================================================================
class UIDisplay : public scx::Descriptor {
public:

  UIDisplay();
  ~UIDisplay();

  virtual void close() 
  {};

  virtual int fd();

  Display* get_dpy();

protected:

  // We read and write to this using Xlib calls
  virtual scx::Condition endpoint_read(void* buffer,int n,int& na) 
  { return scx::Error; };
  virtual scx::Condition endpoint_write(const void* buffer,int n,int& na) 
  { return scx::Error; };

  Display* m_dpy;

};

//=========================================================================
UIDisplay::UIDisplay()
{
  m_dpy = XOpenDisplay(0);
  
}

//=========================================================================
UIDisplay::~UIDisplay()
{
  if (m_dpy) {
    XCloseDisplay(m_dpy);
  }
}

//=========================================================================
int UIDisplay::fd()
{
  return ConnectionNumber(m_dpy);
}

//=========================================================================
Display* UIDisplay::get_dpy()
{
  return m_dpy;
}

//=========================================================================
class UIDispatcher : public scx::Stream {
public:

  UIDispatcher(UIDisplay& display);
  ~UIDispatcher();

  virtual scx::Condition event(scx::Stream::Event e);
 
protected:

  UIDisplay& m_display;
};

//=========================================================================
UIDispatcher::UIDispatcher(UIDisplay& display)
  : scx::Stream("UI dispatcher"),
    m_display(display)
{
  enable_event(scx::Stream::Readable,true);
}

//=========================================================================
UIDispatcher::~UIDispatcher()
{

}

//=========================================================================
scx::Condition UIDispatcher::event(scx::Stream::Event e)
{
  if (e == scx::Stream::Readable) {
    Display* dpy = m_display.get_dpy();
    while (XPending(dpy)) {
      XEvent xe;
      XNextEvent(dpy, &xe);
    }
  }

  return scx::Ok;
}

SCONEX_MODULE(UIModule);

//=========================================================================
UIModule::UIModule() 
  : scx::Module("ui",scx::version()),
    m_display(0)
{

}

//=========================================================================
UIModule::~UIModule()
{

}

//=========================================================================
std::string UIModule::info() const
{
  return "User interface module";
}

//=============================================================================
int UIModule::init()
{
  m_display = new UIDisplay();
  if (!m_display->get_dpy()) {
    delete m_display;
    std::cerr << "Could not open display\n";
    return 1;
  }

  m_display->add_stream(new UIDispatcher(*m_display));
  scx::Kernel::get()->connect(m_display);
  return 0;
}

//=============================================================================
bool UIModule::close()
{
  return true;
}

//=============================================================================
void UIModule::provide(const std::string& type,
		       const scx::ScriptRef* args,
		       scx::ScriptObject*& object)
{
  if (type == "Window") 
    object = scx::ScriptInt::create(args);
}
