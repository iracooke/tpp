#!/usr/bin/perl -w

#############################################################################
# Program       : run_marimba.pl                                            #
# Author        : Luis Mendoza                                              #
# Date          : 8.08.08                                                   #
# Lase Updated  : 3.03.09                                                   #
# SVN Info      : $Id$                                                      #
#                                                                           #
# MaRiMba utility script                                                    #
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
use Getopt::Std;

my %options;
my $filter_opts;
my $sort_opts;

# Where things are
my $cmd_spectrast = 'spectrast';
my $cmd_filterMRM = 'filterMRM.pl';
my $cmd_sort = 'sort';
my $cmd_minfilter = 'min_sort.pl';

# Process inputs and options
getopts('m:M:t:s:X:L:DxukSNIQi:p:P:z:Z:T:l:r:R:', \%options);

my $speclib = shift || &usage("Spectral Library File");
my $protdb  = shift || &usage("Protein Database");
my $outfile = shift || &usage("Output File");

&usage("Maximum number of transitions per peptide") unless($options{t});

my @exclude = split(/\,/, $options{X}) if ($options{X});


# Starting up...
print "Run MaRiMba started at: ".scalar(localtime)."\n";


# Refresh library entries (tryptic peptides only)
print "...Refreshing library against database\n";
if ($options{u}){
    if ($options{k}){
      print "...Filtering out unmapped peptides in a tryptic context only\n";
      &run_command("$cmd_spectrast -cNtmp_refreshed -c_RTO -cD$protdb -cu $speclib","spectrast", 10);
    }
    else {
      print "...Filtering out unmapped peptides\n";
      &run_command("$cmd_spectrast -cNtmp_refreshed -cD$protdb -cu $speclib","spectrast", 10);
    }
} else {
    if ($options{k}){
      print "...Filtering out non-proteotypic and unmapped peptides in a tryptic context only\n";
      &run_command("$cmd_spectrast -cNtmp_refreshed -c_RTO -cD$protdb -cd -cu $speclib","spectrast", 10);
    }
    else {
      print "...Filtering out non-proteotypic and unmapped peptides\n";
      &run_command("$cmd_spectrast -cNtmp_refreshed -cD$protdb -cd -cu $speclib","spectrast", 10);
    }
}

my $lib_to_use = 'tmp_refreshed.splib';

# Restrict to list of peptides/proteins?
if ($options{r}) {
    print "[WARN] Options -r and -R are mutually exclusive.  Ignoring -R...\n" if ($options{R});
    print "...Restricting input library to peptides on list\n";
    &run_command("$cmd_spectrast -cNtmp_restricted -cT$options{r} tmp_refreshed.splib","spectrast", 15);
    $lib_to_use = 'tmp_restricted.splib';

} elsif ($options{R}) {
    print "...Restricting input library to proteins on list\n";
    &run_command("$cmd_spectrast -cNtmp_restricted -cO$options{R} tmp_refreshed.splib","spectrast", 16);
    $lib_to_use = 'tmp_restricted.splib';
}


# Create consensus library
print "...Creating consensus library\n";
&run_command("$cmd_spectrast -cNtmp_consensus -cJU -cAC $lib_to_use","spectrast", 20);


# Apply quality filters
print "...Applying quality filters\n";
&run_command("$cmd_spectrast -cNtmp_consensus_Q2 -cAQ -cL2 -cl5 tmp_consensus.splib","spectrast", 30);


# Apply user-defined filters in SpectraST
$filter_opts  = 'NTT==2 & NMC==0';
$filter_opts .= " & PrecursorMZ>=$options{m}" if ($options{m});
$filter_opts .= " & PrecursorMZ<=$options{M}" if ($options{M});
foreach my $res (@exclude) {
    $filter_opts .= " & Name!~$res";
}
print "...Performing initial peptide filtering\n";
&run_command("$cmd_spectrast -cNtmp_filtered  -cf\"$filter_opts\" tmp_consensus_Q2.splib","spectrast", 40);


# Create preliminary MRM list
print "...Creating preliminary MRM list\n";
&run_command("$cmd_spectrast -cNtmp_MRM -cM -cQ$options{t} tmp_filtered.splib","spectrast",50);


# Filter out non-unique peptides, as well as user-specified transitions
print "...Applying user-defined fiters\n";
$filter_opts  = " -O $outfile.unsorted";
$filter_opts .= " -i $options{i}" if ($options{i});
$filter_opts .= " -p $options{p}" if ($options{p});
$filter_opts .= " -P $options{P}" if ($options{P});
$filter_opts .= " -z $options{z}" if ($options{z});
$filter_opts .= " -Z $options{Z}" if ($options{Z});
$filter_opts .= " -L $options{L}" if ($options{L});
$filter_opts .= " -T $options{T}" if ($options{T});
$filter_opts .= " -m $options{l}" if ($options{l}); # mind the mapping!
$filter_opts .= " -I" if ($options{I});
$filter_opts .= " -N" if ($options{N});
$filter_opts .= " -S" if ($options{S});
$filter_opts .= " -Q" if ($options{Q});
$filter_opts .= " -M" if ($options{x}); # mind the mapping!
&run_command("$cmd_filterMRM $filter_opts tmp_MRM.mrm $protdb","filterMRM",80);

print "...Sorting results\n";
$sort_opts = "-k1,1 -k3,3 -k12,12 -k5,5rn";
&run_command("$cmd_sort $sort_opts $outfile.unsorted > $outfile.sorted","sort", 100);
$filter_opts  = " -O $outfile";
$filter_opts .= " -s $options{s}" if ($options{s});
print "...Restricting to minimum number of transitions per peptide\n" if ($options{s});
&run_command("$cmd_minfilter $filter_opts $outfile.sorted","min", 120);

# Clean-up
unless ($options{D}) {
    print "...Deleting temporary files\n";
    unlink <tmp_refreshed.*>;
    unlink <tmp_restricted.*>;
    unlink <tmp_consensus*>;
    unlink <tmp_filtered.*>;
    unlink <tmp_MRM.*>;
    unlink "$outfile.unsorted";
    unlink "$outfile.sorted";
    unlink "spectrast.log";
}


print "Job finished at: ".scalar(localtime)."\n\n";
exit(0);

###################### subs ######################

sub run_command {
    my $command = shift;
    my $exename = shift || "(program unspecified)";
    my $errcode = shift || 99;

    my $command_stdout = `$command`;
    my $status = $?;
    print $command_stdout;
    &report_error("there was a problem with $exename: " . $status / 256, $errcode) if ($?);

    return 1;
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
    print << "EOU";
Usage: $0 [options] <splib file> <protein database> <output file>

Generate a peptide list for MRM analysis via SpectraST

Options:
  -s <num>		Minimum number of transitions per peptide
  -t <num>              Maximum number of transitions per peptide (required)
  -m <num>              Min m/z of peptide
  -M <num>              Max m/z of peptide
  -p <num>              Min pI of peptide
  -P <num>              Max pI of peptide
  -r <file>             Restrict library entries to peptides contained in <file>
  -R <file>             Restrict library entries to proteins contained in <file>
  -X <aa1>,<aa2>...     Comma-separated list of residues to exclude
  -L <num1>,<num2>..    Comma-separated list of fragment ion lengths to exclude
  -T <string>           Exclude any residues in the string on the N-Terminus
  -x                    Exclude all modifications except iodoacetamide Cys C[160]
  -u                    Allow non-proteotypic peptides
  -k			When mapping peptides to proteins, consider peptides in a tryptic context only
  -l <aa1><dmass1>,...  Comma-separated list of amino acids and delta masses to use 
                        for generating modified peptide MRM transitions
  -i <string>           Output only those Q3 ions matching ion types in <string>
  -z <num1>,<num2>...   Comma-separated list of valid Q1 charges
  -Z <num1>,<num2>...   Comma-separated list of valid Q3 charges
  -N                    Allow neutral loss
  -S                    Allow secondary small neutral losses (e.g. water or ammonia)
  -I                    Allow non monoisotopic peaks
  -Q                    Allow mass-shifted ions

  -D                    Debug mode (do not erase intermediate files)
EOU

    exit(1);
}
