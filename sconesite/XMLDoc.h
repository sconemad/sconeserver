/* SconeServer (http://www.sconemad.com)

Sconesite XML document

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

#ifndef sconesiteXMLDoc_h
#define sconesiteXMLDoc_h

#include "sconex/FilePath.h"
#include "sconex/Date.h"
#include "sconex/ArgObject.h"
#include "sconex/Mutex.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/HTMLtree.h>

typedef std::map<std::string,std::string> XMLAttrs;

class Context;

//=========================================================================
class ArticleHeading {
public:
  ArticleHeading(int level, const std::string& name, int index);
  ~ArticleHeading();

  int level() const;
  const std::string& name() const;
  int index() const;
  
  void clear();
  void add(int level, const std::string& name, int index);

  const ArticleHeading* lookup_index(int index) const;
  std::string lookup_anchor(int index) const;
  std::string lookup_section(int index) const;
  scx::Arg* get_arg(
    const std::string& anchor_prefix = "",
    const std::string& section_prefix = ""
  ) const;

private:
  
  int m_level;
  std::string m_name;
  int m_index;
  
  typedef std::vector<ArticleHeading*> ArticleHeadingList;
  ArticleHeadingList m_subs;
};


//=========================================================================
class ArticleBody : public scx::ArgObjectInterface {

public:
  
  virtual const std::string& get_name() const =0;
  virtual const scx::FilePath& get_root() const =0;
  virtual const std::string& get_file() const =0;
  virtual scx::FilePath get_filepath() const =0;

  virtual bool process(Context& context) =0;
  virtual bool purge(const scx::Date& purge_time) =0;

  virtual const ArticleHeading& get_headings() const =0;
  
};


//=========================================================================
class XMLDoc : public ArticleBody {

public:

  XMLDoc(const std::string& name,
         const scx::FilePath& root,
         const std::string& file);
  
  virtual ~XMLDoc();
  
  virtual const std::string& get_name() const;
  virtual const scx::FilePath& get_root() const;
  virtual const std::string& get_file() const;
  virtual scx::FilePath get_filepath() const;

  virtual bool process(Context& context);

  const scx::Date& get_modtime() const;

  void parse_error(const std::string& msg);

  virtual bool purge(const scx::Date& purge_time);
  // Unload the article if it hasn't been accessed since purge_time
  
  // ArgObject interface
  virtual std::string name() const;
  virtual scx::Arg* arg_resolve(const std::string& name);
  virtual scx::Arg* arg_lookup(const std::string& name);
  virtual scx::Arg* arg_method(const scx::Auth& auth,const std::string& name,scx::Arg* args);

  static void get_node_text(std::string& txt, xmlNode* node);

protected:

  virtual void process_node(Context& context, xmlNode* node);
  
  virtual void handle_open();
  virtual void handle_close();
  
  bool open();
  void close();  

  std::string m_name;
  scx::FilePath m_root;
  std::string m_file;
  scx::Date m_modtime;

  xmlDoc* m_xmldoc;
  std::string m_errors;
  
  scx::Date m_last_access;
  int m_clients;
  bool m_opening;

  static scx::Mutex* m_clients_mutex;
};

#endif
