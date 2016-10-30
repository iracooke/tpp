#!/usr/bin/perl -w

#############################################################################
# Program       : pkl2mzXML.pl                                              #
# Author        : Zhi Sun                                                   #
# Date          : 11.12.08                                                  #
#                                                                           #
#                                                                           #
#                                                                           #
#                                                                           #
# Copyright (C) 2008 Zhi Sun                                                # 
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
# Institute for Systems Biology                                              #
# 401 Terry Avenue North                                                       #
# Seattle, WA  98109  USA                                                   #
# akeller@systemsbiology.org                                                #
#                                                                           #
#############################################################################


use strict;

if(@ARGV <1)
{
    print "pkl2mzXML\n";
    print "---------\n\n";
    print "please input a pkl file, or multiple files with *.pkl\n";
    exit(-1);
}

`mkdir convert`;

foreach (@ARGV)
{
  my $file = $_;
  my $dtaFileName='';
  my $basename = '';
  my $counter="00000";
  my $mz =0;
  $basename = $file;
  $basename =~ s/\..*$//;
  `mkdir convert/$basename`;
  open(INPUT, "<$file") or die "cannot open file $_\n";
  my @lines = <INPUT>;
  close INPUT;
  foreach my $ln(@lines)
  {
    chomp $ln; 
    next if($ln =~ /^$/);
    my @elm = split(/\s+/, $ln);
    if(@elm ==3)
    {
      $counter++;
      $dtaFileName=$file."_$counter.$counter.$elm[2].dta";
      open(OUT, ">convert/$basename/$dtaFileName") or die "cannot open $dtaFileName\n";
      $mz = $elm[0]*$elm[2];
      print OUT $mz." $elm[2]";
    }
    else
    {
      print OUT "\n";
      print OUT "$ln";   
    }
  }
  `dta2mzxml -byname -charge 'convert/$basename/*.dta'`;
}

close OUT;
`rm -rf convert`;
exit;
