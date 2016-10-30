#!/usr/bin/perl -w

#############################################################################
# Program       : renamedat.pl                                              #
# Author        : Luis Mendoza                                              #
# Date          : 12.12.11                                                  #
# SVN Info      : $Id: renamedat.pl 6527 2014-05-29 20:04:00Z slagelwa $
#                                                                           #
# Rename FXXXXX.dat files to original mz(X)ML base name                     #
# Copyright (C) 2011 Luis Mendoza                                           #
#                                                                           #
# This library is free software; you can redistribute it and/or             #
# modify it under the terms of the GNU Lesser General Public                #
# License as published by the Free Software Foundation; either              #
# version 2.1 of the License, or (at your option) any later version.        #
#                                                                           #
# This library is distributed in the hope that it will be useful,           #
# but WITHOUT ANY WARRANTY; without even the implied warranty of            #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         #
# General Public License for more details.                                  #
#                                                                           #
# You should have received a copy of the GNU Lesser General Public          #
# License along with this library; if not, write to the Free Software       #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA #
#                                                                           #
# Institute for Systems Biology                                             #
# 401 Terry Avenue North                                                    #
# Seattle, WA  98109  USA                                                   #
# lmendoza@isb                                                              #
#                                                                           #
#############################################################################

use strict;
use File::Basename;

my $failed = 0;

my $USAGE=<<"EOU";
$0
Rename FXXXXX.dat files to original mzML/mzXML base name

Usage:
   $0 file.dat
   $0 *.dat

EOU

die "I need at least one input file! Exiting.\n\n$USAGE" unless (@ARGV);

my @datfiles = @ARGV;

for my $datfile (@datfiles) {
    my $basename;
    open(INFILE, $datfile) || die "Cannot open $datfile. Exiting.\n\n";

    my $linenum = 0;
    while(<INFILE>) {

	if (/Conversion of (.*).mzX?ML to mascot generic/i) {
	    $basename = $1;
	    last;
	}

	if (/^FILE=(.*).mgf/) {  # as a fall-back, in case there is no COM line
	    my $fullpath = $1;
	    fileparse_set_fstype("MSWin32") if ($fullpath =~ /.:\\/); # force Windows path parsing?
	    $basename = fileparse($fullpath);
	}

	last if ($linenum++ > 100);
    }
    close INFILE;

    if ($basename) {
	my $newname = "$basename.dat";

	if (-e $newname) {
	    print "$datfile: $newname already exists; SKIPPING\n";
	    $failed++;
	} else {
	    print "$datfile ---> $newname ... ";

	    if (rename $datfile, $newname) {
		print "success!\n";
	    } else {
		print "FAILED ** $! **\n";
		$failed++;
	    }
	}

    } else {
	print "$datfile: Basename not found; SKIPPING\n";
	$failed++;
    }

}

print "\nDONE! ($failed files failed)\n\n";
exit($failed);
