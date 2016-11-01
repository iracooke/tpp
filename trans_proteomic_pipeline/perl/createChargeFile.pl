#!/usr/bin/perl

###############################################################################
# Program     : createChargeFile.pl
# Author      : Zhi Sun
# 2009/06
#
#
# Copyright (C) 2009 Zhi Sun
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
# Description : 
#
# input file: CPM out of ms2 file
# input dir: contain a list of dta file
# CPM project info:http://pcarvalho.com/patternlab
# in ms2 file: S -> scan num
#              Z -> predicated charge
#
# ./createChargeFile.pl -i 022008_F10_ETD_2.afterCPM.ms2 -f ms2
# ./createChargeFile.pl -d 022008_F10_ETD_2 -f dta
#
# output format: <spectrum_id>	<charges>
###############################################################################


use strict;
use Getopt::Long;

$Getopt::Long::autoabbrev = 1;
$Getopt::Long::ignorecase = 0;
my %options;
GetOptions(\%options,'help|h', 'inputFile=s', 'outputChargeFile=s',
          'directory=s','format=s');

my $inputfile = $options{'inputFile'};
my $outputfile= $options{'outputChargeFile'};
my $inputdir = $options{'directory'};
my $format = $options{'format'};

if((! $options{'inputFile'} && !$options{'directory'}) || 
    ! $options{'format'})
{
  printUsage();
}

my %scan=();
my $scannum=0;
my $charge=0;

if( $inputfile && lc($format) eq 'ms2')
{
  if(! $outputfile)
  {
    $outputfile=$inputfile;
	$outputfile=~ s/\..*$//;
	$outputfile .=".charge";
  }

  open(IN, "<$inputfile") or die "cannot open $inputfile\n";
  foreach (<IN>)
  {
    chomp;
    next if(/^H\s+/);
    next if(/^\d+\.\d+\s+\d+\.\d+/);
    if(/^S\s+(\d+)\s+/)
    {
      $scannum=$1;
    }
    if(/^Z\s+(\d+)\s+/)
    {
      $charge=$1;
      push (@{$scan{$scannum}},$charge);
    }
  
  }
}
if($inputdir && lc($format) eq 'dta')
{
  if(! $outputfile)
  {
    $inputdir =~ /.*\/(.*)/;
    $outputfile=$1.".charge";
  }
  print "$outputfile\n";

  open(OUT, ">$outputfile") or die "cannot open $outputfile\n";

  my @dta = `ls $inputdir | grep 'dta'`;
  foreach my $filename (@dta)
  {
    chomp $filename;
	next if($filename =~ /^$/);
    $filename =~ /.*((\d+)\.){2}(\d)\.dta$/;
    $scannum = $2;
	$charge = $3;
	$scannum =~ s/^0+//;
	push (@{$scan{$scannum}},$charge); 
  }
}

open (OUT, ">$outputfile") or die "cannot open $outputfile\n";
print OUT "#1.scan number 2.charges\n";
foreach my $num (sort {$a <=> $b} keys %scan)
{
  my @charges = @{$scan{$num}};
  print OUT "$num\t". join(",", @charges)."\n";
}


close OUT;


exit;
#*********************************************************************
sub printUsage {
  print( <<"  END" );
    Usage:  createChargeFile.pl [ -i -o -h]
	required:
	  -i, --inputFile or
	  -d, --directory
	  -f, --format: format of input file or directory
	              the version only support CPM output of ms2 format
				  and dta directory
	optional:
      -h, --help          Print this usage information and exit
      -o, --outputChargeFile

  END
  exit;   
 }

