/* SconeServer (http://www.sconemad.com)

Sconesite Image module

Copyright (c) 2000-2009 Andrew Wedgbury <wedge@sconemad.com>

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


#include "sconesite/ArgFile.h"
#include "sconesite/Article.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"

#include <Magick++.h>
using namespace Magick;

const char* IMAGE_DIR = ".image";

//=========================================================================
class ImageModule : public scx::Module {
public:

  ImageModule();
  virtual ~ImageModule();

  virtual std::string info() const;

  virtual int init();
  
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);

  virtual bool connect(
    scx::Descriptor* endpoint,
    scx::ArgList* args
  );

protected:

private:

};

SCONESERVER_MODULE(ImageModule);

//=========================================================================
ImageModule::ImageModule(
) : scx::Module("image",scx::version())
{
  ::InitializeMagick(NULL);
}

//=========================================================================
ImageModule::~ImageModule()
{

}

//=========================================================================
std::string ImageModule::info() const
{
  return "Image manipulation";
}

//=========================================================================
int ImageModule::init()
{
  return Module::init();
}

//=============================================================================
scx::Arg* ImageModule::arg_lookup(const std::string& name)
{
  // Sub-objects
  
  if ("thumbnail" == name) {
    return new_method(name);
  }

  return SCXBASE Module::arg_lookup(name);
}

//=============================================================================
scx::Arg* ImageModule::arg_method(
  const scx::Auth& auth,
  const std::string& name,
  scx::Arg* args
)
{
  scx::ArgList* l = dynamic_cast<scx::ArgList*>(args);

  if (name == "thumbnail") {
    scx::Arg* a_art = l->get(0);
    if (!a_art) return new scx::ArgError("No article specified");
    scx::ArgObject* a_obj = dynamic_cast<scx::ArgObject*>(a_art);
    if (!a_obj) return new scx::ArgError("No article specified");
    Article* article = dynamic_cast<Article*>(a_obj->get_object());
    if (!article) return new scx::ArgError("No article specified");

    const scx::FilePath& root = article->get_root();

    scx::Arg* a_file = l->get(1);
    if (!a_file) return new scx::ArgError("No file specified");
    //    ArgFile* file = dynamic_cast<ArgFile*>(a_file);
    scx::FilePath source = root + a_file->get_string();
    scx::FileStat source_stat(source);

    if (!source_stat.exists()) return new scx::ArgError("Image does not exist");

    scx::Arg* a_thumb = l->get(2);
    std::string thumb = "100x100";
    if (a_thumb) thumb = a_thumb->get_string();

    scx::FilePath dest = root + IMAGE_DIR + thumb + a_file->get_string();
    scx::FileStat dest_stat(dest);

    if (!dest_stat.exists() || 
	(dest_stat.time() < source_stat.time())) {
      // Thumbnail needs to be generated
      scx::FilePath::mkdir(root + IMAGE_DIR + thumb,true,0777);
      log("Generating " + thumb + " thumbnail for " + source.path());
      try {
	Image image;
	image.read(source.path());
	image.transform(thumb);
	image.write(dest.path());
      } catch (Exception& e) {
	return new scx::ArgError(e.what());
      }
    }

    // Generate markup
    std::string href_root = "/" + article->get_href_path();
    std::string img_href = href_root + a_file->get_string();
    std::string thumb_href = href_root + IMAGE_DIR + "/" + thumb + "/" + a_file->get_string();

    return new scx::ArgString("<a href='"+img_href+"'><img src='"+thumb_href+"'/></a>");
  }

  return SCXBASE Module::arg_method(auth,name,args);
}

//=========================================================================
bool ImageModule::connect(
  scx::Descriptor* endpoint,
  scx::ArgList* args
)
{
  return true;
}

