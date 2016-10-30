#!/usr/bin/perl 

# a little script to help users debug their installations:
#
# By pointing a browser at http://localhost/tpp-bin/check_env.pl it tests:
# 1) whether tpp-bin is set up to allow execution of scripts (does anything show up?)
# 2) whether the environment variables are set properly

print "Content-type: text/html\n\n"; 
print "<HTML><BODY BGCOLOR=\"\#FFFFFF\">\n";
while (($key, $val) = each %ENV) { 
  print "$key = $val<BR/>\n"; 
}
print "</BODY></HTML>\n";

