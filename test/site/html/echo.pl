#!/usr/bin/perl -w

use IO::Handle;
STDOUT->autoflush(1);

print "HELLO\n";
while (<STDIN>) {
  print "TEST: $_";
}
print "GOODBYE\n";
