/* SconeServer (http://www.sconemad.com)

SconeServer daemon entry point

Copyright (c) 2000-2006 Andrew Wedgbury <wedge@sconemad.com>

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
      kernel->connect_config_console();
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
      std::cout << kernel->name()
                << "-" << kernel->version().get_string()
                << "\n";

      if (opt_version) {
        std::cout << "Built for " << scx::build_type()
                  << " on " << scx::build_time().code() << "\n";
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
  
  return run();
}
