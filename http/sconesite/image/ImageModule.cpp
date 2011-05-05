/* SconeServer (http://www.sconemad.com)

Sconesite Image module

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

#include "http/sconesite/Article.h"

#include "sconex/ModuleInterface.h"
#include "sconex/Module.h"
#include "sconex/FilePath.h"
#include "sconex/FileStat.h"
#include "sconex/ScriptTypes.h"

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

  // ScriptObject methods
  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  virtual scx::ScriptRef* script_method(const scx::ScriptAuth& auth,
					const scx::ScriptRef& ref,
					const std::string& name,
					const scx::ScriptRef* args);

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
scx::ScriptRef* ImageModule::script_op(const scx::ScriptAuth& auth,
				       const scx::ScriptRef& ref,
				       const scx::ScriptOp& op,
				       const scx::ScriptRef* right)
{
  if (op.type() == scx::ScriptOp::Lookup) {
    const std::string name = right->object()->get_string();
    
    // Methods
    if ("thumbnail" == name) {
      return new scx::ScriptMethodRef(ref,name);
    }
  }

  return scx::Module::script_op(auth,ref,op,right);
}

//=============================================================================
scx::ScriptRef* ImageModule::script_method(const scx::ScriptAuth& auth,
					   const scx::ScriptRef& ref,
					   const std::string& name,
					   const scx::ScriptRef* args)
{
  if (name == "thumbnail") {

    const Article* article = scx::get_method_arg<Article>(args,0,"article");
    if (!article) 
      return scx::ScriptError::new_ref("No article specified");

    const scx::ScriptString* a_image = 
      scx::get_method_arg<scx::ScriptString>(args,1,"image");
    if (!a_image) 
      return scx::ScriptError::new_ref("No image specified");

    const scx::FilePath& root = article->get_root();
    scx::FilePath source = root + a_image->get_string();
    scx::FileStat source_stat(source);
    if (!source_stat.exists()) 
      return scx::ScriptError::new_ref("Image does not exist");

    const scx::ScriptString* a_size = 
      scx::get_method_arg<scx::ScriptString>(args,2,"size");
    std::string size = "100x100";
    if (a_size) size = a_size->get_string();

    scx::FilePath dest = root + IMAGE_DIR + size + a_image->get_string();
    scx::FileStat dest_stat(dest);

    if (!dest_stat.exists() || 
	(dest_stat.time() < source_stat.time())) {
      // Thumbnail needs to be generated
      scx::FilePath::mkdir(root + IMAGE_DIR + size,true,0777);
      log("Generating " + size + " thumbnail for " + source.path());
      try {
	Image image;
	image.read(source.path());
	image.transform(size);
	image.write(dest.path());
      } catch (Exception& e) {
	return scx::ScriptError::new_ref(e.what());
      }
    }

    // Generate markup
    std::string href_root = "/" + article->get_href_path();
    std::string img_href = href_root + a_image->get_string();
    std::string thumb_href = href_root + IMAGE_DIR + "/" + size + "/" + 
                             a_image->get_string();

    return scx::ScriptString::new_ref(
      "<a href='"+img_href+"'><img src='"+thumb_href+"'/></a>");
  }

  return scx::Module::script_method(auth,ref,name,args);
}
