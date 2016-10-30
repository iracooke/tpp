#!/usr/bin/perl


# File: calctppstats.pl
# Description: pepXML statistics generation and reporting
# Date: February 28, 2008
#
# Copyright (C) 2008 Eric Deutsch, ISB Seattle
#
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA



use strict;
use warnings;

use Getopt::Long;
use FindBin;
use lib "$FindBin::Bin";
use StatUtilities;
use CGI;

my %options;
GetOptions( \%options, 'input_file=s', 'charge', 'modifications', 'massdiff',
             'model|M', 'peptide_count', 'residues|r', 'glyco_motifs',
             'help|?','decoy_string|d:s',
             'tsv', 'num_missed_cleavages|n', 'net|e', 'model09',
	     'modification_combinations', 'FDR01', 'full', 'write_summary',
	     'require_charge:i','histogram_resolution:f',
	     'filterByMassDiff:s','FDRthresh|F:f', 'pthresh|p:f', 'protein_count',
          );

my $q;
if ($ENV{REQUEST_METHOD}) {
  print "Content-type: text/plain\n\n";
  $q = new CGI;
  if ($q->param('input_file')) {
    $options{input_file} = $q->param('input_file');
    $options{full} = 'yes';
  }
}

printUsage() if $options{help};

if ($options{full}) {
  $options{charge} = 'full';
  $options{modifications} = 'full';
  $options{massdiff} = 'full';
  $options{peptide_count} = 'full';
  $options{peps_per_run} = 'full';
  $options{num_missed_cleavages} = 'full';
  $options{net} = 'full';
  $options{FDR01} = 'full' if (!$options{FDRthresh} and !$options{pthresh});
  $options{decoy_string} = 'DECOY' unless ($options{decoy_string});
}

my $P_threshold = '0.9';
my $P_threshold_format = '0.90';
my $rounded_P_threshold_format = '0.90';
my $threshold_string = 'P>0.9';
my $threshold_string_long = 'P>0.90';

if ($options{pthresh} and $options{pthresh} >= 0.0 and $options{pthresh} <= 1.0) {
    $P_threshold = $options{pthresh};
    $P_threshold_format = $options{pthresh};
    $rounded_P_threshold_format = '' . $options{pthresh};
    $threshold_string = 'P>'. $options{pthresh};
    $threshold_string_long = 'P>' . $options{pthresh};
}

if ($options{FDR01}) {
    $options{FDRthresh} = 0.010;
}

my $tsv = '';

my $PepP = '';
my $ProP = '';
my $pwd = `pwd`;
chomp $pwd;
my $P9cnt = 0;
my $FDR1cnt = 0;
my $FDR1P;
my $actual_FDR_thresh;
my $Pcnt = 0;
my $Scnt = 0;
my $buffer = '';
my $assumed_charge;
my %P9cntbycharge;
my %Qcntbycharge;
my %Qcntbyrun;
my %ntt;
my $ntt;
my %models;
my $warnning_Elabel=0;
my $cnt_ratio=0;

my $decoy_string = $options{decoy_string} || 'DECOY';

my %filterBy;
if ($options{filterByMassDiff}) {
  if ($options{filterByMassDiff} =~ /([-\d\.]+):([-\d\.]+)/) {
    $filterBy{MassDiffFloor} = $1;
    $filterBy{MassDiffCeiling} = $2;
    print "INFO: Discarding all entries with massdiff outside $1 - $2\n";
  } else {
    die("ERROR: Unable to parse filterByMAssDiff parameter value '$options{filterByMassDiff}'");
  }
}

my %spectra;
my %query_spectra;
my $query_spectrum_root;
my $iProphet_mode = 0;

#### Use specified name or guess the filename
my $infile = $options{input_file} || 'interact-prob.xml';
$infile = 'interact-prob.xml' if (! -e $infile);
$infile = 'interact-spec.xml' if (! -e $infile);
$infile = 'interact-specall.xml' if (! -e $infile);
$infile = 'interact.xml' if (! -e $infile);
$infile = 'interact.pep.xml' if (! -e $infile);
$infile = 'interact-prob.pep.xml' if (! -e $infile);
$infile = $options{input_file} if (! -e $infile);
$infile = '' if (! $infile);


#### Determine the file root
my $rootname = $infile;
unless ($rootname =~ s/\.pep.\.xml$//) {
  $rootname =~ s/\.xml$//;
}


#### If the user opted to write out a summary, open the file
if ($options{write_summary}) {
  open(OUTFILE,">$rootname.summary.txt") ||
    die("ERROR: Unable to write output file $rootname.summary.txt");
}


#### Temporary file for testing
#open(TESTFILE,">$rootname.test.txt") ||
#    die("ERROR: Unable to write output file $rootname.test.txt");


my $massdiff = -1;
my @massdiffs;
my %modifications;
my $peptide;
my $modified_peptide;
my %modification_combinations;
my %glyco_motifs;
my %this_peptide_modifications;

my $peptide_sequence;
my %residues;
my $total_residues = 0;

my %static_mods;
my $static_mod_aa = '';
my $static_mod_token = '';
my $experiment_label='';

my $est_tot_num_corr = 0;
my %roc_sens;
my %roc_err;
my %roc_nCorr;
my %roc_nIncorr;
my %error_point;
my %all_ids;

my $num_missed_cleavages = 0;
my %num_missed_cleavages;

my $num_decoy_P9cnt=0;
my %num_decoy_P9cnt_bycharge;
my $num_decoy_belowP1cnt=0;
my $total_num_belowP1cnt=0;

my %all_proteins;
my %all_unambiguous_proteins;
my @proteins = ();

# Henry adds:
my @num_decoy_byprob;
for (my $ppp = 0; $ppp <= 100; $ppp++) {
    $num_decoy_byprob[$ppp] = 0;
}
my $num_decoy = 0;
# End Henry

my ($is_decoy,$protein_name,$probability);

my @decoy_probabilities;

my %peps_per_run=();

my $pp_incorr = 0.0;
my $pp_corr = 0.0;

if ($infile && open(INFILE,$infile)) {
    my $line;
    while ($line = <INFILE>) {

        if ($line =~ /peptideprophet_summary .+ est_tot_num_correct="([\d\.]+)"/) {
	    $est_tot_num_corr = $1;
	}


	if ($line =~ /roc_data_point min_prob="([\d\.]+)" sensitivity="([\d\.]+)" error="([\d\.]+)" num_corr="([\d\.]+)" num_incorr="([\d\.]+)"/) {
		$roc_sens{$1} = $2;
                $roc_err{$1} =  $3;
                $roc_nCorr{$1} = $4;
		$roc_nIncorr{$1} = $5;
	}

	if ($line =~ /error_point error="([\d\.]+)" min_prob="([\d\.]+)" num_corr="([\d\.]+)" num_incorr="([\d\.]+)"/) {
		$error_point{min_prob}->{$1} = $2;
                $error_point{num_corr}->{$1} =  $3;
                $error_point{num_incorr}->{$1} = $4;
		if ($options{FDRthresh} and $1 <= $options{FDRthresh}) {		
		    $FDR1P = $2;
                    $actual_FDR_thresh = $1;
                    $P_threshold = $FDR1P;
                    $P_threshold_format = $FDR1P;
                    $rounded_P_threshold_format = '' . $FDR1P;
                    $threshold_string = 'FDR ' . $1;
                    $threshold_string_long = 'FDR ' . $1;
		}
		
		      

	}

	if ($line =~ /aminoacid_modification aminoacid="(\w)" massdiff="([\d\.\-\+]+)" mass="([\d\.\-\+]+)" variable="([Y|N])"/) {
	    if ($4 eq 'N') {
		# static mod
                unless ($static_mods{$1}) {
		    $static_mods{$1} = $1 . sprintf("[%3i]", $3);
		    if ($static_mod_aa) { 
			$static_mod_aa .= '|';
		    }
		    $static_mod_aa .= $1;

		}
	    }

	}

     if ($line =~ /spectrum="(.+?)" start_scan.*index="\d+"\s*((retention_time_sec="\d+(\.\d+)?"\s*)?experiment_label="(.+?)")?/) {
       $query_spectrum_root = $1;
         $experiment_label = $5;
         if ($line =~ /spectrum="(.+)\.\d" start_scan.*index="\d+"\s*((retention_time_sec="\d+(\.\d+)?"\s*)?experiment_label="(.+?)")?/) {
               $query_spectrum_root = $1;
               $experiment_label = $5;
         }


	#print "query_spectrum_root=$query_spectrum_root==\n";
	$modified_peptide = '';
	%this_peptide_modifications = ();
        $peptide = '';
	#ning add
	$is_decoy=0;

	unless ($spectra{$query_spectrum_root}) {
	  $spectra{$query_spectrum_root} = 1;
	  $Scnt++;
	}
        if ($line =~ /assumed_charge="(\d+)"/) {
	  $assumed_charge = $1;
          $Qcntbycharge{$assumed_charge}++;
	} else {
	  die("ERROR: Unable to determine charge");
	}
      }


      if ($line =~ /mixture_model precursor_ion_charge=\"(\d).+est_tot_correct="([\d\.]+)/) {
	$models{$1} = $2;
      }

      if ($line =~ /massdiff=\"([\d\.\-\+]+)\"/) {
	$massdiff = $1;
	$massdiff =~ s/\+//;
	#print "massdiff = $massdiff\n";
      }

      if ($line =~ /search_hit hit_rank="[\d]+" peptide="(.+?)"/) {
	 $peptide = $1;
      }

      # This is likely redundant with the option above...
      if ($line =~ / peptide=\"(.+?)\"/) {
        $peptide_sequence = $1;
      }


      #### Get the current protein name
      if ($line =~ /search_hit.+protein=\"(.+?)\" num/) {
	$protein_name = $1;
        push @proteins, $protein_name;
	#### And set the decoy flag if it matches the deocy string
	if ($protein_name =~ /^${decoy_string}/) {
	  $is_decoy = 1;
	}
      }

      #### HENRY: also check alternative proteins to see if there is any non-decoy entries
      if ($line =~ /<alternative_protein protein=\"(.+?)\"/) {
        my $alt_protein = $1;
	push @proteins, $alt_protein;
        if (not ($alt_protein =~ /^${decoy_string}/)) {
          $is_decoy = 0;
        }
      }

      if ($line =~ / num_tol_term=\"(\d)\"/) {
	$ntt = $1;
      }

      if ($line =~ /modified_peptide=\"(.+?)\"/) {
	$modified_peptide = $1;
      }

      if ($line =~ /mod_aminoacid_mass position="(\d+)" mass="([\d\.]+)"/) {
	$this_peptide_modifications{$1} = int($2);
      }

      if ($line =~ /num_missed_cleavages=\"(\d)\"/) {
	$num_missed_cleavages = $1;
      }

      # Henry adds:
      if ($line =~ /lib_remark\" value=\"(.*)\"/) {
         # SpectraST's way of saying this is a decoy
         if ($1 =~ /^${decoy_string}/) {
           $is_decoy = 1;
         } else {
           $is_decoy = 0;
         }
      }
      # End Henry

      my $trigger_line = 0;
      #### If this line contains an iProphet probability
      if ($line =~ /interprophet_result probability="([\d\.e\-]+)"/) {
        $probability = $1;
        $iProphet_mode = 1;
        $trigger_line = 1;
        my $query_spectrum;
        if($experiment_label) {
          $query_spectrum = $query_spectrum_root."_$experiment_label.$assumed_charge";
        } else {
          $query_spectrum = $query_spectrum_root.".$assumed_charge";
        }
        if ($query_spectra{$query_spectrum}) {
          print "Duplicate spectrum_query $query_spectrum: Probs: $probability , $query_spectra{$query_spectrum}\n";
          next;
        } else {
            $query_spectra{$query_spectrum} = $probability;
        }
      }

      if($iProphet_mode && !$warnning_Elabel && !$experiment_label)
	  {
	    print "WARNNING: no experiment label. The statistic may not be correct\n";
		$warnning_Elabel=1;
	  }
      #### If we're not in iProphet mode, then just trigger on a plain probability
      #### Note that at present we don't detect this until after the first spectrum!! FIXME
      if (!$iProphet_mode && $line =~ /probability="([\d\.e\-]+)" al/) {
        # Henry adds:
        # Hits of charge state having invalid models should be excluded completely.
        # Currently, regardless of whether the prob is 0, -1, -2, or -3 (etc),
        # they are counted as negatives. This potentially skews the decoy ratio at
        # very low probabilities in case there are some potential good hits in charge
        # states with rejected models by PeptideProphet due to whatever fitting problems.
        # Note that the way to tell whether the probability is from an invalid model is
        # value="0" (as opposed to 0.0000) or value="-1" (some negative integers)
        if ($1 eq '0' or $1 < 0) {
          $trigger_line = 0;
        } else {
	  $probability = $1;
	  $trigger_line = 1;
        }
        # End Henry
      }


      #### If this line contains the primary probability value
      if ($trigger_line) {
	

        # Henry adds:
        #### Count all decoys,and by prob
        if ($is_decoy == 1) {
          $num_decoy++;
          $num_decoy_byprob[int($probability * 100)]++;
        }
        # End Henry

	#### Count up extremely low probabilities for decoy counting purposes
        if ($probability <= 0.001) {
	  $total_num_belowP1cnt++;
	  if ($is_decoy == 1) {
	    $num_decoy_belowP1cnt++;
	  }
	}

	#### Save an array of decoy probabilities
        if ($probability >= 0.3 && $is_decoy) {
	  #print "DECOY probability=$probability\n";
	  push(@decoy_probabilities,$probability);
	}

	#### Count up spectra that exceed FDR threshold
        if ($FDR1P && ($probability >= $FDR1P)) {
	  $FDR1cnt++;
	}


	#### If the probability exceeds the threshold
        if ($probability >= $P_threshold) {

	  #### One option is to only look at one charge state
	  if ($options{require_charge}) {
	    if ($assumed_charge != $options{require_charge}) {
	      next;
	    }
	  }

	  #### One option is to only include data within a given mass range
	  if (defined($filterBy{MassDiffFloor})) {
	    if ($massdiff <= $filterBy{MassDiffFloor} or $massdiff >= $filterBy{MassDiffCeiling}) {
	      next;
	    }
	  }

	  $pp_incorr += (1.0 - $probability);
	  $pp_corr += $probability;

	  #### Look at all proteins mapped to
	  if (scalar(@proteins) == 1) {
	      if (exists $all_unambiguous_proteins{$proteins[0]}) {
		  $all_unambiguous_proteins{$proteins[0]}++;
	      } else {
                  $all_unambiguous_proteins{$proteins[0]} = 1;
	      }
	  }

	  foreach my $pr (@proteins) {
	      if (exists $all_proteins{$pr}) {
		  $all_proteins{$pr}++;
	      } else {
		  $all_proteins{$pr} = 1;
	      }
	      
	  }

	  #### If this is a decoy hit, count it
	  if ($is_decoy == 1) {
	    $num_decoy_P9cnt++;
	    $num_decoy_P9cnt_bycharge{$assumed_charge}++;
	    #print TESTFILE "$protein_name\n";
	  }

	  $P9cnt++;
          $P9cntbycharge{$assumed_charge}++;
	  push(@massdiffs,$massdiff);

    	  #### Count the individual residues if requested
	  if ($options{residues}) {
	    $total_residues += length($peptide_sequence);
	    for (my $i=0; $i<length($peptide_sequence); $i++) {
	      $residues{substr($peptide_sequence,$i,1)}++;
	    }
	  }


	  $num_missed_cleavages{$num_missed_cleavages}++;
	  $ntt{$ntt}++;

	  #### If there is a modification to the peptide, handle it
	  if ($modified_peptide) {
	    $peptide = $modified_peptide;

	    #### fix string for static modifications
	    my $new_modified_peptide = '';
	    if ($modified_peptide =~ /^(n\[\d+\])/) {
	      $new_modified_peptide .= $1;
	    }
	    for (my $res=0; $res<length($peptide_sequence); $res++) {
	      $new_modified_peptide .= substr($peptide_sequence,$res,1);
	      if ($this_peptide_modifications{($res+1)}) {
		  $new_modified_peptide .= '['.$this_peptide_modifications{($res+1)}.']';
	      }
	    }

	    #### Properly handle C terminus modifications
	    if ($modified_peptide =~ /(c\[\d+\])$/) {
		$new_modified_peptide .= $1;
	    }

	    #print "$modified_peptide \t $new_modified_peptide\n";
	    $modified_peptide = $new_modified_peptide;

	    my @combination = ();
	    while ($modified_peptide =~ /(\w\[\d+\])/g) {
	      $modifications{$1}++;
	      push(@combination,substr($1,0,1));
	      my $tmp = $modified_peptide;
	      $modified_peptide = $';                         ### '
		  
	      if ($1 eq 'N[115]' && $tmp =~ /(N\[115\]\w\w)/) {
		  $glyco_motifs{$1}++;
	      }
	    }


	    if (scalar(@combination) > 0) {
	      $modification_combinations{join('',sort(@combination))}++;
	    }
	    #print "\n";
	    
	    #### Else if no modification specified, make a note
	  } else {
	    $modifications{'None'}++;
	  }

          $all_ids{$peptide}++;
          my $run_name = $query_spectrum_root;
          $run_name =~ s/(.*)(\.\d+\.\d+)/$1/;
          $peps_per_run{$run_name}++;

	} # if (prob > threshold)
	$Pcnt++;
	@proteins = ();

      } # if (trigger_line)

    } # while (line = <INFILE>)

    my $passCount = $P9cnt;
#    if ($options{FDRthresh}) {
#      $passCount = $FDR1cnt;
#    }

    $cnt_ratio = ( $Scnt ) ? $passCount/$Scnt : 0;
    $PepP = sprintf("PepP  %6i/%6i  %.3f  ",$passCount,$Scnt,$cnt_ratio);
    $tsv .= sprintf("%d\t%d\t%.3f\t",$passCount,$Scnt,$cnt_ratio);


  } else {
    $PepP = '                            ';
  }

if ($options{FDRthresh} != $actual_FDR_thresh) {
    print "WARNING: Specified FDR threshold of $options{FDRthresh} is not found in the <error_point> field of the pepXML file.\n";
    print "WARNING: A prob threshold of $P_threshold is used instead, which corresponds approximately to FDR=$actual_FDR_thresh.\n" ;
}

  close(INFILE);

  ### Process protXML file. In our protXML, protein groups are ordered by 
  ### decreasing probability, facilitating our analysis.

  my $protein_info = 0;
  my $PP9cnt = 0;
  my $aP9cnt = 0;
  my $Prot9cnt = 0;
  my $ProtGrp_cnt = 0;
  my $prob_sum = 0.0;
  my $prot_FDR;
  my $decoy_cnt = 0;
  my $Prot_cnt = 0;
  my $prot_FDR_bin = 0;
  my $nbins = 4;
  my @protFDR_decoy_bin_cnt;
  my $i;
  for ($i = 0; $i < $nbins; $i++) {
    $protFDR_decoy_bin_cnt[$i] = -1; #means this bin has not been visited
  }
  my $prot_decoy_FDR = 0;
  my $nprot_above_thresh = 0;
  my $protFDR01prob = 0;
  my $protFDR01grp_cnt = 0;
  my $protFDR01prot_cnt = 0;
  my @protFDR_prot_bin_cnt;

  $infile =~ s/\.pep\.xml$/.prot.xml/;
  $infile =~ s/\.prot\.xml$/-prot.xml/ if (! -e $infile);
  $infile = 'interact-prob-prot.xml' if (! -e $infile);
  $infile = 'interact-prob.prot.xml' if (! -e $infile);
  $infile = 'interact-spec-prot.xml' if (! -e $infile);
  $infile = 'interact-prot.xml' if (! -e $infile);
  $infile = 'interact-prob_all-prot.xml' if (! -e $infile);

  if ($infile && open(INFILE,$infile)) {
    print "INFO: Reading ProteinProphet file '$infile'\n";
    $protein_info = 1;
    my $line;
    while ($line = <INFILE>) {
      # Count *peptides* with initial/adjusted prob > threshold
      if ($line =~ /initial_probability="([\d\.]+)".+nsp_adjusted_probability="([\d\.]+)"/) {
      	if ($1 >= $P_threshold) {
	  $PP9cnt++;
	}
      	if ($2 >= $P_threshold) {
	  $aP9cnt++;
	}
      }

      # Count protein IDs with nonzero prob (zero prob in high-prob
      #  group usually means protein not needed to explain data)
      if ($line =~ /protein_name="(.+)" n_indistinguishable_proteins="\d+" probability="([\d\.]+)"/) {
      	if ($2 > 0.0) {
          $Prot_cnt++; # count all proteins
      	  if ($2 >= 0.9) {
	    $Prot9cnt++; # count proteins with prob > 0.9
	  }
	  if ($1 =~ /^${decoy_string}/) {
            $decoy_cnt++; # count decoys
          }
        }
      }

      # Calculate info related to protein group FDR.
      if ($line =~ /<protein_group.+ probability="([\d\.]+)"/) {
        my $prot_prob=$1;
        $ProtGrp_cnt++; # count number of protein groups
        $prob_sum += $prot_prob; # keep running sum of protein group probs
        $prot_FDR = 1.0 - ($prob_sum/$ProtGrp_cnt);
        $prot_FDR_bin = int($prot_FDR * 100) - 1;
        # if we have just entered a new FDR bin, gather and print stats
        if (($prot_FDR_bin >= 0) && ($prot_FDR_bin < $nbins) &&
              ($protFDR_decoy_bin_cnt[$prot_FDR_bin] == -1)) {
          $protFDR_decoy_bin_cnt[$prot_FDR_bin] = $decoy_cnt;
          $protFDR_prot_bin_cnt[$prot_FDR_bin] = $Prot_cnt;
          if ($prot_FDR_bin == 0) {  # collect stats for FDR=0.01
            $protFDR01prob = $prot_prob;
            $protFDR01grp_cnt = $ProtGrp_cnt;
            $protFDR01prot_cnt = $Prot_cnt;
          }
        }
      }
    }
    #if ($options{FDRthresh}) {
    #  $nprot_above_thresh = $protFDR01grp_cnt;  #FDR=0.01
    #}  else {
      $nprot_above_thresh = $Prot9cnt;
    #}
    $ProP = sprintf("| ProP  %6i (%6i,%6i)  ",
            $nprot_above_thresh,$PP9cnt,$aP9cnt);
    $tsv .= sprintf("%d\t%d\t",$nprot_above_thresh,$Prot9cnt);

  } else {
    $ProP = '                            ';
    unless ($Scnt) {
      print "ERROR: No pepXML or protXML to process\n";
      printUsage();
      exit;
    }
  }

  close(INFILE);


  my $Qual = '';

  if (1 == 0) {
    my $subdir = $pwd;
    $subdir =~ s/^.+\///;
    my $UHQSdir = "../${subdir}_UHQS";
    if ( -e $UHQSdir ) {
      my @results = `grep "scan num" $UHQSdir/*.mzXML | wc -l`;
      my $result = $results[0];
      if ($result) {
        chomp($result);
      }
      $Qual = sprintf("| QualS  %6i %6.3f  ",$result,$result/$Scnt);
      $tsv .= sprintf("%d\t%.3f",$result,$result/$Scnt);
    }

  } else {
    $infile = 'qualscore_results';  # lists high quality unassigned spectra
    if (open(INFILE,$infile)) {
      my $spectra;
      while (my $line = <INFILE>) {
        chomp($line);
	my @columns = split(/\s+/,$line);
	my $spectrum = $columns[0];
	$spectrum =~ s/\.\d\.dta$//;
        if (exists($spectra->{$spectrum})) {
          if ($spectra->{$spectrum}->[1] < $columns[1]) {
	    $spectra->{$spectrum} = \@columns;
	  }
	} else {
	  $spectra->{$spectrum} = \@columns;
	}
      }
      close(INFILE);

      my $QScounters;
      while (my ($spectrum_name,$spectrum_data) = each %{$spectra}) {
	if ($spectrum_data->[1] >= $P_threshold) {
	  $QScounters->{P9cnt}++;
	}
	if ($spectrum_data->[1] >= 0.95) {
	  $QScounters->{P95cnt}++;
	}
	if ($spectrum_data->[2] >= 1.0) {
	  $QScounters->{HQcnt}++;
	}
	if ($spectrum_data->[1] >= $P_threshold && $spectrum_data->[2] >= 1.0) {
	  $QScounters->{P9HQcnt}++;
	}
	if ($spectrum_data->[1] < $P_threshold && $spectrum_data->[2] >= 1) {
	  $QScounters->{UHQScnt}++;
	}
      }
      my $result = $QScounters->{UHQScnt};

      if (0) {
	print "Total spectra: ".scalar(keys(%{$spectra}))."\n";
	print "P>=$P_threshold spectra: ".$QScounters->{P9cnt}."\n";
	print "P>=0.95 spectra: ".$QScounters->{P95cnt}."\n";
	print "HQ spectra: ".$QScounters->{HQcnt}."\n";
	print "P>=$P_threshold HQ spectra: ".$QScounters->{P9HQcnt}."\n";
	print "UHQS spectra: ".$QScounters->{UHQScnt}."\n";
      }


      $Qual = sprintf("| QualS  %6i %6.3f  ",$result,$result/$Scnt);
      $tsv .= sprintf("%d\t%.3f",$result,$result/$Scnt);
    }
  }


#### Print out the summary information in the desired format
if ($options{tsv}) {
    print "$tsv\t$pwd\n";
} else {
    print "$PepP $ProP $Qual $pwd\n";
}


#### Write to console
print "Spectra identified at $threshold_string_long=$P9cnt\n";
print "Spectra searched=$Scnt\n";
print "Efficiency ID'd/searched=".sprintf("%.3f",$cnt_ratio)."\n";
if ($options{FDRthresh}) {
    print "Approx. P threshold for $threshold_string=$P_threshold\n";
}

#ning add
if ($num_decoy_P9cnt || $decoy_string) {
    my $PepPro_FDR;
    $PepPro_FDR = $pp_incorr/$P9cnt;
    $num_decoy_belowP1cnt = -1 unless ($num_decoy_belowP1cnt);
    $total_num_belowP1cnt = -2 unless ($total_num_belowP1cnt);
    my $decoy_fraction = $num_decoy_belowP1cnt/$total_num_belowP1cnt;
    my $decoy_FDR = $num_decoy_P9cnt / $decoy_fraction / $P9cnt;
    
    printf ("PP Incor IDs $threshold_string=%.1f\n", $pp_incorr);
    print "Decoy IDs at $threshold_string=$num_decoy_P9cnt\n";
    print "Decoy IDs at P<0.001=$num_decoy_belowP1cnt\n";
    print "Total IDs at P<0.001=$total_num_belowP1cnt\n";
    print "Decoy fraction P<0.001=".sprintf("%.4f",$decoy_fraction)."\n";
    print "FDR based on PepPro=".sprintf("%.4f",$PepPro_FDR)."\n";
    print "FDR based on decoys=".sprintf("%.4f",$decoy_FDR)."\n";
    print "FDR after discarding decoys=".sprintf("%.4f",$decoy_FDR * (1-$decoy_fraction))."\n";
    foreach my $key (sort(keys(%num_decoy_P9cnt_bycharge))) {
	print "Decoy IDs at $threshold_string (charge $key)=$num_decoy_P9cnt_bycharge{$key}\n";
    }
}

#### Write out information to summary file
if ($options{write_summary}) {
    
    print OUTFILE "Spectra identified at $threshold_string_long=$P9cnt\n";
    print OUTFILE "Spectra searched=$Scnt\n";
    print OUTFILE "Efficiency ID'd/searched=".sprintf("%.3f",$cnt_ratio)."\n";
    if ($options{FDRthresh}) {
	print OUTFILE "Approx. P threshold for $threshold_string=$P_threshold\n";
    }    

    #ning add
    if ($num_decoy_P9cnt || $decoy_string) {
        my $PepPro_FDR;
	$PepPro_FDR = $pp_incorr/$P9cnt;
	$num_decoy_belowP1cnt = -1 unless ($num_decoy_belowP1cnt);
	$total_num_belowP1cnt = -2 unless ($total_num_belowP1cnt);
	my $decoy_fraction = $num_decoy_belowP1cnt/$total_num_belowP1cnt;
        my $decoy_FDR = $num_decoy_P9cnt / $decoy_fraction / $P9cnt;
	printf OUTFILE ("PP Incor IDs $threshold_string=%.1f\n", $pp_incorr);
	print OUTFILE "Decoy IDs at $threshold_string=$num_decoy_P9cnt\n";
	print OUTFILE "Decoy IDs at P<0.001=$num_decoy_belowP1cnt\n";
	print OUTFILE "Total IDs at P<0.001=$total_num_belowP1cnt\n";
	print OUTFILE "Decoy fraction P<0.001=".sprintf("%.4f",$decoy_fraction)."\n";
	print OUTFILE "FDR based on PepPro=".sprintf("%.4f",$PepPro_FDR)."\n";
	print OUTFILE "FDR based on decoys=".sprintf("%.4f",$decoy_FDR)."\n";
        
	print OUTFILE "FDR after discarding decoys=".sprintf("%.4f",$decoy_FDR * (1-$decoy_fraction))."\n";
        foreach my $key (sort(keys(%num_decoy_P9cnt_bycharge))) {
	    print OUTFILE "Decoy IDs at $threshold_string (charge $key)=$num_decoy_P9cnt_bycharge{$key}\n";
        }
    }
    
}



#### If this is an empty file, return
exit unless ($P9cnt);


if ($options{model} || $options{model09}) {
    print "PeptideProphet Model:\n";

    foreach my $model (sort(keys(%models))) {
      my $est = $models{$model};
      if ($est < .1) {
	$est = 'NO OR BAD MODEL';
      } else {
        $est = "est_tot_correct = $est";
      }
      print "  Charge $model: $est\n";
    }

    if (scalar(keys(%roc_sens)) < 0.0001) {
	print "  NO OR BAD MODEL  \n";
    } else {
	
        if ($est_tot_num_corr >= 0.0001) {
          print "  Total estimated num correct = $est_tot_num_corr\n";
        }
        print "  minprob  sensit.  error  nCorr  nIncorr #decoys>prob\n";
	foreach( sort(keys(%roc_sens)) ) {
          # Henry adds:
          # Counting decoys at each probability bin (rather than just P>0.9 and P<0.0001)
          # This allows us to compare decoy-estimated FDR to PP-predicted FDR
          # at all probability cutoffs
          
          my $num_decoy_below_prob = 0;
          for (my $pp = 0; $pp < int($_ * 100); $pp++) {
            $num_decoy_below_prob += $num_decoy_byprob[$pp];
          }
          my $num_decoy_at_above_prob = $num_decoy - $num_decoy_below_prob;

	  unless ($options{model09} && $_ ne $P_threshold_format) {
	    print ("  P=$_ : $roc_sens{$_}, $roc_err{$_}, $roc_nCorr{$_}, $roc_nIncorr{$_}, $num_decoy_at_above_prob\n");
	  }
          # End Henry
	}
    }
}


#### Print out the sensitivity and threshold at FDR 1%
#if ($options{FDR01}) {
#  my $total_num_correct = $roc_nCorr{'0.00'};
#  if ($total_num_correct) {
#    my $num_corr = $error_point{num_corr}->{'0.010'};
#    print "For FDR 0.01, sensitivity=".sprintf("%.3f",$num_corr/$total_num_correct)." at P=$error_point{min_prob}->{'0.010'}\n";
    #### Write out information to summary file
#    if ($options{write_summary}) {
#      print OUTFILE "P threshold for FDR 0.01=$error_point{min_prob}->{'0.010'}\n";
#      print OUTFILE "Sensitivity for FDR 0.01=".sprintf("%.3f",$num_corr/$total_num_correct)."\n";
#    }

#  } else {
#    print "  NO OR BAD MODEL  \n";
#  }
#}


if ($options{peptide_count}) {
    print "Peptide Count at $threshold_string:\n";
    my $unique_ids;
    my $singletons;
    my $doubletons;

    foreach (keys(%all_ids)) {
        $unique_ids++;
        if ($all_ids{$_} <= 1.001) {
            $singletons++;
        } elsif ($all_ids{$_} <= 2.001) {
            $doubletons++;
        }
        
    }
    printf("  Distinct peptides = $unique_ids \n");
    printf("  Singletons = $singletons (%1.2f) \n", $singletons / $unique_ids);
    printf("  Doubletons = $doubletons (%1.2f) \n", $doubletons / $unique_ids);


    #### Write out information to summary file
    if ($options{write_summary}) {
      print OUTFILE "Distinct peptides at $threshold_string_long=$unique_ids\n";
      print OUTFILE "Singly observed peptides at $threshold_string_long=$singletons\n";
    }

  }
	
    if ($options{peps_per_run}) {
      foreach (keys(%peps_per_run)) {
        printf("  %-40s: %6d\n", $_, $peps_per_run{$_});
      }
      #### Write out information to summary file
      if ($options{write_summary}) {
        foreach (keys(%peps_per_run)) {
          printf(OUTFILE "  %s=%d\n", $_, $peps_per_run{$_});
        }
      }
    }

	
  if ($options{charge}) {
    foreach my $charge (sort(keys(%P9cntbycharge))) {
      print "  Charge $charge: ".$P9cntbycharge{$charge}."  ($threshold_string)\t".
	"Queries=".$Qcntbycharge{$charge}."\n";
      #### Write out information to summary file
      if ($options{write_summary}) {
	print OUTFILE "IDs $threshold_string_long with charge $charge=$P9cntbycharge{$charge}/$Qcntbycharge{$charge}\n";
      }
    }
  }


  #### Analyze Number of Enzymatic/Tryptic Termini (NET/NTT)
  if ($options{net}) {
    foreach my $ntt (sort(keys(%ntt))) {
      print "  Enzymatic Termini $ntt: $ntt{$ntt}\n";
      #### Write out information to summary file
      if ($options{write_summary}) {
	print OUTFILE "IDs $threshold_string_long with NTT $ntt=$ntt{$ntt}\n";
      }
    }
    if ($ntt{1} && $ntt{2}) {
      my $semiFraction = sprintf("%.4f",( $ntt{1} / ( $ntt{1} + $ntt{2} ) ));
      print "  Fraction semi-tryptics: $semiFraction\n";
      if ($options{write_summary}) {
	print OUTFILE "Fraction semi-tryptics=$semiFraction\n";
      }
    }
  }


  if ($options{residues}) {
    print "Observed residues at $threshold_string:\n";
    foreach my $residue (sort(keys(%residues))) {
      printf("  $residue %7i %7.2f percent\n",$residues{$residue},
        $residues{$residue}/$total_residues*100);
    }
  }

  #### Analyze number of missed cleavages
  if ($options{num_missed_cleavages}) {
    my ($noMissedCleavages,$missedCleavages);
    foreach my $num_missed_cleavages (sort(keys(%num_missed_cleavages))) {
      $noMissedCleavages = $num_missed_cleavages{$num_missed_cleavages} if ($num_missed_cleavages == 0);
      $missedCleavages += $num_missed_cleavages{$num_missed_cleavages} if ($num_missed_cleavages > 0);
      print "  N missed cleavages $num_missed_cleavages: ".$num_missed_cleavages{$num_missed_cleavages}."  ($threshold_string)\n";
      #### Write out information to summary file
      if ($options{write_summary}) {
	print OUTFILE "Missed cleavages $num_missed_cleavages=$num_missed_cleavages{$num_missed_cleavages}\n";
      }
    }

    if ($noMissedCleavages && $missedCleavages) {
      my $missedCleavagesFraction = sprintf("%.4f",$missedCleavages / ( $missedCleavages + $noMissedCleavages ) );
      print "  Fraction of missed cleavages: $missedCleavagesFraction\n";
      #### Write out information to summary file
      if ($options{write_summary}) {
	print OUTFILE "Fraction of missed cleavages=$missedCleavagesFraction\n";
      }
    }

  }
	
	
	if ($options{modifications}) {
	    print "Total Modifications at $threshold_string:\n";
	    foreach my $mod (sort(keys(%modifications))) {
		print "  Modification: $mod: $modifications{$mod}\n";
		#### Write out information to summary file
		if ($options{write_summary}) {
		  print OUTFILE "IDs $threshold_string_long with modification $mod=$modifications{$mod}\n";
		}
	    }
	}
	
	
	if ($options{modification_combinations}) {
	    print "Modification combinations at $threshold_string:\n";
	    foreach my $mod (sort(keys(%modification_combinations))) {
		print "  Modification combination $mod: $modification_combinations{$mod}\n";
	    }
	}
	
	
	if ($options{glyco_motifs}) {
	    my %other_N_motifs;
	    my %N_notif_trailing_chars;
	    print "N[115] motifs:\n";
	    my $n_NXST = 0;
	    foreach my $motif (sort(keys(%glyco_motifs))) {
		print "  Motif $motif: $glyco_motifs{$motif}\n";
		if ($motif =~ /N\[115\]\w[ST]/) {
		    $n_NXST += $glyco_motifs{$motif};
		} else {
		    $other_N_motifs{$motif} = $glyco_motifs{$motif};
		}
		my $last_char = substr($motif,7,1);
		$N_notif_trailing_chars{$last_char} += $glyco_motifs{$motif};
	    }
	    print "NXS/T motifs: $n_NXST\n";
	    print "Non NXS/T motifs:\n";
	    foreach my $motif (sort(keys(%other_N_motifs))) {
		print "  Motif $motif: $other_N_motifs{$motif}\n";
	    }
	    print "All Nx? motifs:\n";
	    foreach my $last_char (sort(keys(%N_notif_trailing_chars))) {
		print "  Motif N[115]x$last_char: $N_notif_trailing_chars{$last_char}\n";
	    }

	}
	
	
	if ($options{massdiff}) {
	    my $sbeams = new StatUtilities;
	    my $massdiffavg = $sbeams->average(values=>\@massdiffs);
	    my $massdiffstdev = $sbeams->stdev(values=>\@massdiffs);
	    #### Write out information to summary file
	    if ($options{write_summary}) {
	      print OUTFILE "Massdiff mean=".sprintf("%.3f",$massdiffavg)."\n";
	      print OUTFILE "Massdiff stdev=".sprintf("%.3f",$massdiffstdev)."\n";
	    }
	    my $buf = '*************************************************************************';
	    print "Mass diffs for P > $P_threshold: Mean: $massdiffavg  StDev: $massdiffstdev\n";

	    if (1 == 1) {
		my $bin_size = $options{histogram_resolution} || 0.5;
		my $bin_shift = $bin_size/2.0;
		my $result = $sbeams->histogram(data_array_ref=>\@massdiffs,bin_size=>$bin_size,);
		#print "Mass diffs for P > $P_threshold: Mean: $result->{mean}  StDev: $result->{stdev}\n";
		print "Mass diffs for P > $P_threshold: Min: $result->{minimum}  Max: $result->{maximum}\n";
		my $hmin = int(($result->{minimum}-$bin_size+0.000001)/$bin_size)*$bin_size - $bin_shift;
		my $hmax = int(($result->{maximum}+$bin_size-0.000001)/$bin_size)*$bin_size + $bin_shift;
		#print "hmin=$hmin, hmax=$hmax\n";
		$result = $sbeams->histogram(data_array_ref=>\@massdiffs,bin_size=>$bin_size,
					     min=>$hmin,max=>$hmax);
		
		my $max = $result->{ymax};
		my @xaxis = @{$result->{xaxis}};
		my @yaxis = @{$result->{yaxis}};
		my $ymax = 0;
		for (my $i=0; $i< scalar(@xaxis); $i++) {
		    $ymax = $yaxis[$i] if ($yaxis[$i] > $ymax);
		}
		for (my $i=0; $i< scalar(@xaxis); $i++) {
		    my $bar = substr($buf,0,$yaxis[$i]/$ymax*50);
		    printf("%6.3f -> %6.3f %6i |%s\n",$xaxis[$i],$xaxis[$i]+$bin_size,$yaxis[$i],$bar);
		}
	    }
	}

#print protein info (based on PepPro/iPro results only; no ProteinProphet)
if ($options{protein_count}) {

    my $num_unique_proteins = scalar(keys (%all_unambiguous_proteins));
    my $num_unique_decoy_proteins = 0;
    my $num_unique_unmapped_proteins = 0;

    foreach my $pro (keys (%all_unambiguous_proteins)) {
	if ($pro =~ /$decoy_string/) {
	    $num_unique_decoy_proteins++;
	}
	if ($pro =~ /UNMAPPED/) {
	    $num_unique_unmapped_proteins++;
	}
    }

    print "Distinct proteins mapped to unambiguously=$num_unique_proteins\n";
    print "Distinct $decoy_string proteins mapped to unambiguously=$num_unique_decoy_proteins\n";
    print "Distinct UNMAPPED proteins mapped to unambiguously=$num_unique_unmapped_proteins\n";
    if ($options{write_summary}) {
	print OUTFILE "Distinct proteins mapped to unambiguously=$num_unique_proteins\n";
	print OUTFILE "Distinct $decoy_string proteins mapped to unambiguously=$num_unique_decoy_proteins\n";
	print OUTFILE "Distinct UNMAPPED proteins mapped to unambiguously=$num_unique_unmapped_proteins\n";
    }

    $num_unique_proteins = scalar(keys (%all_proteins));
    $num_unique_decoy_proteins = 0;
    $num_unique_unmapped_proteins = 0;

    foreach my $pro (keys (%all_proteins)) {
	if ($pro =~ /$decoy_string/) {
	    $num_unique_decoy_proteins++;
	}
	if ($pro =~ /UNMAPPED/) {
	    $num_unique_unmapped_proteins++;
	}
    }

    print "Distinct proteins maximally spanned=$num_unique_proteins\n";
    print "Distinct $decoy_string proteins maximally spanned=$num_unique_decoy_proteins\n";
    print "Distinct UNMAPPED proteins maximally spanned=$num_unique_unmapped_proteins\n";

    if ($options{write_summary}) {
	print OUTFILE "Distinct proteins maximally spanned=$num_unique_proteins\n";
	print OUTFILE "Distinct $decoy_string proteins maximally spanned=$num_unique_decoy_proteins\n";
	print OUTFILE "Distinct UNMAPPED proteins maximally spanned=$num_unique_unmapped_proteins\n";
    }
}

#### Write out protein information (ProteinProphet)
if ($protein_info) {
  print "ProteinProphet info:\n";
  print "  Proteins identified at P>0.9=$Prot9cnt\n";
  print "  Distinct modified peptides at P>$P_threshold=$PP9cnt\n";
  printf("  P threshold for protein group FDR 0.01=%.4f\n", $protFDR01prob);
  print "  Protein groups identified at FDR 0.01=$protFDR01grp_cnt\n";  
  for ($i = 0; $i < $nbins; $i++) {
    if ($protFDR_decoy_bin_cnt[$i] == -1) { next; }  #this bin unpopulated
    my $percentage;
    if ($protFDR_prot_bin_cnt[$i] == 0) {
      $percentage = 0.0;
    } else {
      $percentage = $protFDR_decoy_bin_cnt[$i] / $protFDR_prot_bin_cnt[$i];
    }
    printf("  Decoy IDs P>0 at protein group FDR 0.0%d=%d (%.4f)\n",
              $i+1, $protFDR_decoy_bin_cnt[$i], $percentage);
  }
  if ($options{write_summary}) {
    print OUTFILE
     "ProtPro distinct modified peptides at P>$P_threshold=$PP9cnt\n";
    #print OUTFILE
    # "Distinct modified peptides at adjusted P>$P_threshold0=$aP9cnt\n";
    print OUTFILE "Proteins identified at P>0.9=$Prot9cnt\n";
    printf OUTFILE ("P threshold for protein group FDR 0.01=%.4f\n",
      $protFDR01prob);
    print OUTFILE "Protein groups identified at FDR 0.01=$protFDR01grp_cnt\n";  
    print OUTFILE
      "Proteins included in groups at FDR 0.01=$protFDR01prot_cnt\n";  
    for ($i = 0; $i < $nbins; $i++) {
      if ($protFDR_decoy_bin_cnt[$i] == -1) { next; }  #this bin unpopulated
      my $percentage;
      if ($protFDR_prot_bin_cnt[$i] == 0) {
        $percentage = 0.0;
      } else {
        $percentage = $protFDR_decoy_bin_cnt[$i] / $protFDR_prot_bin_cnt[$i];
      }
      printf OUTFILE ("Decoy IDs P>0 at protein group FDR 0.0%d=%d (%.4f)\n",
              $i+1, $protFDR_decoy_bin_cnt[$i],
              $percentage);
    }
  }
}

#### Close summary file
if ($options{write_summary}) {
  close(OUTFILE);
}


#### Write decoy file
if ($options{write_summary} && @decoy_probabilities && 0) {
  open(OUTFILE,">$rootname.decoy.txt") ||
    die("ERROR: Unable to write output file $rootname.decoy.txt");

  foreach my $point ( qw ( 0.80 0.85 0.90 0.95 0.98 0.99 ) ) {
    print OUTFILE "0\t$point\t$roc_nIncorr{$point}\n";
  }

  my $sbeams = new StatUtilities;
  my $result = $sbeams->histogram(data_array_ref=>\@decoy_probabilities,bin_size=>0.01,
			       min=>0.80,max=>1.01);

  my @xaxis = @{$result->{xaxis}};
  my @yaxis = @{$result->{yaxis}};

  for (my $i=0;$i<scalar(@xaxis);$i++) {
    print OUTFILE "1\t$xaxis[$i]\t$yaxis[$i]\n";
  }

  for (my $i=scalar(@xaxis)-2;$i>=0;$i--) {
    $yaxis[$i] = $yaxis[$i]*2 + $yaxis[$i+1]
  }

  my %ndecoyt2;
  for (my $i=0;$i<scalar(@xaxis);$i++) {
    print OUTFILE "2\t$xaxis[$i]\t$yaxis[$i]\n";
    $ndecoyt2{$xaxis[$i]}=$yaxis[$i];
  }

  my $cor1 = $roc_nIncorr{'0.90'}/$ndecoyt2{'0.9'};
  my $cor2 = $roc_nIncorr{'0.95'}/$ndecoyt2{'0.95'};
  print OUTFILE "3\t$cor1\t$cor2\n";

  close(OUTFILE);

}



exit;



sub printUsage {
  print( <<"  END" );

Usage:  calctppstat.pl [ -i -M -p -d -r -g -n -t ]

  -h, --help          Print this usage information and exit
  -i, --input_file    pepXML file to analyze
  -d, --decoy_string  Set prefix for decoy protein names (default: DECOY)
      --full          Full set of options: to selecting all *'ed options
* -c, --charge        Print information about charge states encountered
  -M, --model         calculate and print model information
      --model09       Print just the model information for P >= $P_threshold
*     --FDR01         Equivalent to --FDRthresh 0.01
*     --peptide_count Print out peptide class stats, i.e. num singletons, etc.
*     --massdiff      Calculate and print histogram of calc vs. theor. mass
  -r, --residues      Print out amino acid usage information
* -n, --num_missed_cleavages  Calculate and print missed cleavage stats.
  -g, --glyco_motifs  Print information on NXS/T motifs
* -e, --net           Calculate and print number of enzymatic termini.
*     --modifications Calculate and print number of modification instances.
      --modification_combinations
                      Calculate and print number of modification combinations.
  -t, --tsv           Format output as TSV
      --require_charge=n  Only compute statistics for assumed_charge=n data
  -w, --write_summary Write a summary output file like interact.summary.txt
      --filterByMassDiff n.nn:m.mm  Only consider peptides with mass diff
                                    between n.nn and m.mm inclusive
  -p <value>, --pthresh <value>       Use probability threshold of <value>
  -f <value>, --FDRthresh <value>     Use FDR threshold of <value>



  END
  exit;
}
