/* SconeServer (http://www.sconemad.com)

Sconex configuration file

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

#include "sconex/ConfigFile.h"
#include "sconex/File.h"
#include "sconex/ScriptEngine.h"
#include "sconex/FilePath.h"
#include "sconex/FileStat.h"

namespace scx {

//=============================================================================
ConfigFile::ConfigFile(
  const FilePath& filename
)
  : m_filename(filename)
{
  DEBUG_COUNT_CONSTRUCTOR(ConfigFile);
}
	
//=============================================================================
ConfigFile::~ConfigFile()
{
  DEBUG_COUNT_DESTRUCTOR(ConfigFile);
}

//=============================================================================
bool ConfigFile::load(ScriptRef* ctx)
{
  //  ctx->log("LOAD " + m_filename.path());
  File conf;
  conf.open(m_filename,File::Read);
  ScriptEngine* script = new ScriptEngineExec(ScriptAuth::Admin,
					      ctx->ref_copy());
  conf.add_stream(script);
  script->event(Stream::Readable);
  //  ctx->log("DONE " + m_filename.path());

  return true;
}

};
