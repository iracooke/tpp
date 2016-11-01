#!/usr/bin/perl 

##
## Version 3.0   2005.02.28
## Copyright (c) 2005 John Wilkins
## Sequence Specific Retention Calculator
## Authors: Oleg Krokhin, Vic Spicer, John Cortens

=license

Use of this software governed by the Artistic license, as reproduced here:

The Artistic License for all X! software, binaries and documentation

Preamble
The intent of this document is to state the conditions under which a
Package may be copied, such that the Copyright Holder maintains some 
semblance of artistic control over the development of the package, 
while giving the users of the package the right to use and distribute 
the Package in a more-or-less customary fashion, plus the right to 
make reasonable modifications. 

Definitions
"Package" refers to the collection of files distributed by the Copyright 
	Holder, and derivatives of that collection of files created through 
	textual modification. 

"Standard Version" refers to such a Package if it has not been modified, 
	or has been modified in accordance with the wishes of the Copyright 
	Holder as specified below. 

"Copyright Holder" is whoever is named in the copyright or copyrights 
	for the package. 

"You" is you, if you're thinking about copying or distributing this Package. 

"Reasonable copying fee" is whatever you can justify on the basis of 
	media cost, duplication charges, time of people involved, and so on. 
	(You will not be required to justify it to the Copyright Holder, but 
	only to the computing community at large as a market that must bear 
	the fee.) 

"Freely Available" means that no fee is charged for the item itself, 
	though there may be fees involved in handling the item. It also means 
	that recipients of the item may redistribute it under the same
	conditions they received it. 

1. You may make and give away verbatim copies of the source form of the 
Standard Version of this Package without restriction, provided that 
you duplicate all of the original copyright notices and associated 
disclaimers. 

2. You may apply bug fixes, portability fixes and other modifications 
derived from the Public Domain or from the Copyright Holder. A 
Package modified in such a way shall still be considered the Standard 
Version. 

3. You may otherwise modify your copy of this Package in any way, provided 
that you insert a prominent notice in each changed file stating how and 
when you changed that file, and provided that you do at least ONE of the 
following: 

a.	place your modifications in the Public Domain or otherwise make them 
	Freely Available, such as by posting said modifications to Usenet 
	or an equivalent medium, or placing the modifications on a major 
	archive site such as uunet.uu.net, or by allowing the Copyright Holder 
	to include your modifications in the Standard Version of the Package. 
b.	use the modified Package only within your corporation or organization. 
c.	rename any non-standard executables so the names do not conflict 
	with standard executables, which must also be provided, and provide 
	a separate manual page for each non-standard executable that clearly 
	documents how it differs from the Standard Version. 
d.	make other distribution arrangements with the Copyright Holder. 

4. You may distribute the programs of this Package in object code or 
executable form, provided that you do at least ONE of the following: 

a.	distribute a Standard Version of the executables and library files, 
	together with instructions (in the manual page or equivalent) on 
	where to get the Standard Version. 
b.	accompany the distribution with the machine-readable source of the 
	Package with your modifications. 
c.	give non-standard executables non-standard names, and clearly 
	document the differences in manual pages (or equivalent), together 
	with instructions on where to get the Standard Version. 
d.	make other distribution arrangements with the Copyright Holder. 

5. You may charge a reasonable copying fee for any distribution of 
this Package. You may charge any fee you choose for support of 
this Package. You may not charge a fee for this Package itself. 
However, you may distribute this Package in aggregate with other 
(possibly commercial) programs as part of a larger (possibly 
commercial) software distribution provided that you do not a
dvertise this Package as a product of your own. You may embed this 
Package's interpreter within an executable of yours (by linking); 
this shall be construed as a mere form of aggregation, provided that 
the complete Standard Version of the interpreter is so embedded. 

6. The scripts and library files supplied as input to or produced as 
output from the programs of this Package do not automatically fall 
under the copyright of this Package, but belong to whomever generated 
them, and may be sold commercially, and may be aggregated with this 
Package. If such scripts or library files are aggregated with this 
Package via the so-called "undump" or "unexec" methods of producing 
a binary executable image, then distribution of such an image shall 
neither be construed as a distribution of this Package nor shall it 
fall under the restrictions of Paragraphs 3 and 4, provided that you 
do not represent such an executable image as a Standard Version of 
this Package. 

7. C subroutines (or comparably compiled subroutines in other languages) 
supplied by you and linked into this Package in order to emulate 
subroutines and variables of the language defined by this Package 
shall not be considered part of this Package, but are the equivalent 
of input as in Paragraph 6, provided these subroutines do not change 
the language in any way that would cause it to fail the regression 
tests for the language. 

8. Aggregation of this Package with a commercial distribution is always 
permitted provided that the use of this Package is embedded; that is, 
when no overt attempt is made to make this Package's interfaces visible 
to the end user of the commercial distribution. Such use shall not be 
construed as a distribution of this Package. 

9. The name of the Copyright Holder may not be used to endorse or promote 
products derived from this software without specific prior written permission. 

10. THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED 
WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF 
MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. 

The End
 
=cut

use CGI qw(:all);
my $cgi = CGI->new();


# ============================================================
# initialization

my $seqs = $cgi->param("seqs");
my $corA = $cgi->param("corA");
my $corB = $cgi->param("corB");
my $sVer = $cgi->param("sver");

#### Also allow command line usage
use Getopt::Long;
our %OPTIONS;
GetOptions(\%OPTIONS,"help","sequences:s","A:f","B:f","source_file:s",
	   "algorithm_version:s","output_format:s",
);
if ($OPTIONS{help}) {
  print usage();
  exit;
}
$seqs = $OPTIONS{sequences} unless ($seqs);
$corA = $OPTIONS{A} unless ($corA);
$corB = $OPTIONS{B} unless ($corB);
$sVer = $OPTIONS{algorithm_version} unless ($sVer);

#### Determine desired output format
my $output_format = $OPTIONS{output_format} || 'html';
$output_format = lc($output_format);
$output_format = 'html' unless ($output_format =~ /tsv|html/);

#### Read information from source file if available
unless ($seqs) {
  my $infile = $OPTIONS{source_file};
  if ($infile) {
    if (-e $infile) {
      if (open(INFILE,$infile)) {
	while (my $line = <INFILE>) {
	  next if ($line =~ /^[\#\>]/);
	  chomp($line);
	  if ($line =~ /^\s*([A-Z]+)\s*$/) {
	    $seqs .= "$1\n";
	  } else {
	    print STDERR "WARNING: Unable to parse '$line' from file\n";
	  }
	}
      } else {
	print STDERR "ERROR: Unable to open '$infile'\n";
      }
    } else {
      print STDERR "ERROR: File '$infile' is not found\n";
    }
  }
}


$badAA = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

&writeHTMLheader() if ($output_format eq 'html');

if ($sVer=~/3\.0/) {
  &initializeGlobals3(); &ReadParmFile3();}
elsif ($sVer=~/2\.0/) {
  &initializeGlobals2(); &ReadParmFile2();}
else {
  &initializeGlobals1(); &ReadParmFile1();}

$seqs=~tr/a-z/A-Z/; # all uppercase
$seqs=~s/\n/\//g;   # convert newline to "/" character
$seqs=~s/\s//g;     # remove all other white space.

my $corab = 0;

if ((defined $corA) && (defined $corB) &&
    ($corA=~/^-?(\d+\.?\d*)|(\.\d+)$/) &&
    ($corB=~/^-?(\d+\.?\d*)|(\.\d+)$/) )   {$corab=1;}

my $anyBadAAs = "nope";

# ============================================================
# loop until done
@Seqs=split(/\//,$seqs); # peptides delimited by "/" characters
foreach $seq (@Seqs)
{
   $seq=~s/[^A-Z]//g;     # peel non-alphabetic characters
   my $sqLnX = length $seq;
   $seq=~s/[$badAA]//g;      # peel letters which cannot be read as a.a.s

   my $sqOut = "";
   my $sqLen = length $seq; 
   my $i = 0;
   for ($i=0;$i<$sqLen;$i+=10)
   {
      if ($i !=0 && $i % 40 == 0)  { $sqOut .= "<br>";  }
      $sqOut .= substr $seq, $i, 10 ;
#     $sqOut .= " ";  # uncomment to split output sequence into groups of 10 aa's separated by spaces.
   }
   $sqOut .= " ($sqLen)";
   if ($sqLen<$sqLnX) {
     $sqOut .= "\*";
     $anyBadAAs = "yup";
   }
   my $hPhoby = 0.0;

   if    ($sVer=~/3\.0/)  { $hPhoby = &TSUM3($seq); }
   elsif ($sVer=~/2\.0/)  { $hPhoby = &TSUM2($seq); }
   else                   { $hPhoby = &TSUM1($seq); }

   my $hPhOut=sprintf "%10.2f",$hPhoby;

   my @line = ( $seq,$sqLen,$hPhOut );

   if ($corab==1)  {
      my $rTime = sprintf "%9.1f",($corB * $hPhoby + $corA);
      push(@line,$rTime);
   } else {
      push(@line,'n/a');
   }

   if ($output_format eq 'tsv') {
     print join("\t",@line)."\n";
   } else {
     print "<tr class=\"bodyText\"><td>$sqOut</td>\n";
     print "<td>$line[2]</td>\n";
     print "<td>$line[3]</td>\n";
     print "</tr>\n";
   }

}



if ($anyBadAAs=~m/yup/) {
  print "<tr class=\"bodyTextWhite\"><td colspan=3>";
  print "\* One or more invalid amino acid letter-code was removed from this peptide string</td></tr> \n" ; }

&writeHTMLend if ($output_format eq 'html');


# ============================================================
# ============           SUBROUTINES              ============

sub writeHTMLheader 
{
  print <<End_of_header;
Content-type: text/html

<html>
<head>
<title>Sequence Specific Retention Calculator</title>
<link href="/ssrcalc/SSRCalc.css" rel="stylesheet" type="text/css">
</head>
<body>
<table border=0 cellPadding=0 cellSpacing=0 width="700">
	<tr><td>
	<table cellPadding=3 cellSpacing=3 width="100%" border=0>
		<tr><td class="headline" valign=top>
		Sequence Specific Retention Calculator
		</td>
		<td align=right width="20%" valign=center>
		<font face="Arial, Helvetica" size=-2>
		Version 3.0.0<br>©2004<br>Manitoba Centre for Proteomics
		</font>
		</td></tr>
	</table>
	</td></tr>
	<tr><td align="center">
	<table cellPadding=5 cellSpacing=5 width="90%" border=0>
		<tr>
		<td width="60%"><a href="SSRCalcHelp.htm#PEPTIDE_SEQUENCES">Sequence</a></td>
		<td width="20%"><a href="SSRCalcHelp.htm#HYDROPHOBICITY">Relative<br>Hydrophobicity</a></td>
		<td width="20%"><a href="SSRCalcHelp.htm#RETENTION">Retention<br>Time</a></td>
		</tr>

End_of_header

}

# ============================================================

sub writeHTMLend 
{
  print <<End_of_html;
	</table>
	</td></tr>
	<tr><td>&nbsp;</td></tr>
	<tr><td colspan=2 align=left><hr width="70%" color=steelblue size=4></td></tr>
	<tr><td>
	</td></tr>
</table>
</body>
</html>
End_of_html

}
# ============================================================

sub initializeGlobals1 {

# Length Scaling lenght limits and scaling factors
	$LPLim = 20;      # long peptide lower length limit
	$SPLim = 10;      # short peptide upper length limit
	$LPSFac = 0.015;  # long peptide scaling factor
	$SPSFac = -0.027; # short peptide scaling factor

# N terminus weight correction factors
	$NTF1=.42; $NTF2=.22; $NTF3=.05; 
	
# overall correction factor
	$K=1;
}

# ============================================================

sub initializeGlobals2 {

  $SSRCVERSION=2;

# control variables, 0 means leaving them ON, 1 means turning them OFF
	$NOELECTRIC=0;
	$NOCLUSTER=0;
	$NODIGEST=1;


# Length Scaling length limits and scaling factors
	$LPLim = 20;      # long peptide lower length limit
	$SPLim = 8;       # short peptide upper length limit
	$LPSFac = 0.0245; # long peptide scaling factor
	$SPSFac = -0.05;  # short peptide scaling factor


# UnDigested (missed cuts) scaling Factors
	$UDF21=.13; $UDF22=.08;   # rightmost
	$UDF31=.05; $UDF32=.05;   # inside string

# total correction values, 20..30 / 30..40 / 40..50 /50..500
	$SUMSCALE1=.22; $SUMSCALE2=.29; $SUMSCALE3=.33; $SUMSCALE4=.39;

# clusterness scaling: i.e. weight to give cluster correction.
	$KSCALE=0.15;

# isoelectric scaling factors
	$Z01=-.030;    $Z02=0.6;    $NDELTAWT = 1.0;   # negative delta values
	$Z03=0.00;     $Z04=0.0;    $PDELTAWT = 1.0;   # positive delta values

# proline chain scores
	$PPSCORE=2.0;	$PPPSCORE=4.0;	$PPPPSCORE=6.0;
}

# ============================================================

sub initializeGlobals3 {

  $SSRCVERSION=3;

# control variables, 0 means leaving them ON, 1 means turning them OFF
	$NOELECTRIC=0;
	$NOCLUSTER=0;
	$NODIGEST=0;
	$NOSMALL=0;
	$NOHELIX1=0;
	$NOHELIX2=0;
	$NOEHEL=0;


# Length Scaling length limits and scaling factors
	$LPLim = 20;      # long peptide lower length limit
	$SPLim = 8;       # short peptide upper length limit
	$LPSFac = 0.0270; # long peptide scaling factor
	$SPSFac = -0.055;  # short peptide scaling factor


# UnDigested (missed cuts) scaling Factors
	$UDF21=0.0; $UDF22=0.0;   # rightmost
	$UDF31=1.0; $UDF32=0.0;   # inside string

# total correction values, 20..30 / 30..40 / 40..50 /50..500
	$SUMSCALE1=0.27; $SUMSCALE2=0.33; $SUMSCALE3=0.38; $SUMSCALE4=0.447;

# clusterness scaling: i.e. weight to give cluster correction.
	$KSCALE=0.4;

# isoelectric scaling factors
	$Z01=-0.03;    $Z02=0.60;    $NDELTAWT = 0.8;   # negative delta values
	$Z03= 0.00;    $Z04=0.00;    $PDELTAWT = 1.0;   # positive delta values
	 
# proline chain scores
	$PPSCORE=1.2;	$PPPSCORE=3.5;	$PPPPSCORE=5.0;

# helix scaling factors
	$HELIX1SCALE=1.6;	$HELIX2SCALE=0.255;

}

# ============================================================

sub ReadParmFile1 {
# read residue weights (hi (tel?), gi (cf?)) for formula
# from parameter file.

	$idx=0;
	$a=open(F1,"SSRCalc.par");
	while ($n=<F1>)
	{
	   ($t1,$t2,$t3)=split(" ",$n);
	   $tel{$t1}=$t2; $cf{$t1}=$t3;
	   $idx++;
           $badAA=~s/$t1//i;
	}
	close(F1);
}

# ============================================================

sub ReadParmFile2 {
  my $pSet = "";
  my $cnt01=0 , $cnt02=0 , $cnt03=0 ;

  open(F1,"$par_dir/SSRCalc2.par") or die "Cannae open parameter file, Capn";
  PARMLOOP:while (<F1>) 
  {
    chomp;
    s/#.*//;			        # delete comments
    s/\s//g;				# delete white
    next PARMLOOP unless length;			# if anything is left...
    # Which parms?
    if    (/\<AAPARAMS\>/i)  
    {
      $pSet="AAPARAMS";  
      next PARMLOOP;
    }  
    elsif (/\<ISOPARAMS\>/i) 
    {
      $pSet="ISOPARAMS"; 
      next PARMLOOP;
    }  
    elsif (/\<CLUSTCOMB\>/i) 
    {
      $pSet="CLUSTCOMB"; 
      next PARMLOOP;
    }  
    elsif (/\<HELIXPATTERN\>/i) 
    {
      $pSet="HELIXPATTERN"; 
      next PARMLOOP;
    }  
    elsif (/\<\/.+\>/i) 
    {
      $pSet=""; 
      next PARMLOOP;
    }  

    if ($pSet=~/AAPARAMS/)
    {
      (my ($aa1,$rc,$rc1,$rc2,$rcn,$rcn2,$krh,$amass,$ct_,$nt_,$pk_))=split(/\|/,$_);
      $RC{$aa1}=$rc; $RC1{$aa1}=$rc1; $RC2{$aa1}=$rc2; #  Retention Factors
      $RCN{$aa1}=$rcn; $RCN2{$aa1}=$rcn2;  
      $UndKRH{$aa1}=$krh;                              # Factors for aa's near undigested KRH.
      $AMASS{$aa1}=$amass;                           # aa masses in Daltons
      $CT{$aa1}=$ct_; $NT{$aa1}=$nt_; $PK{$aa1}=$pk_;  # Iso-electric factors
      $badAA=~s/$aa1//i;

      $cnt01++;
    }
    elsif ($pSet=~/ISOPARAMS/)
    {
      (my ($e1,$e2,$e3))=split(/\|/,$_);
      $emin[$cnt02]=$e1;
      $emax[$cnt02]=$e2;
      $eK[$cnt02]=$e3;
      $cnt02++;
    }
    elsif ($pSet=~/CLUSTCOMB/)
    {
      (my ($s1,$s2))=split(/\|/,$_); 
      $s1=~tr/l/5/;
      $s1=~tr/v/1/;
      $val1[$cnt03]=$s2; 
      $pick[$cnt03]=$s1;
      $cnt03++;
    }
  }
}

# ============================================================

sub ReadParmFile3 {
  my $pSet = "";
  my $cnt01=0 , $cnt02=0 , $cnt03=0, $cnt04=0 ;

  my $infile = "../params/SSRCalc3.par";

  if ($ENV{SSRCalc}) {
      $infile = "$ENV{SSRCalc}/$infile";
  }
  elsif ($ENV{SSRCALC}) {
      $infile = "$ENV{SSRCALC}/$infile";
  }

  open(F1,$infile) or die "ERROR: Unable to open parameter file \"$infile\"";
  PARMLOOP:while (<F1>) 
  {
    chomp;
    s/#.*//;			        # delete comments
    s/\s//g;				# delete white
    next PARMLOOP unless length;			# if anything is left...
    # Which parms?
    if    (/\<AAPARAMS\>/i)  
    {
      $pSet="AAPARAMS";  
      next PARMLOOP;
    }  
    elsif (/\<ISOPARAMS\>/i) 
    {
      $pSet="ISOPARAMS"; 
      next PARMLOOP;
    }  
    elsif (/\<CLUSTCOMB\>/i) 
    {
      $pSet="CLUSTCOMB"; 
      next PARMLOOP;
    }  
    elsif (/\<HELIXPATTERNS\>/i) 
    {
      $pSet="HELIXPATTERNS"; 
      next PARMLOOP;
    }  
    elsif (/\<\/.+\>/i) 
    {
      $pSet=""; 
      next PARMLOOP;
    }  

    if ($pSet=~/AAPARAMS/)
    {
      (my
        ($aa1,$rc,$rc1,$rc2,$rcn,$rcn2,$rcS,$rc1S,$rc2S,$rcnS,$rcn2S,$krh,$amass,$ct_,$nt_,$pk_,$bsc,$cmul)
      )	=split(/\|/,$_);
      $RC{$aa1}=$rc; $RC1{$aa1}=$rc1; $RC2{$aa1}=$rc2; #  Retention Factors
      $RCN{$aa1}=$rcn; $RCN2{$aa1}=$rcn2;  
      $RCS{$aa1}=$rcS; $RC1S{$aa1}=$rc1S; $RC2S{$aa1}=$rc2S; #  Short Peptide Retention Factors
      $RCNS{$aa1}=$rcnS; $RCN2S{$aa1}=$rcn2S;  
      $UndKRH{$aa1}=$krh;                              # Factors for aa's near undigested KRH.
      $AMASS{$aa1}=$amass;                           # aa masses in Daltons
      $CT{$aa1}=$ct_; $NT{$aa1}=$nt_; $PK{$aa1}=$pk_;  # Iso-electric factors
      $H2BASCORE{$aa1}=$bsc; $H2CMULT{$aa1}=$cmul;    #helicity2 bascore & connector multiplier.
      $badAA=~s/$aa1//i;

      $cnt01++;
    }
    elsif ($pSet=~/ISOPARAMS/)
    {
      (my ($e1,$e2,$e3))=split(/\|/,$_);
      $emin[$cnt02]=$e1;
      $emax[$cnt02]=$e2;
      $eK[$cnt02]=$e3;
      $cnt02++;
    }
    elsif ($pSet=~/CLUSTCOMB/)
    {
      (my ($s1,$s2))=split(/\|/,$_); 
      $s1=~tr/l/5/;
      $s1=~tr/v/1/;
      $val1[$cnt03]=$s2; 
      $pick[$cnt03]=$s1;
      $cnt03++;
    }
    elsif ($pSet=~/HELIXPATTERNS/)
    {
      (my ($pat1,$val1))=split(/\|/,$_);
      if    (length($pat1)==4) {$HlxScore4{$pat1}=$val1;}
      elsif (length($pat1)==5) {$HlxScore5{$pat1}=$val1;}
      elsif (length($pat1)==6) {$HlxScore6{$pat1}=$val1;}
      $cnt04++;
    }
  }
}

# ============================================================
# compute sum from amino acids - v 1 algorithm
sub TSUM1
{
  my ($sq1)=@_;
  my $i;
  my $pick;
  my $tsum1=0;
  for ($i=0;$i<length($sq1);$i++)
  {
    $pick=substr($sq1,$i,1);
    $tsum1 += $tel{$pick};
  }
  
  $VV1=&NtermCorrV1($sq1); 
  $tsum1-=$VV1; 
  $VV2=&length_scale(length($sq1)); 
  $tsum1*=$VV2; 

  # jul 29 correction
  if ($tsum1>38)  { $tsum1=$tsum1-($tsum1-38)*.3; }
    
  return $tsum1*$K;
}


# ============================================================
# compute sum from amino acids plus correction factors - v 2 algorithm
sub TSUM2
{
  my ($sq2)=@_;
  my $tsum2=0;
  my ($i, $pick, $sze, $edge);
  
  # core summation
  $sze=length($sq2); $edge=0;
  
  
  if ($sze<4) {return $tsum2; } 
  $tsum2 = $RC1{substr($sq2,0,1)}        # Sum weights for 1st,
         + $RC2{substr($sq2,1,1)}        #                      second,
         + $RCN{substr($sq2,$sze-1,1)}   #                              last, 
         + $RCN2{substr($sq2,$sze-2,1)}; #                                    and second last a.a.

  for ($i=2;$i<$sze-2;$i++)
  {
    $tsum2+=$RC{substr($sq2,$i,1)};       # sum weights for a.a.s in the middle.
  }

  # 1- undigested parts
  $v1=&undigested($sq2); 
  $tsum2 -= $v1;

  # 2- clusterness # NB:weighting of v1 is now done in subrtn.
  $v1=&clusterness($sq2); 
  $tsum2 -= $v1 ;
  
  # 2.5- proline fix
  $v1=&proline($sq2); 
  $tsum2 -= $v1;
  
  # 3- length scaling correction
  $v1=&length_scale($sze); 
  $tsum2 *= $v1; 
  
  # 4- total sum correction
  if ($tsum2>=20 && $tsum2<30 )  { $tsum2=$tsum2-($tsum2-18)*$SUMSCALE1; }
  if ($tsum2>=30 && $tsum2<40)   { $tsum2=$tsum2-($tsum2-18)*$SUMSCALE2; }
  if ($tsum2>=40 && $tsum2<50)   { $tsum2=$tsum2-($tsum2-18)*$SUMSCALE3; }
  if ($tsum2>=50 )               { $tsum2=$tsum2-($tsum2-18)*$SUMSCALE4; }
  
  # 4.5- isoelectric change
  $v1=&newiso($sq2,$tsum2);
  $tsum2 += $v1;

  $K=1;
  return $tsum2*$K;
}

# ============================================================
# compute sum from amino acids plus correction factors - v 3 algorithm
sub TSUM3
{
  my ($sq3)=@_;
  my $tsum3=0;
  my ($i, $pick, $sze, $edge);
  
  # core summation
  $sze=length($sq3); $edge=0;
  
  
  if ($sze<4) {return $tsum3; } # peptide is too short to have any retention

  if ($sze<10) { 			# short peptides use short peptide retention weights
    $tsum3 = $RC1S{substr($sq3,0,1)}        # Sum weights for 1st,
           + $RC2S{substr($sq3,1,1)}        #                      second,
           + $RCNS{substr($sq3,$sze-1,1)}   #                              last, 
           + $RCN2S{substr($sq3,$sze-2,1)}; #                                    and second last a.a.

    for ($i=2;$i<$sze-2;$i++)
    {
      $tsum3+=$RCS{substr($sq3,$i,1)};       # sum weights for a.a.s in the middle.
    }
  }
  else {				# longer peptides use regular retention weights
    $tsum3 = $RC1{substr($sq3,0,1)}        # Sum weights for 1st,
           + $RC2{substr($sq3,1,1)}        #                      second,
           + $RCN{substr($sq3,$sze-1,1)}   #                              last, 
           + $RCN2{substr($sq3,$sze-2,1)}; #                                    and second last a.a.

    for ($i=2;$i<$sze-2;$i++)
    {
      $tsum3+=$RC{substr($sq3,$i,1)};       # sum weights for a.a.s in the middle.
    }
  }

  # 1- smallness - adjust based on tsum score of peptides shorter than 20 aa's.
  $tsum3 += &smallness($sze,$tsum3);

  # 2- undigested parts
  $tsum3 -= &undigested($sq3);

  # 3- clusterness # NB:weighting of v1 is now done in subrtn.
  $tsum3 -= &clusterness($sq3);
  
  # 4- proline fix
  $tsum3 -= &proline($sq3);
  
  # 5- length scaling correction
  $tsum3 *= &length_scale($sze);  
  
  # 6- total sum correction
  if ($tsum3>=20 && $tsum3<30 )  { $tsum3-=(($tsum3-18)*$SUMSCALE1); }
  if ($tsum3>=30 && $tsum3<40)   { $tsum3-=(($tsum3-18)*$SUMSCALE2); }
  if ($tsum3>=40 && $tsum3<50)   { $tsum3-=(($tsum3-18)*$SUMSCALE3); }
  if ($tsum3>=50 )               { $tsum3-=(($tsum3-18)*$SUMSCALE4); }
  
  # 7- isoelectric change
  $tsum3 += &newiso($sq3,$tsum3);
 
  # 8- helicity corrections  #NB: HELIX#SCALE-ing is now done in subrtn.
  $tsum3 += &helicity1($sq3);
  $tsum3 += &helicity2($sq3);
  $tsum3 += &helectric($sq3);

  $K=1;
  return $tsum3*$K;
}

# ============================================================
# N-terminus weight correction factor - v 1 algorithm
sub NtermCorrV1
{
	my ($sq)=@_;
	$c01=substr($sq,0,1);
	$c02=substr($sq,1,1);
	$c03=substr($sq,2,1);
	
	
	return ($NTF1*$cf{$c01}+$NTF2*$cf{$c02}+$NTF3*$cf{$c03});
}

# ============================================================
# process based on proline - v 2,3 algorithm
sub proline
{
	my ($sq)=@_;
	
	$_=$sq;
	$score=0;
	if (/PP/)   { $score=$PPSCORE;   }
	if (/PPP/)  { $score=$PPPSCORE;  }
	if (/PPPP/) { $score=$PPPPSCORE; }
	
	return $score;
}

# ============================================================
# process based on new isoelectric stuff - v 2,3 algorithms
sub newiso
{
	my ($sq,$tsum)=@_;
	my($i,$mass,$cf1,$delta1,$corr01,$pi1,$lmass);
	
	if ($NOELECTRIC==1) { return 0; }
	
	
	# compute mass
	$mass=0;
	for ($i=0;$i<length($sq);$i++)
	{
		$cf1=substr($sq,$i,1);
		$mass=$mass+$AMASS{$cf1};
	}
	
	# compute isoelectric value
	$pi1=&electric($sq);
	$lmass=1.8014*log($mass);
		
	# make mass correction
	$delta1=$pi1-19.107+$lmass;
	if ($delta1<0)
	{
		# apply corrected value as scaling factor
		$corr01=($tsum*$Z01+$Z02)*$NDELTAWT*$delta1;
	}
	
	if ($delta1>0)
	{
		$corr01=($tsum*$Z03+$Z04)*$PDELTAWT*$delta1;
	}
		
	return $corr01;
}

# ============================================================
# scaling based on length - v 1,2,3 algorithms
sub length_scale
{
	my ($sqlen)=@_;
	my $LS=1;
	
	if    ($sqlen<$SPLim) {$LS = 1+$SPSFac*($SPLim-$sqlen); }
	elsif ($sqlen>$LPLim) {$LS = 1/(1+$LPSFac*($sqlen-$LPLim)); }
	return $LS;
}

# ============================================================
# correct for things not digested - v 3 algorithm
# rules: look for K R  H on right edge, apply UDF21*E6{i-1}+UDF22*E6{i-2}
# look for R K H throughout string, apply UDF31*(E6{i+1}+E6{i-1}+UDF32*(E6{i+2}+E6{i-2})
# also look for pairs of permutations, treat as single item
# return big sum 
sub undigested
{
	my ($sq)=@_;
	my ($xx,$re,$csum,$dd,$op1,$op2,$op3,$op4);
	
	if ($NODIGEST==1) { return 0; }
	
	$xx=length($sq)-1;
	$re=substr($sq,$xx,1); 
	$csum=0;
	
	# rightmost
	if ($re eq "R" || $re eq "K" || $re eq "H")
	{
		$op1=substr($sq,$xx-1,1); # left by 1
		$op2=substr($sq,$xx-2,1); # left by 2
		$csum=$UDF21*$UndKRH{$op1}+$UDF22*$UndKRH{$op2};
	}
	
	# scan through string, starting at second and ending two before left
	for ($dd=0;$dd<$xx;$dd++)
	{
		
		$re=substr($sq,$dd,1);
		if ($re eq "K" || $re eq "R" || $re eq "H")
		{
			$op1=substr($sq,$dd-1,1); # left by 1
			$op2=substr($sq,$dd-2,1); # left by 2
			$op3=substr($sq,$dd+1,1); # right by 1
			$op4=substr($sq,$dd+2,1); # right by 2
			$csum=$csum+$UDF31*($UndKRH{$op1}+$UndKRH{$op3})+$UDF32*($UndKRH{$op2}+$UndKRH{$op4});
		}
		
		
	}
	
	return $csum;			
}

# ============================================================
# compute clusterness of a string - v 2,3 algorithm
# code W,L,F,I as 5
# code M,Y,V as 1
# code all others as 0
sub clusterness
{
  my($sq)=@_;
  my($cc,$score,$i,$x1,$occurs,$pt,$sk,$addit);
  
  if ($NOCLUSTER==1) { return 0; }

  $cc = $sq; #cluster coded sq
  if ($SSRCVERSION==3)
  {
    $cc=~tr/LIW/5/;  # strong
    $cc=~tr/AMYV/1/; # moderate
  }
  else
  {
    $cc=~tr/WFLI/5/; # strong
    $cc=~tr/MYV/1/;  # moderate
  }
  $cc=~tr/A-Z/0/;   # zero
  $cc="0" . $cc . "0"; # zero-pad it
    
  $score=0;
  for ($i=0;$i<@pick;$i++) 
  {
    $pt=$pick[$i]; $sk=$val1[$i];
    $occurs=0; 
    $x1= "0" . $pt . "0"; # pad it out
    while ($cc=~/$x1/g) { $occurs++; } # count occurrences
      	
    if ($occurs>0)
    { 
      $addit=$sk*$occurs;
      $score=$score+$addit; 
    }
  }
  return $score*$KSCALE;
}

# ============================================================
#    - v 2,3 algorithms
sub electric
{
	my ($sq)=@_;
	my($ss,$s1,$s2,$i,$z,$best,$min,$check,$e,$pk0,$pk1);
	
	my %aaCNT = (K=>0, R=>0, H=>0, D=>0, E=>0, C=>0, Y=>0);
	
	# if ($NOELECTRIC==1) { return 1; }
	
	# get c and n terminus acids
	$ss=length($sq); 

	$s1=substr($sq,0,1); 
	$s2=substr($sq,$ss-1,1);

	$pk0=$CT{$s1};
	$pk1=$NT{$s2};


	# count them up
	for ($i=0;$i<$ss;$i++)
	{
		$e=substr($sq,$i,1);
		if ($e=~m/[KRHDECY]/) {$aaCNT{$e}++;}
	}

	# cycle through pH values looking for closest to zero

	# coarse pass
	$best=0; $min=100000; $step1=.3;
	for ($z=.01;$z<=14;$z=$z+$step1)
	{
		$check=&CalcR($z,$pk0,$pk1,\%aaCNT); if ($check<0) { $check=0-$check; }
		if ($check<$min) { $min=$check; $best=$z; }
	}
	my $best1=$best;
	
	# fine pass
	$min=100000;
	for ($z=$best1-$step1;$z<=$best1+$step1;$z=$z+.01)
	{
		$check=&CalcR($z,$pk0,$pk1,\%aaCNT); if ($check<0) { $check=0-$check; }
		if ($check<$min) { $min=$check; $best=$z; }
	}
	
	return $best;
	
}

# ============================================================
# compute R - v 2,3 algorithms
sub CalcR
{
	my ($pH,$PK0,$PK1,$CNTref)=@_;
	
	my $cr0 =
  	                _partial_charge( $PK0,     $pH    )  # n terminus
   + $CNTref->{K} * _partial_charge( $PK{K},   $pH    )  # lys
   + $CNTref->{R} * _partial_charge( $PK{R},   $pH    )  # arg
   + $CNTref->{H} * _partial_charge( $PK{H},   $pH    )  # his
   - $CNTref->{D} * _partial_charge( $pH,      $PK{D} )  # asp
   - $CNTref->{E} * _partial_charge( $pH,      $PK{E} )  # glu
   - $CNTref->{Y} * _partial_charge( $pH,      $PK{Y} )  # try
   -                _partial_charge( $pH,      $PK1   ); # c terminus

# The following was taken out of the formula for R
#  - $CNTref->{C} * _partial_charge( $pH,      $PK{C} )    # cys

	
	return $cr0;
}

# ============================================================
# compute partial charge - v 2,3 algorithms
sub _partial_charge  {

   my $cr = 10 ** ( $_[0] - $_[1] );
   return $cr / ( $cr + 1 );

}

# ============================================================
# convert electric to scaler - v 2,3 algorithms
sub electric_scale
{
	my ($v)=@_;
	my($i,$best);
	$best=1;
	
	# if ($NOELECTRIC==1) { return 1; }
	
	for ($i=0;$i<@emin;$i++)
	{
		if ($v>$emin[$i] && $v<$emax[$i]) { $best=$eK[$i]; }
	}
	return $best;
}

# ============================================================
# adjustment based on tsum score of peptides shorter than 20 aa's. - v 3 algorithm
sub smallness
{
  my ($sqlen,$tsum)=@_;
  if ($NOSMALL==1) { return 0; }
	
  if ($sqlen<20)
  {
    if ($tsum/$sqlen<0.9) {return 3.5*(0.9-$tsum/$sqlen);}
  }
	
  if ($sqlen<15)
  {
    if ($tsum/$sqlen>2.8) {return 2.6*($tsum/$sqlen-2.8);}
  }
	
  return 0;
}

# ============================================================
# helicity1 adjust for short helices or sections - v 3 algorithm
sub helicity1
{
  my($sq)=@_;
  my($hc,$i,$sum);
  my($hc4,$hc5,$hc6,$sc4,$sc5,$sc6,$trmAdj4,$trmAdj5,$trmAdj6);

  if ($NOHELIX1==1) { return 0; }

  $hc=$sq; 		# helicity coded sq
  $hc=~tr/PHRK/z/;  
  $hc=~tr/WFIL/X/; 
  $hc=~tr/YMVA/Z/; 
  $hc=~tr/DE/O/; 
  $hc=~tr/GSPCNKQHRT/U/; 

  $sum=0; $sqlen=length($sq);
  for ($i=0;$i<$sqlen;$i++)
  {
    $hc4=substr($hc,$i,4);
    $sc4=$HlxScore4{$hc4}; 
    if ($sc4>0)
      { $trmAdj4=&heli1TermAdj($hc4,$i,$sqlen); }

    $hc5=substr($hc,$i,5);	
    $sc5=$HlxScore5{$hc5}; 
    if ($sc5>0)
      { $trmAdj5=&heli1TermAdj($hc5,$i,$sqlen); }

    $hc6=substr($hc,$i,6);
    $sc6=$HlxScore6{$hc6}; 
    if ($sc6>0)
      { $trmAdj6=&heli1TermAdj($hc6,$i,$sqlen); }

    if    ($sc6>0) {  $sum += ($sc6 * $trmAdj6); $i=$i+1;  }
    elsif ($sc5>0) {  $sum += ($sc5 * $trmAdj5); $i=$i+1;  } 
    elsif ($sc4>0) {  $sum += ($sc4 * $trmAdj4); $i=$i+1;  } 

  }
  return  $HELIX1SCALE*$sum;
}

# ============================================================
# called by helicity1  - v 3 algorithm
sub heli1TermAdj
{
  my($ss1,$ix2,$sqlen)=@_;
  my($m,$where);

  for ($i=0;$i<length($ss1);$i++)
  {
    $m=substr($ss1,$i,1);
    if ($m eq "O" || $m eq "U") { $where=$i; break; }
  }

  $where=$where+$ix2;

  if ($where<2) { return .2; }
  if ($where<3) { return .25; }
  if ($where<4) { return .45; }

  if ($where>$sqlen-3) { return .2; }
  if ($where>$sqlen-4) { return .75; }
  if ($where>$sqlen-5) { return .65; }

  return 1; 

}


# ============================================================
# helicity2 adjust for long helices - v 3 algorithm
sub helicity2
{
  my($sq)=@_;
  my ($h2FwBk,$Bksq,$FwHiscor,$FwGscor,$BkHiscor,$BkGscor);
  my ($h2mult,$lenMult,$NoPMult);

  if ($NOHELIX2==1) { return 0; }

  $Bksq = reverse $sq;
  ($FwHiscor,$FwGscor) = &heli2Calc($sq); 
  ($BkHiscor,$BkGscor) = &heli2Calc($Bksq); 
  if ($BkGscor>$FwGscor)	
    { $h2FwBk = $BkHiscor; }
  else 	        
    { $h2FwBk = $FwHiscor; }

  $lenMult=0;
  if (length($sq)>30) { $lenMult=1; }

  $NoPMult=0.75;
  if ($sq=~/P/) { $NoPMult=0; }
	
  $h2mult = 1+ $lenMult + $NoPMult;	

  return $HELIX2SCALE * $h2mult * $h2FwBk;
}

# ============================================================
# called by helicity2  - v 3 algorithm
sub heli2Calc
{
  my ($sq)=@_;
  my($pass1,$i);
  my($look1,$m,$lc);
  my($pat,$zap,$f1,$f2,$f3,$subt,$sq2);
  my($view)=0;
  my($sp);
  my($llim)=50;
  my($hiscore)=0;
  my($skore,$best,$best_pos);
  my($spacer);
	
  if (length($sq)<11) { return 0; }
	
  my($prechop)=$sq;

  chop($sq); chop($sq);
  $sq=substr($sq,2);
		
  $pass1=$sq;
  $pass1=~tr/WFILYMVA/1/; 
  $pass1=~tr/GSPCNKQHRTDE/0/; 
	
  $gscore=0;
	
  for ($i=0;$i<=length($pass1);$i++)
  {
    $m=substr($pass1,$i,1);
    if ($m==1) 
    {
      $lc=substr($pass1,$i,$llim);
      $sq2=substr($sq,$i,$llim);
      $pat="";
      $zap=0; $subt=0;
			
      while ($zap<=$llim && $subt<2)
      {
        $f1=substr($lc,$zap,1);
        $f2=substr($lc,$zap-1,1);
        $f3=substr($lc,$zap+1,1);

        if ($f1==1) 
        { 
          if ($zap>0) { $pat=$pat . "--"; }
          $pat=$pat  . substr($sq2,$zap,1) ;
          goto bail; 
        }
        				
        if ($f2==1 && $f1==0) 
        { 
          $subt++; 
          if ($subt<2) { $pat=$pat . "->" . substr($sq2,$zap-1,1);  }
          goto bail; 
        }
        				
        if ($f3==1 && $f1==0) 
        { 
          $subt++; 
          if ($subt<2) { $pat=$pat . "<-" . substr($sq2,$zap+1,1);  }
          goto bail; 
        }
    				
    bail:				
        if ($f1+$f2+$f3==0) { $zap=1000; }
        $zap=$zap+3;
      }
			
      if (length($pat)>4) 
      { 
        $traps=$prechop;
        $skore=&evalH2pattern($pat,$traps,$i-1,"*");
        if ($skore>=$hiscore) { $hiscore=$skore; $best=$pat; $best_pos=$i; } 
      }
    }
  }
	
  if ($hiscore>0) 
  { 
    $gscore=$hiscore; 
    $traps=$prechop;
    $hiscore=&evalH2pattern($best,$traps,$best_pos-1,"+");
		
    return $hiscore,$gscore;
  }
  else
  {
    return 0,0;
  }
}

# ============================================================
# called by heli2calc  - v 3 algorithm
sub evalH2pattern
{
  my($pattern,$testsq,$posn,$etype)=@_;
  my($f01)=substr($pattern,0,1);
  my($prod1)=$H2BASCORE{$f01};
  my($i,$mult,$fpart,$gpart,$s3);
  my($testAAl,$testAAr,$iss);
	
  $mult=1;
  my($OFF1)=2;
	
  $testAAl=substr($testsq,$OFF1+$posn,1); 
  $testAAr=substr($testsq,$OFF1+$posn+2,1);
  $testsq=substr($testsq,$OFF1+$posn+1);
	
  $mult=&connector($f01,$testAAl,$testAAr,"--");	
  $prod1=$prod1*$mult;
	
  if ($etype eq "*") { $prod1=$prod1*25; } 

  if ($mult==0) 
  { 
    return 0; 
  }
	
	
  my($acount)=1;
			
  for ($i=1;$i<length($pattern);$i=$i+3)
  {
    $fpart=substr($pattern,$i,2);
    $gpart=substr($pattern,$i+2,1);
    $s3=$H2BASCORE{$gpart};  

    if ($fpart eq "--") { $iss=0; $far1=""; $far2=""; } 
    if ($fpart eq "<-") { $iss=1; $far1=substr($testsq,$i+1,1); $far2=""; } 
    if ($fpart eq "->") { $iss=-1; $far1=""; $far2=substr($testsq,$i+3,1); } 
		
    $testAAl=substr($testsq,$i+1+$iss,1);
    $testAAr=substr($testsq,$i+3+$iss,1);
		
    $mult=&connector($gpart,$testAAl,$testAAr,$fpart);
				
    if ($etype eq "*")
    {
      if ($mult!=0 || $acount<3) { $prod1=$prod1*25*$s3*$mult; } 
    }
		
    if ($etype eq "+")
    {
      $prod1=$prod1+$s3*$mult;
    }
	
    if ($mult==0)
    {
      return $prod1;
    }
    else 
    {
    }
		
    $acount++;
		
  }
  return $prod1;
}

# ============================================================
# called by evalH2pattern  - v 3 algorithm
sub connector
{
  my($acid,$lp,$rp,$ct)=@_;
  my($mult)=1;
	
  if ($ct eq "<-") { $mult *= 0.2; }
  if ($ct eq "->") { $mult *= 0.1; }

  $mult *= $H2CMULT{$lp};
  if ($lp ne $rp)  { $mult *= $H2CMULT{$rp}; }

  if ($acid =~ /A|Y|V|M/)
  {
    if (($lp =~ /P|G/)||($rp =~ /P|G/)) { $mult = 0; }
    if ($ct eq "->" || $ct eq "<-") { $mult=0; }
  }
  	
  if ($acid =~ /L|W|F|I/)
  {
    if ((($lp =~ /P|G/) || ($rp =~ /P|G/)) && ($ct ne "--") ) { $mult=0; }
    if ((($far1 =~ /P|G/) || ($far2 =~ /P|G/)) && ($ct eq "<-" || $ct eq "->") ) { $mult=0; }
  }
	
  return $mult;
}

# ============================================================

sub helectric
{
  my($sq)=@_;
	
  if ($NOEHEL==1) { return 0; }
	
  if (length($sq)>14) { return 0; }
	
  my($co)=substr($sq,-4);
  local($mpart);
	
  if (substr($co,0,1) eq "D" || substr($co,0,1) eq "E")
  {
		
    $mpart=substr($co,1,2); 

    if ($mpart =~ /P|G|K|R|H/) { return 0; } 
		
    $mpart=~tr/LI/X/;
    $mpart=~tr/AVYFWM/Z/;
    $mpart=~tr/GSPCNKQHRTDE/U/;
		
    $_=$mpart;
    if (/XX/) { return 1; }
    if (/ZX/) { return 0.5; }
    if (/XZ/) { return 0.5; }
    if (/ZZ/) { return 0.4; }
    if (/XU/) { return 0.4; }
    if (/UX/) { return 0.4; }
    if (/ZU/) { return 0.2; }
    if (/UZ/) { return 0.2; }
		
  }
	
  return 0;
	
	
}



# ============================================================

sub usage {

  my $PROG_NAME = "SSRCalc3.pl";
  my $usage = <<EOU;
Usage: $PROG_NAME [OPTIONS]
Options:
  --help                 Print usage information
  --sequences            One or more peptide sequences separated by /
  --A                    Coefficient A
  --B                    Coefficient B
  --algorithm_version    Version of the algorithm to use
  --source_file          external file of peptide sequences to read from
  --output_format        Format for result, html or tsv

 e.g.: $PROG_NAME --alg 3.0 --seq PEPTIDE --B 5 --output tsv

EOU

  return($usage);

}
