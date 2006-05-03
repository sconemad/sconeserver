/* SconeServer (http://www.sconemad.com)

SconeX Library entry code

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


#include "sconex/sconex.h"

#ifdef WIN32

//
// Win32 - DllMain
//

BOOL APIENTRY DllMain(
  HANDLE hModule,
  DWORD  ul_reason_for_call,
  LPVOID lpReserved
)
{
  switch (ul_reason_for_call)
    {
      case DLL_PROCESS_ATTACH: {
        // Start up Windows Sockets
        WSADATA WSAData;
        int status = WSAStartup(MAKEWORD(2,2),&WSAData);
        if (status != 0) {
          return false;
        }
        break;
      } 
        
      case DLL_PROCESS_DETACH: {
        // Shut down Windows Sockets
        WSACleanup();
        break;
      }
        
      case DLL_THREAD_ATTACH: {
        
      }
        
      case DLL_THREAD_DETACH: {
        break;
      }
    }
  
  return true;
}

#else

//
// UNIX - nothing required
//

#endif
