/* SconeServer (http://www.sconemad.com)

Test Build

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

#ifndef testbuilderBuild_h
#define testbuilderBuild_h

#include "BuildProfile.h"

//#########################################################################
class BuildStep {

public:

  BuildStep(const std::string& command);

private:

  std::string m_command;
  

};

//#########################################################################
class Build {

public:

  Build(const BuildProfile& profile);

  ~Build();

private:

  std::string m_profile;
  std::string m_id;

  std::list<BuildStep*> m_steps;
  
};

#endif
