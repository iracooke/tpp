#!/usr/bin/perl

use strict;


if(@ARGV == 0) {
    print STDERR " usage: compareProts.pl <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison to tab delim file 'compare.xls'\n\n";
    print STDERR " or to rename output file: \n";
    print STDERR "        compareProts.pl -Nalternative.xls <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison to user specified excel file 'alternative.xls'\n\n";
    print STDERR " or to specify input columns to be reported (along with protein names): \n";
    print STDERR "        compareProts.pl -Nalternative.xls -h'protein probability' -h'ASAPRatio mean' <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison with protein probability and ASAPRatio mean columns to user specified tab delim file 'alternative.xls'\n\n";
    print STDERR " or to specify NO columns to be reported (along with protein names): \n";
    print STDERR "        compareProts.pl -Nalternative.xls -h <tab delim file1><tab delim file2> ...\n";

    print STDERR "\n\n";
    exit(1);
}

my @FILES = (); #@ARGV;
my $outfile = 'compare.xls';

my @info = ();

print "here: ", join('|', @ARGV), "\n";

# check for outfile
for(my $k = 0; $k < @ARGV; $k++) {
    if($ARGV[$k] =~ /^\-N(\S+)$/) {
	$outfile = $1;
    }
    elsif($ARGV[$k] =~ /^\-h\'(.*)\'$/) {
	push(@info, $1);
    }
    elsif($ARGV[$k] =~ /^\-h\'(.*)$/) {
	my $info = '';
	while($k < @ARGV && $ARGV[$k] !~ /\'$/) {
	    $info .= $ARGV[$k++];
	}
	if($k < @ARGV && $ARGV[$k] =~ /(.*)\'$/) {
	    $info .= $1;
	    push(@info, $info);
#	    print "adding $info....\n";
	}
	else {
	    print "error: no end found for $info...\n";
	    exit(1);
	}
	#push(@info, $1);
    }
    elsif($ARGV[$k] =~ /^\-h(.*)$/) {
	push(@info, $1);
    }
    else {
	push(@FILES, $ARGV[$k]);
    }
}


die "no input excel files specified\n" if(@FILES == 0);

#my @prots = ();
my @inds_ref = ();
my @inds = (0);
my %all_prots = ();

my %prot_ind_groups = (); # one for each input file, pter to hash by protein, array of other group members
my %prot_grp_indeces = (); # for each protein, index of subset group


#@info = ('group_no', 'prot_prob', 'ASAPRatio mean') if(@info == 0); # default
@info = ('entry no.', 'protein probability', 'ratio mean') if(@info == 0); # default
@info = () if(@info == 1 && ! $info[0]);

my %info = ();
foreach(@info) {
    my %next = ();
    for(my $f = 0; $f < @FILES; $f++) {
	$next{$FILES[$f]} = -1;
    }
    $info{$_} = \%next;
}

my %prots = ();

for(my $f = 0; $f < @FILES; $f++) {

    my %next = ();
    $prot_ind_groups{$FILES[$f]} = \%next;

#foreach(@FILES) {
#    print "$_....\n";
    #print STDERR "$ARGV[$f]...\n";
    print STDERR " $FILES[$f]...\n";
    $prots{$FILES[$f]} = extractProts($FILES[$f]);
    #$prots{$ARGV[$f]} = extractProts($ARGV[$f]);
    #push(@prots, extractProts($_));
    push(@inds_ref, 0);
} # next file

#exit(1);
# now look at by subset
#print "total: \n";
#foreach(keys %prots) { print "$_ "; } print "\n"; exit(1);


#foreach(@inds_ref) {
#    print "$_ ";
#}
#print "\n";
my $index = 1;
while(increment(\@inds_ref, $index++)) {
    my @next = ();
    foreach(@inds_ref) {
	push(@next, $_);
    }
    push(@inds, \@next);

#    foreach(@inds_ref) {
#	print "$_ ";
#    }
#    print "\n";

}
$index--;
#print "final index: $index\n"; exit(1);
my @all_prots = keys %all_prots;


# now group each entry according to where it falls
my @grp_members = (0); # null for first group (all zeros)
my %founds = ();
for(my $k = 1; $k < $index; $k++) {
    my @next_group = ();
    for(my $j = 0; $j < @all_prots; $j++) {
	my $f;
	if(! exists $founds{$j}) {
	    my @included_files = ();
	    for($f = 0; $f < @FILES; $f++) {
		if(${$inds[$k]}[$f] != exists ${$prots{$FILES[$f]}}{$all_prots[$j]}) { # not a member
		    $f = @FILES + 1; # no match
		}
		push(@included_files, $f) if(${$inds[$k]}[$f]);
	    } # next file
	    if($f == @FILES) { # match

		push(@next_group, $all_prots[$j]);
		$prot_grp_indeces{$all_prots[$j]} = $k; # set the subset group
		$founds{$j}++;
	    }
	} # if not already allocated to group
    } # next protein
    push(@grp_members, \@next_group);
}
# header

# now go through each grp_members list and combine subsets for each
for(my $k = 1; $k < @grp_members; $k++) {
    for(my $p = 0; $p < @{$grp_members[$k]}; $p++) {
	# go through each relevant file looking for common sibs
	my $protein = ${$grp_members[$k]}[$p];
	my @files = @{getFiles($inds[$k], \@FILES)};
	#my @fileinds = @{getFileInds($inds[$k], \@FILES)};
	my @sibs = keys %{${$prot_ind_groups{$files[0]}}{$protein}};
	if(@sibs > 0) {
	    my @merge = (); 
	    for(my $s = 0; $s < @sibs; $s++) {
		my $verbose = $protein eq 'IPI00219997' || $sibs[$s] eq 'IPI00219997';
		my $merge = included($sibs[$s], \@{$grp_members[$k]}); # make sure member of current group
		#print "prot $protein with sib $sibs[$s], merge: $merge with ", join(' ', @files), "\n" if($verbose);
		#my $merge = 1; # until proven otherwise
		for(my $f = 0; $f < @files; $f++) {
		#for(my $f = 0; $f < @fileinds; $f++) {
		    #$merge = 0 if($merge && ! included($sibs[$s], \@{$grp_members[$k]})); # make sure member of current group

                    #! exists ${$prot_ind_groups{$files[$f]}}{$sibs[$s]});
		    $merge = 0 if($merge && ! exists ${${$prot_ind_groups{$files[$f]}}{$protein}}{$sibs[$s]});
#		    $merge = 0 if($merge && ! exists ${${$prot_ind_groups{$files[$fileinds[$f]]}}{$protein}}{$sibs[$s]});
		    $f = @files if (! $merge); # done
		    
		}
		#print "prot $protein with sib $sibs[$s], merge: $merge with ", join(' ', @files), "\n" if($verbose);
		push(@merge, $sibs[$s]) if($merge);
		#print "length of @merge: ", scalar @merge, " with ", join(' ', @merge), "\n" if($verbose);
	    } # next sibling

	    if(@merge > 0) { # if have something to merge with starting protein

		# must undef each member of merge for each relevant file for this protein
		# must replace this entry of protein list with newmerge
		# must remove all sibs from entry list (at a greater k than current, necessarily)
		my $newprotein = $protein;
		for(my $m = 0; $m < @merge; $m++) {
		    $newprotein .= ',' . $merge[$m];
		}
		#print "adding new protein: $newprotein for $protein in files: ", join(',', @files), "\n";
		#print "list: ", join(' ', @{$grp_members[$k]}), "\n\n";
		for(my $f = 0; $f < @files; $f++) {

		    for(my $m = 0; $m < @merge; $m++) {
			if(0 && exists ${${$prot_ind_groups{$files[$f]}}{$protein}}{$merge[$m]}) {

			    delete ${${$prot_ind_groups{$files[$f]}}{$protein}}{$merge[$m]};
			    die "error, defined!\n" if(exists ${${$prot_ind_groups{$files[$f]}}{$protein}}{$merge[$m]});
			}

			delete ${${$prot_ind_groups{$files[$f]}}{$protein}}{$merge[$m]} if(exists ${${$prot_ind_groups{$files[$f]}}{$protein}}{$merge[$m]});
		    }
		    ${$prot_ind_groups{$files[$f]}}{$newprotein} = ${$prot_ind_groups{$files[$f]}}{$protein}; # make the switch
		} # next file in group
		${$grp_members[$k]}[$p] = $newprotein; # set it here
		$prot_grp_indeces{$newprotein} = $prot_grp_indeces{$protein}; # set subset group
		# remove merges from list
		for(my $pp = $p+1; $pp < @{$grp_members[$k]}; $pp++) {

		    for(my $m = 0; $m < @merge; $m++) {
#print "${$grp_members[$k]}[$pp] vs $merge[$m]\n";
			if(${$grp_members[$k]}[$pp] eq $merge[$m]) {
			    @{$grp_members[$k]} = @{$grp_members[$k]}[0 .. $pp-1, $pp+1 .. $#{$grp_members[$k]}]; # remove this entry
			    $pp--; # adjust
			    @merge = @merge[0 .. $m-1, $m+1 .. $#merge]; # remove this entry
			    $m = @merge; # done
			    $pp = @{$grp_members[$k]} if(@merge == 0); # nothing more to look for
			}
		    } # next merge
		} # next prot on list
		die "could not find entries for ", join(',', @merge), " with regard to $protein and ", join(',', @files), "\nentries considered: ", join(' ', @{$grp_members[$k]}[$p+1 .. $#{$grp_members[$k]}]), "\nentire list: ", join(' ', @{$grp_members[$k]}), "\n" if(@merge > 0);
		    

	    } # if do merge

	} # if have siblings


    } # next protein on list
} # next subset list



# output
open(OUT, ">$outfile") or die "cannot open $outfile $!\n";


print OUT "file_combo\tprotein";
for(my $k = 0; $k < @FILES; $k++) {
    for(my $i = 0; $i < @info; $i++) {
	print OUT "\t$info[$i]";
    }
}
print OUT "\n";

#exit(1);


for(my $k = 1; $k < $index; $k++) {
    #print "k: $k (vs $index)\n";
    my @files = @{getFiles($inds[$k], \@FILES)};
    #my $flat_file = join(',', @files);

    #print "proteins found in ", getFileCombo($inds[$k], \@ARGV), " (total: ", scalar @{$grp_members[$k]}, ")\n";
    for(my $j = 0; $j < @{$grp_members[$k]}; $j++) {
	(my $flat_file, my $supersib) = getFlatFile(\@files, ${$grp_members[$k]}[$j]);
	if($supersib) {
	    print OUT "$flat_file\t[${$grp_members[$k]}[$j]]";
	}
	else {
	    print OUT "$flat_file\t${$grp_members[$k]}[$j]";
	}
	for(my $f = 0; $f < @files; $f++) {
	    print OUT "\t";
	    if(exists ${$prots{$files[$f]}}{${$grp_members[$k]}[$j]}) {


		my @parsed = split("\t", ${$prots{$files[$f]}}{${$grp_members[$k]}[$j]});
		for(my $i = 0; $i < @info; $i++) {
		    if(${$info{$info[$i]}}{$files[$f]} >= 0) {
			print OUT "$parsed[${$info{$info[$i]}}{$files[$f]}]";
		    }
		    else {
			print OUT "?";
		    }
		    # print tab here????
		    print OUT "\t" if($i < @info - 1);
		} # next info
	    } # if exists
	    else {
		my @sub_prots = split(',', ${$grp_members[$k]}[$j]);
		if(exists ${$prots{$files[$f]}}{$sub_prots[0]}) {
		    my @parsed = split("\t", ${$prots{$files[$f]}}{$sub_prots[0]});
		    for(my $i = 0; $i < @info; $i++) {
			if(${$info{$info[$i]}}{$files[$f]} >= 0) {
			    print OUT "$parsed[${$info{$info[$i]}}{$files[$f]}]";
			}
			else {
			    print OUT "?";
			}
			# print tab here????
			print OUT "\t" if($i < @info - 1);
		    }
		}
		else {
		    for(my $i = 0; $i < @info; $i++) {
			print OUT "?";
			print OUT "\t" if($i < @info - 1);
		    }


		}

	    }
	} # next file
	print OUT "\n";
    } # next protein in that combo
}

close(OUT);
print STDERR " comparison results written to file $outfile\n\n";

exit(1);

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

# puts asterisks next to each file with nonzero number of sibling entries remaining (as warning)
# returns second argument true if prot has a sib which is in a superset subgroup w/ respect to it
sub getFlatFile {
(my $fileptr, my $prot) = @_;
my $output = '';
my $first = 1;
my $supersib = 0; # until proven
foreach(@{$fileptr}) {
    $output .= ',' if(! ($output eq '')); # not the first time
    $output .= $_;
    #$output .= '*' if(scalar keys %{${$prot_ind_groups{$_}}{$prot}} > 0);
    if(scalar keys %{${$prot_ind_groups{$_}}{$prot}} > 0) {
	$output .= '*';
	if($first) {
	    # check for supersets
	    my @sibs = keys %{${$prot_ind_groups{$_}}{$prot}};
	    for(my $s = 0; $s < @sibs; $s++) {
		if(inSubsetGroup($prot_grp_indeces{$prot}, $prot_grp_indeces{$sibs[$s]})) {
		    $supersib = 1;
		    $s = @sibs; # done
		}
	    }
	    $first = 0;
	}
    } # if have sibs
}
return ($output, $supersib);
}



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

sub included {
(my $entry, my $listptr) = @_;
foreach(@{$listptr}) {
    return 1 if($entry eq $_);
}
return 0;
}

sub inSubsetGroup {
(my $ind1, my $ind2) = @_;
return 0 if($ind1 == $ind2);
for(my $k = 1; $k < $index; $k++) {
    for(my $f = 0; $f < @FILES; $f++) {
	return 0 if(${$inds[$ind1]}[$f] && ! ${$inds[$ind2]}[$f]); 
    }
}
# still here
return 1;
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

sub getFileInds {
(my $inds, my $fileptr) = @_;
my @output = ();
die "have ", scalar @{$inds}, " inds and ", scalar @{$fileptr}, " files\n" if(@{$inds} != @{$fileptr});
for(my $k = 0; $k < @{$inds}; $k++) {
    #print "${$inds}[$k] for ${$fileptr}[$k]...\n";
    if(${$inds}[$k]) {
	push(@output, $k);
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

sub extractProts {
(my $file) = @_;
my %output = ();
open(FILE, $file) or die "cannot open $file $! \n";
# get annotation line for protein name
my @parsed;
my $prot_index = -1;
my $first = 1;

while(<FILE>) {
    chomp();
    @parsed = split("\t");
    if($first) {
	if(! /^\s*[a-z,A-Z,\',\",0-9]\s*/ || ! /\s*[a-z,A-Z,\',\",0-9]\s*$/) {
	    print STDERR " Warning: Make sure $file is a tab delimited file, NOT a real Excel file\n\n";
	    exit(1);
	}
	
	for(my $k = 0; $k < @parsed; $k++) {
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
    else {
	# split apart and make references to siblings in file
	my @prots = split(',', $parsed[$prot_index]);
	for(my $p = 0; $p < @prots; $p++) {
	    my %next = ();
	    for(my $n = 0; $n < @prots; $n++) {
		$next{$prots[$n]}++ if($n != $p); # no self
	    }
	    ${$prot_ind_groups{$file}}{$prots[$p]} = \%next; # original indistinguishable entry sibs
	    $output{$prots[$p]} = $_; # store it
	    $all_prots{$prots[$p]}++;
	}

	#$output{$parsed[$prot_index]} = $_; # store it
	#print "adding $parsed[$prot_index]...\n";
	#$all_prots{$parsed[$prot_index]}++;
    }
	
} # next entry
close(FILE);
return \%output;
}
