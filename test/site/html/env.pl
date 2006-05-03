#!/usr/bin/perl

print "Content-Type: text/html\r\n";
print "\r\n";

print "<html><head></head><body>\n";
print "<h1>CGI Environment variables<\h1>\n";
print "<table>\n";
print "<tr> <th>Variable</th> <th>Value</th> </tr>\n";
foreach $v ("SERVER_NAME",
            "SERVER_PORT",
            "SCRIPT_NAME",
            "QUERY_STRING",
            "REQUEST_METHOD",
            "CONTENT_TYPE",
            "CONTENT_LENGTH") {
  print "<tr> <td>". $v ." : </td> <td>". $ENV{$v} ."</td> </tr>\n";
}
print "</table>\n";
print "</body></html>\n";


