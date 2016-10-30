#!/usr/bin/perl 

#
# a little script to create passwords for TPP GUI
# usage: tpp_crypt.pl PASSWORD SALT
#
# where SALT is traditionally isbTPPspc
# so for the guest account, whose password is "guest"
# tpp_crypt.pl guest isbTPPspc >.password
#

my $encrypted_string = crypt($ARGV[0],$ARGV[1]); #take the string and the salt and put through crypt() 

print $encrypted_string 
; 
