/* SconeServer (http://www.sconemad.com)

SconeServer daemon entry point

Copyright (c) 2000-2005 Andrew Wedgbury <wedge@sconemad.com>

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
#include "sconex/Kernel.h"

#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif

#ifndef CONF_PATH
#  define CONF_PATH "etc"
#endif

#ifndef MOD_PATH
#  define MOD_PATH "."
#endif

#ifndef VAR_PATH
#  define VAR_PATH "var"
#endif

// Command line option defaults
static bool opt_foreground = false;
static bool opt_load_config = true;
static std::string conf_path = CONF_PATH;
static std::string mod_path = MOD_PATH;
static std::string var_path = VAR_PATH;

//===========================================================================
int run()
{
  // Restart loop
  while (true) {

    scx::Kernel* kernel = scx::Kernel::get();
    
    // Set paths
    kernel->set_conf_path(conf_path);
    kernel->set_mod_path(mod_path);
    kernel->set_var_path(var_path);
    
    // Set config autoloading
    kernel->set_autoload_config(opt_load_config);
    
    // Init server
    if (kernel->init()) {
      break;
    }

    // Connect console if running in foreground
    if (opt_foreground) {
#ifndef WIN32
      kernel->connect_config_console();
#endif
    }

    // Run server
    if (kernel->run()) {
      break;
    }
    
  }

  if (opt_foreground) {
    std::cout << std::endl;
  }

  return 0;
}

#ifdef WIN32

// Globals
char* szServiceName = "SconeServer";
char* szServiceDesc = "An object orientated network server framework.";
char* szRegKey = "Software\\SconeMAD\\SconeServer";

// Service data
SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE service_status_handle;
HANDLE hThread;
HWND hWndMain;

// Prototypes
void WINAPI Service_Main(DWORD argc,LPTSTR *argv);
void WINAPI Service_Ctrl(DWORD opcode);
void load_registry();

#endif

//=============================================================================
int main(int argc,char* argv[])
{
  std::string arg;
  int err=1;
  
  // Look for version/help options initially as a special case
  if (argc == 2) {
    arg = argv[1];
    bool opt_version = (arg == "-v" || arg == "--version");
    bool opt_help = (arg == "-h" || arg == "--help");

    if (opt_version || opt_help) {
      scx::Kernel* kernel = scx::Kernel::get();
      std::cout << kernel->name() << "-"
                << kernel->version().get_string() << "\n";

      if (opt_version) {
        std::cout << "Built on " << scx::build_time().code()
                  << " for " << scx::build_type() << "\n";
      }

      if (opt_help) {
        std::cout
          << "\n" << kernel->info() << "\n"
          << "Command line options:\n"
          << " -h         Print this help and exit\n"
          << " -v         Print version information and exit\n"
          << " -f         Launch in foreground with console\n"
          << " -n         Don't read main configuration file\n"
          << " -c=PATH    Set configuration path\n"
          << " -m=PATH    Set module path\n"
          << " -l=PATH    Set var (log) path\n"
          << "\n";
      }
      // Exit if version/help was provided
      return 0;
    }

  }

#ifndef WIN32

  // POSIX startup

  int c;
  extern char* optarg;
  while ((c = getopt(argc,argv,"fnc:m:l:")) >= 0) {
    switch (c) {
      case '?': return 1;
      case 'f': opt_foreground = true;  break;
      case 'n': opt_load_config = false; break;
      case 'c': conf_path = optarg;  break;
      case 'm': mod_path = optarg;   break;
      case 'l': var_path = optarg;   break;
    }
  }

  if (!opt_foreground) {
    // Daemonize
    pid_t pid;
    if ((pid=fork()) != 0) {
      exit(0);
    }
    setsid();
    if ((pid=fork()) != 0) {
      exit(0);
    }
    umask(0);
  }

  // Don't terminate on silly signals
  signal(SIGHUP,SIG_IGN);
  signal(SIGPIPE,SIG_IGN);
  
  err = run();
  
#else

  // WIN32 startup

  // Read options from registry
  load_registry();
  
 
  if (arg == "-s") {

    // Start system service (called from SCM)

    SERVICE_TABLE_ENTRY dispatch_table[] = {
      {szServiceName,Service_Main},
      {0,0}
    };

    if (!StartServiceCtrlDispatcher(dispatch_table)) {
      std::cerr << "ERROR: Unable to start service\n";

    } else {
      err=0;
    }

  } else if (arg == "-f") {

    // Just run it as a regular process

    opt_foreground = true;
    err = run();

  } else {
 
    // Start/create/delete system service

    SC_HANDLE schSCManager = OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
    if (!schSCManager) {
      std::cerr << "ERROR: Unable to open service control manager\n";

    } else {

      // Try opening the service
      SC_HANDLE schService = OpenService(
        schSCManager,
        szServiceName,
        SERVICE_ALL_ACCESS);

      if (arg == "-d") {

        // Delete the service
        if (!schService) {
          std::cerr
            << "ERROR: Unable to open service, or service does not exist\n";

        } else {
          if (!DeleteService(schService)) {
            std::cerr << "ERROR: Unable to delete service\n";

          } else {
            std::cout << "Service deleted\n";
            err=0;
          }
        }

      } else if (arg == "-x") {

        // Stop the service
        if (!schService) {
          std::cerr
            << "ERROR: Unable to open service, or service does not exist\n";

        } else {
          std::cout << "Stopping service...\n";
          SERVICE_STATUS ss;
          if (!ControlService(schService,SERVICE_CONTROL_STOP,&ss)) {
            std::cerr << "ERROR: Unable to stop service\n";

          } else {
            std::cout << "done\n";
            err=0;
          }
        }

      } else {

        // Create/start the service
        if (schService == NULL) {

          // Create the service as it doesn't already exist
          std::cout << "Service does not exist, creating...\n";

          std::string path = argv[0];
          path += " -s";

          schService = CreateService( 
            schSCManager,
            szServiceName,
            szServiceName,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            path.c_str(),
            NULL,
            NULL,
            NULL,                      // no dependencies 
            NULL,
            NULL);

          if (!schService) {
            std::cerr << "ERROR: Unable to create service\n";

          } else {
            // Set additional info (description)
            SERVICE_DESCRIPTION sdesc;
            sdesc.lpDescription = szServiceDesc;
            ChangeServiceConfig2(schService,SERVICE_CONFIG_DESCRIPTION,&sdesc);
          }
        }

        if (schService) {

          // Start the service
          std::cout << "Starting service...\n";
          if (!StartService(schService,0,0)) {
            std::cerr << "ERROR: Unable to start service\n";

          } else {
            std::cout << "done\n";
            err=0;
          }
        }
      }

      if (schService) {
        CloseServiceHandle(schService);
      }

      CloseServiceHandle(schSCManager);
    }
  }

#endif
  
  return err;
}

#ifdef WIN32

//===========================================================================
void WINAPI Service_Main(DWORD argc, LPTSTR *argv)
{
  DWORD status;
  DWORD specificError=0;

  service_status.dwServiceType = SERVICE_WIN32;
  service_status.dwCurrentState = SERVICE_START_PENDING;
  service_status.dwControlsAccepted =
    SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
  service_status.dwWin32ExitCode = 0;
  service_status.dwServiceSpecificExitCode = 0;
  service_status.dwCheckPoint = 0;
  service_status.dwWaitHint = 0;

  service_status_handle = RegisterServiceCtrlHandler(
    szServiceName,Service_Ctrl);

  if (service_status_handle == 0) {
    return;
  }

  // Initialization
  status = 0;

  if (status != NO_ERROR) {
    // Unable to start service
    service_status.dwCurrentState = SERVICE_STOPPED;
    service_status.dwCheckPoint = 0;
    service_status.dwWaitHint = 0;
    service_status.dwWin32ExitCode = status;
    service_status.dwServiceSpecificExitCode = specificError;
    SetServiceStatus (service_status_handle, &service_status);
    return;
  }

  // Initialization complete - report running status.
  service_status.dwCurrentState       = SERVICE_RUNNING;
  service_status.dwCheckPoint         = 0;
  service_status.dwWaitHint           = 0;
  if (!SetServiceStatus(service_status_handle,&service_status)) {
    status = GetLastError();
  }

  // Run service
  specificError = run();

  // Stopped again
  service_status.dwCurrentState = SERVICE_STOPPED;
  service_status.dwCheckPoint = 0;
  service_status.dwWaitHint = 0;
  service_status.dwWin32ExitCode = status;
  service_status.dwServiceSpecificExitCode = specificError;
  SetServiceStatus (service_status_handle, &service_status);

  return;
}

//===========================================================================
void WINAPI Service_Ctrl(DWORD Opcode)
{
  switch(Opcode) {

    case SERVICE_CONTROL_PAUSE:
      // PAUSE
      //      paused=1;

      service_status.dwCurrentState = SERVICE_PAUSED;
      break;

    case SERVICE_CONTROL_CONTINUE:
      // CONTINUE
      //      paused=0;

      service_status.dwCurrentState = SERVICE_RUNNING;
      break;

    case SERVICE_CONTROL_STOP:
      // STOP
      SendMessage(hWndMain,WM_CLOSE,0,0);

      service_status.dwWin32ExitCode = 0;
      service_status.dwCurrentState = SERVICE_STOPPED;
      service_status.dwCheckPoint = 0;
      service_status.dwWaitHint = 0;
      break;
  }

  // Send current status.
  SetServiceStatus(service_status_handle,&service_status);
  return;
}

//===========================================================================
void load_registry()
{
  HKEY key=0;
  RegOpenKeyEx(HKEY_LOCAL_MACHINE,szRegKey,0,KEY_READ,&key);
  DWORD type;
  char buffer[MAX_PATH];
  unsigned long size;
  
  size = sizeof(buffer);
  if (ERROR_SUCCESS ==
      RegQueryValueEx(key,"conf_path",0,&type,(BYTE*)buffer,&size)) {
    conf_path = buffer;
  }

  size = sizeof(buffer);
  if (ERROR_SUCCESS ==
      RegQueryValueEx(key,"mod_path",0,&type,(BYTE*)buffer,&size)) {
    mod_path = buffer;
  }

  size = sizeof(buffer);
  if (ERROR_SUCCESS ==
      RegQueryValueEx(key,"var_path",0,&type,(BYTE*)buffer,&size)) {
    var_path = buffer;
  }

  RegCloseKey(key);
}


#endif
