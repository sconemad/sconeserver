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

#include <sconex/Kernel.h>
#include <sconex/File.h>

#include <getopt.h>

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
static bool opt_daemonize = false;
static bool opt_interactive = false;
static bool opt_load_config = true;
static std::string conf_path = CONF_PATH;
static std::string mod_path = MOD_PATH;
static std::string var_path = VAR_PATH;
static std::string pid_file;


//===========================================================================
// PidFile - Creates a locked file containing the process ID, which is
// removed on destruction.
class PidFile {
public:
  PidFile() {}
  
  bool create(const scx::FilePath& path) {
    m_path = path;
    if (m_path.path().empty()) return true;
    if (scx::Ok != m_file.open(m_path, scx::File::Write | scx::File::Create |
                               scx::File::Truncate | scx::File::Lock, 0644)) {
      // Failed to open or lock the pidfile
      return false;
    }
    std::ostringstream oss;
    oss << getpid() << "\n";
    m_file.write(oss.str());
    return true;
  }
  
  ~PidFile() {
    if (m_file.is_open()) {
      m_file.close();
      scx::FilePath::rmfile(m_path);
    }
  }

private:
  scx::FilePath m_path;
  scx::File m_file;
};


//===========================================================================
int run()
{
  scx::Kernel* kernel = scx::Kernel::get();

  // Set paths
  kernel->set_conf_path(conf_path);
  kernel->set_mod_path(mod_path);
  kernel->set_var_path(var_path);
  
  // Set config autoloading
  kernel->set_autoload_config(opt_load_config);
    
  // Restart loop
  while (true) {

    // Enter the kernel loop
    if (kernel->run(opt_interactive)) {
      break;
    }

  }

  delete kernel;

  if (opt_interactive) {
    std::cout << std::endl;
  }

  return 0;
}

//=============================================================================
int main(int argc,char* argv[])
{
  scx::Kernel* kernel = scx::Kernel::create("sconeserver", scx::version());

  std::string arg;
  
  // Look for version/help options initially as a special case
  if (argc == 2) {
    arg = argv[1];
    bool opt_version = (arg == "-v" || arg == "--version");
    bool opt_help = (arg == "-h" || arg == "--help");

    if (opt_version || opt_help) {
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
          << " -d         Daemonize on launch\n"
          << " -i         Launch with interactive console\n"
          << " -n         Don't read main configuration file\n"
          << " -c PATH    Set configuration path\n"
          << " -m PATH    Set module path\n"
          << " -l PATH    Set var (log) path\n"
          << " -p FILE    Set file to which the process ID is written\n"
          << "\n";
      }
      // Exit if version/help was provided
      return 0;
    }

  }

  int c;
  extern char* optarg;
  while ((c = getopt(argc,argv,"dinc:m:l:p:")) >= 0) {
    switch (c) {
    case '?': return 1;
    case 'd': opt_daemonize = true; break;
    case 'i': opt_interactive = true;  break;
    case 'n': opt_load_config = false; break;
    case 'c': conf_path = optarg; break;
    case 'm': mod_path = optarg; break;
    case 'l': var_path = optarg; break;
    case 'p': pid_file = optarg; break;
    }
  }

  if (opt_daemonize) {
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

  // Create PID file
  PidFile pf;
  if (!pf.create(pid_file)) {
    std::cout << "ERROR: Server is already running!\n";
    exit(1);
  }
  
  // Don't terminate on silly signals
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sigaction(SIGPIPE,&sa,0);

  // Seed the basic random number generator
  timeval tv;
  gettimeofday(&tv,0);
  srand(tv.tv_usec);
  
  return run();
}
