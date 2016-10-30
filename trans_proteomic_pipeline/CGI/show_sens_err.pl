#!/usr/bin/perl
#############################################################################
# Program       : show_sens_err.pl                                          #
# Author        : Andrew Keller <akeller@systemsbiology.org>                #
# Date          : 3.28.03                                                   #
#                                                                           #
# ProteinProphet                                                            #
#                                                                           #
# Program       : ProteinProphet T.M.                                       #   
# Author        : Andrew Keller <akeller@systemsbiology.org>                # 
# Date          : 11.27.02                                                  #
#                                                                           #
#                                                                           #
# Copyright (C) 2003 Andrew Keller                                          #
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
# Andrew Keller                                                             #
# Insitute for Systems Biology                                              #
# 1441 North 34th St.                                                       #
# Seattle, WA  98103  USA                                                   #
# akeller@systemsbiology.org                                                #
#                                                                           #
#############################################################################

#
# gather TPP version info
#
my $CGI_HOME;
my $SERVER_ROOT;
my $CGI_HOME_FULLPATH;
my $USING_LOCALHOST = ( ($^O eq 'cygwin' ) || ($^O eq 'MSWin32' ) );
my $WINDOWS_CYGWIN = ($^O eq 'cygwin' );
my $xslt = '/usr/bin/xsltproc';

if ( $^O eq 'linux' ) {  # linux installation
    $CGI_HOME = '/tpp/cgi-bin/';  
    $CGI_HOME_FULLPATH = '/tools/bin/TPP/tpp/cgi-bin/';
    $xslt = '/usr/bin/xsltproc';
} elsif (( $^O eq 'cygwin' )||($^O eq 'MSWin32' )) { # windows installation
    $CGI_HOME = '/tpp-bin/';
	my ($serverRoot) = ($ENV{'WEBSERVER_ROOT'} =~ /(\S+)/);
	if ($WINDOWS_CYGWIN && $serverRoot =~ /\:/ ) {
	    $serverRoot = `cygpath '$serverRoot'`;
	    ($serverRoot) = ($serverRoot =~ /(\S+)/);
	}
	if ($^O eq 'MSWin32' ) {
		$serverRoot =~ s/\\/\//g;  # get those path seps pointing right!
	}
	# make sure ends with '/'
	$serverRoot .= '/' if($serverRoot !~ /\/$/);
	$SERVER_ROOT = $serverRoot;

	if ($^O eq 'MSWin32' ) {
	    $xslt = $SERVER_ROOT.'..'.$CGI_HOME.'xsltproc.exe --novalid'; # disconnect dtd check (since only has web server reference name)
	} else {
	    $xslt = '/usr/bin/xsltproc -novalid'; # disconnect dtd check (since only has web server reference name)
	}
} # end configuration


# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath);
use lib realpath(dirname($0));
use tpplib_perl; # exported TPP lib function points
$TPPVersionInfo = tpplib_perl::getTPPVersionInfo();

print "Content-type: text/html\n\n";

%box = &tpplib_perl::read_query_string;      # Read keys and values
$| = 1; # autoflush



my $xslt = '/usr/bin/xsltproc';

my $xmlfile;
if(exists $box{'xmlfile'}) {
    $xmlfile = $box{'xmlfile'};
}
else {
    if(@ARGV > 0) {
	$xmlfile = $ARGV[0];
    }
    else {
	print "have no xmlfile\n"; exit(1);
    }
}
if ($^O eq 'MSWin32' ) {
	$xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
}

my $version = '';
if(exists $box{'version'}) {
    $version = $box{'version'};
}

my $num_correct_prots = 0;
my %sens = ();
my %errs = ();
my $pngfile;
if($xmlfile =~ /^(\S+\.)xml(\.gz)?$/) {
    $pngfile = $1 . 'png';

    my $tempxslfile = $xmlfile . '.tmp.xsl';
    my $temptxtfile = $xmlfile . '.tmp.txt';
# extract out needed info
    if(! open(XSL, ">$tempxslfile")) {
	print "cannot open $tempxslfile $!\n";
	exit(1);
    }
    print XSL '<?xml version="1.0"?>';
    print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">';

    print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
    print XSL '</xsl:text></xsl:variable>';
    print XSL '<xsl:variable name="tab"><xsl:text>&#x09;</xsl:text></xsl:variable>', "\n";
    my $newline = '<xsl:value-of select="$newline"/>';
    my $tab = '<xsl:value-of select="$tab"/>';

    print XSL '<xsl:template match="protx:protein_summary">';

    print XSL '#num_predicted_correct_prots=<xsl:value-of select="protx:protein_summary_header/@num_predicted_correct_prots"/>' . $newline;
    print XSL '<xsl:apply-templates select="protx:protein_summary_header/protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/protx:protein_summary_data_filter">';
    print XSL '<xsl:sort select="@min_probability" order="ascending" data-type="number"/>';
    print XSL '</xsl:apply-templates>';

    print XSL '</xsl:template>';

    print XSL '<xsl:template match="protx:protein_summary_data_filter">';
    print XSL '<xsl:value-of select="@min_probability"/>' . $tab . '<xsl:value-of select="@sensitivity"/>' . $tab . '<xsl:value-of select="@false_positive_error_rate"/>' . $newline;
    print XSL '</xsl:template>';
    
    print XSL '</xsl:stylesheet>';
    close(XSL);

# now apply the template

    my $tmpxmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile); # decompress .gz if needed
    if($xslt =~ /xsltproc/) {
	open DATA, "$xslt $tempxslfile $tmpxmlfile |"; 
    }
    else {
	open DATA, "$xslt $tmpxmlfile $tempxslfile |"; 
    }
    open(OUT, ">$temptxtfile");

    while(<DATA>) {
	my $line = $_;
	print OUT $line if ($line !~ /\<\?xml.*version.*$/);
	
	if($line =~ /\#num\_predicted\_correct\_prots\=(\S+)/) {
	    $num_correct_prots = $1;
	}
	elsif($line !~ /\<\?xml.*version.*$/) {
	    chomp();
	    my @parsed = split /\s+/, $line;
	    if($#parsed >= 2) {
		$sens{$parsed[0]} = $parsed[1];
		$errs{$parsed[0]} = $parsed[2];
	    }
	}
    }
    close(DATA);
    close(OUT);

    unlink($tmpxmlfile) if ($tmpxmlfile ne $xmlfile); # did we decompress xml.gz?

    if(! -e $pngfile) {
# now write the script
	open(SCR, ">$xmlfile.tmp.script");
	print SCR "set terminal png;\n";
	print SCR "set output \"$pngfile\";\n";
	print SCR "set border;\n";
	printf SCR "set title \"Estimated Sensitivity (fraction of %0.1f total) and Error Rates\";\n", $num_correct_prots;
	print SCR "set xlabel \"Min Protein Prob\";\n";
	print SCR "set ylabel \"Sensitivity or Error\";\n";
	print SCR "set xtics (\"0\" 0, \"0.2\" 0.2, \"0.4\" 0.4, \"0.6\" 0.6, \"0.8\" 0.8, \"1\" 1.0);\n";
	print SCR "set grid;\n";
	print SCR "set size 0.75,0.8;\n";
	print SCR "plot \"$xmlfile.tmp.txt\" using 1:2 title 'sensitivity' with lines, \\\n";
	print SCR " \"$xmlfile.tmp.txt\" using 1:3 title 'error' with lines\n";
	close(SCR);
	system("rm -f $outfile") if(-e $outfile);
	if ($^O eq 'MSWin32' ) {
	my $resul = system("wgnuplot $xmlfile.tmp.script");  # make outfile...
	} else {
	my $resul = system("gnuplot $xmlfile.tmp.script");  # make outfile...
	}
	unlink("$xmlfile.tmp.script") if(-e "$xmlfile.tmp.script");

    } # if png not already exists
    unlink($tempxslfile) if(-e $tempxslfile);
    unlink($temptxtfile) if(-e $temptxtfile);
    
} # if valid xmlfile name
else {
    print "illegal xmlfile name: $xmlfile\n"; exit(1);
}

# make local reference
my $local_pngfile = $pngfile;
my $local_xmlfile = $xmlfile;
# check for cygwin
#
if(exists $ENV{'WEBSERVER_ROOT'}) {
    # make sure ends with '/'
    my ($serverRoot) = ($ENV{'WEBSERVER_ROOT'} =~ /(\S+)/);
	if ($^O eq 'MSWin32' ) {
		$serverRoot =~ s/\\/\//g;  # get those path seps pointing right!
	}
    #check if cygwin
    if (-f '/bin/cygpath') { 
	$serverRoot = `cygpath -u '$serverRoot'`;
	}
	($serverRoot) = ($serverRoot =~ /(\S+)/);
    
	# make sure ends with '/'
	$serverRoot .= '/' if($serverRoot !~ /\/$/);
	$local_pngfile =~ s/\\/\//g;  # get those path seps pointing right!
	if((length $serverRoot) <= (length $local_pngfile) && 
	   index((lc $local_pngfile), (lc $serverRoot)) == 0) {
	    $local_pngfile = '/' . substr($local_pngfile, (length $serverRoot));
	}else {
	    print STDERR "potential problem (se1): >$local_pngfile< is not mounted under webserver root: >$serverRoot<\n";
	}
	if (-f '/bin/cygpath') {
   	$local_xmlfile = `cygpath -w '$local_xmlfile'`;
	}
	if($local_xmlfile =~ /^(\S+)\s?/) {
	    $local_xmlfile = $1;
	}
} else {
    die "cannot find WEBSERVER_ROOT environment variable\n";
}

# here output figure along with related text
print "<HTML><PRE><HEAD><TITLE>XML Viewer: Sensitivity/Error (" . $TPPVersionInfo . ")</TITLE></HEAD>";
print '<b>ProteinProphet<sup><font size="3">&reg;</font></sup> Predicted Sensitivity and Error Rate Info</b>', "\n";

print 'Dataset: ', $xmlfile, "\n";

print '<font color="blue">Predicted Total Number of Correct Entries: ' . $num_correct_prots . '</font>';
print "\n\n";
print "<TABLE><TR><TD>";

print "<IMG SRC=\"$local_pngfile\">";
print "</TD><TD><PRE>";

print "<font color=\"red\">sensitivity</font>\tfraction of all correct proteins\n\t\twith probs &gt;= min_prob\n\n";
print "<font color=\"green\">error</font>\t\tfraction of all proteins with\n\t\tprobs &gt;= min_prob that are incorrect\n";
print "\n\n";
if(1) {
print "    min_prob <font color=\"red\">sens     </font><font color=\"green\">err      </font>\t<font color=\"red\"># corr</font>\t<font color =\"green\"># incorr</font>\n\n";
foreach(reverse sort keys %sens) {
    printf "    %0.2f     <font color=\"red\">%0.3f    </font><font color=\"green\">%0.3f\t</font><font color=\"red\">%0.0f</font>\t<font color=\"green\">%0.0f</font><br>", $_, $sens{$_}, $errs{$_}, $sens{$_} * $num_correct_prots, $sens{$_} * $num_correct_prots * $errs{$_} / (1.0 - $errs{$_}) if($errs{$_} < 1.0);
}
} # if 
else {
my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'rules="all" cellpadding="2" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
my $RESULT_TABLE_SUF = '</table>';
print $RESULT_TABLE_PRE . $RESULT_TABLE;
print '<tr><td><b><font color="brown">min prob</font></b></td><td><b><font color="brown">sens</font></b></td><td><b><font color="brown">error</font></b></td><td><b><font color="brown"># correct</font></b></td><td><b><font color="brown"># incorrect</font></b></td></tr>';
foreach(reverse sort keys %sens) {
    printf "<tr><td>%0.2f</td><td><font color=\"red\">%0.3f</font></td><td><font color=\"green\">%0.3f</font></td><td><font color=\"red\">%0.0f</font></td><td><font color=\"green\">%0.0f</font></td></tr>", $_, $sens{$_}, $errs{$_}, $sens{$_} * $num_correct_prots, $sens{$_} * $num_correct_prots * $errs{$_} / (1.0 - $errs{$_}) if($errs{$_} < 1.0);
}
print $RESULT_TABLE_SUF;

}
print "</TD></TR></TABLE>";
print "</PRE></HTML>";







