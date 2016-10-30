#!/usr/bin/perl -w

#############################################################################
# Program       : fileDownloader.pl                                         #
# Author        : Luis Mendoza                                              #
# Date          : 19.07.08                                                  #
# SVN Info      : $Id: fileDownloader.pl 4511 2009-08-03 20:36:34Z pcbrefugee $
#                                                                           #
# Download a file via wget with options for md5 verification and unzipping  #
# Copyright (C) 2008 Luis Mendoza                                           #
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
# 1441 North 34th St.                                                       #
# Seattle, WA  98103  USA                                                   #
# lmendoza@isb                                                              #
#                                                                           #
#############################################################################

use strict;

# keep it simple for now; will add options as required
my $filename = shift || &usage("file name");
my $filemd5  = shift || &usage("md5 sum");

my $base_dir  = ''; # needed for obscure reasons involving win32, petunia, and the perl_paths makefile step
my $md5cmd   = '/usr/bin/md5sum';     # full path to md5sum
my $wgetcmd  = '/usr/bin/wget';       # full path to wget
my $unzipcmd = '/usr/bin/unzip -u';   # full path to unzip (-u extracts and updates files)

my $base_url = 'http://www.peptideatlas.org/cgi/fileDownloader.pl';

&report_error(
	      "File $filename already exists locally. Please remove or rename and re-attempt download.",
	      1)
    if -e $filename;

&download();
&verifymd5();
&unzip() if ($filename =~ /\.zip$/);

print "Done!\n";
exit(0);

############################################
# da subs
############################################
sub unzip {
    print "...Unzipping\n";
    system("$unzipcmd $filename");
    &report_error("there was a problem with unzip: $?",1) if ($?);

    print "...Deleting original zip file\n";
    unless (unlink "$filename") {
	&report_error("there was a problem deleting zip file. Please delete later.",0);
    }

}

sub verifymd5 {
    print "...Verifying md5\n";
    my @md5res = split /\s+/, `$md5cmd $filename`;
    &report_error("there was a problem with md5sum: $?",1) if ($?);
    my $md5sum = $md5res[0];

    &report_error("md5 checksum does not match input! - Please delete this file ($filename) and re-download.",1)
	if ($md5sum ne $filemd5);
}

sub download {
    print "...Downloading\n";
    my $url = "${base_url}?file_name=$filename&src=speclib";
    system("$wgetcmd -q -O $filename \"$url\"");

    &report_error("there was a problem with wget: $?",1) if ($?);
}

sub report_error {
    my $errstring = shift;
    my $fatal = shift || 0;

    print "ERROR: $errstring\n";
    exit($fatal) if $fatal;
}

sub usage {
    my $bad_param = shift || '';
    print "ERROR: Missing input parameter: $bad_param\n\n" if $bad_param;
    print "Usage: $0 <file name> <md5 sum>\n\n";
    exit(1);
}
