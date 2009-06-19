/* SconeServer (http://www.sconemad.com)

Test Builder Control Stream

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


#include "TestBuilderModule.h"
#include "TestBuilderControlStream.h"
#include "BuildProfile.h"

#include "http/HTTPModule.h"
#include "http/Request.h"
#include "http/MessageStream.h"
#include "http/Status.h"

#include "sconex/Stream.h"
#include "sconex/Date.h"
#include "sconex/Kernel.h"

const char ALPHA[] =
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char ALPHANUM[] =
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"1234567890_";

//=========================================================================
TestBuilderControlStream::TestBuilderControlStream(
  TestBuilderModule& module
) : http::ResponseStream("testbuilder::control"),
    m_module(module)
{

}

//=========================================================================
TestBuilderControlStream::~TestBuilderControlStream()
{
  
}

//=========================================================================
std::string html_option(
  const std::string& name,
  const std::string& value,
  const std::string& def_value
)
{
  return "<option value='" + value + "' " +
    (value == def_value ? "selected" : "") + " >" +
    name + "</option>\n";
}

//=========================================================================
std::string html_navlink(
  const std::string& value,
  const std::string& cur_value,
  const std::string& href,
  const std::string& li_class = ""
)
{
  std::string li = !li_class.empty() ?
    "<li class='index-" + li_class + "'>" :
    "<li>";
  
  return li + "<a href='" + href + "'>" +
    (value == cur_value ? ("<b>" + value + "</b>") : value) +
    "</a></li>\n";
}

//=========================================================================
scx::Condition TestBuilderControlStream::send(http::MessageStream& msg)
{
  const http::Request& req = msg.get_request();
  const scx::Uri& uri = req.get_uri();
  std::string base = "/" + uri.get_path();
  
  msg.get_response().set_header("Content-Type","text/html");
  msg.get_response().set_header("Cache-Control","no-cache");
  msg.get_response().set_header("Last-Modified",scx::Date::now().string());
  
  // Get the profile name, if any
  std::string profile = get_opt("profile");
  
  // Get the build ID, if any
  std::string build = get_opt("build");
  
  // Get the build filter, if any
  std::string showbuilds = get_opt("showbuilds");
  
  std::string alert;
  std::string alert_title;
  std::string error_profile;
  
  // New profile
  if (is_opt("new_profile")) {
    // Validate new profile name
    if (profile.empty() ||
        profile.find_first_not_of(ALPHANUM) != std::string::npos ||
        profile.find_first_not_of(ALPHA) == 0 ||
        !m_module.add_profile(profile)) {
      alert_title = "Error creating profile";
      if (m_module.lookup_profile(profile)) {
        alert =
          "<p>"
          "A profile named \"" + profile + "\" already exists."
          "</p>\n";
      } else {
        alert =
          "<p>"
          "Profile name \"" + profile + "\" is invalid."
          "</p>\n"
          "<p>"
          "A profile name can only contain letters, numbers and "
          "underscores, and must start with a letter."
          "</p>";
      }
      error_profile = profile;
      profile = "";
    }
  }
  
  // Save profile (or submit build)
  if (is_opt("save_profile") ||
      is_opt("start_build")) {
    BuildProfile* profile_obj = m_module.lookup_profile(profile);
    profile_obj->set_source_method(get_opt("source_method"));
    profile_obj->set_source_uri(get_opt("source_uri"));
    profile_obj->set_configure_command(get_opt("configure_command"));
    profile_obj->set_make_targets(get_opt("make_targets"));
    m_module.save_profiles();
  }
  
  // Remove profile
  if (is_opt("remove_profile")) {
    if (m_module.remove_profile(profile)) {
      alert_title = "Profile removed";
      alert =
        "<p>"
        "The profile \"" + profile + "\" has been removed."
        "</p>\n";
      profile = "";
    } else {
      alert_title = "Error removing profile";
      alert =
        "<p>"
        "The profile \"" + profile + "\" could not be removed."
        "</p>\n";
    }
    m_module.save_profiles();
  }
  
  // Start build
  if (is_opt("start_build")) {
    build = m_module.submit_build(profile);
    if (!build.empty()) {
      alert_title = "Build submitted";
      alert =
        "<p>"
        "Build \"" + build + "\" has been submitted using the "
        "profile \"" + profile + "\""
        "</p>\n"
        "<p>"
        "<a href='" + base + "?build=" + build + "'>"
        "Click here to view the progress of this build.</a>"
        "</p>\n";
      build = "";
      profile = "";
    } else {
      alert_title = "Error submitting build";
      alert =
        "<p>"
        "An error occurred when submitting a build with profile \"" + profile + "\"."
        "</p>\n";
    }
  }

  // Abort build
  if (is_opt("abort_build")) {    
    if (m_module.abort_build(build)) {
      alert_title = "Build aborted";
      alert =
        "<p>"
        "Build \"" + build + "\" has been aborted."
        "</p>\n"
        "<p>\n";
      build = "";
      profile = "";
    } else {
      alert_title = "Error aborting build";
      alert =
        "<p>"
        "An error occurred when attempting to abort build \"" + build + "\"."
        "</p>\n";
    }
  }

  // Remove build
  if (is_opt("remove_build")) {    
    if (m_module.remove_build(build)) {
      alert_title = "Build removed";
      alert =
        "<p>"
        "Build \"" + build + "\" has been removed."
        "</p>\n"
        "<p>\n";
      build = "";
      profile = "";
    } else {
      alert_title = "Error removing build";
      alert =
        "<p>"
        "An error occurred when attempting to remove build \"" + build + "\"."
        "</p>\n";
    }
  }

  // Page title
  
  std::string title;
  if (!profile.empty()) {
    title = "Profile: " + profile;
  } else if (!showbuilds.empty()) {
    title = "All " + showbuilds + " builds";
  } else if (!build.empty()) {
    title = "Build: " + build;
  } else {
    title = "All builds";
  }
  
  // Header
  write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
        "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
        "<html>\n"
        "<head>\n"
        "<title>TestBuilder - " + title + "</title>\n"
        "<link rel='shortcut icon' href='/favicon.ico' />\n"
        "<link rel='stylesheet' href='/main.css' type='text/css' />\n"
        "<link rel='alternate' title='RSS' href='/index.xml' "
        "type='application/rss+xml' />\n"
        "</head>\n");
  
  // Start body and title section
  write("<body>\n"
        "<div class='titlebar'>\n"
        "<img src='title_sconeserver.jpg' alt='SconeServer'>\n"
        "</div>\n");
  

  // Get a dump of the current build information
  scx::Arg* buildstats = m_module.arg_lookup("buildstats");
  
  // Do side section
  write("<div id='side'>\n");
  {
    write("<h2>TestBuilder</h2>\n"
          "<table>\n"
          "<tr> <td>Hostname:</td> <td>" +
          scx::Kernel::get()->get_system_nodename() + "</td> </tr>\n"
          "<tr> <td>System:</td> <td>" +
          scx::Kernel::get()->get_system_version() + "</td> </tr>\n"
          "<tr> <td>Hardware:</td> <td>" +
          scx::Kernel::get()->get_system_hardware() + "</td> </tr>\n"
          "</table>\n"
          "<ul>\n"
          "<li><a href='doc/mod_testbuilder.html'>TestBuilder docs</a></li>\n"
          "<li><a href='doc/index.html'>SconeServer docs</a></li>\n"
          "<li><a href='http://sconeserver.sourceforge.net'>SconeServer web site</a></li>\n"
          "</ul>\n");

    write("<h2>Profiles</h2>\n"
          "<form method='get' action = '" + base + "'>\n"
          "<table>\n"
          "<tr> <td><input type='text' name='profile' size='16' "
          "value='" + error_profile + "' /></td>\n"
          "<td><input type='submit' name='new_profile' class='button' "
          "value='new' /></td>"
          "</tr>\n"
          "</table>\n"
          "</form>\n"
          "<ul>\n");
    
    // Get profiles from module
    scx::Arg* a1 = m_module.arg_lookup("profiles");
    scx::ArgList* l1 = dynamic_cast<scx::ArgList*>(a1);
    for (int i1=0; i1<l1->size(); ++i1) {
      std::string cur_profile = l1->get(i1)->get_string();
      write(html_navlink(cur_profile,profile,base + "?profile=" + cur_profile));
    }
    delete a1;
    write("</ul>\n");
    
    write("<h2>View</h2>\n"
          "<ul>\n");
    if (build.empty() && showbuilds.empty() && profile.empty()) {
      write("<li><a href='" + base + "'><b>ALL</b></a></li>");
    } else {
      write("<li><a href='" + base + "'>ALL</a></li>");
    }
    write(html_navlink("UNSTARTED",showbuilds,
                       base + "?showbuilds=UNSTARTED","UNSTARTED"));
    write(html_navlink("RUNNING",showbuilds,
                       base + "?showbuilds=RUNNING","RUNNING"));
    write(html_navlink("PASSED",showbuilds,
                       base + "?showbuilds=PASSED","PASSED"));
    write(html_navlink("FAILED",showbuilds,
                       base + "?showbuilds=FAILED","FAILED"));
    write(html_navlink("ABORTED",showbuilds,
                       base + "?showbuilds=ABORTED","ABORTED"));
    write("</ul>\n");
    
    write("<h2>Builds</h2>\n"
          "<ul>\n");
    
    l1 = dynamic_cast<scx::ArgList*>(buildstats);
    for (int i1=0; i1<l1->size(); ++i1) {
      const scx::Arg* a2 = l1->get(i1);
      const scx::ArgList* l2 = dynamic_cast<const scx::ArgList*>(a2);
      std::string cur_build = l2->get(1)->get_string();
      std::string state = l2->get(2)->get_string();
      write(html_navlink(cur_build,build,base + "?build=" + cur_build,state));
    }        
    write("</ul>\n");
    
  }
  write("</div>\n");
  
  
  // Main body section

  write("<h1>" + title + "</h1>\n");
  
  // Alert message
  if (!alert.empty()) {
    write("<div class='box'>\n"
          "<h2>" + alert_title + "</h2>\n" +
          alert + "\n"
          "</div>\n");
  } else {
    
    // Display profile
    if (!profile.empty()) {

      BuildProfile* profile_obj = m_module.lookup_profile(profile);
      if (profile_obj) {
      
        write("<div class='box'>\n"
              "<form method='post' action = '" + base + "'>\n"
              "<input type='hidden' value='" + profile +
              "' name='profile' />\n"
              "<table>\n");
        
        std::string source_method =
          html_esc(profile_obj->get_source_method());
        write("<tr>\n"
              "<td>Source method:</td>"
              "<td>\n"
              "<select name='source_method'>\n");
        
        scx::Arg* a1 = m_module.arg_lookup("source_methods");
        scx::ArgList* l1 = dynamic_cast<scx::ArgList*>(a1);
        for (int i1=0; i1<l1->size(); ++i1) {
          const scx::Arg* a2 = l1->get(i1);
          const scx::ArgList* l2 = dynamic_cast<const scx::ArgList*>(a2);
          std::string name = l2->get(0)->get_string();
          std::string desc = l2->get(1)->get_string();
          write(html_option(desc,name,source_method));
        }
        delete a1;
        write("</select>\n"
              "</td>\n"
              "</tr>\n");
        
        std::string source_uri =
          html_esc(profile_obj->get_source_uri());
        write("<tr>\n"
              "<td>Source location:</td>\n"
              "<td><input type='text' value='" + source_uri +
              "' name='source_uri' size='60' /></td>\n"
              "</tr>\n");
        
        std::string configure_command =
          html_esc(profile_obj->get_configure_command());
        write("<tr>\n"
              "<td>Configure command:</td>\n"
              "<td><input type='text' value='" + configure_command +
            "' name='configure_command' size='60' /></td>\n"
              "</tr>\n");
        
        std::string make_targets =
          html_esc(profile_obj->get_make_targets());
        write("<tr>\n"
              "<td>Make targets:</td>\n"
              "<td><input type='text' value='" + make_targets +
              "' name='make_targets' size='20' /></td>\n"
              "</tr>\n");
        
        write ("</table>\n"
               "<p>\n"
               "<input type='submit' name='save_profile' class='button' "
               "value='Save profile' />\n"
               "<input type='submit' name='remove_profile' class='button' "
               "value='Remove profile' />\n"
               "<input type='submit' name='start_build' class='button' "
               "value='Start build' />\n"
               "</p>\n"
               "</form>\n"
               "</div>\n");
      }
    }
    
    // Builds
    scx::ArgList* l1 = dynamic_cast<scx::ArgList*>(buildstats);
    int disp = 0;
    
    for (int i1=0; i1<l1->size(); ++i1) {
      
      const scx::Arg* a2 = l1->get(i1);
      const scx::ArgList* l2 = dynamic_cast<const scx::ArgList*>(a2);
      
      std::string build_profile = l2->get(0)->get_string();
      if (!profile.empty() && profile != build_profile) {
        // If there is a profile selected, then only display builds
        // for this profile
        continue;
      }
      
      std::string id = l2->get(1)->get_string();
      if (!build.empty() && build != id) {
        // If there is a build selected, then only display this build
        continue;
      }
      
      std::string state = l2->get(2)->get_string();
      if (!showbuilds.empty() && showbuilds != state) {
        // If there is a build selected, then only display this build
        continue;
      }
      
      std::string href = "/builds/" + id;
      ++disp;
      
      std::string profile_href = base + "?profile=" + build_profile;
      std::string build_href = base + "?build=" + id;
      write("<div class='box'>\n"
            "<h2 class='build-" + state + "'>"
            "<span class='date'><a href='" + profile_href + "'>" +
            build_profile + "</a></span> " +
            "Build <a href='" + build_href + "'>" + id + "</a> (" +
            state + ")</h2>\n");
      
      write("<table cellspacing='0px' class='steps'>\n"
            "<tr> <th>Step</th> <th>Start time</th> "
            "<th>Duration</th> <th>State</th> <th>Logs</th> </tr>\n");
      const scx::Arg* a3 = l2->get(3);
      const scx::ArgList* l3 = dynamic_cast<const scx::ArgList*>(a3);
      for (int i3=0; i3<l3->size(); ++i3) {
        
        const scx::Arg* a4 = l3->get(i3);
        const scx::ArgList* l4 = dynamic_cast<const scx::ArgList*>(a4);
        
        std::string step_name = l4->get(0)->get_string();
        std::string step_start = l4->get(1)->get_string();
        std::string step_duration = l4->get(2)->get_string();
        std::string step_state = l4->get(3)->get_string();
        std::string log_link;
        if (step_state != "UNSTARTED") {
          log_link =
            "<a href='" + href + "/testbuilder_" + step_name + ".log'>View</a>";
        }
        
        write("<tr class='" + step_state + "'> "
              "<td>" + step_name + "</td>"
              "<td><nobr>" + step_start + "</nobr></td>"
              "<td>" + step_duration + "</td>"
              "<td>" + step_state + "</td>"
              "<td>" + log_link + "</td> "
              "</tr>\n");
      }
      write("</table>\n");
      
      write("<form method='post' action = '" + base + "'>\n"
            "<p>\n"
            "<input type='hidden' value='" + id + "' name='build' />\n");
      write("<input type='submit' name='remove_build' class='button' value='Remove build' />\n");
      if (state == "UNSTARTED" ||
          state == "RUNNING") {
        write("<input type='submit' name='abort_build' class='button' value='Abort build' />\n");
      }
      if (state != "UNSTARTED") {
        write(" <a href='" + href + "'>[ View files ]</a>");
      }
      write("</p>\n"
            "</form>\n"
            "</div>\n");
    }
    
    // Display note if no builds displayed
    if (!disp) {
      write("<div class='box'>\n"
            "<h2>No builds to display</h2>\n");
      if (!showbuilds.empty()) {
        write("<p>"
              "There are currently no builds in this state. "
              "<a href='" + base + "'>Click here to show all builds.</a>"
              "</p>\n");
      } else if (!profile.empty()) {
        write("<p>"
              "There are currently no builds using this profile. "
              "Press the 'Start Build' button to start one."
              "</p>");
      } else if (!build.empty()) {
        write("<p>"
              "Unknown build. "
              "<a href='" + base + "'>Click here to show all builds.</a>"
              "</p>\n");
      } else {
        write("<p>"
              "To start a build, select the required profile from the "
              "list on the left, and press the 'Start Build' button."
              "</p>"
              "<p>"
              "To create a new build profile, enter a name in the box "
              "on the left, and press the 'New' button."
              "</p>\n");
      }

      write("</div>\n");
    }
    
  } // !alert
  
  // End body
  write("<address>\n"
        "Copyright (c) 2006 Andrew Wedgbury / wedge (at) sconemad (dot) com\n"
        "</address>\n"
        "</body\n"
        "</html>\n");
  
  delete buildstats;
  
  return scx::Close;
}
