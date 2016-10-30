#!/usr/bin/perl -w


use strict;

$, = ' ';
$\ = "\n";

# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath cwd);
use lib realpath(dirname($0));
use tpplib_perl; # exported TPP lib function points
my $WIN = 500 ;

my $MinSamplePct = 0.01;

if(@ARGV == 0) {
    print STDERR " usage: compareProts.pl [-D3] [OPTIONS] <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison to tab delim file 'compare.xls'\n\n";
    print STDERR " or to rename output file: \n";
    print STDERR " or to rename output file: \n";
    print STDERR "        compareProts.pl -Nalternative.xls <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison to user specified excel file 'alternative.xls'\n\n";
    print STDERR " or to specify input columns to be reported (along with protein names): \n";
    print STDERR "        compareProts.pl -Nalternative.xls -h'protein probability' -h'ASAPRatio mean' <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison with protein probability and ASAPRatio mean columns to user specified tab delim file 'alternative.xls'\n\n";
    print STDERR " or to specify NO columns to be reported (along with protein names): \n";
    print STDERR "        compareProts.pl -Nalternative.xls -h <tab delim file1><tab delim file2> ...\n";
	
	print STDERR "\n\nOPTIONS:\n";
	print STDERR "\t-p<minimum probability>\n";
	print STDERR "\t-s<minimum percentage of samples containing each protein>\n";
	print STDERR "\t-w<protein window size>\n";
	print STDERR "\t-nP\tDisable Protein Clustering\n";
	print STDERR "\t-nF\tDisable File Clustering\n";
    print STDERR "\n\n";
    exit(1);
}

my $D3 = 0;

my $D3_HEADER = 

'/d3-328.js"></script>
    <style type="text/css">


a:link {text-decoration:none;}    /* unvisited link */
a:visited {text-decoration:none;} /* visited link */
a:hover {text-decoration:underline;}   /* mouse over link */
a:active {text-decoration:underline;}  /* selected link */

svg {
  font: 10px sans-serif;
  shape-rendering: crispEdges;
}

.help {
  font: 10px sans-serif;
  shape-rendering: crispEdges;
  color: #DD0000;
}

path.dot {
  fill: white;
  stroke-width: 1.5px;
}

td      {
    font-family: Helvetica, Arial, sans-serif; 
    font-size: 9pt; 
    vertical-align: top
    
}


button {
  left: 275px;
  position: absolute;
  top: 145px;
  width: 80px;
}


#TableDiv {
            display: table-cell; 
            border: 2px solid black;
            margin: 0;
            }

.data {
        /* borders for the whole table */
        table-layout: auto;
        /*border: none;*/
        background: #aaaaaa;
        border-collapse: collapse;
        border: 1px solid black;
	display: table-cell; 
        margin: 0;
      }
td {
	border-left: 2px solid white;
	border-right: 2px solid white;
	width: 33%;
  }

td.ratio {
        width: 10%;
  }

td.link  {
        width: 20%;
  }


td.desc {
        width: 70%;
  
  }


tr.even {
                background: #DDE0FF;
		border: 1px solid steelblue;
        }



tr.odd {
               background: #eeeeee;
	       border: 1px solid cyan;
	       	      
       }



table.top {
                background: #FFE000;
		border: 1px solid red;
		width: 100%;
		text-align: left;
        }

table.spacer {
                background: lightgray;
		border: 1px solid lightgray;
		width: 100%;
		text-align: left;
        }


table.even {
                background: #DDE0FF;
		border: 1px solid steelblue;
		width: 100%;
	        
        }



table.odd {
               background: #eeeeee;
	       border: 1px solid cyan;
	       width: 100%;
	       text-align: left;
	   }
 
 body {
         background-color: #aaaaaa;

	}
.spacer {
  height: 30px;
  width: 10px;
  fill: none;
  border: 1px solid;
  display: inline;
}

    </style>
  </head>
  <body>
    <script type="text/javascript">

';

my @FILES = (); #@ARGV;

my $minProb = 0.9;
my $protCluster = 1;
my $fileCluster = 1;
my $outfile = 'compare.xls';

my @info = ();

# check for outfile
for(my $k = 0; $k < @ARGV; $k++) {
    if($ARGV[$k] =~ /^\-N(\S+)$/) {
	$outfile = $1;
    }
    elsif($ARGV[$k] =~ /^\-p(.*)$/) {
	$minProb = $1;
    }
	elsif($ARGV[$k] =~ /^\-s(.*)$/) {
	$MinSamplePct = $1;
    }
	elsif($ARGV[$k] =~ /^\-w(.*)$/) {
	$WIN = $1;
    }
    elsif($ARGV[$k] =~ /^\-h(.*)$/) {
	push(@info, $1);
    }
    elsif($ARGV[$k] =~ /^\-D3$/) {
	$D3 = 1;
    }
	elsif($ARGV[$k] =~ /^\-nP$/) {
	$protCluster = 0;
    }
	elsif($ARGV[$k] =~ /^\-nF$/) {
	$fileCluster = 0;
    }
    else {
		push(@FILES, $ARGV[$k]);
    }
}


die "no input excel files specified\n" if(@FILES == 0);

my $JS_HOME = 'ISB/html/js/';

if ( $^O eq 'linux' ) {
    $JS_HOME = 'tpp/html/js/';  
#
# Cygwin Configuration
#
} elsif ( ($^O eq 'cygwin' )||($^O eq 'MSWin32' )) {
    $JS_HOME = 'ISB/html/js/';
}


#my @all_prots = ();
my @inds_ref = ();
my @inds = ();
my %all_prots = ();
#@info = ('group_no', 'prot_prob', 'ASAPRatio mean') if(@info == 0); # default
@info = ('entry no.', 'protein probability',  'ratio mean', 'group probability', 'percent share of spectrum ids', 'group percent share of spectrum ids') if(@info == 0); # default
@info = () if(@info == 1 && ! $info[0]);

my %info = ();

foreach(@info) {
	$_ =~ s/'//g ; ############'################
    my %next = ();
    for(my $f = 0; $f < @FILES; $f++) {
	$next{$FILES[$f]} = -1;
    }
    $info{$_} = \%next;
}

my %prots = ();

my %prot_groups = ();

for(my $f = 0; $f < scalar(@FILES); $f++) {
#foreach(@FILES) {
#    print "$_....\n";
    #print STDERR "$ARGV[$f]...\n";
    print STDERR " $FILES[$f]...\n";
    $prots{$FILES[$f]} = extractProts($FILES[$f]);

    $prot_groups{$FILES[$f]} = extractProtGroups($FILES[$f]);
    #$prots{$ARGV[$f]} = extractProts($ARGV[$f]);
    #push(@all_prots, extractProts($_));
    push(@inds_ref, 1);
} # next file

#exit(1);
# now look at by subset
#print "total: \n";
#foreach my $key1  (keys %prots) { foreach my $key2 (keys $prots{$key1}) { print "$key1, $key2 => $prots{$key1}{$key2}"; } } print "\n"; exit(1);


#foreach(@inds_ref) {
#    print "$_ ";
#}
print "\n";
my $index = 1;
#while(increment(\@inds_ref, $index++)) {
 for(my $k = 0; $k <= $#inds_ref; $k++) {
	    ${$inds[0]}[$k] = 1;	
 }
	
#$index--;



#print "final index: $index\n"; exit(1);
my @all_prots = keys %all_prots;
#foreach(@all_prots) {
#    print "$_ ";
#}

# now group each entry according to where it falls
my @grp_members = (); 
my %missing_prots_byfile;
my @founds = ();
for(my $k = 0; $k <= $#all_prots; $k++) {
	$founds[$k] = 0;
}
for(my $k = 0; $k < $index; $k++) {
    my @next_group = ();
    for(my $j = 0; $j <= $#all_prots; $j++) {
	my $f;
	my $mem = 1;
	#if(!$founds[$j]) {
	    for($f = 0; $f <= $#FILES; $f++) {

		#		${$prots {$FILES[$f]}{${$prot_groups{$FILES[$f]}}{$all_prots[$j]}} }

				if ( !exists (${$prot_groups{$FILES[$f]}}{$all_prots[$j]}) ) {
					if (! exists($missing_prots_byfile{$FILES[$f]}) ){
						@ { $missing_prots_byfile{$FILES[$f]} } = ();
					}
					#TODO: Implement this SMARTLY !!!
					#push(@ { $missing_prots_byfile{$FILES[$f]} },  $all_prots[$j]);
					$mem = 0;
				}
				elsif (${$inds[$k]}[$f] != exists ${$prot_groups{$FILES[$f]}}{$all_prots[$j]} ) { # not a member
					if (! exists($missing_prots_byfile{$FILES[$f]}) ){
						@ { $missing_prots_byfile{$FILES[$f]} } = ();
					}
					#TODO: Implement this SMARTLY !!!
					#push(@ { $missing_prots_byfile{$FILES[$f]} },  $all_prots[$j]);
					$mem = 0;
				}
				elsif (exists ${$prot_groups{$FILES[$f]}}{$all_prots[$j]} ) {
					$founds[$j]++;
				}
				
		} # next file
			
		if($founds[$j] >= scalar(@FILES)*$MinSamplePct) { # match
			
			push(@next_group, $all_prots[$j]);
				#print $founds[$j]."\n";
		}
	#	} # if not already allocated to group
    } # next protein
    push(@grp_members, \@next_group);
}
# header


for(my $k = 0; $k < $index; $k++) {
    for(my $j = 0; $j <= $#all_prots; $j++) {
		my $f;
		my $mem = 1;
		if($founds[$j] >= scalar(@FILES)*$MinSamplePct) {
			for($f = 0; $f <= $#FILES; $f++) {
				if ( !exists (${$prot_groups{$FILES[$f]}}{$all_prots[$j]}) ) {
						if (! exists($missing_prots_byfile{$FILES[$f]}) ){
							@ { $missing_prots_byfile{$FILES[$f]} } = ();
						}
						#TODO: Implement this SMARTLY !!!
						push(@ { $missing_prots_byfile{$FILES[$f]} },  $all_prots[$j]);
						$mem = 0;
				}
				elsif (${$inds[$k]}[$f] != exists ${$prot_groups{$FILES[$f]}}{$all_prots[$j]} ) { # not a member
					if (! exists($missing_prots_byfile{$FILES[$f]}) ){
						@ { $missing_prots_byfile{$FILES[$f]} } = ();
					}
					#TODO: Implement this SMARTLY !!!
					push(@ { $missing_prots_byfile{$FILES[$f]} },  $all_prots[$j]);
					$mem = 0;
				}				
			}
		 }
	 }
}


#exit(1);$index
my @files;
my %shareByFileAndProt;
my %distProtAndProt;
my %distFileAndFile;
my %protFileCount;
my @fileClusters;
my @fileOrder;
my @protClusters;
my @protOrder;
	
for(my $k = 0; $k < $index; $k++) {
    #print "k: $k (vs $index)\n";
    @files = @{getFiles($inds[$k], \@FILES)};
    my $flat_file = join('_', @files);
    $flat_file =~ s/\//-/g;
	
	if ($outfile eq "") {
		$outfile = "COMPARE_".$flat_file ;
    }
# output
    open(OUT, ">$outfile") or die "cannot open $outfile $!\n";
    
    my $get_rat = 0;
    print OUT "file_combo\tprotein";
    for(my $kk = 0; $kk < scalar(@files); $kk++) {
	for(my $i = 0; $i < scalar(@info); $i++) {
	    print OUT "\t$info[$i]";
	    if($info[$i] eq "percent share of spectrum ids") {
		$get_rat = 1;
	    }
	}
    }
  
	if (scalar(@files)>=2 && $get_rat == 1 && $k == $index-1) {
		
########### D3 OUTPUT START ######################
	if ($D3==1)  { 
my $TPPhostname = tpplib_perl::get_tpp_hostname();
	
my $d3outfile = $outfile.".d3.html";
print "D3 Output File: $d3outfile \n";
open(D3OUT, ">$d3outfile") or die "cannot open $outfile $!\n";;

print D3OUT ' 

<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
    <title>Heatmap</title>
    <script type="text/javascript" src="';

print D3OUT '/'.$JS_HOME;

print D3OUT $D3_HEADER;

	print D3OUT "var data = [";
}
########### D3 OUTPUT ######################

	print OUT "\tratio of max group spectrum share pct";
    }
    print OUT "\n";
	
	
	for(my $ff = 0; $ff < scalar(@files); $ff++) {	
		for(my $jj = 0; $jj < scalar(@{$missing_prots_byfile{$files[$ff]}}); $jj++) {	
			if (!(${$missing_prots_byfile{$files[$ff]}}[$jj]=~ /DECOY/)) {
				my $printfile = substr($files[$ff], rindex($files[$ff], "/")+1);
				#print D3OUT '{"file": "' . $printfile . '", "protein": "' .  ${$missing_prots_byfile{$files[$ff]}}[$jj] . '", "psm_id_share": 0, "probability": 0, "group_probability": 0}, ';
			}
			$protFileCount{${$missing_prots_byfile{$files[$ff]}}[$jj]} = 1;
			${$shareByFileAndProt{$files[$ff]}}{${$missing_prots_byfile{$files[$ff]}}[$jj]} = 0;
		}	
	}
	
    #print "proteins found in ", getFileCombo($inds[$k], \@ARGV), " (total: ", scalar @{$grp_members[$k]}, ")\n";
    for(my $j = 0; $j < scalar(@{$grp_members[$k]}); $j++) {
		print OUT "$flat_file\t${$grp_members[$k]}[$j]";
		for(my $f = 0; $f < scalar(@files); $f++) {
			if (exists(${$prots{$files[$f]}}{${$grp_members[$k]}[$j]})) {
				# continue;
			
				my @parsed = split("\t", ${$prots{$files[$f]}}{${$grp_members[$k]}[$j]});
				for(my $i = 0; $i < scalar(@info); $i++) {
					print OUT "\t";
					if(${$info{$info[$i]}}{$files[$f]} >= 0) {
						print OUT "$parsed[${$info{$info[$i]}}{$files[$f]}]";
					}
					else {
						print OUT "?";
					}
				} # next info
				if (scalar(@files)>2 && $D3 == 1 && $k==$index-1 && !(${$grp_members[$k]}[$j] =~ /DECOY/)) {
					${$shareByFileAndProt{$files[$f]}}{${$grp_members[$k]}[$j]} = $parsed[${$info{"percent share of spectrum ids"}}{$files[$f]}];
					my $printfile = substr($files[$f], rindex($files[$f], "/")+1);
					#print D3OUT '{"file": "' . $printfile . '", "protein": "' .  ${$grp_members[$k]}[$j] . '", "psm_id_share": ' .  $parsed[${$info{"percent share of spectrum ids"}}{$files[$f]}] .', "probability": '  .  $parsed[${$info{"protein probability"}}{$files[$f]}]  .', "group_probability": '  .  $parsed[${$info{"group probability"}}{$files[$f]}] .  '}, ';
				}
				if (!exists($protFileCount{${$grp_members[$k]}[$j]})) {
					$protFileCount{${$grp_members[$k]}[$j]} = 1;
				}			
				else {
					$protFileCount{${$grp_members[$k]}[$j]}++;
				}
			}
		} # next file
	
		
		if (scalar(@files)==2) {
			my @parsed0 = split("\t", ${$prots{$files[0]}}{${$grp_members[$k]}[$j]});
			my @parsed1 = split("\t", ${$prots{$files[1]}}{${$grp_members[$k]}[$j]});
			
			for(my $i = 0; $i < @info; $i++) {
			print OUT "\t";

			if($info[$i] eq "percent share of spectrum ids") {
				if ($parsed0[${$info{$info[$i]}}{$files[0]}] == 0 && $parsed1[${$info{$info[$i]}}{$files[1]}] == 0) {
				print OUT "?";
				}
				elsif ($parsed0[${$info{$info[$i]}}{$files[0]}] == 0) {
				print OUT "-inf";
				}
				elsif ($parsed1[${$info{$info[$i]}}{$files[1]}] == 0) {
				print OUT "inf";
				}
				else {
				if ($D3==1 && scalar(@files)==2 && $k==$index-1) {
	########### D3 OUTPUT ######################
								# print D3OUT '{"proteinDesc": "' . $parsed0[${$info{"description"}}{$files[0]}] . '", "proteinLink": "' . $parsed0[${$info{"uniprot link"}}{$files[0]}] . '", '
								 #. '"logRatio": ' . log($parsed0[${$info{$info[$i]}}{$files[0]}]/$parsed1[${$info{$info[$i]}}{$files[1]}]) . '}, ';
							}
				print OUT log($parsed0[${$info{$info[$i]}}{$files[0]}]/$parsed1[${$info{$info[$i]}}{$files[1]}]);
			
				}
			}
			
			} # next info
		}
	
		print OUT "\n";
    } # next protein in that combo
	my %protScores;
	my %usedProts;
	my $count =0;
	my $total =0;
    if (scalar(@files)>=2 && $get_rat == 1 && $D3==1 && $k==$index-1) {
	########### D3 OUTPUT ######################
            
			#print D3OUT "var protDists = [";
			$count = 0;
			for(my $kk = 0; $kk < scalar(@all_prots); $kk++) {
				if ($all_prots[$kk] =~ /DECOY/) {	
					next;
				}
				my $sum = 0;
				for(my $ff = 0; $ff < scalar(@files); $ff++) {	
					if (exists (${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]}) ) {
						$sum += ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]};
					}
				}
				#$protScores{$all_prots[$kk]} = $sum/$protFileCount{$all_prots[$kk]};
				if ($sum > 0) {
 					$protScores{$all_prots[$kk]} = $sum/scalar(@files);				
					$count ++;
				}	
				
			}
			print STDERR "Found $count proteins \n";
			
			
	for(my $ff = 0; $ff < scalar(@files); $ff++) {	
		for(my $jj = 0; $jj < scalar(@{$missing_prots_byfile{$files[$ff]}}); $jj++) {	
			if (exists($protScores{${$missing_prots_byfile{$files[$ff]}}[$jj]}) ) {
				my $printfile = substr($files[$ff], rindex($files[$ff], "/")+1);
				print D3OUT '{"file": "' . $printfile . '", "protein": "' .  ${$missing_prots_byfile{$files[$ff]}}[$jj] . '", "psm_id_share": 0, "probability": 0, "group_probability": 0}, ';
			}
		}	
	}
	
    #print "proteins found in ", getFileCombo($inds[$k], \@ARGV), " (total: ", scalar @{$grp_members[$k]}, ")\n";
	for(my $j = 0; $j < scalar(@{$grp_members[$k]}); $j++) {
		#print OUT "$flat_file\t${$grp_members[$k]}[$j]";
		for(my $f = 0; $f < scalar(@files); $f++) {
			if (exists(${$prots{$files[$f]}}{${$grp_members[$k]}[$j]})) {
				# continue;
			
				my @parsed = split("\t", ${$prots{$files[$f]}}{${$grp_members[$k]}[$j]});

				if (scalar(@files)>2 && $D3 == 1 && $k==$index-1 && exists($protScores{${$grp_members[$k]}[$j]})) {
					${$shareByFileAndProt{$files[$f]}}{${$grp_members[$k]}[$j]} = $parsed[${$info{"percent share of spectrum ids"}}{$files[$f]}];
					my $printfile = substr($files[$f], rindex($files[$f], "/")+1);
					print D3OUT '{"file": "' . $printfile . '", "protein": "' .  ${$grp_members[$k]}[$j] . '", "psm_id_share": ' .  $parsed[${$info{"percent share of spectrum ids"}}{$files[$f]}] .', "probability": '  .  $parsed[${$info{"protein probability"}}{$files[$f]}]  .', "group_probability": '  .  $parsed[${$info{"group probability"}}{$files[$f]}] .  '}, ';
				}
	
			}
		}
	}	
	# next file
			print D3OUT "];";
			my @keys = sort { $protScores{$b} <=> $protScores{$a} } keys(%protScores);
			
			@all_prots = @keys;
			$count = 0;
			for(my $k = 0; $k < scalar(@all_prots); $k++) {	
					for(my $x = 0; $x < scalar(@files); $x++) {
					
						for(my $xx = 0; $xx < scalar(@files); $xx++) {	
							my $diff = 0;
							if ( exists(${$shareByFileAndProt{$files[$x]}}{$all_prots[$k]}) && exists(${$shareByFileAndProt{$files[$xx]}}{$all_prots[$k]}) && 
								${$shareByFileAndProt{$files[$x]}}{$all_prots[$k]} >= 0 && ${$shareByFileAndProt{$files[$xx]}}{$all_prots[$k]} >=0) {
									$diff = ${$shareByFileAndProt{$files[$x]}}{$all_prots[$k]}-${$shareByFileAndProt{$files[$xx]}}{$all_prots[$k]};
							}
							elsif ( exists(${$shareByFileAndProt{$files[$x]}}{$all_prots[$k]}) && ${$shareByFileAndProt{$files[$x]}}{$all_prots[$k]} >=0) {
									$diff = ${$shareByFileAndProt{$files[$x]}}{$all_prots[$k]};
							}
							elsif ( exists(${$shareByFileAndProt{$files[$xx]}}{$all_prots[$k]}) && ${$shareByFileAndProt{$files[$xx]}}{$all_prots[$k]}) {
									$diff = ${$shareByFileAndProt{$files[$xx]}}{$all_prots[$k]};
							}
							#if ($diff < 0){
							#	$diff *= -1;
							#}
							$diff = $diff*$diff;
							
							if (exists(${$distFileAndFile{$files[$x]}}{$files[$xx]}) ) {
								${$distFileAndFile{$files[$x]}}{$files[$xx]} += $diff;	
							}
							else {
								${$distFileAndFile{$files[$x]}}{$files[$xx]} = $diff;		
							}
						}
					

					}

					
					my $total = ($#all_prots**2/2); 
					if ($all_prots[$k] =~ /DECOY/) {
						$count++;
						next;
					}
					else {
						my $tmp = $k + 1;#- $WIN;
						if ($tmp < 0) {
							$tmp = 0;
						}
						# #for(my $kk = $tmp; $kk < $k+$WIN && $kk < scalar(@all_prots); $kk++) {
						for(my $kk = $tmp; $kk < scalar(@all_prots); $kk++) {
							my $dist = 0;
							
							if ($kk ==$k || $all_prots[$kk] =~ /DECOY/) {
								$count++;
								next;
							}
							else {
							
								# for(my $ff = 0; $ff < scalar(@files); $ff++) {	
										# if ( exists(${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]}) && exists(${$shareByFileAndProt{$files[$ff]}}{$all_prots[$k]}) &&
											# ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]} >=0 && ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$k]} >= 0) {
											
											# my $diff = ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]}-${$shareByFileAndProt{$files[$ff]}}{$all_prots[$k]};
											# #if ($diff < 0){
											# #	$diff *= -1;
											# #}
											# $dist += $diff*$diff;
										# }
										# elsif (exists(${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]}) && ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]} >=0 ) {
											 # $dist += ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]}*${$shareByFileAndProt{$files[$ff]}}{$all_prots[$kk]};
										# }
										# elsif (exists(${$shareByFileAndProt{$files[$ff]}}{$all_prots[$k]}) && ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$k]} >=0 ) {
											 # $dist += ${$shareByFileAndProt{$files[$ff]}}{$all_prots[$k]}*${$shareByFileAndProt{$files[$ff]}}{$all_prots[$k]};
										# }
															
								# }
								
								getProtProtDist($all_prots[$k], $all_prots[$kk]);
								
								$count++;
								if ($count % 100000 == 0) {
									print STDERR "Processed $count / $total"
								}
								# if ( $all_prots[$kk] =~ /P35579/ && $all_prots[$k] =~ /P35527/ ) {
									# print STDERR "DDS: breakpt 1\n";
								# }
								# if ( $all_prots[$k] =~ /P35579/ && $all_prots[$kk] =~ /P35527/ ) {
									# print STDERR "DDS: breakpt 2\n";
								# }
								# if ( $all_prots[$k] =~ /P35579/ && $all_prots[$kk] =~ /B7Z596/ ) {
									# print STDERR "DDS: breakpt 3\n";
								# }
								# if ( $all_prots[$kk] =~ /P35579/ && $all_prots[$k] =~ /B7Z596/ ) {
									# print STDERR "DDS: breakpt 4\n";
								# }
						
								
								#print D3OUT '{"prot1": "' . $all_prots[$k] . '", "prot2": "' . $all_prots[$kk] . '", '
								#		 . '"dist": ' . $dist . '}, ';
							}
						}
					}
			
		
				}
					
				for(my $x = 0; $x < scalar(@files); $x++) {	
						for(my $xx = 0; $xx < scalar(@files); $xx++) {	
							${$distFileAndFile{$files[$x]}}{$files[$xx]} = sqrt(${$distFileAndFile{$files[$x]}}{$files[$xx]});		
						}
						
					}
				# for(my $k = 0; $k < scalar(@all_prots); $k++) {	
					# for(my $kk = 0; $kk < scalar(@all_prots); $kk++) {	
						# if (exists(${$distProtAndProt{$all_prots[$k]}}{$all_prots[$kk]}	) ) {
							# ${$distProtAndProt{$all_prots[$k]}}{$all_prots[$kk]} = sqrt(${$distProtAndProt{$all_prots[$k]}}{$all_prots[$kk]});
						# }
					
					# }
				# }
				
			#print D3OUT "];";	
			print D3OUT "var protOrder = [";
			#my @keys = sort { $protScores{$a} <=> $protScores{$b} } keys(%protScores);
			
			my $order = 0;
			$usedProts{$keys[0]} = 1;
			print D3OUT '{"protein": "'.$keys[0].'", "order": '. $order . ', "dist_to_prev": 0 }, ';
			push @protOrder, $keys[0];
			$count = 0;
			for (my $keys_i = 1; $keys_i < scalar(@keys); $keys_i++) {
				my $next_best = -1;
				my $best_dist = -1;
				my $tmp = $keys_i + 1 ;
				if ($tmp < 0) {
					$tmp = 0;
				}
				# #for (my $next = $tmp; $next < $keys_i+$WIN && $next < scalar(@keys); $next++) {
				for (my $next = $tmp; $next < scalar(@keys); $next++) {
					if (!exists($usedProts{$keys[$next]})) {
						if (exists( ${$distProtAndProt{$protOrder[$#protOrder]}}{$keys[$next]} ) ) {
							if ($best_dist < 0 || $best_dist > ${$distProtAndProt{$protOrder[$#protOrder]}}{$keys[$next]}) {
								$best_dist = ${$distProtAndProt{$protOrder[$#protOrder]}}{$keys[$next]};
								$next_best = $next;
							}
							
						}
						elsif (exists( ${$distProtAndProt{$keys[$next]}}{$protOrder[$#protOrder]} ) ) {
							if ($best_dist < 0 || $best_dist > ${$distProtAndProt{$keys[$next]}}{$protOrder[$#protOrder]}) {
								$best_dist = ${$distProtAndProt{$keys[$next]}}{$protOrder[$#protOrder]};
								$next_best = $next;
							}
						}
					}

				}	
				$count++;
				if ($count % 100000 == 0) {
					print STDERR "Processed $count / $total \n"
				}
				if ($next_best > 0) {
					$order++;
					push @protOrder, $keys[$next_best];
					$usedProts{$keys[$next_best]} = 1;
					print D3OUT '{"protein": "'.$keys[$next_best].'", "order": '. $order . ', "dist_to_prev": '. $best_dist . '}, ';	
					
					$tmp = $keys[$next_best];
					$keys[$next_best] = $keys[$keys_i];
					$keys[$keys_i] = $tmp;
				}
				elsif (!exists($usedProts{$keys[$keys_i]})) {
					$order++;
					push @protOrder, $keys[$keys_i];
					$usedProts{$keys[$keys_i]} = 1;
					if (exists(${$distProtAndProt{$protOrder[$#protOrder-1]}}{$protOrder[$#protOrder]}) ) {
						$best_dist = ${$distProtAndProt{$protOrder[$#protOrder-1]}}{$protOrder[$#protOrder]};
					}
					elsif (exists(${$distProtAndProt{$protOrder[$#protOrder]}}{$protOrder[$#protOrder-1]}) ) {
						$best_dist = ${$distProtAndProt{$protOrder[$#protOrder]}}{$protOrder[$#protOrder-1]};
					}
					else {
						$best_dist = getProtProtDist($protOrder[$#protOrder],$protOrder[$#protOrder-1]);
						#${$distProtAndProt{$protOrder[$#protOrder]}}{$protOrder[$#protOrder-1]} = $best_dist;
						#${$distProtAndProt{$protOrder[$#protOrder-1]}}{$protOrder[$#protOrder]} = $best_dist;
					}
					print D3OUT '{"protein": "'.$keys[$keys_i].'", "order": '. $order . ', "dist_to_prev": '. $best_dist . '}, ';	
				}
			}
			
			print STDERR "Ordered " . scalar(keys %usedProts) . " proteins \n";	

		
			
			print D3OUT "];";	
			
			if ($protCluster) {
				computeProteinClusters();
			}
			
			print D3OUT "var fileOrder = [";
	
		
			my %usedFiles;
			$order = 0;
			my $printfile = substr($files[0], rindex($files[0], "/")+1);
			print D3OUT '{"file": "'.$printfile.'", "order": '. $order . ', "dist_to_prev": 0}, ';
			push @fileOrder, $files[0];
			
			$usedFiles{$files[0]} = 1;
			$count = 0;
			for (my $files_i = 0; $files_i < scalar(@files); $files_i++) {
				my $next_best = -1;
				my $best_dist = -1;
				my $tmp =  0;
				for (my $next = $tmp;  $next < scalar(@files); $next++) {
					if (!exists($usedFiles{$files[$next]})) {
							if (exists( ${$distFileAndFile{$fileOrder[$#fileOrder]}}{$files[$next]} ) ) {
								if ($best_dist < 0 || $best_dist > ${$distFileAndFile{$fileOrder[$#fileOrder]}}{$files[$next]}) {
									$best_dist = ${$distFileAndFile{$fileOrder[$#fileOrder]}}{$files[$next]};
									$next_best = $next;
								}
							}
							elsif (exists( ${$distFileAndFile{$files[$next]}}{$fileOrder[$#fileOrder]} ) ) {
								if ($best_dist < 0 || $best_dist > ${$distFileAndFile{$files[$next]}}{$fileOrder[$#fileOrder]}) {
									$best_dist = ${$distFileAndFile{$files[$next]}}{$fileOrder[$#fileOrder]};
									$next_best = $next;
								}
							}
						
					}	
				}	
				$count++;
				if ($count % 100 == 0) {
					print STDERR "Processed $count / $total \n"
				}
				if ($next_best > 0) {
					$order++;
					push @fileOrder, $files[$next_best];
					$usedFiles{$files[$next_best]} = 1;
					$printfile = substr($files[$next_best], rindex($files[$next_best], "/")+1);
					print D3OUT '{"file": "'.$printfile.'", "order": '. $order . ', "dist_to_prev": '. $best_dist . '}, ';	
					
					$tmp = $files[$next_best];
					$files[$next_best] = $files[$files_i];
					$files[$files_i] = $tmp;
				}
				elsif (!exists($usedFiles{$files[$files_i]}) )  {
					$order++;
					push @fileOrder, $files[$files_i];
					$usedFiles{$files[$files_i]} = 1;
					if (exists(${$distFileAndFile{$fileOrder[$#fileOrder-1]}}{$fileOrder[$#fileOrder]}) ) {
						$best_dist = ${$distFileAndFile{$fileOrder[$#fileOrder-1]}}{$fileOrder[$#fileOrder]};
					}
					elsif (exists(${$distFileAndFile{$fileOrder[$#fileOrder]}}{$fileOrder[$#fileOrder-1]}) ) {
						$best_dist = ${$distFileAndFile{$fileOrder[$#fileOrder]}}{$fileOrder[$#fileOrder-1]};
					}
					
					$printfile = substr($files[$files_i], rindex($files[$files_i], "/")+1);
					print D3OUT '{"file": "'.$printfile.'", "order": '. $order .', "dist_to_prev": '. $best_dist .  '}, ';	
				}
			}
			my $protCount = scalar(keys %usedProts);
			my $fileCount = scalar(keys %usedFiles);
			print STDERR "Ordered " . scalar(keys %usedFiles) . " Files \n";	

		
			
			print D3OUT "];";	
			
			computeFileClusters();
			
            print D3OUT '



var maxShare = d3.max(data.map(function(d) { return d.psm_id_share; } ) );
var minShare = d3.min(data.map(function(d) { if (d.probability>=0.9) return d.psm_id_share; } ) );


data.sort(function(a, b) { return  a.psm_id_share > b.psm_id_share ? 1  : a.psm_id_share < b.psm_id_share ? -1 : 0 } );

var fileMap = d3.map(function(d) { return d.file; }  );
var protMap = d3.map(function(d) { return d.protein; }  );

var protScores = d3.map(function(d) { return d.protein; }  );

var refDistMap = d3.map();

var allXY = [];

var refProtMap = d3.map();
var refFile = "'. $files[0].'";

var shareData = [];

var row = '.$protCount.';
var col = '.$fileCount.';
var tcol = 0;
var trow = 0;

var maxProtShare = 0;
var maxProtName = "";
data.forEach
		(function(d,i,a) { 
				//console.log("X="+i+",Y="+d.logRatio);
				if (fileMap.has(d.file)) {
					tcol = fileMap.get(d.file);
				}
				else {
					fileMap.set(d.file, col);
					tcol = col;
					//col += 1;
				}
				
				if (protMap.has(d.protein)) {
					trow = protMap.get(d.protein);
				
					if (protScores.get(d.protein)+d.psm_id_share > maxProtShare) {
						maxProtShare = protScores.get(d.protein)+d.psm_id_share;
						maxProtName = d.protein;
					}
						protScores.set(d.protein, protScores.get(d.protein)+d.psm_id_share);
				}
				else {
					protMap.set(d.protein, row);
					
					if (d.psm_id_share > maxProtShare) {
						maxProtShare = d.psm_id_share;
						maxProtName = d.protein;
					}
					protScores.set(d.protein, d.psm_id_share);
					trow = row;
					//row += 1;
				}
				
				if (d.file == refFile) {
					refProtMap.set(d.protein, d.psm_id_share);
				}
				shareData.push({ "file": d.file, "protein": d.protein, "share": d.psm_id_share });
				allXY.push({ "gprob": d.group_probability, "prob": d.probability, "score": d.psm_id_share, "row": trow, "col": tcol, "protein": d.protein, "file": d.file });
			} 
				  
		);
		



//allXY.map(function(d) { if (d.file == refFile) return d.protein; }  );		

//Compute Distances to each refFile

allXY.forEach
		(function(d, i, a) {
				var dist = 0;					
				
				if (! refProtMap.has(d.protein)) {
					dist = d.score ;
				}
				else {
					dist = (d.score-refProtMap.get(d.protein));
					if (dist < 0) {
						dist = dist*-1;
					}
				}
			
				if (refDistMap.has(d.file)) {
					refDistMap.set(d.file, refDistMap.get(d.file)+dist );				
				}
				else {
					refDistMap.set(d.file, dist);				
				}
			}
		);	
var protDists = [];
		


var distBYprotANDprot = d3.nest()
	.key(function(d) { return d.prot1; })
    .key(function(d) { return d.prot2; })
    .map(protDists, d3.map); 
		
//var fileOrder = [];
var f = 0;

// refDistMap.forEach 
	// (function(d, i, a) {
				// fileOrder.push( {"file": d, "order": i ,"distToRef": refDistMap.get(d) }) }
		// );	

//fileOrder.sort( function(a, b) { return  a.distToRef < b.distToRef ? -1  : a.distToRef > b.distToRef ? 1 : 0  ; }  );


fileOrder.forEach
		(function(d,i,a) { 
			refDistMap.set(d.file, i); }
		);
		

protOrder.forEach
		(function(d,i,a) { 
			refProtMap.set(d.protein, i); }
		);
		
allXY = [] ;	
fileMap = d3.map();
protMap = d3.map( );

row = 0;
col = 0;



data.sort(function(a, b) { 
								return  a.psm_id_share > b.psm_id_share ? 1  : a.psm_id_share < b.psm_id_share ? -1 : 0  ;
						} 
		)
		
	.forEach
		(function(d,i,a) { 
				//console.log("X="+i+",Y="+d.logRatio);
				if (fileMap.has(d.file)) {
					trow = fileMap.get(d.file);
				}
				else {
					refFile = d.file;
					fileMap.set(d.file, col);
					tcol = col;
					col += 1;
				}
				
				if (protMap.has(d.protein)) {
					trow = protMap.get(d.protein);
				}
				else {
					protMap.set(d.protein, row);
					trow = row;
					row += 1;
				}
				
				//if (d.file == refFile) {
				//	refProtMap.set(d.protein, d.psm_id_share);
				//}
				
				allXY.push({ "gprob": d.group_probability, "prob": d.probability, "score": d.psm_id_share, "col": refDistMap.get(d.file), "row": refProtMap.get(d.protein), "protein": d.protein, "file": d.file });
			} 
				  
		);
		
		

		
		

var colorLow = "steelblue", colorMed = "magenta", colorHigh = "red";

var colorScale = d3.scale.log()
.domain([minShare, (maxShare+minShare)/2, maxShare])
.range([colorLow, colorMed, colorHigh]);

var ttlH = 248;
var ttlW = 152;
var w = 31;
var h = 19;
var bottom_ttlH = ttlH;
var right_ttlW = 10;
var PADWID = 1200;
var PADHT = 1200;
 // Insert an svg element (with margin) for each plate in our dataset. 
var svg = d3.select("body")
	  .append("svg")
      .attr("width", col*w+ttlW+PADWID)
      .attr("height", (row+10)*h+ttlH+PADHT);
	  	  
	 
	  
//add grouping and rect 

fileOrder.forEach
		(function(d,i,a) { 

		return svg.append("text")
			.attr("x", (i+1)*w-w/2+ttlW)
			.attr("y", 0)
			.text(d.file)
			.attr("writing-mode", "tb")
			.attr("glyph-orientation-vertical", "90");
	});
	
var selectProts = [];
	
fileOrder.forEach
		(function(d,i,a) { 

		return svg.append("text")
			.attr("x", (i+1)*w-w/2+ttlW)
			.attr("y", (row+1)*h+ttlH)
			.text(d.file)
			.attr("writing-mode", "tb")
			.attr("glyph-orientation-vertical", "90");
	});
	

svg.selectAll("protName")
	.data(protOrder)	
	.enter().append("text")
	.attr("x", 0)
	.attr("y", function(d){ return (d.order+1)*h-2+ttlH ;} )
	.attr("class", "protName")
	.text(function(d){ return d.protein; })
	.append("title").text(function(d){ return d.order; });
		
svg.selectAll("checkbox")
			.data(protOrder)	
			.enter().append("rect")
			.attr("x", ttlW - 12)
			.attr("y", function(d){ return (d.order+1)*h-6+ttlH ;} )
			.attr("class", "checkbox")
			.attr("width", 5)
			.attr("height", 5)
			.attr("stroke", "black")	
			.attr("fill", function(d) { return "transparent"; } )	
			.on("click", function(d) { 	
										if (selectProts[d.order]>0) 
											selectProts[d.order] = 0;
										else
											selectProts[d.order] = 1;
											
										if (selectProts[d.order]==1) 
											d3.select(this).attr("fill", "black");
										else 
											d3.select(this).attr("fill",  "transparent");
										
									})
			.append("title").text(function(d) { return d.protein; } ); 
		
		
		';
if ($fileCluster) {
print D3OUT '		
var scale = 10;		
	fileClusters.forEach
			(function(d,i,a) { 
				var xpt = d.left*w+w/2+ttlW;
				var xlen = (d.right-d.left)*w;
				var ht = (d.dist)*scale;
				var ypt = row*h+ttlH+ht+bottom_ttlH;
				
				svg.append("rect")
							 .attr("class", "fileTree")	
							 .attr("stroke", "black")
							 .attr("fill", "black")
							 .attr("x",  xpt)							
							 .attr("y",  ypt)
							 .attr("width", xlen)						
							 .attr("height", 1);	
				svg.append("rect")
							 .attr("class", "fileTree")	
							 .attr("stroke", "black")
							 .attr("fill", "black")
							 .attr("x",  xpt)							
							 .attr("y",  ypt-ht)
							 .attr("width", 1)						
							 .attr("height", ht);	
				svg.append("rect")
							 .attr("class", "fileTree")	
							 .attr("stroke", "black")
							 .attr("fill", "black")
							 .attr("x",  xpt+xlen)							
							 .attr("y",  ypt-ht)
							 .attr("width", 1)						
							 .attr("height", ht);	
			}
			);
	';
}



if ($protCluster) {
print D3OUT '
	
	var scale = 50;		
	protClusters.forEach
		(function(d,i,a) { 
			var wd = (d.dist)*scale;
			var xpt = col*w+ttlW+wd+right_ttlW;
			
			var ypt = d.left*h+h/2+ttlH;
			var ylen = (d.right-d.left)*h;
		
			svg.append("rect")
						 .attr("class", "fileTree")	
						 .attr("stroke", "black")
						 .attr("fill", "black")
						 .attr("x",  xpt)							
						 .attr("y",  ypt)
						 .attr("width", 1)						
						 .attr("height", ylen);	
			svg.append("rect")
						 .attr("class", "fileTree")	
						 .attr("stroke", "black")
						 .attr("fill", "black")
						 .attr("x",  xpt-wd)							
						 .attr("y",  ypt)
						 .attr("width", wd)						
						 .attr("height", 1);	
			svg.append("rect")
						 .attr("class", "fileTree")	
						 .attr("stroke", "black")
						 .attr("fill", "black")
						 .attr("x",  xpt-wd)							
						 .attr("y",  ypt+ylen)
						 .attr("width", wd)						
						 .attr("height", 1);	
		}
		);
	';
}

print D3OUT '	
svg.selectAll("data")
	.data(allXY)
	.enter().append("rect")
	.attr("class", "data")	
    .attr("y", function(d){return d.row*h+ttlH})
    .attr("x", function(d){return d.col*w+ttlW})
    .attr("width", w)
    .attr("height", h)
    .attr("stroke", function(d) { return "white"; } )
    .attr("fill", function(d) {if (d.score > 0) return colorScale(d.score);  return "#eeeeee";} ) 
	.on("click", function(d) { 				
				var prot = d.protein;
				if (prot.indexOf("|") != -1) { 
					prot = d.protein.substr(prot.indexOf("|")+1);
				}
				if (prot.indexOf("|") != -1) { 
					prot = prot.substr(0, prot.indexOf("|"));
				}
				return window.open("http://www.uniprot.org/uniprot/"+prot); 
			} )
	.append("title").text(function(d) { return d.file+",\nProtein = "+d.protein+",\n% share of spectrum Ids = "+d.score+",\nProtein Prob = "+d.prob+",\Protein Group Prob = "+d.gprob;} ) ;
    //.attr("transform", function(d) { //console.log("translate(" + x(d.x) + "," + y(d.y) + ")");
    //                                return "translate(" + x(d.col) + "," + y(d.row) + ")"; });

d3.select("svg").append("rect")
	.attr("class", "go")
		.attr("x", 0)
		.attr("y", (protOrder.length+1)*h+ttlH+5)
		.attr("width", 120)
		.attr("height", 2.5*h)
		.attr("stroke", "green")	
		.attr("fill", "lightgreen")	
		.on("click", function() { return linkUniprot(); })
		.append("title").text("Uniprot Search");
	

d3.select("svg").append("text")
		 .attr("class", "go")
			 .attr("x", 30)
			 .attr("y", (protOrder.length+1)*h+ttlH+20)
			 .attr("width", 120)
			 .attr("height", 4*h)
			 .on("click", function() { return linkUniprot(); })
			 .attr("stroke", "steelblue")	
			 .attr("fill", "white")		
			 .text("Search Uniprot");
			
			

function linkUniprot() {
	var prots = [];
	protOrder.forEach
		(function(d,i,a) { 
			if (selectProts[i] == 1) {
				var prot = d.protein;
				
				if (prot.indexOf("|") != -1) { 
					prot = d.protein.substr(prot.indexOf("|")+1);
				}
				if (prot.indexOf("|") != -1) { 
					prot = prot.substr(0, prot.indexOf("|"));
				}
			
				
				if (prot.indexOf("-") != -1) {
					prots.push(prot);
				}
				else {
					prots.push("accession:"+prot);
				}
			}
		});
	var link = "http://www.uniprot.org/uniprot/?sort=score&query=";
	prots.forEach
		(function(d,i,a) { 
				link += d;
				if (i < prots.length-1) 
					link += "+OR+";
			
		});
	return window.open(link);
	

}
	
  </script>
  </body>
</html>


';


close(D3OUT);
########### D3 OUTPUT END ######################
}
	elsif (scalar(@files)>2 && $get_rat == 1 && $D3==1 && $k==$index-1) {
		print D3OUT "];";
	}	
	close(OUT);
}


print STDERR " comparison results written to file $outfile\n\n";

exit(0);

# take in tab delimited files for 2 protein prophet outputs, and compile 3 lists:
# output1, output2, and both

my $first = $ARGV[0];
my $second = $ARGV[1];
my %first_prots = %{extractProts($first)};
my %second_prots = %{extractProts($second)};

my @firsts = ();
my @seconds = ();
my @boths = ();

foreach(keys %first_prots) {
    if(exists $second_prots{$_}) {
	push(@boths, $_);
    }
    else {
	push(@firsts, $_);
    }
}
foreach(keys %second_prots) {
    if(! exists $first_prots{$_}) {
	push(@seconds, $_);
    }
}

printf "only in $first: (%d)\n", scalar @firsts;
foreach(@firsts) {
    print "$_\n";
}
print "\n\n";
printf "only in $second: (%d)\n", scalar @seconds;
foreach(@seconds) {
    print "$_\n";
}
print "\n\n";
printf "in both $first and $second: (%d)\n", scalar @boths;
foreach(@boths) {
    print "$_\n";
}
print "\n\n\n";

sub getFileCombo {
(my $inds, my $fileptr) = @_;
my $output = '';
die "have ", scalar @{$inds}, " inds and ", scalar @{$fileptr}, " files\n" if(@{$inds} != @{$fileptr});
for(my $k = 0; $k < @{$inds}; $k++) {
    #print "${$inds}[$k] for ${$fileptr}[$k]...\n";
    if(${$inds}[$k]) {
	$output .= ' ' if(! ($output eq ''));
	$output .= ${$fileptr}[$k];
    }
} # next index
return $output;
}

sub getFiles {
(my $inds, my $fileptr) = @_;
my @output = ();
die "have ", scalar @{$inds}, " inds and ", scalar @{$fileptr}, " files\n" if(@{$inds} != @{$fileptr});
for(my $k = 0; $k < @{$inds}; $k++) {
    #print "${$inds}[$k] for ${$fileptr}[$k]...\n";
    if(${$inds}[$k]) {
	push(@output, ${$fileptr}[$k]);
    }
} # next index
return \@output;
}


sub increment {
(my $init, my $index) = @_;
my $factor;
my $check = 1;
for(my $k = 0; $k < @{$init}; $k++) {
    $check *= 2;
}
return 0 if($index > $check - 1);

for(my $k = 0; $k < @{$init}; $k++) {
    $factor = 1;
    $factor = 1;
    for(my $j = 0; $j < $k; $j++) {
	$factor *= 2;
    }
    if($index % $factor == 0) { # switch
	if(${$init}[$k] == 1) {
	    ${$init}[$k] = 0;
	}
	else {
	    ${$init}[$k] = 1;
	}
    }
} # next dataset
return 1;
}

sub extractProtGroups {
(my $file) = @_;
my %output = ();
open(FILE, $file) or die "cannot open $file $! \n";
# get annotation line for protein name
my @parsed;
my $prot_index = -1;
my $entry_index = -1;
my $protProb_index = -1;
my $protGrpProb_index = -1;
my $first = 1;

while(<FILE>) {
    chomp();
    @parsed = split("\t");
    if($first) {
	if(! /^\s*[a-z,A-Z,\',\"]/ || ! /[a-z,A-Z,\',\"]\s*$/) {
	    print STDERR " Warning: Make sure $file is a tab delimited file, NOT a real Excel file\n\n";
	    exit(1);
	}
	
	for(my $k = 0; $k < @parsed; $k++) {
	    if ($parsed[$k] eq 'entry no.') {
		$entry_index = $k;
	    }
	    if ($parsed[$k] eq 'protein probability') {
		$protProb_index = $k;
	    }
	    if ($parsed[$k] eq 'group probability') {
		$protGrpProb_index = $k;
	    }
	    if($parsed[$k] eq 'protein') {
		$prot_index = $k;
		#$k = @parsed;
	    }
	    else {
		for(my $i = 0; $i < @info; $i++) {
		    if($parsed[$k] eq $info[$i]) {
			#print "info $info[$i] for $file: $k\n";
				${$info{$info[$i]}}{$file} = $k;
		    }
		}
	    }
	}
	$first = 0;
	if($prot_index == -1) {
	    die "could not find column for protein name in $file\n";
	}
    }
    elsif ($parsed[$protProb_index]>$minProb) {
	my @arr = split "\t", $_;
	my @grp = split /[a-z]/, $arr[$entry_index];
	my @protarr = split /,/, $parsed[$prot_index];
		for (my $i=0; $i <= $#protarr; $i++) {
		#$output{$parsed[$prot_index]} = $grp[0]; # store it
			$output{$protarr[$i]} = $grp[0]; # store it
		}
	#print "adding $parsed[$prot_index]...\n";
	#$all_prots{$parsed[$prot_index]}++;
    }
	
} # next entry
close(FILE);
return \%output;
}


sub extractProts {
(my $file) = @_;
my %output = ();
open(FILE, $file) or die "cannot open $file $! \n";
# get annotation line for protein name
my @parsed;
my $prot_index = -1;
my $entry_index = -1;
my $protProb_index = -1;
my $protGrpProb_index = -1;
my $first = 1;

while(<FILE>) {
    chomp();
    @parsed = split("\t");
    if($first) {
	if(! /^\s*[a-z,A-Z,\',\"]/ || ! /[a-z,A-Z,\',\"]\s*$/) {
	    print STDERR " Warning: Make sure $file is a tab delimited file, NOT a real Excel file\n\n";
	    exit(1);
	}
	
	for(my $k = 0; $k < @parsed; $k++) {
	    if ($parsed[$k] eq 'entry no.') {
		$entry_index = $k;
	    }
	    if ($parsed[$k] eq 'protein probability') {
		$protProb_index = $k;
	    }
	    if ($parsed[$k] eq 'group probability') {
		$protGrpProb_index = $k;
	    }
	    if($parsed[$k] eq 'protein') {
		$prot_index = $k;
		#$k = @parsed;
	    }
	    else {
		for(my $i = 0; $i < @info; $i++) {
		    if($parsed[$k] eq $info[$i]) {
			#print "info $info[$i] for $file: $k\n";
			${$info{$info[$i]}}{$file} = $k;
		    }
		}
	    }
	}
	$first = 0;
	if($prot_index == -1) {
	    die "could not find column for protein name in $file\n";
	}
    }
    elsif ($parsed[$protProb_index]>$minProb) {
	my @arr = split "\t", $_;
	my @grp = split /[a-z]/, $arr[$entry_index];
	my @protarr = split /,/, $parsed[$prot_index];
	for (my $i=0; $i <= $#protarr; $i++) {
			#$output{$parsed[$prot_index]} = $_; # store it
			$output{$protarr[$i]} = $_; 
	#print "adding $parsed[$prot_index]...\n";
			#$all_prots{$parsed[$prot_index]}++;
			$all_prots{$protarr[$i]}++;
		}
	}
	
} # next entry
close(FILE);
return \%output;
}

sub getProtProtDist {
	my $prot1 = shift;
	my $prot2 = shift;
	my $dist = 0;
	if (exists(${$distProtAndProt{$prot1}}{$prot2}) ) {
		$dist = ${$distProtAndProt{$prot1}}{$prot2};
	}
	elsif (exists(${$distProtAndProt{$prot2}}{$prot1}) ) {
		$dist = ${$distProtAndProt{$prot2}}{$prot1};
	}
	else {
						
					
	
					
					
		for(my $ff = 0; $ff < scalar(@files); $ff++) {	
				if ( exists(${$shareByFileAndProt{$files[$ff]}}{$prot1}) && exists(${$shareByFileAndProt{$files[$ff]}}{$prot2}) &&
					${$shareByFileAndProt{$files[$ff]}}{$prot1} >=0 && ${$shareByFileAndProt{$files[$ff]}}{$prot2} >= 0) {
					
					my $diff = ${$shareByFileAndProt{$files[$ff]}}{$prot1}-${$shareByFileAndProt{$files[$ff]}}{$prot2};
					#if ($diff < 0){
					#	$diff *= -1;
					#}
					$diff = $diff*$diff;
					$dist += $diff;
				}
				elsif (exists(${$shareByFileAndProt{$files[$ff]}}{$prot1}) && ${$shareByFileAndProt{$files[$ff]}}{$prot1} >=0 ) {
					 $dist += ${$shareByFileAndProt{$files[$ff]}}{$prot1}*${$shareByFileAndProt{$files[$ff]}}{$prot1};
				}
				elsif (exists(${$shareByFileAndProt{$files[$ff]}}{$prot2}) && ${$shareByFileAndProt{$files[$ff]}}{$prot2} >=0 ) {
					 $dist += ${$shareByFileAndProt{$files[$ff]}}{$prot2}*${$shareByFileAndProt{$files[$ff]}}{$prot2};
				}
									
		}
		
			#${$distProtAndProt{$prot2}}{$prot1} = sqrt($dist);
			${$distProtAndProt{$prot1}}{$prot2} = sqrt($dist);
	
	}
	
	return $dist;
}

sub computeFileClusters {

	for(my $ff = 0; $ff < scalar(@files); $ff++) {	
		push(@fileClusters, [$ff, $ff, 0]);
	
	}
	my @new_fileClusters;
	my @tmp_fileClusters;
	print D3OUT "var fileClusters = [";	
	while (scalar(@fileClusters) > 2) {
		print STDERR "fileClusters: ".$#fileClusters."\n";
		@new_fileClusters = ();
		@tmp_fileClusters = ();
		my $dist =0;
		for(my $ff = 0; $ff < scalar(@fileClusters); $ff++) {
				my $left = ${$fileClusters[$ff]}[0];
				my $right = ${$fileClusters[$ff]}[1];
				my $leftFWD = -1;
				my $rightFWD = -1;
				my $leftBAK = -1;
				my $rightBAK = -1;
				
				if ( $ff+1 < scalar(@fileClusters) ) {
					$leftFWD = ${$fileClusters[$ff+1]}[0];
					$rightFWD = ${$fileClusters[$ff+1]}[1];	
				}
				if ( $ff-1 > 0  ) {
					$leftBAK = ${$fileClusters[$ff-1]}[0];
					$rightBAK = ${$fileClusters[$ff-1]}[1];	
				}
				
				if ($leftBAK >= 0 && $rightFWD >= 0) {
					#pick closest other cluster FWD or BAK to add to cluster
					my $ht_left = getFileClusterHeight($leftBAK, $right);
					my $ht_right = getFileClusterHeight($left, $rightFWD);
					
					if ( $ht_left < $ht_right ) {
						#left is closer
						
						push(@new_fileClusters, [$leftBAK, $right, $ht_left]);
					}
					else {
						push(@new_fileClusters, [$left, $rightFWD, $ht_right]);
					}
				
				
				}
				elsif ($leftBAK >= 0 ) {
					my $ht_left = getFileClusterHeight($leftBAK, $right);
					#left is only option 
					push(@new_fileClusters, [$leftBAK, $right, $ht_left]);
				}	
				elsif ($rightFWD >= 0 ) {
					my $ht_right = getFileClusterHeight($left, $rightFWD);				
					push(@new_fileClusters, [$left, $rightFWD, $ht_right]);
				}
				
				elsif ($left-1 >=0 && $right+1 < scalar(@files) ) {
					#pick closest other cluster FWD or BAK to add to cluster
					my $ht_left = getFileClusterHeight($left-1, $right);
					my $ht_right = getFileClusterHeight($left, $right+1);
					if ( $ht_left < $ht_right ) {
						#left is closer
						
						push(@new_fileClusters, [$left-1, $right, $ht_left]);
					}
					else {
						push(@new_fileClusters, [$left, $right+1, $ht_right]);
					}
				}	
				elsif ($left-1 >= 0 ) {
					my $ht_left = getFileClusterHeight($left-1, $right);
					#left is only option 
					push(@new_fileClusters, [$left-1, $right, $ht_left]);
				}
				elsif ($right+1 < scalar(@files) ) {
					my $ht_right = getFileClusterHeight($left, $right+1);				
					push(@new_fileClusters, [$left, $right+1, $ht_right]);
				}
		
		}
		
		#resolve overlaps
		for( my $ff = 0; $ff < scalar(@new_fileClusters); $ff++) {	
			my $left = ${$new_fileClusters[$ff]}[0];
			my $right = ${$new_fileClusters[$ff]}[1];
			my $dist = ${$new_fileClusters[$ff]}[2];

			if ($ff+1 < scalar(@new_fileClusters)) {
				my $left1 = ${$new_fileClusters[$ff+1]}[0];
				my $right1 = ${$new_fileClusters[$ff+1]}[1];
				my $dist1 = ${$new_fileClusters[$ff+1]}[2];
				if (${$new_fileClusters[$ff]}[1] >= ${$new_fileClusters[$ff+1]}[0]) {
					#overlapping cluster
					#pick one that's closer that's tighter
					if ($dist1 > $dist) {
						#keep left cluster, adjust right one
								
						$left1 = $right+1;
						if ($left1 > $#fileOrder) {
							splice @new_fileClusters,$ff+1, 1;
							$ff--;
						}
						else {
							#adjust main cluster
							${$new_fileClusters[$ff]}[1] = $right;
							${$new_fileClusters[$ff]}[2] = getFileClusterHeight($left, $right);
								
							if ($left1 > $right1) {	
								#cluster ff+1 collapses
								
								splice @new_fileClusters,$ff+1, 1;
								$ff--;
							}
							else {
								#cluster ff+1 still live, adjust it
								
								${$new_fileClusters[$ff+1]}[0] = $left1;
								${$new_fileClusters[$ff+1]}[1] = $right1;
								${$new_fileClusters[$ff+1]}[2] = getFileClusterHeight($left1, $right1);
							}
						}
					}
					else {
						#keep right adjust, adjust left  one
							
						${$new_fileClusters[$ff+1]}[0] = $left1;
						${$new_fileClusters[$ff+1]}[1] = $right1;
						${$new_fileClusters[$ff+1]}[2] = getFileClusterHeight($left1, $right1);		
						
						$right = $left1 - 1;	
						
						if ($right < 0 || $right < $left) {
							splice @new_fileClusters,$ff, 1;
							$ff--;
						}
						else {
							#cluster ff still there
							${$new_fileClusters[$ff]}[0] = $left;
							${$new_fileClusters[$ff]}[1] = $right;
							${$new_fileClusters[$ff]}[2] = getFileClusterHeight($left, $right);
							
						}													
					}
					
				
				}
				else {
					
				
				}
			}
		
		}
		
		@fileClusters = @new_fileClusters;
		for( my $ff = 0; $ff < scalar(@fileClusters); $ff++) {	
			print D3OUT '{ "left":' . ${$fileClusters[$ff]}[0] . ', "right":' . ${$fileClusters[$ff]}[1] . ', "dist":' . ${$fileClusters[$ff]}[2] . ' }, ';	
		
		}
		
		
	}
	
	
	# Final cluster goes here
	my $dist = getFileClusterHeight(0, $#files);
	print D3OUT '{ "left":' . 0 . ', "right":' . $#files . ', "dist":' . $dist . ' }, ';	
	
	print D3OUT "];";	
	


}

sub getFileClusterHeight() {
	my $left = shift;
	my $right = shift;
	if ($left ==$right) {
		return 0;
	}
	my $dist =  ${$distFileAndFile{$fileOrder[$left]}}{$fileOrder[$right]} ;
								
	for (my $x = $left+1; $x<= $right; $x++) {
		for (my $xx = $x+1; $xx<= $right; $xx++) {
				if ($dist < ${$distFileAndFile{$fileOrder[$x]}}{$fileOrder[$xx]} ) {
						$dist = ${$distFileAndFile{$fileOrder[$x]}}{$fileOrder[$xx]};
					}	
								
		}
							
	}
	return $dist;

}


sub computeProteinClusters {

	for(my $ff = 0; $ff < scalar(@all_prots); $ff++) {	
		push(@protClusters, [$ff, $ff, 0]);
	
	}
	my @new_protClusters;
	my @tmp_protClusters;
	print D3OUT "var protClusters = [";	
	while (scalar(@protClusters) > 2) {
		print STDERR "protClusters: ".$#protClusters."\n";
		@new_protClusters = ();
		@tmp_protClusters = ();
		my $dist =0;
		for(my $ff = 0; $ff < scalar(@protClusters); $ff++) {
				my $left = ${$protClusters[$ff]}[0];
				my $right = ${$protClusters[$ff]}[1];
				my $leftFWD = -1;
				my $rightFWD = -1;
				my $leftBAK = -1;
				my $rightBAK = -1;
				
				if ( $ff+1 < scalar(@protClusters) ) {
					$leftFWD = ${$protClusters[$ff+1]}[0];
					$rightFWD = ${$protClusters[$ff+1]}[1];	
				}
				if ( $ff-1 > 0  ) {
					$leftBAK = ${$protClusters[$ff-1]}[0];
					$rightBAK = ${$protClusters[$ff-1]}[1];	
				}
				
				if ($leftBAK >= 0 && $rightFWD >= 0) {
					if ($rightFWD > $#protOrder || $rightFWD < 0) {
						print STDERR "DEBUG here\n";
					}
					#pick closest other cluster FWD or BAK to add to cluster
					my $ht_left = getProtClusterHeight($leftBAK, $right);
					my $ht_right = getProtClusterHeight($left, $rightFWD);
					if ( $ht_left < $ht_right ) {
						#left is closer
						
						push(@new_protClusters, [$leftBAK, $right, $ht_left]);
					}
					else {
						push(@new_protClusters, [$left, $rightFWD, $ht_right]);
					}
				
				
				}
				elsif ($leftBAK >= 0 ) {
					my $ht_left = getProtClusterHeight($leftBAK, $right);
					#left is only option 
					push(@new_protClusters, [$leftBAK, $right, $ht_left]);
				}	
				elsif ($rightFWD >= 0 ) {
					if ($rightFWD > $#protOrder) {
						print STDERR "DEBUG here\n";
					}
					my $ht_right = getProtClusterHeight($left, $rightFWD);				
					push(@new_protClusters, [$left, $rightFWD, $ht_right]);
				}
				
				elsif ($left-1 >=0 && $right+1 < scalar(@all_prots) ) {
					#pick closest other cluster FWD or BAK to add to cluster
					my $ht_left = getProtClusterHeight($left-1, $right);
					my $ht_right = getProtClusterHeight($left, $right+1);
					if ( $ht_left < $ht_right ) {
						#left is closer
				
						push(@new_protClusters, [$left-1, $right, $ht_left]);
					}
					else {
						if ($right+1 > $#protOrder) {
							print STDERR "DEBUG here\n";
						}
						push(@new_protClusters, [$left, $right+1, $ht_right]);
					}
				}	
				elsif ($left-1 >= 0 ) {
					my $ht_left = getProtClusterHeight($left-1, $right);
					#left is only option Proc
					push(@new_protClusters, [$left-1, $right, $ht_left]);
				}
				elsif ($right+1 < scalar(@all_prots) ) {
					my $ht_right = getProtClusterHeight($left, $right+1);		
					if ($right+1 > $#protOrder) {
						print STDERR "DEBUG here\n";
					}					
					push(@new_protClusters, [$left, $right+1, $ht_right]);
				}
				
					
		}
		
		#resolve overlaps
		for( my $ff = 0; $ff < scalar(@new_protClusters); $ff++) {	
			my $left = ${$new_protClusters[$ff]}[0];
			my $right = ${$new_protClusters[$ff]}[1];
			my $dist = ${$new_protClusters[$ff]}[2];
			if ($ff+1 < scalar(@new_protClusters)) {
				my $left1 = ${$new_protClusters[$ff+1]}[0];
				my $right1 = ${$new_protClusters[$ff+1]}[1];
				my $dist1 = ${$new_protClusters[$ff+1]}[2];
			
				if (${$new_protClusters[$ff]}[1] >= ${$new_protClusters[$ff+1]}[0]) {
					#overlapping cluster
					#pick one that's closer that's tighter
					if ($dist1 > $dist) {
						#keep left cluster, adjust right one
								
						$left1 = $right+1;
						if ($left1 > $#protOrder) {
								splice @new_protClusters,$ff+1, 1;
								$ff--;
						}
						else {
							#adjust main cluster
							${$new_protClusters[$ff]}[1] = $right;
							${$new_protClusters[$ff]}[2] = getProtClusterHeight($left, $right);

							
							if ($left1 > $right1) {	
								#cluster ff+1 collapses
								
								splice @new_protClusters,$ff+1, 1;
								$ff--;
							}
							else {
								#cluster ff+1 still live, adjust it
								#if ($left1 == 12) {
								#	print STDERR "DEBUG here\n";
								#}
								${$new_protClusters[$ff+1]}[0] = $left1;
								${$new_protClusters[$ff+1]}[1] = $right1;
								${$new_protClusters[$ff+1]}[2] = getProtClusterHeight($left1, $right1);
							}
						}
					}
					else {
						#keep right adjust, adjust left  one
				
					
						${$new_protClusters[$ff+1]}[0] = $left1;
						${$new_protClusters[$ff+1]}[1] = $right1;
						${$new_protClusters[$ff+1]}[2] = getProtClusterHeight($left1, $right1);		
						$right = $left1 - 1;							
						if ($right < 0 || $right < $left) {
							splice @new_protClusters,$ff, 1;
							$ff--;
						}
						else {
							#cluster ff still there

							${$new_protClusters[$ff]}[0] = $left;
							${$new_protClusters[$ff]}[1] = $right;
							${$new_protClusters[$ff]}[2] = getProtClusterHeight($left, $right);
							
						}													
					}
					
				
				}
				else {
					
				
				}
			}
		
		}
		
		@protClusters = @new_protClusters;
		for( my $ff = 0; $ff < scalar(@protClusters); $ff++) {	
			print D3OUT '{ "left":' . ${$protClusters[$ff]}[0] . ', "right":' . ${$protClusters[$ff]}[1] . ', "dist":' . ${$protClusters[$ff]}[2] . ' }, ';	
		
		}
		
		
	}
	
	
	# Final cluster goes here
	my $dist = getProtClusterHeight(0, $#all_prots);
	print D3OUT '{ "left":' . 0 . ', "right":' . $#all_prots . ', "dist":' . $dist . ' }, ';	
	
	print D3OUT "];";	
	


}

sub getProtClusterHeight() {
	my $left = shift;
	my $right = shift;
	if ($left ==$right) {
		return 0;
	}
	my $dist =  0;

	$dist = getProtProtDist($protOrder[$left], $protOrder[$right])	;
		
		
								
	for (my $x = $left+1; $x<= $right; $x++) {
		for (my $xx = $x+1; $xx<= $right; $xx++) {
					my $tmp = getProtProtDist($protOrder[$x], $protOrder[$xx]);
					if ($dist < $tmp ) {
							$dist = $tmp;
					}
				}					
								
		}
							
	
	return $dist;

}
