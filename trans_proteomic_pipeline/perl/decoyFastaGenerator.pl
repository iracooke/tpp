#!/usr/bin/perl -w 
#############################################################################
# Program       : decoyFastaGenerator.pl                                    #
# Author        : David Shteynberg                                          #
# Date          : 1.28.2012                                                 #
#                                                                           #
#                                                                           #
#                                                                           #
#                                                                           #
# Copyright (C) 2013 David Shteynberg                                       # 
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


my $usage = "
Usage: $0 <fasta input file> <decoy prefix> <fasta output file>\n";

die $usage unless @ARGV >= 3;

my $fasta_in_file = shift;
my $prefix = shift;
my $fasta_out_file = shift;

open(FASTAIN, "<$fasta_in_file") ||
    die "Couldn't read file $fasta_in_file: $!\n";

open(FASTAOUT, ">$fasta_out_file") ||
    die "Couldn't write file $fasta_out_file: $!\n";



# two global hash tables that store the peptide and its shuffled
# sequence(array format)
my %pep1_shuffle_seq=();
my %pep2_shuffle_seq=();
my $def = "";
my $seq = "";
my $zer = 0;
my $dec_index = 0;
my $verbose = 0;
my $next_def = "";
my $next_seq = "";

while(<FASTAIN>) {

    if ($_ =~ /^(>.*)$/) { #definition
	$def = $next_def;
	$seq = $next_seq;
	if ($next_def ne "" && $def ne ""  && $seq ne "" ) {
	    randomize_protein($def, $seq, $prefix);	
	}
        $next_def = $1;
	$next_seq = "";
	
    }
    elsif ($_ =~ /^(.*)$/) {
	$next_seq .= $1
    }
    

}
randomize_protein($next_def, $next_seq, $prefix);	


sub randomize_protein {
    my $def = shift;
    my $pseq = shift;
    my $prefix = shift;
    
    $dec_index++;

    print FASTAOUT $def . "\n";
    print FASTAOUT $pseq . "\n";

    $def =~ s/^>(\w+).*/>$prefix$zer\_$1\_$dec_index Randomized Protein Sequence/;
    $zer = $zer ? 0 : 1;
    print FASTAOUT $def . "\n";
    my @seq = split(//, $pseq);
    my @pep = ();
    my $peplen = 0;
    for (my $i=0; $i<scalar(@seq); $i++) {
	if ($i==0 && $seq[$i] =~ /[mM]/) {
	    print FASTAOUT $seq[$i];
	}
	elsif ($seq[$i] =~ /[krKR\*]/ && $i < scalar(@seq)-1 && $seq[$i+1] !~ /[pP]/ && $i < $#seq ) {
	    dump_peptide(\@pep);
	    @pep = ();
	    print FASTAOUT $seq[$i];
	}
	elsif ($seq[$i] =~ /[krKR\*]/ &&  $i < scalar(@seq)-1 && $seq[$i+1] =~ /[pP]/ && $i < $#seq ) {
	    push(@pep, $seq[$i]);
	}
	elsif ($seq[$i] =~ /[pP]/ && $i > 0 && $seq[$i-1] =~ /[krKR]/) {
	    push(@pep, $seq[$i]);
	}
	else {
	    push(@pep, $seq[$i]);
	}
    }
    dump_peptide(\@pep);
    print FASTAOUT "\n";
}

sub dump_peptide {
    my $pepref = shift;
    my @pep = @{$pepref};
    if (scalar(@pep)>1) {
        my @initial_peptide  = @pep;
	
	my $string_pep = join('',@initial_peptide);	
	if( ($zer && exists $pep1_shuffle_seq{$string_pep} ) || (!$zer && exists $pep2_shuffle_seq{$string_pep})  )
	{
	    my $peptide_sequence = "";

	    if ($zer) {
		$peptide_sequence = $pep1_shuffle_seq{$string_pep};
	    }
	    else {
		$peptide_sequence = $pep2_shuffle_seq{$string_pep};
	    }


	    @pep = split(//,$peptide_sequence);
	}
	else
	{
	    my $iterations = 0;
	    my $reject = 0;
	    while ($reject || join('',@initial_peptide) eq join('',@pep)) {
		$reject = 0;
		randomize_input(\@pep);
		$iterations++;
		
		#### If new peptide begins with P, reject it
		if ($pep[0] eq 'P' && $iterations<10) {
		    $reject = 1;
		    if ($verbose) {
			print STDERR "[$iterations] Ooops, peptide starts with P:".
			join('',@pep).".  (was ".join('',@initial_peptide).") Try again...\n";
		    }
		    next;
		}
		
		if (join('',@initial_peptide) eq join('',@pep) && $iterations>9) {
		    if ($verbose) {
			print STDERR "Shuffled ended up the same: ".
			    join('',@initial_peptide)."==".join('',@pep)."\n";
		    }
		    last;
		}
	    }
	    # store shuffled peptide in hash table
	    if ($zer) {
		$pep1_shuffle_seq{$string_pep}=join('',@pep);
	    }
	    else {
		$pep2_shuffle_seq{$string_pep}=join('',@pep);
	    }
	}
    }
    for (my $j=0; $j<scalar(@pep); $j++) {
	print FASTAOUT $pep[$j];		
    }
    @pep = ();


}

sub randomize_input {
   #randomize the array 
   #(see Shuffle Perl Cookbook CH.4)
    my $array = shift;
    my $i;
    for ($i = @$array; --$i; ) {
        my $j = int rand ($i+1);
        next if $i == $j;
	
	my $aa1 = $$array[$i];

	my $aa2 = $$array[$j];

	if ($$array[$i] =~ /kKrR/ && $i < @$array && $$array[$i+1] =~ /pP/) {
	    while ($j == @$array || $j == $i) {
		$j = int rand ($i+1);
	    } 
	     @$array[$i,$j] = @$array[$j,$i];

	     @$array[$i+1,$j+1] = @$array[$j+1,$i+1];
	    next;
	    
	}

	if ($$array[$j] =~ /kKrR/ && $j < @$array && $$array[$j+1] =~ /pP/) {
	    next;
	}

	if ($$array[$i] =~ /pP/ && $i > 0 && $$array[$i-1] =~ /kKrR/) {
	    while ($j == 0 || $j == $i) {
		$j = int rand ($i+1);
	    } 
	    @$array[$i,$j] = @$array[$j,$i];
	    @$array[$i-1,$j-1] = @$array[$j-1,$i-1];
	    next;
	}
	
        @$array[$i,$j] = @$array[$j,$i];
    }
    
}
    
