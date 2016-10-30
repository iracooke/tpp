#!/usr/bin/perl
#
###############################################################################
# Program     : replaceall.pl
# Author      : Eric Deutsch <edeutsch at systemsbiology.org>
# 2001/03/26
#
#
# Copyright (C) 2008 Eric Deutsch
#                                                                           
# This library is free software; you can redistribute it and/or             
# modify it under the terms of the GNU Lesser General Public                
# License as published by the Free Software Foundation; either              
# version 2.1 of the License, or (at your option) any later version.        
#                                                                           
# This library is distributed in the hope that it will be useful,           
# but WITHOUT ANY WARRANTY; without even the implied warranty of            
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         
# General Public License for more details.                                  
#                                                                           
# You should have received a copy of the GNU Lesser General Public          
# License along with this library; if not, write to the Free Software       
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
#                                                                           
# Insitute for Systems Biology                                              
# 1441 North 34th St.                                                       
# Seattle, WA  98103  USA                                                   
#
#
#
#
# Description : This program is a search and replace function for one or
#               more files.  For example, to replace "Happy" with "SAD"
#               in all .cgi programs in the current directory:
#
#  % replaceall.pl Happy SAD *.cgi
#
###############################################################################

  use strict;

  main();
  exit(0);


###############################################################################
sub main {

  #### Parse the input parameters
  my @args = @ARGV;

  my $srcstr;
  my $silent = 0;
  while (1) {
    my $first_param = shift(@ARGV) || usage();
    if ($first_param eq '--silent') {
      $silent = 1;
    } else {
      $srcstr = $first_param;
      last;
    }
  }


  my $dststr = shift(@ARGV) || usage();
  my $filename;
  my $line;
  my $origline;
  my $changed;


  #### Loop over each supplied filename
  while ($filename = shift(@ARGV)) {

    print "Processing $filename\n";

    #### Get the properties of the original file and open it
    my @properties = stat($filename);
    my $mode = $properties[2];
    open (INFILE,$filename)
      || die "Unable to open $filename for read";

    #### Open a change file for output
    open (OUTFILE,"> $filename.replaceall")
      || die "Unable to open $filename.replaceall for write";

    #### Begin with a cleared changed flag
    $changed=0;

    #### Read through the file line by line, making changes as appropriate
    while ($line = <INFILE>) {
      $origline=$line;
      $line =~ s/$srcstr/$dststr/g;
      if ($line ne $origline) {
        $changed++;
	unless ($silent) {
	  print "$filename - $origline";
	  print "$filename + $line";
	}
      }
      print OUTFILE $line;

    }
	close(INFILE); close(OUTFILE);

    #### If there was a change made, then mv the change file to the
    #### original file and set the original mode
    if ($changed) {
      rename("$filename.replaceall",$filename);
	  chmod($mode,$filename);

    #### If no change was made, then just remove the change file
    } else {
      unlink("$filename.replaceall");
    }

  }

}


###############################################################################
# usage: Print usage information
###############################################################################
sub usage {
  die "replaceall.pl searchstring replacestring file1 [file2 ...]";
}
