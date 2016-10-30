#!/tools/bin/perl -w
#
# get_prots.pl
# Extract protein info from a protXML/XLS file
# 2011 luis@isb

use strict;
use Getopt::Std;

my %options;
my %heading_map;
my %prot_output;
my %pep_output;
my %max_cov; #max group pct share of specs
my %max_cov_rank; #max group pct share of specs
my %pct_share;
my %pct_share_rank;


# column headings in prot.xls output
my $entry_str        = 'entry no.';
my $coverage_str     = 'percent coverage';
my $peptide_seq_str  = 'peptide sequence';
my $protein_str      = 'protein';
my $protein_prob_str = 'protein probability';
my $protein_len_str = 'protein length';
my $group_prob_str   = 'group probability';
my $tot_spectra_str  = 'tot indep spectra';
my $pct_spectra_str  = "percent share of spectrum ids";

my $description_str  = 'description';
my $peptide_nsp_str  = 'nsp adjusted probability';
my $non_degen_str    = 'is nondegenerate evidence';
my $weight_str    = 'weight';

my $grppct_spectra_str  ="max group pct share of spectrum ids";

# Process inputs and options
getopts('dI:X:p:', \%options);

my $file  = shift || &usage("ProtXML/Excel Input File");
my $filter_in = $options{I} || '';
my $filter_out = $options{X} || '';
my $minProb = $options{p} || -1;
my $debug = $options{d} || '';

&usage("File $file not found.  Please specify an input file!\n") unless (-e $file);

my $basename;
my $xlsfile;
if ($file =~ /\.xml$/) {
    $basename = $`;
    $xlsfile = "$basename.xls";
} elsif ($file =~ /\.xls$/) {
    $basename = $`;
    $xlsfile = $file;
} else {
    &usage("File $file is not of a recognized type (xml/xls).  Please specify another input file\n");
}

&writeXLS($file) if (! -e $xlsfile);

&readProtXLSFile($xlsfile);



exit;


##########
sub readProtXLSFile {
    my $infile = shift;

    print "Attempting to read tabular data from $infile\n";
    open(PROT, "$infile") || die "Cannot open file:$infile:$!";

    my $headers = 1;

    while(<PROT>) {
	
	my $columns = $_;
	my @columns = split /\t/, $columns;
	

	next if ($#columns <= 0);
	if ($headers) {
	    my $i = 0;
	    foreach (@columns) {
		$heading_map{$_} = $i;
		print "[col$i]$_\n" if ($debug);
		$i++;
	    }
	    $headers = 0;
	    next;
	}
       
	my @key = split /[a-z]/, $columns[$heading_map{$entry_str}] ; 
	if (!exists($prot_output{$columns[$heading_map{$entry_str}]})) {
	    $prot_output{$columns[$heading_map{$entry_str}]} = $columns;
	    $pep_output{$columns[$heading_map{$entry_str}]} = $columns[$heading_map{$peptide_seq_str}]. " " .$columns[$heading_map{$weight_str}]. " " 
		.$columns[$heading_map{$non_degen_str}]. " " .$columns[$heading_map{$peptide_nsp_str}];
	}
	else {
	    $pep_output{$columns[$heading_map{$entry_str}]} .= ",".  $columns[$heading_map{$peptide_seq_str}]. " " .$columns[$heading_map{$weight_str}]. " " 
		.$columns[$heading_map{$non_degen_str}]. " " .$columns[$heading_map{$peptide_nsp_str}];
	}
	if ($columns[$heading_map{$pct_spectra_str}] ne "") {
		if (!exists($max_cov{$key[0]}) || $max_cov{$key[0]} < $columns[$heading_map{$pct_spectra_str}]) {
			$max_cov{$key[0]} = $columns[$heading_map{$pct_spectra_str}];
		}		
		$pct_share{$columns[$heading_map{$entry_str}]} = $columns[$heading_map{$pct_spectra_str}];
	    }
	else {
	    $pct_share{$columns[$heading_map{$entry_str}]} = 0;
	}
    }
    close(PROT);

    #compute rank
    my $last = -1;
    my $rank = 0;
    foreach my $key ( sort { $pct_share{$b} <=> $pct_share{$a} } keys(%pct_share) ) {
	if ($last == -1 || $last != $pct_share{$key}) {
	    $rank++;
	}
	$pct_share_rank{$key} = $rank;
    }

    #compute rank
    $last = -1;
    $rank = 0;
    foreach my $key (sort { $max_cov{$b} <=> $max_cov{$a} } keys(%max_cov)  ) {
	if ($last == -1 || $last != $max_cov{$key}) {
	    $rank++;
	}
	$max_cov_rank{$key} = $rank;
    }
	
    
    my $entry_no     = $entry_str;
    my $pct_coverage = $coverage_str;
    my $proteins     = $protein_str;
    my $links = 'uniprot link';
    my $protein_prob = $protein_prob_str;
    my $peptide_prob = $peptide_nsp_str;
    my $protein_len = $protein_len_str;
    my $group_prob   = $group_prob_str;
    my $tot_spectra  = $tot_spectra_str;
    my $pct_spectra  = "percent share of spectrum ids";
    my $pct_spectra_rank  = "percent share of spectrum ids rank";
    my $grppct_spectra  = "max group pct share of spectrum ids";
    my $grppct_spectra_rank  = "max group pct share of spectrum ids rank";
    my $prot_desc    = $description_str;
    my $sequence     = $peptide_seq_str;
     $peptide_prob = $peptide_nsp_str;
    my $is_nondegen  = $non_degen_str;
    my $peptides = 'peptides';

    my $tot_pct = 0;
     $rank = 1;


    open(XLS, ">${infile}_filtered.xls");
    open(JSON, ">${infile}_filtered.json");
    print JSON "var data = [";
    #FIRST PASS gets groups to report
    my %output_groups;

    my @sortedkeys = 
	sort { 
	    my @b1 = split /[a-z]/, $b; 
	    my @a1 = split /[a-z]/, $a; 
	    my @b2 = split /[0-9]/, $b; 
	    my @a2 = split /[0-9]/, $a; 
	    if (exists($max_cov{$a1[0]}) && exists($max_cov{$b1[0]})) {
			if ($max_cov{$a1[0]} == $max_cov{$b1[0]}) {
			if ( $a1[0] eq $b1[0] ) {
				if (length($a2[$#a2]) == length($b2[$#b2])) {
				$a2[$#a2] cmp $b2[$#b2];			     
				}
				else {
				length($a2[$#a2]) <=> length($b2[$#b2]);
				}
				
			}
			else {
				$a1[0] <=> $b1[0];
			}
			}
			else {
			$max_cov{$b1[0]} <=> $max_cov{$a1[0]}
			}
	    }
	}  
    keys(%prot_output) ;
	
	my $line = -1;
    foreach my $key (@sortedkeys) {
	$line++;
	$key =~ s/=/_/g;  # clean up equals signs.  Might have to revisit this if we want to pull links from file...
	chomp (my @tmp_arr = split /\t/, $prot_output{$key});

	my @k1 = split /[a-z]/, $key; 

	
	if ($tmp_arr[0] && $tmp_arr[ $heading_map{$peptide_seq_str} ] ) {
		next if ($tmp_arr[$heading_map{$protein_prob_str}] < $minProb);
	    
		if ($filter_out) {
		    my $cnt1 = $tmp_arr[$heading_map{$protein_str}] =~ s/,/,/g;
		    my $cnt2 = $tmp_arr[$heading_map{$protein_str}] =~ s/$filter_out/$filter_out/g;
		    next if ($cnt1 == $cnt2-1);
		}
		
		next if ($filter_in && ($tmp_arr[$heading_map{$protein_str}] !~ $filter_in));

	    if ($entry_no eq $tmp_arr[$heading_map{$entry_str}] ) {
		$sequence     = $tmp_arr[$heading_map{$peptide_seq_str}];
		$peptide_prob = $tmp_arr[$heading_map{$peptide_nsp_str}];
		$is_nondegen  = $tmp_arr[$heading_map{$non_degen_str}];
		$protein_len = $tmp_arr[$heading_map{$protein_len_str}];
		$protein_len =~ s/;//g;
		$peptides     .= ",$sequence ($is_nondegen) ($peptide_prob)";
		
	    } else { #if (!$filter_in || ($proteins =~ $filter_in)) {
		$links = &makeLinks($proteins) ; #if ($filter_in && ($proteins =~ $filter_in));

		#print XLS "$entry_no\t$group_prob\t$protein_prob\t$protein_len\t$proteins\t$links\t$prot_desc\t$pct_coverage\t$tot_spectra\t$pct_spectra\t$pct_spectra_rank\t$grppct_spectra\t$grppct_spectra_rank\t$peptides\n";
		$output_groups{$k1[0]} = 1;
		
		$entry_no     = $tmp_arr[$heading_map{$entry_str}];
		$pct_coverage = $tmp_arr[$heading_map{$coverage_str}] || 0.0;
		$proteins     = $tmp_arr[$heading_map{$protein_str}];
		$protein_prob = $tmp_arr[$heading_map{$protein_prob_str}]|| 0.0;
		$protein_len = $tmp_arr[$heading_map{$protein_len_str}]|| 0;
		$protein_len =~ s/;//g;
		$group_prob   = $tmp_arr[$heading_map{$group_prob_str}]|| 0.0;
		$tot_spectra  = $tmp_arr[$heading_map{$tot_spectra_str}]|| 0.0;
		$pct_spectra  = $tmp_arr[$heading_map{$pct_spectra_str}]|| 0.0;
		$pct_spectra_rank  = $pct_share_rank{$key} || 0.0;
		$grppct_spectra  =  $max_cov{$k1[0]} || 0.0;
		$grppct_spectra_rank = $max_cov_rank{$k1[0]} || 0.0;
		$prot_desc    = $tmp_arr[$heading_map{$description_str}];
		$sequence     = $tmp_arr[$heading_map{$peptide_seq_str}];
		$peptide_prob = $tmp_arr[$heading_map{$peptide_nsp_str}];
		$is_nondegen  = $tmp_arr[$heading_map{$non_degen_str}];
		$tot_pct += not ($pct_spectra =~ /[a-zA-Z]/ ) ? $pct_spectra : 0 ;		
		$peptides     = $pep_output{$key};#"$sequence ($is_nondegen) ($peptide_prob)";

	    }

	}

    }
    $tot_pct += $pct_spectra;	
    

    $entry_no     = $entry_str;
    $pct_coverage = $coverage_str;
    $proteins     = $protein_str;
    $links = 'uniprot link';
    $protein_prob = $protein_prob_str;
    $protein_len = $protein_len_str;
    $peptide_prob = $peptide_nsp_str;
    $group_prob   = $group_prob_str;
    $tot_spectra  = $tot_spectra_str;
    $pct_spectra  = "percent share of spectrum ids";
    $pct_spectra_rank  = "percent share of spectrum ids rank";
    $grppct_spectra  = "max group pct share of spectrum ids";
    $grppct_spectra_rank  = "max group pct share of spectrum ids rank";
    $prot_desc    = $description_str;
    $sequence     = $peptide_seq_str;
    $peptide_prob = $peptide_nsp_str;
    $is_nondegen  = $non_degen_str;
    $peptides = 'peptides';
    
    $tot_pct = 0;
    $rank = 1;

	$line = -1;
    foreach my $key (@sortedkeys) {
	
	$line++;
	$key =~ s/=/_/g;  # clean up equals signs.  Might have to revisit this if we want to pull links from file...
	chomp (my @tmp_arr = split /\t/, $prot_output{$key});
	
	my @k1 = split /[a-z]/, $key; 
	
	
	if ($tmp_arr[0] && $tmp_arr[ $heading_map{$peptide_seq_str} ] ) {
	    
	    next if ($tmp_arr[$heading_map{$protein_prob_str}] < $minProb);
		
		next if (!exists($output_groups{$k1[0]}));

		
	    if ($filter_out) {
		my $cnt1 = $tmp_arr[$heading_map{$protein_str}] =~ s/,/,/g;
		my $cnt2 = $tmp_arr[$heading_map{$protein_str}] =~ s/$filter_out/$filter_out/g;
		next if ($cnt1 == $cnt2-1);
	    }

	    if ($entry_no eq $tmp_arr[$heading_map{$entry_str}] ) {
		$sequence     = $tmp_arr[$heading_map{$peptide_seq_str}];
		$peptide_prob = $tmp_arr[$heading_map{$peptide_nsp_str}];
		$is_nondegen  = $tmp_arr[$heading_map{$non_degen_str}];
		$protein_len = $tmp_arr[$heading_map{$protein_len_str}];
		$protein_len =~ s/;//g;
		$peptides     .= ",$sequence ($is_nondegen) ($peptide_prob)";
		
	    } else { 
		$links = $proteins eq $protein_str ? $links : &makeLinks($proteins); # if ($filter_in && ($proteins =~ $filter_in));

		if (!$filter_in || ($proteins =~ $filter_in)) {
		    print XLS "$entry_no\t$group_prob\t$protein_prob\t$protein_len\t$proteins\t$links\t$prot_desc\t$pct_coverage\t$tot_spectra\t$pct_spectra\t$pct_spectra_rank\t$grppct_spectra\t$grppct_spectra_rank\t$peptides\n";

		    if (not $protein_len =~ /[a-zA-Z]/  ) {
			print JSON '{"proteins": "' .  $proteins . '", "psm_id_share": ' . $pct_spectra .', "probability": '.$protein_prob.', "length": '.$protein_len.', "group_probability": '.$group_prob.'}, '."\n";

		    }
		#$output_groups{$k1[0]} = 1;
		}
		elsif (not ($pct_spectra =~ /[a-zA-Z]/)) {
		    $tot_pct -= $pct_spectra;
		}
		$entry_no     = $tmp_arr[$heading_map{$entry_str}];
		$pct_coverage = $tmp_arr[$heading_map{$coverage_str}] || 0.0;
		$proteins     = $tmp_arr[$heading_map{$protein_str}];
		$protein_prob = $tmp_arr[$heading_map{$protein_prob_str}]|| 0.0;
		$protein_len = $tmp_arr[$heading_map{$protein_len_str}]|| 0;
		$protein_len =~ s/;//g;
		$group_prob   = $tmp_arr[$heading_map{$group_prob_str}]|| 0.0;
		$tot_spectra  = $tmp_arr[$heading_map{$tot_spectra_str}]|| 0.0;
		$pct_spectra  = $tmp_arr[$heading_map{$pct_spectra_str}]|| 0.0;
		$pct_spectra_rank  = $pct_share_rank{$key}|| 0.0;
		$grppct_spectra  =  $max_cov{$k1[0]} || 0.0;
		$grppct_spectra_rank = $max_cov_rank{$k1[0]}|| 0.0;
		$prot_desc    = $tmp_arr[$heading_map{$description_str}];
		$sequence     = $tmp_arr[$heading_map{$peptide_seq_str}];
		$peptide_prob = $tmp_arr[$heading_map{$peptide_nsp_str}];
		$is_nondegen  = $tmp_arr[$heading_map{$non_degen_str}];
		$tot_pct += not ($pct_spectra =~ /[a-zA-Z]/ )? $pct_spectra : 0 ;		
		$peptides     = $pep_output{$key};#"$sequence ($is_nondegen) ($peptide_prob)";

	    }

	}

    }

    if (!$filter_in || ($proteins =~ /$filter_in/)) {
	$tot_pct += $pct_spectra;	    
    }

    if (!$filter_in || ($proteins =~ /$filter_in/) || $pct_spectra =~ /[a-zA-Z]/) {

	print XLS "$entry_no\t$group_prob\t$protein_prob\t$protein_len\t$proteins\t$links\t$prot_desc\t$pct_coverage\t$tot_spectra\t$pct_spectra\t$pct_spectra_rank\t$grppct_spectra\t$grppct_spectra_rank\t$peptides\n";
	if (not $protein_len =~ /[a-zA-Z]/  ) {
	    print JSON '{"proteins": "' .  $proteins . '", "psm_id_share": ' . $pct_spectra .', "probability": '.$protein_prob.', "length": '.$protein_len.', "group_probability": '.$group_prob.'},'."\n";
	    
	}
    }
    print JSON "];";
    close XLS;
    close JSON;
    print "Sum of % spectrum id's:  $tot_pct\n\n";

}


sub makeLinks {
    my $prots = shift || '';
    my $links = '';
    my $howmany = 0;

    chomp (my @tmp_prots = split /,/, $prots);

    for my $prt (@tmp_prots) {
	my $tmp_filt = $filter_in;
	$tmp_filt =~ s/\|/\\\|/g;
	if ($prt =~ /(tr|sp)\|([^\|]+)\|.*$tmp_filt/) {
	    $links .= "http://www.uniprot.org/uniprot/$2";
	    return $links; # just keep the first one...
	}
	

    }

    for my $prt (@tmp_prots) {
	
	
	if ($prt =~ /(tr|sp)\|([^\|]+)\|.*/) {
	    $links .= "http://www.uniprot.org/uniprot/$2";
	    return $links; # just keep the first one...
	}
	

    }
    

    for my $prt (@tmp_prots) {
	
	
	if ($prt =~ /(ENS[A-Z0-9]+).*/) {
	    $links .= "http://www.ensembl.org/Multi/Search/Results?species=all;idx=;q=$1";
	    return $links; # just keep the first one...
	}
	
	
    }
    

    
    return $links;
}



##########
sub writeXLS {
    my $xmlfile = shift;

    # yea, do this later...

}

##########
sub usage {
    my $bad_param = shift || '';
    print "ERROR: Missing or bad input parameter: $bad_param\n" if $bad_param;
    print << "EOU";
Usage: $0 [options] <file> [<filter in string>] [<filter out string>]
Generate xls pritein output from protXML results file

<File> is a tab-delimited (xls) output file from ProteinProphet.

Options:
  -I        Filter Include String
  -X        Filter Exclude String
  -d        Enable debug messages

EOU

    exit(1);
}
