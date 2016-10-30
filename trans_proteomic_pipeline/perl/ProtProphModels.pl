#!/usr/bin/perl -w

#############################################################################
# Program       : ProtProphModels.pl                                        #
# Author        : Henry Lam and David Shteynberg                            #
# Date          : 7.28.08                                                   #
#                                                                           #
#                                                                           #
#                                                                           #
# Copyright (C) 2008 Henry Lam , David Shteynberg                           # 
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
use Getopt::Long;

$, = ' ';
$\ = "\n";

# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath cwd);
use lib realpath(dirname($0));
use tpplib_perl; # exported TPP lib function points


my $pos_color = 2;
my $neg_color = 1;
my $gnuplot_cmd = tpplib_perl::getGnuplotBinary();

my %options;

if (scalar(@ARGV) <= 0) {
    print "ProtProphModels.pl by David Shteynberg (based on original ProphetModels.pl by Henry Lam)";
    print "  Utility to plot ProteinProphet and iProphet models using gnuplot.";
    print '';
    print "Usage: ProtProphModels.pl <options>";
    print "Options: -i <FILE> -- Specify protXML file to processed.";
    print "         -d <STR>  -- Specify protein prefix indicating decoy to be counted.";
    print "         -x <STR>  -- Specify protein prefix indicating decoy to be excluded.";
    print "         -r <NUM>  -- Specify decoy ratio. Will guess from P<0.002 hits if not specified.";
    print "         -k        -- Don't delete intermediate files.";
    print "         -c        -- Use confidence not the probability.";
    print "";

    exit;
}


GetOptions( \%options, 'decoy_string|d=s', 'exclude_string|x=s', 'input_file|i=s', 'decoy_ratio|r=s', 'keep_intermediate_files|k', 'use_confidence|c');


my $pwd = cwd();
chomp $pwd;

my $infile = '';
if ($options{'input_file'}) {
    $infile = $options{'input_file'};
} elsif ($ARGV[-1] =~ /\.xml(\.gz)?/) {
    $infile = $ARGV[-1];
} else {
    die "No input xml file specified.";
}


my $tmpinfile = tpplib_perl::uncompress_to_tmpfile($infile); # decompress .gz if needed


print "Analyzing $infile ...";
open(INFILE, $tmpinfile) or die "Cannot open file $tmpinfile";


(my $infile_pfx = $infile) =~ s/(.*)\.xml(\.gz)?/$1/g;

my %roc;
my %model;
my $decoy_string = $options{decoy_string} || 'DECOY';
my $decoy_ratio = $options{decoy_ratio} || -1;
my $exclude_string = $options{exclude_string} || 'nEbRaSkA';

if ($decoy_ratio >= 0) {
    print "Assuming decoy ratio of $decoy_ratio";
}

# initializing arrays
my @pp_prob_array;
my @pp_prob_array_decoy;
my @pp_cum_neg_array;
my @ip_prob_array;
my @ip_prob_array_decoy;
my @ip_cum_neg_array;

for (my $ppp = 0; $ppp <= 500; $ppp++) {
    $pp_prob_array[$ppp] = 0;
    $pp_prob_array_decoy[$ppp] = 0;
    $pp_cum_neg_array[$ppp] = 0.0;
    $ip_prob_array[$ppp] = 0;
    $ip_prob_array_decoy[$ppp] = 0;
    $ip_cum_neg_array[$ppp] = 0.0;
}

my $line = '';
my $cur_model = '';
my $is_decoy = 0;
my $has_points = 0;
my $num_decoy = 0;
my $num_hits = 0;
my $pp_total_neg = 0.0;
my $ip_total_neg = 0.0;
my $num_hits_set = -1;
my $exclude = 0;
my $charge = 0;
my $has_fval = 0;
my $in_prot_group = 0;
my $pp_prob = 0;
my $pp_conf = 0;

while ($line = <INFILE>) {


    if ($line =~ /^\s*<protein_summary_data_filter min_probability=\"(.*?)\" sensitivity=\"(.*)\" false_positive_error_rate=\"(.*)\" predicted_num_correct=\"(.*)\" predicted_num_incorrect=\"(.*)\"/) {
	$roc{$1}->{"error"} = $3;
	$roc{$1}->{"ncorr"} = $4;
	$roc{$1}->{"nincorr"} = $5;

    }
    
    if ($line =~ /^\s*<protein_group.*probability=\"(.*?)\"/) {
	if ($options{"use_confidence"}) {
	    $pp_prob = 0;
	}
	else {
	    $pp_prob = $1;
	}
    }

    if ($line =~ /^\s*<protein protein_name=\"(.*?)\".*probability=\"(.*?)\".*confidence=\"(.*?)\"/) {

	if ($exclude || $is_decoy || !$in_prot_group) {
	    if ($1 !~ /^$exclude_string/) {
		$exclude = 0;
		if ($1 =~ /^$decoy_string/) {
		    $is_decoy = 1;
		} else {
		    $is_decoy = 0;
		}
		
	    } 
	    else {
		$exclude = 1;
	    }
	}

	if ($exclude == 0) {
	    if ($options{"use_confidence"} && $3 > $pp_prob) {
		$pp_prob = $3;
	    }
	}


	$in_prot_group = 1;

    }

    if ($line =~ /^\s*<\/protein_group>/) {


	if ($exclude == 0) {
	    $pp_prob = 1.0 if ($pp_prob < 0.0);
	    $pp_prob_array[int($pp_prob * 500)]++;
	    $pp_total_neg += (1 - $pp_prob);
	    $pp_cum_neg_array[int($pp_prob * 500)] += (1 - $pp_prob);
	    if ($is_decoy == 1) {
		$pp_prob_array_decoy[int($pp_prob * 500)]++;
	    }

	    $num_hits++;
	    $num_hits_set++;
	    if ($is_decoy == 1) {
		$num_decoy++;
	    }
	}
	$exclude = 0;
	$is_decoy = 0;
	$in_prot_group = 0;	

    }

    if ($line =~ /^<\/msms_run_summary>/) {
	if ($num_hits_set != -1) {
	    print "  => Total of $num_hits_set hits.";
	}
    }

}
     
close(INFILE);
unlink($tmpinfile) if ($tmpinfile ne $infile); # did we decompress xml.gz?


if ($num_decoy > 0) {

    open(PLOTOUT, ">$infile_pfx" . "_FDR.tsv") or die "Cannot open $infile_pfx" . "_FDR.tsv";
    
    # foreach my $prob_cutoff (sort(keys(%roc))) {

    my $pp_decoy_ratio = $decoy_ratio;
    my $ip_decoy_ratio = $decoy_ratio;

    if ($decoy_ratio < 0) {
	if ($pp_prob_array[0] > 0) {
	    $pp_decoy_ratio = ($pp_prob_array_decoy[0] / $pp_prob_array[0]);
	    print "Using decoy ratio of $pp_decoy_ratio for ProteinProphet.";
	}
	else {
	    $pp_decoy_ratio = -1;
	    print "Cannot determine decoy ratio from ProteinProphet. ";
	}
    }
    
    for (my $prob_cutoff = 0.0; $prob_cutoff <= 1.0; $prob_cutoff += 0.002) {
	my $pp_num_decoy_below_prob = 0;
	my $pp_num_below_prob = 0;
	my $pp_cum_neg_below_prob = 0;
	my $ip_num_decoy_below_prob = 0;
	my $ip_num_below_prob = 0;
	my $ip_cum_neg_below_prob = 0;
	
	for (my $pp = 0; $pp <= int($prob_cutoff * 500); $pp++) {
	    $pp_num_decoy_below_prob += $pp_prob_array_decoy[$pp];
	    $pp_num_below_prob += $pp_prob_array[$pp];
	    $pp_cum_neg_below_prob += $pp_cum_neg_array[$pp];
	    
	    $ip_num_decoy_below_prob += $ip_prob_array_decoy[$pp];
	    $ip_num_below_prob += $ip_prob_array[$pp];
	    $ip_cum_neg_below_prob += $ip_cum_neg_array[$pp];
	}
	
	my $fdr_ip_roc = $roc{$prob_cutoff}->{"error"} || -1.0; 
	
	my $fdr_ip = 0.0;
	my $num_pos_ip = $num_hits - $ip_num_below_prob;
	my $num_corr_ip = $num_pos_ip;

	
	my $fdr_pp = 0.0;
	my $num_pos_pp = $num_hits - $pp_num_below_prob;
	my $num_corr_pp = $num_pos_pp;

	if ($num_pos_pp > 0) {
	    $fdr_pp = ($pp_total_neg - $pp_cum_neg_below_prob) / $num_pos_pp;
	    $num_corr_pp = $num_pos_pp * (1 - $fdr_pp);
	}
	
	my $fdr_ip_decoy = 0.0;
	my $num_corr_ip_decoy = $num_pos_ip;

	my $fdr_pp_decoy = 0.0;
	my $num_corr_pp_decoy = $num_pos_pp;
	if ($num_hits - $pp_num_below_prob > 0 and $pp_decoy_ratio > 0 and $num_pos_pp > 0) {
	    my $num_incorr_pp = ($num_decoy - $pp_num_decoy_below_prob) / $pp_decoy_ratio;
	    $fdr_pp_decoy = $num_incorr_pp / $num_pos_pp;
	    $num_corr_pp_decoy = $num_pos_pp - $num_incorr_pp;
	}
	
	my $PP_uncert = 0;
	if ($fdr_pp*(1-$fdr_pp)/$num_pos_pp > 0){
	    $PP_uncert = sqrt($fdr_pp*(1-$fdr_pp)/$num_pos_pp);
	}
	my $PP_decoy_uncert = 0;
	if ($fdr_pp_decoy*(1-$fdr_pp_decoy)/$num_pos_pp > 0) {
	    $PP_decoy_uncert = sqrt($fdr_pp_decoy*(1-$fdr_pp_decoy)/$num_pos_pp);

	}
	print PLOTOUT $prob_cutoff, $fdr_ip, $fdr_ip_decoy, $fdr_pp, $fdr_pp_decoy, $num_corr_ip, $num_corr_ip_decoy, $num_corr_pp, $num_corr_pp_decoy, $PP_uncert, $PP_decoy_uncert;
    }  
    
    close (PLOTOUT);
    
    open(GPOUT, ">$infile_pfx\_FDR.gp") or die "Cannot open $infile_pfx" . "_FDR.gp";
    
    print GPOUT "set terminal png";
    print GPOUT "set output \"$infile_pfx" . "_FDR.png\"";
    print GPOUT "set size 1.0, 1.0";
    print GPOUT "set border 1";
    print GPOUT "set xtics border nomirror out";
    print GPOUT "set ytics border nomirror out";
    print GPOUT "set xzeroaxis linetype -1 linewidth 1.0";
    print GPOUT "set yzeroaxis linetype -1 linewidth 1.0";
    print GPOUT "set xlabel \"Decoy-estimated FDR\"";
    print GPOUT "set ylabel \"Prophet-predicted FDR\"";
#print GPOUT "set xlabel \"Probability Cutoff\"";
#print GPOUT "set ylabel \"FDR\"";
    print GPOUT "set mxtics";
    print GPOUT "set mytics";
    print GPOUT "set origin 0.0,0.0";
    
    print GPOUT "set yrange [0.0:]";
# print GPOUT, "set label \"$label\" at $x,$y front";
    
# print GPOUT "plot [0.0 to 1.0] \"$infile_pfx" . "_FDR.tsv\" using 1:2 title \"iProphet_PRED\" with line lc $pos_color , \"$infile_pfx" . "_FDR.tsv\" using 1:3 title \"iProphet_DECOY\" with line lc $neg_color , \"$infile_pfx" . "_FDR.tsv\" using 1:4 title \"ProteinProphet_PRED\" with line lc 4 , \"$infile_pfx" . "_FDR.tsv\" using 1:5 title \"ProteinProphet_DECOY\" with line lc 5";
    

    print GPOUT "plot \"$infile_pfx" . "_FDR.tsv\" using 5:4:11:10 title \"ProteinProphet\" with xyerrorbars lc $neg_color , x notitle with line lt 0 lc -1";
    print GPOUT "set output \"$infile_pfx" . "_FDR_5pc.png\"";
    print GPOUT "plot [0.0 to 0.05] \"$infile_pfx" . "_FDR.tsv\" using 5:4:11:10 title \"ProteinProphet\" with xyerrorbars lc $neg_color , x notitle with line lt 0 lc -1";
    print GPOUT "set output \"$infile_pfx" . "_ROC.png\"";
    print GPOUT "set xlabel \"FDR\"";
    print GPOUT "set ylabel \"Number of Correct Hits\"";
    print GPOUT "plot [0.0 to 0.05] \"$infile_pfx" . "_FDR.tsv\" using (\$5-\$11):9 title \"ProteinProphet(DECOY) Low\" with lines lc 5, \"$infile_pfx" . "_FDR.tsv\" using (\$5+\$11):9 title \"ProteinProphet(DECOY) High\" with lines lc 5,  \"$infile_pfx" . "_FDR.tsv\" using 4:8 title \"ProteinProphet(PREDICTED)\" with lines lc $neg_color ";	
    

    
    close(GPOUT);
    
    `$gnuplot_cmd $infile_pfx\_FDR\.gp`;
    
    unless ($options{"keep_intermediate_files"}) {
	`rm -f $infile_pfx\_FDR\.gp $infile_pfx\_FDR\.tsv`;
    }
}
