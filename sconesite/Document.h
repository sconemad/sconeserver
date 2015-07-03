/* SconeServer (http://www.sconemad.com)

Sconesite document

Copyright (c) 2000-2015 Andrew Wedgbury <wedge@sconemad.com>

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

#ifndef sconesiteDocument_h
#define sconesiteDocument_h

#include "Heading.h"

#include <sconex/FilePath.h>
#include <sconex/Date.h>
#include <sconex/ScriptBase.h>
#include <sconex/ScriptTypes.h>
#include <sconex/Mutex.h>
#include <sconex/Provider.h>

class Context;

typedef std::map<std::string,std::string> NodeAttrs;

//=========================================================================
// Document - Sconesite document interface
//
class Document : public scx::ScriptObject {
public:

  typedef std::vector<std::string> ErrorList;

  static scx::ScriptRefTo<Document>* find(const std::string& name,
					  const scx::FilePath& root);

  Document(const std::string& name,
	   const scx::FilePath& root,
	   const std::string& file);

  virtual ~Document();

  // Get the document name  
  const std::string& get_name() const;

  // Get the directory containing the document
  const scx::FilePath& get_root() const;
  
  // Get the document's file name
  const std::string& get_file() const;

  // Get the full path of the document's file
  scx::FilePath get_filepath() const;

  // Get document headings
  const Heading& get_headings() const;

  // Get document parse errors
  const ErrorList& get_errors() const;
  
  // Log any reported errors in processing the document
  void log_errors() const;

  // Close the document if it isn't being processed and it was last
  // accessed over purge_time ago. Designed to free up memory.
  bool purge(const scx::Date& purge_time);

  // Process the document within the specified context
  bool process(Context& context);

  static void register_document_type(const std::string& type,
				     scx::Provider<Document>* factory);
  static void unregister_document_type(const std::string& type,
				       scx::Provider<Document>* factory);

  // ScriptObject methods
  virtual std::string get_string() const;

  virtual scx::ScriptRef* script_op(const scx::ScriptAuth& auth,
				    const scx::ScriptRef& ref,
				    const scx::ScriptOp& op,
				    const scx::ScriptRef* right=0);

  typedef scx::ScriptRefTo<Document> Ref;

protected:

  bool open();

  // Report an error in processing the document
  void report_error(const std::string& error);

  virtual bool is_open() const =0;
  virtual bool handle_open() =0;
  virtual bool handle_process(Context& context) =0;
  virtual void handle_close() =0;

  std::string m_name;
  scx::FilePath m_root;
  std::string m_file;

  scx::Date m_modtime;
  scx::Date m_last_access;
  
  int m_clients;
  volatile bool m_opening;
  static scx::Mutex* m_clients_mutex;
  
  Heading m_headings;
  ErrorList m_errors;
  
  static void init();

  static scx::ProviderScheme<Document>* s_document_providers;
  
};

#endif
