#!/usr/bin/perl -w
use strict;
use Getopt::Std;

my %options;

# Process inputs and options
getopts('s:O:', \%options);

my $mrmFile = shift || &usage("Transition List File");

open(MRM, "<$mrmFile") ||   die "Could not open file: $mrmFile.\n";
open(HEADER, "<header.txt") ||  die "Could not open file: header.txt.\n";

my $header  = <HEADER>;
my @linearray = ();
my $count = 0;

# get the first key (peptide) to compare
my $oneline = <MRM>;
chomp($oneline);
my @linearr = split(/\s+/, $oneline);
my $modpep = $linearr[2];
my $charge = $linearr[11];
my $key = $modpep . "/$charge";
my $nextkey;
push (@linearray, $oneline);
$count = 1;

open(OUTFILE, ">$options{O}") ||   die "Could not open file: $options{O}.\n" unless (!$options{O});
if ($options{O}){
  print OUTFILE $header;
}
else {
  print $header;
}

while(my $line = <MRM>) {
  chomp($line);
  @linearr = split(/\s+/, $line);

  $modpep = $linearr[2];
  $charge = $linearr[11];
  $nextkey = $modpep . "/$charge";
  if ($nextkey eq $key){
    push(@linearray, $line);
    $count++;
  }
  else {
    if (!$options{s} || $count >= $options{s}){
      for my $a (@linearray){
	if ($options{O}){
	  print OUTFILE "$a\n";
	} else {
          print "$a\n";
	}
      }
    }

    $key = $nextkey;
    @linearray = ();
    push(@linearray, $line);
    $count = 1;
  }
}

# do it one more time for last peptide
if (!$options{s} || $count >= $options{s}){
  for my $a (@linearray){
    if ($options{O}){
      print OUTFILE "$a\n";
    } else {
      print "$a\n";
    }
  }
}

close(OUTFILE) unless (!$options{O});

sub usage {
    my $bad_param = shift || '';
    print "ERROR: Missing input parameter: $bad_param\n\n" if $bad_param;
    print << "EOU";
Usage: $0 [options] <mrm file>

Set the number of minimum transition in MaRiMba.

Options:
  -s <num>              Minimum number of transitions per peptide (required)
  -O <filename>		Write output to file <file_name> instead of STDOUT
EOU
    exit(1);
}
