#!/usr/bin/perl

# read *.charge file produced by createChargeFile.pl and update
# precursorCharge in original mzXML file. If more than one charge is pre# dicted for one spectrum, the possibleCharges field will hold the exta # charge information.
# ./mergeCharges.pl -c 022008_F10_ETD_2.charge -i 022008_F10_ETD_2.mzXML

use strict;
use Getopt::Long;

$Getopt::Long::autoabbrev = 0;
$Getopt::Long::ignorecase = 0;
my %options;
GetOptions(\%options,'help|h', 'inputChargeFile|c=s', 'inputmzXMLFile|i=s', 
'outputmzXMLFile|o=s','outputChargeFile|oc=s' );

my $chargefile = $options{'inputChargeFile'};
my $mzxmlfile = $options{'inputmzXMLFile'};
my $output;
my $logfile;

if( $options{'help'})
{
  printUsage();
}

if(! $options{'inputmzXMLFile'} || !$options{'inputChargeFile'})
{
  printUsage();
}

if(! $options{'outputmzXMLFile'})
{
  $output = $mzxmlfile;
  $output =~ s/\.mzXML/\_newz\.mzXML/;
}
else
{
  $output=$options{'outputmzXMLFile'};
}

if(! $options{'outputChargeFile'})
{
  $logfile = $mzxmlfile;
  $logfile =~ s/\.mzXML/\.chargechanges/;
}
else
{
  $logfile=$options{'outputChargeFile'};
}

my %charges=();
open( CHARGE, "<$chargefile") or die "cannot open file $chargefile\n";
foreach (<CHARGE>)
{
  next if(/#/);
  chomp;
  my ($scannum, $charge)=split(/\s+/, $_);
  $charges{$scannum}=$charge;
}

open(IN, "<$mzxmlfile") or die "cannot open $mzxmlfile\n";
open(OUT, "> $output") or die "cannot open $output\n";
open (LOG, ">$logfile") or die "cannot open $logfile\n";
print LOG "spectrum_id\toldcharge\tnewcharges\n";

my ($msLevel, $headerinfo, $scaninfo, %scan);
my (%totalSpectra,$SpectraHaveChargePre,$SpectraHaveChargeAfter, $predictDiffCh);
my ($SpectraNoCh, $useExisting, $predictSameCh, $predictMoreCh);
$SpectraNoCh = 0;
$SpectraHaveChargePre = 0;
$SpectraHaveChargeAfter = 0;
$predictDiffCh = 0;
$SpectraNoCh = 0;
$useExisting = 0;
$predictSameCh = 0;
$predictMoreCh = 0;

my $count=0;
my $header=1;
my $printed=0;
my $scannum=0;
my $prescan=0;
my $lastMsLevel=0;
while (<IN>)
{
  if(/<scan\s+num="(\d+)"/)
  {
	$header=0;
    $scannum=$1;
	if(!$printed)
	{
	  print OUT "$headerinfo";
	  $printed=1;
	}
	$scaninfo .= $_;
  }

  if($header)
  {
    $headerinfo .= $_;
  }
  next if(/<scan/);
  next if ($header);

  $scaninfo .= $_;
  if(/msLevel="(\d+)"/)
  {
    $msLevel=$1;
	$totalSpectra{$msLevel}++;
	$lastMsLevel=$msLevel;
  }
  if(/<\/scan/)
  {
    if($prescan eq $scannum){ next;}
	if($msLevel >1)
	{ 
	  if(defined $charges{$scannum})
	  {
	    my $c=$charges{$scannum};
	    if($scaninfo =~ /precursorCharge="(\d+)"/)
	    {
		  $SpectraHaveChargePre++;
		  if($1 ne $c)
		  {
		    print LOG "$scannum\t$1\t$charges{$scannum}\n";
			if($c =~ /$1/)
			{
			  $predictMoreCh++;
			}
			else
			{
			  $predictDiffCh++;
			}
		  }
		  else
		  {
		    $predictSameCh++;
		  }
          my @charges= split(",", $c);
		  if(scalar @charges ==1)
		  {
		    $scaninfo =~ s/precursorCharge="\d+"/precursorCharge="$c"/;
		  }
		  else
		  {
			my $charges = join(",", @charges);
			$scaninfo =~ s/precursorCharge="\d+"/possibleCharges="$charges"/;
		  }
	    }
		else
		{
		   my @charges= split(",", $c);
		   if(scalar @charges ==1)
		   {
		     $scaninfo =~ s/precursorIntensity="(.*)"\s+/precursorIntensity="$1" precursorCharge=\"$c\"/;
	       }
		   else
		   {
		    my $charges = join(",", @charges);
			$scaninfo =~ s/precursorIntensity="(.*)"\s+/precursorIntensity="$1" possibleCharges="$charges"/;
		   }
		  print LOG "$scannum\t\/\t$charges{$scannum}\n";
		}
		$SpectraHaveChargeAfter++;
	  }
	  else
	  {
	    if($scaninfo =~ /precursorCharge="(\d+)"/)
		{
		  $SpectraHaveChargePre++;
		  $SpectraHaveChargeAfter++;
		  $useExisting++;
	    }
		else
		{
		  $SpectraNoCh++;
		}
		print "no predicted charge info for scan $scannum\n";
	  }
    }
	print OUT "$scaninfo";
	$scaninfo='';
	$prescan=$scannum;
  }
}
if($lastMsLevel == 2)
{
  print OUT "</scan>\n";
}
print OUT "</msRun>\n<index></index>\n<indexOffset></indexOffset>\n</mzXML>\n";
close IN;
close OUT;
`indexmzXML.exe $output`;
`move $output.new $output`;

print "$totalSpectra{2}\t Total MS2 spectra\n";
print "$SpectraHaveChargePre\t spectra already had a charge\n";
print "$predictSameCh\t spectra with new and old charge the same\n";
print "$predictMoreCh\t spectra with new charge includes old charge\n";
print "$predictDiffCh\t spectra with a different new charge\n";
print "$useExisting\t spectra without new charge information (left existing charge)\n";
print "$SpectraNoCh\t have no charge information\n";
print "$SpectraHaveChargeAfter\t spectra now have charge information\n";

exit;
#*********************************************************************
sub printUsage {
  print( <<"  END" );

  Usage:  mergeCharges.pl [ -i -o -c -h -oc ]
  required:
	-i, --inputmzXMLFile
    -c, --inputChargeFile
  optional:
	-h, --help          Print this usage information and exit
	-o, --outputmzXMLFile
    -oc, --outputChargeFile

  END
																								
  exit;
}

