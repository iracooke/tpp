#!/usr/bin/perl

#############################################################################
# Program       : filterMRM.pl                                              #
# Author        : David Shteynberg                                          #
# Date          : 7.28.08                                                   #
# Last Updated  : 3.17.09                                                   #
# SVN Info      : $Id: filterMRM.pl 4816 2010-01-12 18:52:52Z csherwood $   #
#                                                                           #
#                                                                           #
# Copyright (C) 2008 David Shteynberg                                       # 
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
# Insitute for Systems Biology                                              #
# 1441 North 34th St.                                                       #
# Seattle, WA  98103  USA                                                   #
# akeller@systemsbiology.org                                                #
#                                                                           #
#############################################################################


use strict;
use Getopt::Std;
use Cwd 'abs_path';

use vars qw($opt_i $opt_L $opt_p $opt_P $opt_z $opt_Z $opt_S $opt_N $opt_M $opt_T $opt_O $opt_m $opt_I $opt_Q);

my $usage = "
Usage: $0 [options] <MRM file> <fasta file> 

Filter MRM file based on the following options:

  -O <file_name>                                  Write output to file <file_name> instead of STDOUT

  -i <string>                                     Output only those Q3 ions matching ion types in <string>

  -L <num>,<num>,...                              Comma-separated list of fragment ion lengths to be excluded (e.g. 1,2,3)

  -p <num>                                        Min pI of peptide

  -P <num>                                        Max pI of peptide

  -m <aa1><delta mass>,<aa2><delta mass>, ...     Comma-separated list of amino acids and delta masses to use 
                                                  for generating modified peptide MRM transitions
                
  -z <num1>,<num2>...                             Comma-separated list of valid Q1 charges

  -Z <num1>,<num2>...                             Comma-separated list of valid Q3 charges

  -N                                              Allow neutral loss

  -S                                              Allow secondary small neutral losses (e.g. water or ammonia)

  -I                                              Allow non-monoisotopic peaks

  -Q                                              Allow mass-shifted ions

  -M                                              Exclude all modifications except iodoacetamide Cys C[160]

  -T <string>                                     Exclude any residues in the string on the N-Terminus  

\n";


die $usage unless (@ARGV >= 2);

getopts('MSNIQi:L:p:P:z:Z:T:O:m:');

my %aa_masses = 
    (
     'n' => 1.007825,
     'c' => 17.002740,
     'G' => 57.021464,
     'D' => 115.02694,
     'A' => 71.037114,
     'Q' => 128.05858,
     'S' => 87.032029,
     'K' => 128.09496,
     'P' => 97.052764,
     'E' => 129.04259,
     'V' => 99.068414,
     'M' => 131.04048,
     'T' => 101.04768,
     'H' => 137.05891,
     'C' => 103.00919,
     'F' => 147.06841,
     'L' => 113.08406,
     'R' => 156.10111,
     'I' => 113.08406,
     'N' => 114.04293,
     'Y' => 163.06333,
     'W' => 186.07931
   );


my %Q1_charges;
my %Q3_charges;
my %excl_lengths;

if ($opt_z) {
    my @chgs = split(/\,/, $opt_z);
    for (my $i=0; $i<=$#chgs; $i++) {
	$Q1_charges{$chgs[$i]} = 1;
    }
}

if ($opt_Z) {
    my @chgs = split(/\,/, $opt_Z);
    for (my $i=0; $i<=$#chgs; $i++) {
	$Q3_charges{$chgs[$i]} = 1;
    }
}

if ($opt_L) {
    my @lens = split(/\,/,$opt_L);
    for (my $i=0; $i<=$#lens; $i++) {
	$excl_lengths{$lens[$i]} = 1;
    }
}

my $mrmFile = shift;
my $protdb = abs_path(shift);
my $comet_link = "http://localhost/tpp-bin/comet-fastadb.cgi?Db=" . $protdb . "&Pep=";

my %ssrcalc;

CalcSSR();

my %mods;
my $mod_idx=1;

if ($opt_m) {
    my @modarr = split(/\,/, $opt_m);
    for (my $i=0; $i<=$#modarr; $i++) {
	my $aa = substr($modarr[$i],0,1);
	my $delta = substr($modarr[$i],1);
	die "Modification found on $aa is not a number: $delta" unless $delta =~/^-?\d+\.?\d*/;
	$mods{$aa} = $delta;
	
    }
}


open(OUTFILE, ">$opt_O") ||   die "Could not open file: $opt_O.\n" unless (!$opt_O);
open(HEADER, ">header.txt") || die "Could not open file: header.txt";
my $header; 


if ($opt_m) {
    $header = "PROTEIN\tNUM_PROTEINS\tMOD_PEPTIDE\tPEPTIDE_SEQ\tINTENSITY\tQ1_MZ\tQ3_MZ\tSSRCALC_RT\tFRAGMENT_TYPE\tNTERM_RESIDUE\tCTERM_RESIDUE\tQ1_Z\tQ3_Z\tPI\tSINGLE_SAMPLE_MAX_REPS\tTOTAL_NUM_REPS\tALL_FRAGMENTS\tMODIFIED\tMODIFICATION_INDEX\tPEPTIDE_URL\n";
}
else {
    $header = "PROTEIN\tNUM_PROTEINS\tMOD_PEPTIDE\tPEPTIDE_SEQ\tINTENSITY\tQ1_MZ\tQ3_MZ\tSSRCALC_RT\tFRAGMENT_TYPE\tNTERM_RESIDUE\tCTERM_RESIDUE\tQ1_Z\tQ3_Z\tPI\tSINGLE_SAMPLE_MAX_REPS\tTOTAL_NUM_REPS\tALL_FRAGMENTS\tPEPTIDE_URL\n";
}

if (!$opt_O) {
    print $header;
}
else {
    print HEADER $header;
}

open(MRM, "<$mrmFile") ||   die "Could not open file: $mrmFile.\n";
while(my $line = <MRM>) {
    chomp($line);
    my @linearr = split(/\s+/, $line);

    my @numdenom_arr = split(/\//, $linearr[1]);

    my $pep_pI = $linearr[2];

    my $Q1_mz = $linearr[3];

    my $Q3_mz = $linearr[5];

    my $intens = $linearr[6];

    my $all_frags = $linearr[7];

    my $Q1_z = $linearr[8];

    my $pep = $linearr[9];

    my $protnum = $linearr[11];

    my $prot = $linearr[12];

    my @Q3 = split(/\,/,$all_frags);

    my @Q3_t_arr = split(/\//,$Q3[0]);

    my $Q3_type_z = $Q3_t_arr[0];

    my @Q3_z_arr = split(/\^/,$Q3[0]);

    my $Q3_type = $Q3_z_arr[0];

    my $Q3_z;

    if ($#Q3_z_arr > 0) {
	@Q3_z_arr = split(/\//, $Q3_z_arr[1]);
	$Q3_z = $Q3_z_arr[0];
	@Q3_z_arr = split(/i/,$Q3_z);
	$Q3_z = $Q3_z_arr[0];
    }
    else {
	@Q3_z_arr = @Q3_t_arr;
	$Q3_type = $Q3_t_arr[0];
	@Q3_t_arr = split(/i/,$Q3_type);
	$Q3_type = $Q3_t_arr[0];
	$Q3_z = 1;
    }

    my @Q3_length_arr = split(/\D/,$Q3_type);
    my $Q3_length = $Q3_length_arr[1];
    
    if ($Q3[0] =~ m/i/) {
	$Q3_type = $Q3_type."i"; #add 'i' to frag type so that it appears in MRM list
    }


    my @peparr = split(/\//, $pep);

    $pep = $peparr[0];

    my $modpep =  $pep;

    my $has_bad_mod = GetBadMod($modpep);

    $pep =~ s/[nc]?\[\d+\.?\d*\]//g;

    my $bond = GetBond($pep, $Q3_type_z);

    my $nterm_aa = substr($bond, 0, 1);

    my $cterm_aa = substr($bond, 1, 1);

    my $numer = $numdenom_arr[0];
    my $denom = $numdenom_arr[1];

	if (!$opt_M || !$has_bad_mod) {

	    if (!$opt_z ||  exists( $Q1_charges{$Q1_z} ) ) {

		if (!$opt_Z ||  exists( $Q3_charges{$Q3_z} ) ) {

		    if (!$opt_L || ! (exists( $excl_lengths{$Q3_length} ) ) ) {
			
			if ($opt_N || $Q3_z < $Q1_z ) {
			    
			    if (!$opt_i ||  ($Q3_type =~ m/[$opt_i]/) ) {
				
				if ($opt_S || ! ($Q3_type =~ m/\-/) ) {
				    
				    if ($opt_Q || ! ($Q3_type =~ m/\[/) ) {
					
					if (!$opt_p || $pep_pI >= $opt_p) {
					    
					    if (!$opt_P || $pep_pI <= $opt_P) {
						
						if (!$opt_T || ! ($pep =~ m/^[$opt_T]/)) {
						    
						    if($opt_I || ! ($Q3[0] =~ m/i/)) {
							
							my $RT = $ssrcalc{$pep};
							
							my $line = "";
							my $pepurl = $comet_link . $pep;
							
							if ($opt_m) {
							    my $MOD_pep = GetModPep($modpep);
							    my $MOD_Q1_mz = GetModQ1MZ($pep, $Q1_z, $Q1_mz, 'f');
							    my $MOD_Q3_mz = GetModQ3MZ($pep, $Q3_z, $Q3_mz, $Q3_type_z);
							    my $hl;
							    my $modhl;
							    
							    if ($MOD_Q1_mz >= $Q1_mz) {
								$modhl = "H";
								$hl = "L";
							    }
							    else {
								$modhl = "L";
								$hl = "H";
							    }
							    
							    $line = "$prot\t$protnum\t$modpep\t$pep\t$intens\t$Q1_mz\t$Q3_mz\t$RT\t$Q3_type\t$nterm_aa\t$cterm_aa\t$Q1_z\t$Q3_z\t$pep_pI\t$numer\t$denom\t$all_frags\t$hl\t$mod_idx\t$pepurl\n";
							    
							    if ($MOD_Q1_mz != $Q1_mz) {
								$line .= "$prot\t$protnum\t$MOD_pep\t$pep\t$intens\t$MOD_Q1_mz\t$MOD_Q3_mz\t$RT\t$Q3_type\t$nterm_aa\t$cterm_aa\t$Q1_z\t$Q3_z\t$pep_pI\t$numer\t$denom\t$all_frags\t$modhl\t$mod_idx\t$pepurl\n";
							    }
							    $mod_idx++;
							}
							else {
							    $line = "$prot\t$protnum\t$modpep\t$pep\t$intens\t$Q1_mz\t$Q3_mz\t$RT\t$Q3_type\t$nterm_aa\t$cterm_aa\t$Q1_z\t$Q3_z\t$pep_pI\t$numer\t$denom\t$all_frags\t$pepurl\n";
							    
							}
							
							if (!$opt_O) {
							    print $line;
							}
							else {
							    print OUTFILE $line;
							}
							
						    }
						}	
					    }
					}
				    }
				    
				}
			    }
			}
		    }
		}
	    }
	}
}

close(MRM);
close(OUTFILE) unless (!$opt_O);
close(HEADER);

exit(0);


sub GetBadMod {
    my ($pep) = @_;

    if ($pep =~ m/[^C]\[\d+\.?\d*\]/) {
	return 1;
    }

    while ($pep =~ s/C\[(\d+\.?\d*)\]/C/) {
	if ($1 < 158 || $1 > 162)  {
	    return 1;
	}
    }

    return 0;
}


sub GetBond {
    my ($pep, $frag) = @_;

    if ($frag =~ m/\-/) { #internal
	return "--";
    }

    if ($frag =~ m/^[xyz]([1-9][0-9]*)/) {
	if (length($pep) - $1 - 1 < 0 || length($pep) - $1 - 1 > length($pep) - 2) {
	    return "--";
	}
	return substr($pep, length($pep)-$1-1, 2);
    }

    if ($frag =~ m/^[abc]([1-9][0-9]*)/) {
	if ($1 - 1 < 0 || $1 - 1 > length($pep) - 2) {
	    return "--";
	}
	return substr($pep, $1-1, 2);
    }

    return "--";
}

sub GetModPep {
    my ($modpep) = @_;

    my @modpeparr = split(//, $modpep);

    my $outpep = "";
    my $aa = $modpeparr[0];
    if (exists($mods{'n'}) && $aa ne 'n') {
	 $outpep .= "n[" . ($aa_masses{'n'}+$mods{'n'}) ."]";
    }

    for (my $i=0; $i<=$#modpeparr; $i++) {
	$aa = $modpeparr[$i];
	if (exists($mods{$modpeparr[$i]}) && ($i == $#modpeparr || $modpeparr[$i+1] ne "[")) {
	    $outpep .= $modpeparr[$i] . "[" . ($aa_masses{$modpeparr[$i]}+$mods{$modpeparr[$i]}) ."]";
	}
	elsif (exists($mods{$modpeparr[$i]}) && $i != $#modpeparr && $modpeparr[$i+1] eq "[") {
	    my $j = $i+2;
	    my $mass = "";
	    while ($j <= $#modpeparr &&  $modpeparr[$j] ne "]") {
		$mass .=  $modpeparr[$j];
		$j++;
	    }
	    $outpep .= $modpeparr[$i] . "[" . ($mass+$mods{$modpeparr[$i]}) ."]";
	    $i = $j;
	}
	else {
	    $outpep .= $modpeparr[$i];
	}
    }
    if (exists($mods{'c'}) && $aa ne 'c') {
	 $outpep .= "c[" . ($aa_masses{'c'}+$mods{'c'}) ."]";
    }
    return $outpep;
}

sub GetModQ1MZ {
    my ($pep, $Q1_z, $Q1_mz, $pepFrag) = @_;

    my @peparr = split(//, $pep);

    my $delta_mass = 0;
    my $mod_Q1_mz = $Q1_mz * $Q1_z;

    if ($pepFrag eq "f" || $pepFrag eq "n") {
	if (exists($mods{'n'})) {
	    $delta_mass += $mods{'n'};
	}
    }

    if ($pepFrag eq "f" || $pepFrag eq "c") {
	if (exists($mods{'c'})) {
	    $delta_mass += $mods{'c'};
	}
    }

    for (my $i=0; $i<=$#peparr; $i++) {
	if (exists($mods{$peparr[$i]})) {
	    $delta_mass += $mods{$peparr[$i]};
	}
    }

    $mod_Q1_mz += $delta_mass;
    $mod_Q1_mz /= $Q1_z;

    return $mod_Q1_mz;
}

sub GetModQ3MZ {
    my ($pep, $Q3_z, $Q3_mz, $frag) = @_;

    my $delta_mass = 0;
    my $mod_Q3_mz = $Q3_mz * $Q3_z;
    my $subpep="";
    my $type = "";

    if ($frag =~ m/^[xyz]([1-9][0-9]*)/) {
	$subpep = substr($pep, length($pep)-$1);
	$type = 'c';
    }
    elsif ($frag =~ m/^[abc]([1-9][0-9]*)/) {
	$subpep =  substr($pep, 0, $1);
	$type = 'n';
    }

    return GetModQ1MZ($pep, $Q3_z, $Q3_mz, $type);
}


sub CalcSSR {
    unlink("pepSSR.tmp");
    my $tmp_mrmFile_1 = $mrmFile . ".tmp_1";
    my $tmp_mrmFile_2 = $mrmFile . ".tmp_2";
    my $tmp_SSRCalc_file = "tmp_ssrcalc.tsv";

    (system("cut -f10 $mrmFile > $tmp_mrmFile_1") == 0) or die "exiting: error with command \"cut -f10 $mrmFile > $tmp_mrmFile_1\"\n";

    (system("cut -f1 -d/ $tmp_mrmFile_1 > $tmp_mrmFile_2")  == 0) or die "exiting: error with command \"cut -f1 -d/ $tmp_mrmFile_1 > $tmp_mrmFile_2\"\n";

    open(PEPSIN, "<$tmp_mrmFile_2");
    open(PEPSOUT, ">pepSSR.tmp") ;

    while (<PEPSIN>) {
	chomp;

	my $pep = $_;

	$pep =~ s/[nc]?\[\d+\.?\d*\]//g;

	if (!exists($ssrcalc{$pep})) {
	    print PEPSOUT "$pep\n"; 
	    $ssrcalc{$pep} = -1;
	}
    }

    close(PEPSIN);
    close(PEPSOUT);

    my $cmd_stdout = `"SSRCalc3.pl --alg 3.0 --source pepSSR.tmp --output tsv > $tmp_SSRCalc_file"`;
    my $exitcode = $?;
    print $cmd_stdout;
    ($? == 0) or die "exiting: error running \"SSRCalc3.pl --alg 3.0 --source pepSSR.tmp --output tsv > $tmp_SSRCalc_file\"\n";

    open(PEPS, "$tmp_SSRCalc_file");

    while(<PEPS>) {
	chomp;
	my @arr = split /\s+/, $_; 
	$ssrcalc{$arr[0]} = $arr[2];
    }

    unlink("pepSSR.tmp");
    unlink($tmp_mrmFile_1);
    unlink($tmp_mrmFile_2);
    unlink($tmp_SSRCalc_file);
}
