#!/usr/bin/perl -w

print "Content-Type: text/html\r\n";
print "\r\n";

print "<html>\n";
print "<head></head>\n";
print "<body>\n";

print "<h1>HARRY POTTER</h1>\r\n";

open(FILE,"/home/wedge/doc/hp.txt");
while (<FILE>) {

  foreach my $a ('Harry','Potter',
                 'Ron','Weasley','Fred','George','Ginny','Arthur',
                 'Hermione','Granger') {
    s/$a/<span style='color:#0c0'>$a<\/span>/g;
  }
  foreach my $a ('Draco','Malfoy','Lucius',
                 'Crabbe','Goyle',
                 'Lord Voldemort','Voldemort','Dark Lord',
                 'Bellatrix','Narcissa') {
    s/$a/<span style='color:#f00'>$a<\/span>/g;
  }
  foreach my $a ('Albus','Dumbledore',
                 'Severus','Snape',
                 'McGonagall',
                 'Argus','Filch',
                 'Slughorn','Sirius',
                 'Rubeus','Hagrid',
                 'Trelawney',
                 'Half Blood Prince',
                 'Half-Blood Prince') {
    s/$a/<span style='color:#00f'>$a<\/span>/g;
  }
  foreach my $a ('Hogwarts','Gryffindor','Slytherin','Hufflepuff','Ravenclaw') {
    s/$a/<span style='color:#f0f'>$a<\/span>/g;
  }
  s/"([^"]+)"/<big>&ldquo;<\/big> <em>$1<\/em> <big>&rdquo;<\/big>/g;

  if (/^(Chapter \d+.*)$/) {
    print "<h2>$1</h2>\n";
  } else {
    print "<p>\n";
    print $_;
    print "</p>\n";
  }
}
close(FILE);

print "</body>\n";
print "</html>\n";
