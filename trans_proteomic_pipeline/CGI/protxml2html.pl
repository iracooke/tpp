#!/usr/bin/perl
#############################################################################
# Program       : protxml2html.pl                                           #
# Author        : Andrew Keller <akeller@systemsbiology.org>                #
# Date          : 3.28.03                                                   #
# SVN Info      : $Id: protxml2html.pl 6739 2014-11-12 22:38:49Z slagelwa $
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
use strict;
use POSIX;
use File::Spec; # use perl libs instead of depending on /^\/ as fullpath indicator
use File::Basename;
use Cwd qw(realpath);
use lib realpath(dirname($0));
use tpplib_perl; # exported TPP lib function points

print "Content-type: text/html\n\n" if(@ARGV == 0 || ! ($ARGV[0] eq '-file'));

my %box;
%box = &tpplib_perl::read_query_string if $ENV{'REQUEST_METHOD'}; # Read keys and values


#############################################################################
#         C O N F I G U R A T I O N    A R E A
#
# ISB-CYGWIN Release?
#   This is a kludge to hardcode isb-cygwin specific
#   defaults for the following parameters.  In the
#   future these defaults should be automatically filled
#   in by a true build system.  See the code which immediately
#   follows this section for isb-cygwin defults.
#
########################################
# ALL NON-ISB USERS: SET THIS VALUE TO 0
my $ISB_VERSION = 0;
########################################

my $WINDOWS_CYGWIN = -f '/bin/cygpath';
$ISB_VERSION = $WINDOWS_CYGWIN;

# USE OUTSIDE OF ISB, UNCOMMENT THIS LINE
#my $DISTR_VERSION = 1;

# USE INSIDE ISB, UNCOMMENT THIS LINE
my $DISTR_VERSION = 0; 

# forward declare variables
my $CGI_HOME;		# Full path web server home directory
my $HELP_DIR;		# Where all help png's are kept
my $xslt;		# Full path reference to a stylesheet processor 
my $DTD_FILE;		# Full path reference to ProteinProphet_v1.7.dtd
my $WEBSRVR;		# enum { 'IIS', 'APACHE', 'WEBSITEPRO' }
my $SERVER_ROOT = '';	
my $TOP_PATH;
my $CGI_BIN;

# Why define this? We never use this variable...oh but *we* do.  We cleverly 
# rewrite paths in the perl code using simple text subsitutions via a perl
# one-liner buried within the TPP make files.  See the "perl_paths" 
# target/directory.  Now the real question is what value should it have.
my $base_dir;

#
# Linux distribution
#
if ( $^O eq 'linux' ) {
    $TOP_PATH = '/tools/bin/TPP/tpp/';  #DDS: The last '/' is important!
    $CGI_BIN = 'cgi-bin/';
    $CGI_HOME = '/tpp/cgi-bin/';  
    $HELP_DIR = '/tpp/html/'; 
    $xslt = '/usr/bin/xsltproc';  # disconnect dtd check (since only has web server reference name)
#  $DTD_FILE = '/usr/local/tpp/schema/ProteinProphet_v1.9.dtd'; 
    $WEBSRVR = "APACHE";

#
# Cygwin Configuration
#
} elsif ( ($^O eq 'cygwin' )||($^O eq 'MSWin32' )) {
    $CGI_HOME = '/tpp-bin/';
    $HELP_DIR = '/tpp-bin/';

    if(exists $ENV{'WEBSERVER_ROOT'}) {
	my ($serverRoot) = ($ENV{'WEBSERVER_ROOT'} =~ /(\S+)/);
	if ( $WINDOWS_CYGWIN && $serverRoot =~ /\:/ ) {
	    $serverRoot = `cygpath '$serverRoot'`;
	    ($serverRoot) = ($serverRoot =~ /(\S+)/);
	}
	if ($^O eq 'MSWin32' ) {
	    $serverRoot =~ s/\\/\//g;  # get those path seps pointing right!
	}
	# make sure ends with '/'
	$serverRoot .= '/' if($serverRoot !~ /\/$/);
	$SERVER_ROOT = $serverRoot;
	if($SERVER_ROOT eq '') {
	    die "cannot find WEBSERVER_ROOT environment variable\n";
	}
    }
    else {
	die "cannot find WEBSERVER_ROOT environment variable\n";
    }
    if ($^O eq 'MSWin32' ) {
	$xslt = $SERVER_ROOT.'..'.$CGI_HOME.'xsltproc.exe --novalid'; # disconnect dtd check (since only has web server reference name)
    } else {
	$xslt = '/usr/bin/xsltproc -novalid'; # disconnect dtd check (since only has web server reference name)
    }
    $WEBSRVR = "APACHE";
    if ($ENV{'SERVER_SOFTWARE'} =~ m/IIS/) {
	$WEBSRVR = "IIS";
    }
}
# end configuration



# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath);
use lib realpath(dirname($0));

#
# gather TPP version info
#
my $TPPVersionInfo = tpplib_perl::getTPPVersionInfo() || 'version n/a';
my $TPPhostname = "http://" . tpplib_perl::get_tpp_hostname();

my $LC_SERVER_ROOT = lc $SERVER_ROOT; # lower case

my $GO_ONTOLOGY_CGI = 'goOntology';


my $HTML = 0;
my $HTML_ORIENTATION = 1; # whether or not
my $SHTML = 1; # whether or not to use SSI to launch cgi instead of traditional written html file
my $ICAT = 0;
my $GLYC = 0;
my $EXCEL = 0;    # Generate XLS file from command-line (arg = EXCEL) -- outputs to file
my $HTMLGEN = 0;  # Generate HTML file from command-line (arg = HTML) -- outputs to stdout
my $NOGAGGLE = 0; # If set, do not generate Gaggle files (command-line arg = NOGAGGLE)

my $SINGLE_HITS = 0;
my $DISPLAY_MODS = 1;
my $MOD_MASS_ERROR = 0.5;

# get rid of these all
my $CALCULATE_PIES = $ISB_VERSION; #whether to calc pies on the fly
my %prot_entries = (); # for pies, prob for each protein entry

# write out new xsl stylesheet
my $xmlfile;
my $xslfile;
my $pngfile = '';
my $htmlfile;
my $excelfile;

my $gaggleNameValueFile;
my $gaggleNameValueSize;

my $gaggleNameListFile;
my $gaggleNameListSize;

my $gaggleMatrixFile;
my $gaggleMatrixSize;

my $sort_index = -1;
my $start_string = 'start';
my $start_string_comment = '<!--' . $start_string . '-->';
my $USE_INDEX = 1; # whether to use explicit @index rather than num siblings
$USE_INDEX = 1 if(scalar(@ARGV) >= 2 && $ARGV[1] eq 'index');

my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'cellpadding="0" bgcolor="white" class="results">';
my $RESULT_TABLE_SUF = '</table>';

my $checked = 'CHECKED="yes"';

my $inital_xsl = 0;
my $MAX_NUM_ENTRIES = 2000; # if more than that, will filter at min prob
my $MIN_PROT_PROB = 0.1;
# in some environments, form info is transmitted as $ARGV[0]


my $entry_delimiter = 8; # empty cell height for delimiting successive entries

my $initiate = 0;
if(scalar(@ARGV) > 1 && $ARGV[0] eq '-file' && $ARGV[1] =~ /^(\S+\.)xml(\.gz)?$/) { # take file name from arg
	$xmlfile = $ARGV[1];
	if ($^O eq 'MSWin32' ) {
		$xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
	}
	$xmlfile =~ /^(\S+\.)xml(\.gz)?$/; # reevaluate $1
	$xslfile = $1 . 'xsl';
	$excelfile = $1 . 'xls';
	$pngfile = $1 . 'png';
	$gaggleNameValueFile = $1 . 'nv.ggl';
	$gaggleNameListFile = $1 . 'nl.ggl';
	$gaggleMatrixFile = $1 . 'mx.ggl';
	if($SHTML) {
	    $htmlfile = $1 . 'shtml';
	}
	else {
	    $htmlfile = $1 . 'htm';
	}

	# check for icat
	for(my $k = 2; $k <= $#ARGV; $k++) {
	    $ICAT = 1 if($ARGV[$k] eq 'ICAT');
	    $GLYC = 1 if($ARGV[$k] eq 'GLYC');
	    $EXCEL= 1 if($ARGV[$k] eq 'EXCEL');
	    $HTMLGEN = 1 if($ARGV[$k] eq 'HTML');
	    $NOGAGGLE = 1 if($ARGV[$k] eq 'NOGAGGLE');
	}

	$initiate = 1 unless ($EXCEL || $HTMLGEN);

}
elsif(exists $box{'xmlfile'} && $box{'xmlfile'} =~ /^(\S+\.)xml(\.gz)?$/) {
    $xmlfile = $box{'xmlfile'};
	if ($^O eq 'MSWin32' ) {
		$xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
	}
	$xmlfile =~ /^(\S+\.)xml(\.gz)?$/; # reevaluate $1
    $xslfile = $1 . 'xsl';
    $excelfile = $1 . 'xls';
    $pngfile = $1 . 'png';
    $gaggleNameValueFile = $1 . 'nv.ggl';
    $gaggleNameListFile = $1 . 'nl.ggl';
    $gaggleMatrixFile = $1 . 'mx.ggl';
    if($SHTML) {
	$htmlfile = $1 . 'shtml';
    }
    else {
	$htmlfile = $1 . 'htm';
    }
} # if


my $go_level = 0;
my %go_prots = ();
$go_level = $box{'go_level'} if(exists $box{'go_level'});

my $NEW_XML_FORMAT = 1;
# cancel DISPLAY_MODS for inappropriate directories running old version
$DISPLAY_MODS = 0 if(! useXMLFormatLinks($xmlfile));

$| = 1; # autoflush

my $restore_view = exists $box{'restore_view'} && $box{'restore_view'} eq 'yes';

$ICAT = 1 if(exists $box{'icat_mode'} && $box{'icat_mode'} eq 'yes');
$GLYC = 1 if(exists $box{'glyc_mode'} && $box{'glyc_mode'} eq 'yes');
my $pre_existing_xsl = 0;


if(0 && $restore_view && ! -e $xslfile) {
    print "Error: Cannot find stylesheet for most recent view of dataset.  Please recreate.\n\n";
    exit(1);
}

if(exists $box{'custom_settings'}) {
    if($box{'custom_settings'} eq 'current') {
	writeCustomizedSettings($xmlfile, 0, \%box);
    }
    elsif($box{'custom_settings'} eq 'default') {
	writeCustomizedSettings($xmlfile, 1, \%box);

	# full/short menu settings
	my $menu = exists $box{'menu'} ? $box{'menu'} : '';
	my $full_menu = exists $box{'full_menu'} ? $box{'full_menu'} : '';
	my $short_menu = exists $box{'short_menu'} ? $box{'short_menu'} : '';

	%box = %{getCustomizedSettings($xmlfile)}; # immediately get default settings
	$box{'menu'} = $menu if(! ($menu eq '')); 
	$box{'full_menu'} = $full_menu if(! ($full_menu eq '')); 
	$box{'short_menu'} = $short_menu if(! ($short_menu eq '')); 
	
    }
}


if(exists $box{'outfile'}) {

    my $init_outfile = $box{'outfile'}; # for later
    # if windows name, convert it to cywin
    if($WINDOWS_CYGWIN && $box{'outfile'} =~ /\\/) {
	$box{'outfile'} = `cygpath '$box{'outfile'}'`;
	if($box{'outfile'} =~ /^(\S+)\s?/) {
	    $box{'outfile'} = $1;
	}
    } # windows 
    my $outfile = $box{'outfile'} . '.xml';

    if($box{'outfile'} eq '') {
	print " please go back and specify filename for displayed data\n";
	exit(1);
    }
    # check to make sure not same as self and steal full path from xmlfile
    else {
	# steal directory from xml fle
	if(!File::Spec->file_name_is_absolute($outfile)) {
	    my ($vol,$dirs,$files) = File::Spec->splitpath($xmlfile);
	    $outfile = File::Spec->catpath($vol,$dirs,$outfile);
	    if($outfile =~ /^(\S+)\.xml(\.gz)?$/) {
		$box{'outfile'} = $1;
 	    }
	}

	my $index = index($xmlfile, $outfile);
	if((File::Spec->file_name_is_absolute($outfile) && $outfile eq $xmlfile) ||
	   ($index >= 0 && $index + (length $outfile) == (length $xmlfile))) {
	    if($HTML_ORIENTATION && $xmlfile =~ /^(\S+\.)xml(\.gz)?$/) {
		if($SHTML) {
		    print " please go back and specify a filename for displayed data other than $init_outfile.shtml\n";
		}
		else {
		    print " please go back and specify a filename for displayed data other than $init_outfile.htm\n";
		}
	    }
	    else {
		print " please go back and specify a filename for displayed data other than $init_outfile\n";
	    }
	    exit(1);
	}
	if(-e $outfile) {

	    if($HTML_ORIENTATION && $outfile =~ /^(\S+\.)xml(\.gz)?$/) {
		if($SHTML) {
		    print " $init_outfile.shtml already exists.  Please go back and specify an alternative filename for displayed data\n";
		}
		else {
		    print " $init_outfile.htm already exists.  Please go back and specify an alternative filename for displayed data\n";
		}
	    }
	    else {
		print " $init_outfile already exists.  Please go back and specify an alternative filename for displayed data\n";
	    }
	    exit(1);
	}

	my $suffix = $HTML_ORIENTATION ? '.htm' : '.xml';

	$suffix = '.shtml' if($SHTML);
	my $newhtmlfile = $box{'outfile'} . '.htm';
	$newhtmlfile = $box{'outfile'} . $suffix if($SHTML);
	writeXMLFile($box{'outfile'}, \%box, $xslt, $xmlfile); # both temporary xsl file and xmlfile....
	initialize($xslt, $box{'outfile'} . '.xml', $box{'outfile'} . '.xsl', \%box, $newhtmlfile, 0);

	# make local reference
	my $local_datafile = $box{'outfile'} . $suffix;

	if(! $ISB_VERSION) {

	    $local_datafile = $box{'outfile'} . '.xml'; # BSP until shtml fixed

	    # this should be just like after analysis: use local datafile link, but windows name for shtml file

	    if((length $SERVER_ROOT) <= (length $local_datafile) && 
	       index((lc $local_datafile), ($LC_SERVER_ROOT)) == 0) {
		$local_datafile = '/' . substr($local_datafile, (length $SERVER_ROOT));
	    }
	    else {
		die "problem (pr1): $local_datafile is not mounted under webserver root: $SERVER_ROOT\n";
	    }
	    my $windows_outfile = $outfile;
	    if($WINDOWS_CYGWIN) {
		$windows_outfile = `cygpath -w '$outfile'`;
		if($windows_outfile =~ /^(\S+)\s?/) {
		    $windows_outfile = $1;
		}
	    }
	    # use windows path here
	    print " data written to $windows_outfile<br><br>\n";
	    if (!exists $ENV{'WEBSERVER_TMP'}) { # if working in tmpdir, don't give misleading info
		print ' direct your browser to <a target="Win1" href="' .  $local_datafile . '">' . "http://" . $TPPhostname . $local_datafile . '</a>' . "\n\n"; 
	    }
	} # if iis & cygwin
	else { # unix
	    if (!exists $ENV{'WEBSERVER_TMP'}) { # if working in tmpdir, don't give misleading info
		print ' data written to <a target="Win1" href="' .  $local_datafile . '">' . $local_datafile . '</a>', "\n\n";
	    }
	}

    }
}
else {
    if(!$xmlfile) {
	print " No xml file specified; please use the -file option\n";
	&printUsage();
	exit(1);
    }

    if($initiate) {
	initialize($xslt, $xmlfile, $xslfile, \%box, $htmlfile, 0);
	print "\n protein probabilities written to file ";

	if(! $ISB_VERSION) {
	    my $local_ref = $HTML_ORIENTATION ? $htmlfile : $xmlfile;
	    my $windows_ref = $xmlfile;
	    if($WINDOWS_CYGWIN) {
		# get windows name
		$windows_ref = `cygpath -w '$xmlfile'`;
	    }
	    if($windows_ref =~ /^(\S+)\s?/) {
		$windows_ref = $1;
		print $windows_ref;
	    }
	    if((length $SERVER_ROOT) <= (length $local_ref) && 
	       index((lc $local_ref), ($LC_SERVER_ROOT)) == 0) {
		$local_ref = '/' . substr($local_ref, (length $SERVER_ROOT));
		print "\n direct your browser to http://" . $TPPhostname . $local_ref if($HTML_ORIENTATION);
	    }
	    else {
		die "problem (pr2): $local_ref is not mounted under webserver root: $SERVER_ROOT\n";
	    }
	} # if iis & cygwin
	elsif($HTML_ORIENTATION) {
	    print $htmlfile;
	}
	else {
	    print $xmlfile;
	}
	print "\n\n";
    }
    elsif ($EXCEL) {
	print "Writing results to tab-delimited file: $excelfile\n";
	writeTabDelimData($excelfile, $xslt, $xmlfile);
	print "Done!\n";

    }
    elsif ($HTMLGEN) {
	my $outfile = $htmlfile;
	$outfile =~ s/shtml$/html/i;
	print "Writing results to html file: $outfile\n";

	open(HTMLOUT, ">$outfile") or die "cannot open $outfile $!\n";
	my $ fh= select HTMLOUT;
	my $nrows = 0;
	my $ncols = 0;
	unless ($NOGAGGLE) {
	    writeGaggleNameValueData($gaggleNameValueFile, $xslt, $xmlfile);
	    $nrows = writeGaggleNameListData($gaggleNameListFile, $xslt, $xmlfile);
	    $ncols = writeGaggleMatrixData($gaggleMatrixFile, $xslt, $xmlfile);
	}
	writeXSLFile($xslfile, \%box, 0, $nrows, $ncols) if(! $restore_view);
	printHTML($xslt, $xmlfile, $xslfile, \%box);
	select $fh;
	print "Done!\n";

    }
    else {
	if($restore_view) {

	    if(-e $xslfile) {
		$pre_existing_xsl = 1;
		%box = %{readXSLFile($xslfile)};

	    }
	    else {
		%box = %{initialize($xslt, $xmlfile, $xslfile, \%box, $htmlfile, 1)}; # write the xsl file
	    }
	}
	else {
	    %box = %{getCustomizedSettings($xmlfile)} if(exists $box{'restore'} && $box{'restore'} eq 'yes'); # no longer need to keep track of 'restore'
	}
	$go_level = $box{'go_level'} if(exists $box{'go_level'});

	if(exists $box{'excel'} && $box{'excel'} eq 'yes') {
	    writeTabDelimData($excelfile, $xslt, $xmlfile);
	    print "\n"; # write something to prevent cgi timeout
	}

	if(exists $box{'action'} && $box{'action'} eq 'Recompute p-values') {
	    if ($^O eq 'linux') {
		system($TOP_PATH."bin/ASAPRatioPvalueParser $xmlfile");
	    }
	    else {
		system("ASAPRatioPvalueParser $xmlfile");
	    }
	}
	writeGaggleNameValueData($gaggleNameValueFile, $xslt, $xmlfile);
	my $nrows = writeGaggleNameListData($gaggleNameListFile, $xslt, $xmlfile);
	my $ncols = writeGaggleMatrixData($gaggleMatrixFile, $xslt, $xmlfile);
	writeXSLFile($xslfile, \%box, 0, $nrows, $ncols) if(! $restore_view);
	printHTML($xslt, $xmlfile, $xslfile, \%box);
    }
}

sub useXMLFormatLinks {
    (my $file) = @_;
    return $NEW_XML_FORMAT;
}


sub getGoOntology {
(my $proteins, my $go_level) = @_;
# first string together hidden info for ChenWei's program
#return "making pie chart...\n";
# new interface for chen wei
# goOntology <full path curr dir> <web server dir> <level> <prot1> <wt1> <prot2> <wt2> ..
# goOntology <level> <file> <curr dir> <web server dir>

# need to specify the full path name and webserver name for directories
my $dir = '';
if($xmlfile =~ /^(\S+\/)\S+\.xml/) {
    $dir = $1;
}
my $local_dir = $dir;

my $go_file = $dir . 'go_prots.txt';

open(OUT, ">$go_file") or die "cannot open $go_file $!\n";
foreach(keys %{$proteins}) {
    if(/^IPI\:(\S+)\.\d$/) {
	print OUT "$1\t${$proteins}{$_}\n";
    }
    else {
	print OUT "$_\t${$proteins}{$_}\n";
    }
}


close(OUT);


if(! $ISB_VERSION) {

    if(exists $ENV{'WEBSERVER_ROOT'}) {
	# make sure ends with '/'
	my ($serverRoot) = ($ENV{'WEBSERVER_ROOT'} =~ /(\S+)/);
	if ( $serverRoot =~ /\:/ ) {
	    $serverRoot = `cygpath '$serverRoot'`;
	    ($serverRoot) = ($serverRoot =~ /(\S+)/);
	}
	if ($^O eq 'MSWin32' ) {
		$serverRoot =~ s/\\/\//g;  # get those path seps pointing right!
	}
	# make sure ends with '/'
	$serverRoot .= '/' if($serverRoot !~ /\/$/);
	if((length $serverRoot) <= (length $dir)) {
	    $local_dir = '/' . substr($local_dir, (length $serverRoot));
	}else {
	    die "problem (pr3): >$local_dir< is not mounted under webserver root: >$serverRoot<\n";
	}
    }
} # if not ISB version
my $hide = 0;
if($go_level > 100) { # hide
    $go_level = $go_level - 100;
    $hide = 1;
}
if($CALCULATE_PIES) {
    print "<HR/><font color=\"green\">Go Ontology Information: Level $go_level</font><p/>";
    my $command = '/regis/data3/search/akeller/GO_ONTOLOGY/Go.exe ' . $go_level . ' ' . $go_file . ' /regis/data3/search/akeller/GO_ONTOLOGY/';
    $command .= ' HIDE' if($hide);
    system($command);
    unlink($go_file) if(-e $go_file);
# write output to file, which will be hyperlinked at top of output

} # if calc pies


}


sub writeXMLFile {
(my $file, my $boxptr, my $xslt, my $xml) = @_;

my $tempxslfile = $file . '.tmp.xsl';


open(OUT, ">$tempxslfile");
print OUT '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">', "\n";

print OUT '<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>', "\n";


print OUT '<xsl:template match="node() | @*">', "\n";
print OUT '<xsl:copy>', "\n";


print OUT '<xsl:apply-templates select="*[not(self::protx:protein_group or self::protx:protein or self::protx:peptide)] | @*"/>';

print OUT '<xsl:text>' . "\n" . '</xsl:text>';

my $minprob = exists ${$boxptr}{'min_prob'} && ! (${$boxptr}{'min_prob'} eq '') ? ${$boxptr}{'min_prob'} : 0;
my $minntt = exists ${$boxptr}{'min_ntt'} && ! (${$boxptr}{'min_ntt'} eq '') ? ${$boxptr}{'min_ntt'} : 0;

my $maxnmc = exists ${$boxptr}{'max_nmc'} && ! (${$boxptr}{'max_nmc'} eq '') ? ${$boxptr}{'max_nmc'} : -1;
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';
my $exclude_1 = exists ${$boxptr}{'ex1'} && ${$boxptr}{'ex1'} eq 'yes' ? ${$boxptr}{'ex1'} : '';
my $exclude_2 = exists ${$boxptr}{'ex2'} && ${$boxptr}{'ex2'} eq 'yes' ? ${$boxptr}{'ex2'} : '';
my $exclude_3 = exists ${$boxptr}{'ex3'} && ${$boxptr}{'ex3'} eq 'yes' ? ${$boxptr}{'ex3'} : '';

my @inclusions = exists ${$boxptr}{'inclusions'} ? split(' ', ${$boxptr}{'inclusions'}) : ();
my @exclusions = exists ${$boxptr}{'exclusions'} ? split(' ', ${$boxptr}{'exclusions'}) : ();
my @pinclusions = exists ${$boxptr}{'pinclusions'} ? split(' ', ${$boxptr}{'pinclusions'}) : ();
my @pexclusions = exists ${$boxptr}{'pexclusions'} ? split(' ', ${$boxptr}{'pexclusions'}) : ();

# other variables needed?
my $min_asap = exists ${$boxptr}{'min_asap'} && ! ${$boxptr}{'min_asap'} eq '' ? ${$boxptr}{'min_asap'} : 0;
my $max_asap = exists ${$boxptr}{'max_asap'} && ! ${$boxptr}{'max_asap'} eq '' ? ${$boxptr}{'max_asap'} : 0;
my $min_xpress = exists ${$boxptr}{'min_xpress'} && ! ${$boxptr}{'min_xpress'} eq '' ? ${$boxptr}{'min_xpress'} : 0;
my $max_xpress = exists ${$boxptr}{'max_xpress'} && ! ${$boxptr}{'max_xpress'} eq '' ? ${$boxptr}{'max_xpress'} : 0;
my $show_groups = exists ${$boxptr}{'show_groups'} && ! ${$boxptr}{'show_groups'} eq '' ? ${$boxptr}{'show_groups'} : '';
my $min_pepprob = exists ${$boxptr}{'min_pepprob'} && ! ${$boxptr}{'min_pepprob'} eq '' ? ${$boxptr}{'min_pepprob'} : 0;
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ! ${$boxptr}{'filter_asap'} eq '' ? ${$boxptr}{'filter_asap'} : '';

my $show_ggl = exists ${$boxptr}{'show_ggl'} && ${$boxptr}{'show_ggl'} eq 'yes' ? ${$boxptr}{'show_ggl'} : '';

my $filter_xpress = exists ${$boxptr}{'filter_xpress'} ? ${$boxptr}{'filter_xpress'} : '';
my $show_adjusted_asap = (! exists ${$boxptr}{'show_adjusted_asap'} && ! exists ${$boxptr}{'adj_asap'}) || (${$boxptr}{'show_adjusted_asap'} eq 'yes') ?  ${$boxptr}{'show_adjusted_asap'} : '';
#my $show_adjusted_asap = (${$boxptr}{'show_adjusted_asap'} eq 'yes') ?  ${$boxptr}{'show_adjusted_asap'} : '';
my $max_pvalue_display = exists ${$boxptr}{'max_pvalue'} && ! (${$boxptr}{'max_pvalue'} eq '') ? ${$boxptr}{'max_pvalue'} : 1.0;
my $asap_xpress = exists ${$boxptr}{'asap_xpress'} ? ${$boxptr}{'asap_xpress'} : '';
my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';


# add filter_xpress and asap_xpress capabilities here....


# apply-templates select="protx:protein_group"
# SHOW GROUPS
if(! ($show_groups eq '')) {
    print OUT '<xsl:apply-templates select="protx:protein_group[@probability &gt;=\'' . $minprob . '\'';
    print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@ratio_mean &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev &gt;= \'0\'' if(! ($filter_asap eq ''));
    print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'' if(! ($filter_xpress eq ''));

    print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\'' if($min_xpress > 0);
    print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\'' if($max_xpress > 0);

    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\'' if($min_asap > 0);
	print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\'' if($max_asap > 0);

	print OUT ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or ((protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy).'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\') and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\')))' if(! ($asap_xpress eq ''));

    }
    else { # use adjusted values
	print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\'' if($min_asap > 0);
	print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\'' if($max_asap > 0);
	print OUT ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or ((protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\') and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\')))' if(! ($asap_xpress eq ''));

    }
    print OUT ' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'' if($max_pvalue_display < 1.0);
    for(my $e = 0; $e <= $#exclusions; $e++) {
	print OUT ' and not(@group_number=\'' . $exclusions[$e] . '\')';
    }
    print OUT ']"/>';

    if(@inclusions) {
	my $first = 1;
	foreach(@inclusions) {
	    if($first) {
		print OUT '<xsl:apply-templates select="protx:protein_group[@group_number=\'' . $_ . '\'';
		$first = 0;
	    }
	    else {
		print OUT ' or @group_number=\'' . $_ . '\'';
	    }
	}
	print OUT ']"/>';
    } # if have some inclusions
} # show groups
# HIDE GROUPS
else {
    print OUT '<xsl:apply-templates select="protx:protein_group[@probability &gt;=\'' . $minprob . '\'';
    print OUT ' and protx:protein[protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\']' if(! ($filter_asap eq ''));
    print OUT ' and protx:protein[protx:analysis_result[@analysis=\'xpress\'] and protx:XPressRatio/@ratio_mean &gt;= \'0\' and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\']' if(! ($filter_xpress eq ''));
    print OUT ' and protx:protein[protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\']' if($min_xpress > 0);
    print OUT ' and protx:protein[protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\']' if($max_xpress > 0);
    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print OUT ' and protx:protein[protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\']' if($min_asap > 0);
	print OUT ' and protx:protein[protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\']' if($max_asap > 0);
	print OUT ' and (not(protx:protein/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein/protx:analysis_result[@analysis=\'xpress\']) or protx:protein[protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'])' if(! ($asap_xpress eq ''));
    }
    else { # show adjusted
	print OUT ' and protx:protein[protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\']' if($min_asap > 0);
	print OUT ' and protx:protein[protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\']' if($max_asap > 0);
	print OUT ' and (not(protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein/protx:analysis_result[@analysis=\'xpress\']) or protx:protein[protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'])' if(! ($asap_xpress eq ''));
    }
    print OUT ' and protx:protein[protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\]' if($max_pvalue_display < 1.0);
    foreach(@exclusions) {
	print OUT ' and not(@group_number=\'' . $_ . '\')';
    }
    print OUT ']"/>';

if(@inclusions) {
    my $first = 1;
    foreach(@inclusions) {
	if($first) {
	    print OUT '<xsl:apply-templates select="protx:protein_group[@group_number=\'' . $_ . '\'';
	    $first = 0;
	}
	else {
	    print OUT ' or @group_number=\'' . $_ . '\'';
	}
    }
    print OUT ']"/>';
} # if have some inclusions
}
# apply-templates select="protx:protein"
if(! ($show_groups eq '')) {
    print OUT '<xsl:apply-templates select="protx:protein"/>';
}
# HIDE GROUPS
else {

# make sure there are no pexclusions or pinclusions
    print OUT '<xsl:apply-templates select="protx:protein[@probability &gt;=\'' . $minprob . '\'';
    print OUT ' and protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\'' if(! ($filter_asap eq ''));
    print OUT ' and protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'' if($min_xpress > 0);
    print OUT ' and protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'' if($max_xpress > 0);
    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print OUT ' and protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'' if($min_asap > 0);
	print OUT ' and protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'' if($max_asap > 0);
    }
    else { # show adjusted
	print OUT ' and protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'' if($min_asap > 0);
	print OUT ' and protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'' if($max_asap > 0);
    }
    print OUT ' and protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'' if($max_pvalue_display < 1.0);

    foreach(@pexclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print OUT ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	}
    }

    print OUT ']"/>';
    if(@pinclusions > 0) {
	my $first = 1;
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		if($first) {
		    $first = 0;
		    print OUT '<xsl:apply-templates select="protx:protein[(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
		}
		else {
		    print OUT ' or (parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
		}
	    }
	}
	print OUT ']"/>';

    }

}

print OUT '<xsl:apply-templates select="protx:peptide[@nsp_adjusted_probability &gt;= \'' . $min_pepprob . '\'';
print OUT ' and @n_enzymatic_termini &gt;=\''. $minntt . '\'' if($minntt > 0);
print OUT ' and not(@charge=\'1\')' if(! ($exclude_1 eq ''));
print OUT ' and not(@charge=\'2\')' if(! ($exclude_2 eq ''));
print OUT ' and not(@charge=\'3\')' if(! ($exclude_3 eq ''));
print OUT ']"/>';

# apply-templates select="peptide"


print OUT '</xsl:copy>';
print OUT '</xsl:template>', "\n";
print OUT '</xsl:stylesheet>', "\n";
close(OUT);

my $outfile = $file . '.xml'; #'tempfile.xml';


# now compute filter
my $filter = '';
if($minprob > 0) {
    if($show_groups eq '') {
	$filter .= 'min_prot_prob=\'' . $minprob . '\' ';
    }
    else { # group
	$filter .= 'min_group_prob=\'' . $minprob . '\' ';
    }
}
$filter .= 'exclude_illegal_XPRESSRatios=\'Y\' ' if($filter_xpress);
$filter .= 'exclude_illegal_ASAPRatios=\'Y\' ' if($filter_asap);
my $asap_prefix = $show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'} ? '' : 'adj_';
$filter .= 'asap_xpress_consistency=\'Y\' ' if(! ($asap_xpress eq ''));
$filter .= 'min_' . getRatioPrefix($quant_light2heavy) . 'xpress=\'' . $min_xpress . '\' ' if($min_xpress > 0);
$filter .= 'max_' . getRatioPrefix($quant_light2heavy) . 'xpress=\'' . $max_xpress . '\' ' if($max_xpress > 0);
$filter .= $asap_prefix . 'min_' . getRatioPrefix($quant_light2heavy) . 'asap=\'' . $min_asap . '\' ' if($min_asap > 0);
$filter .= $asap_prefix . 'max_' . getRatioPrefix($quant_light2heavy) . 'asap=\'' . $max_asap . '\' ' if($max_asap > 0);
$filter .= 'max_pvalue=\'' . $max_pvalue_display . '\' ' if($max_pvalue_display < 1.0);
$filter .= 'min_pepprob=\'' . $min_pepprob . '\' ' if($min_pepprob > 0);
$filter .= 'minntt=\'' . $minntt . '\' ' if($minntt > 0);
$filter .= 'maxnmc=\'' . $maxnmc . '\' ' if($maxnmc >= 0);
$filter .= 'exclude_1+_peptides=\'Y\' ' if($exclude_1);
$filter .= 'exclude_2+_peptides=\'Y\' ' if($exclude_2);
$filter .= 'exclude_3+_peptides=\'Y\' ' if($exclude_3);
$filter .= 'group_entry_inclusions=\'' . join(',', @inclusions) . '\' ' if(@inclusions > 0);
$filter .= 'group_entry_exclusions=\'' . join(',', @exclusions) . '\' ' if(@exclusions > 0);
$filter .= 'protein_entry_inclusions=\'' . join(',', @pinclusions) . '\' ' if(@pinclusions > 0);
$filter .= 'protein_entry_exclusions=\'' . join(',', @pexclusions) . '\' ' if(@pexclusions > 0);


# if $xmlfile is gzipped, returns tmpfile name, else returns $xmlfile
my $tmp_xml = tpplib_perl::uncompress_to_tmpfile($xml);

if($xslt =~ /xsltproc/) {
    open XALAN, "$xslt $tempxslfile $tmp_xml |" or print "cannot open $xslt\n";;
}
else {
    open XALAN, "$xslt $tmp_xml $tempxslfile |" or print "cannot open $xslt\n";;
}
open(XML, ">$outfile");

my $start = 1;
my $counter = 1;
my $parentfile = $xmlfile;
if($HTML_ORIENTATION && $xmlfile =~ /^(\S+\.)xml(\.gz)?$/) {
    if($SHTML) {
	$parentfile = $1 . 'shtml';
    }
    else {
	$parentfile = $1 . 'htm';
    }
}

# make local reference
my $local_parent = $parentfile;
my $windows_parent = '';
if(! $ISB_VERSION) {
    if((length $SERVER_ROOT) <= (length $local_parent) && 
       index((lc $local_parent), ($LC_SERVER_ROOT)) == 0) {
	$local_parent = '/' . substr($local_parent, (length $SERVER_ROOT));
	if($WINDOWS_CYGWIN) {
	    $windows_parent = `cygpath -w '$outfile'`;
	    if($windows_parent =~ /^(\S+)\s?/) {
		$windows_parent = $1;
	    }
	}
    }
    else {
	die "problem (pr4): $local_parent is not mounted under webserver root: $SERVER_ROOT\n";
    }
} # if iis & cygwin


# check for stylesheet reference
my $xsltproc = $xslt =~ /xsltproc/;
my $doctype = 0;
while(<XALAN>) {
    if($start && /^(\<\?xml\-stylesheet type\=\"text\/xsl\" href\=\")\S+(\"\?\>.*)$/) {
	my $local_xslfile = $file . '.xsl';

	if(! $ISB_VERSION) {
	    if((length $SERVER_ROOT) <= (length $local_xslfile) && 
	       index((lc $local_xslfile), ($LC_SERVER_ROOT)) == 0) {
		$local_xslfile = '/' . substr($local_xslfile, (length $SERVER_ROOT));
	    }
	    else {
		die "problem (pr5): $local_xslfile is not mounted under webserver root: $SERVER_ROOT\n";
	    }
	} # if iis & cygwin
	print XML $1 . $local_xslfile . $2;
	$start = 0;
    }
    elsif($start && ! $doctype && /^\<protx\:protein\_summary/) {
	my $local_xslfile = $file . '.xsl';

	if(! $ISB_VERSION) {
	    if((length $SERVER_ROOT) <= (length $local_xslfile) && 
	       index((lc $local_xslfile), ($LC_SERVER_ROOT)) == 0) {
		$local_xslfile = '/' . substr($local_xslfile, (length $SERVER_ROOT));
	    }
	    else {
		die "problem (pr6): $local_xslfile is not mounted under webserver root: $SERVER_ROOT\n";
	    }
	} # if iis & cygwin
	print XML '<?xml-stylesheet type="text/xsl" href="' . $local_xslfile . '"? xmlns:protx="http://regis-web.systemsbiology.net/protXML">' . "\n";
	print XML;
	$start = 0;
    }
    # make replacements to num predicted correct prots and parent data file, as well as sens / err data
    elsif(/^(.*dataset\_derivation generation\_no\=\")(\d+)(\".*)$/) {
	my $first = $1;
	my $second = $2;
	my $third = $3;
	if(! ($filter eq '')) { # then there is something to do
	    if($xslt =~ /xsltproc/) {
		my $update = $first . ($second+1) . '">';
		my $next = $second + 1;
		my $rest = $third;

		if($third =~ /^(\"\>.*?)(\<data\_filter\s+.*)/) {
		    print XML $first . ($second+1) . $1 . "\n";
		    print XML '<data_filter number="' . ($second+1) . '" parent_file="' . $local_parent;
		    print XML '" windows_parent="' . $windows_parent if($WINDOWS_CYGWIN);
		    print XML '" description="' . $filter . '"/>', "\n";
		    print XML $2 . "\n";
		}
		else {
		    print XML $first . ($second+1) . $third, "\n";
		    # now add current filter
		    print XML '<data_filter number="' . ($second+1) . '" parent_file="' . $local_parent;
		    print XML '" windows_parent="' . $windows_parent if($WINDOWS_CYGWIN);
		    print XML '" description="' . $filter . '"/>', "\n";
		}
	    } # xsltproc
	    else {
		print XML $first . ($second+1) . $third, "\n";
		# now add current filter
		print XML '<data_filter number="' . ($second+1) . '" parent_file="' . $local_parent;
		print XML '" windows_parent="' . $windows_parent if($WINDOWS_CYGWIN);
		print XML '" description="' . $filter . '"/>', "\n";
	    }

	}
	else {
	    print XML;
	}
    }

    elsif($USE_INDEX && /^(.*\<protx\:protein_group.*?group_number\=\")\d+(\".*)$/) {
	print XML $1 . $counter++ . $2;
    }
    else {
	if($start && /DOCTYPE/) {
	    $doctype = 1;
	}
	print XML;
    }
}

close(XML);
close(XALAN);
unlink($tmp_xml) if ($tmp_xml ne $xml); # did we decompress protxml.gz?


$inital_xsl = $counter > $MAX_NUM_ENTRIES;
unlink($tempxslfile) if(-e $tempxslfile);
unlink("$outfile.tmp") if(-e "$outfile.tmp");
${$boxptr}{'restore'} = 'yes'; # for new xsl
${$boxptr}{'xmlfile'} = $file . '.xml';
if(exists $box{'xmlfile'} && $box{'xmlfile'} =~ /^(\S+\.)xml(\.gz)?$/) {
    $xmlfile = $box{'xmlfile'};
    $xslfile = $1 . 'xsl';
    $excelfile = $1 . 'xls';
}

}


sub writeTabDelimData {
    (my $outfile, my $engine, my $xml) = @_;
    my $tempxslfile = $xmlfile . '.tmp.xsl';
    my $tempfile = $xmlfile . '.tmp.xls';

    my $text = exists $box{'text1'} ? $box{'text1'} : '';
    my @select_aas = ();
    if(exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) {
	for(my $k = 0; $k < (length $box{'pep_aa'}); $k++) {
	    my $next = substr($box{'pep_aa'}, $k);
	    if($next =~ /^[a-z,A-Z](\d\d+)/) {
		my $mass = $1;
		push(@select_aas, substr($box{'pep_aa'}, $k, 1) . '[' . $mass . ']');
		$k += (length $mass);
	    }
	    else {
		push(@select_aas, substr($box{'pep_aa'}, $k, 1));
	    }
	}
    }

    unlink($tempxslfile) if(-e $tempxslfile);
    unlink($tempfile) if(-e $tempfile);
    writeXSLFile($tempxslfile, \%box, 1, 0, 0);

    if($xslt =~ /xsltproc/) {
	system("$engine $tempxslfile $xml > $tempfile");
    }
    else {
	system("$engine $xml $tempxslfile > $tempfile");
    }
    open(OUT, ">$outfile") or die "cannot open $outfile $!\n";
    open(IN, "$tempfile") or die "cannot open $tempfile $!\n";
    my $start = 0;
    my $active = 0;
    my $prot_ind = -1;
    my $pep_ind = -1;
    my $first = 1;
    while(<IN>) {
	if(index($_, $start_string_comment) >= 0) {   
	    $start = 1;
	}
	elsif($start) {
	    if(/\S/) {
		$active = 1;
	    }
	    if(/^\<\/form/ || /^\<\/table/) {
		$start = 0;
	    }
	    elsif($active) {
		if(@select_aas > 0 || ! ($text eq '')) {
		    chomp();
		    my @parsed = split('\t');
		    if($first) {
			for(my $p = 0; $p <= $#parsed; $p++) {
			    if($prot_ind == -1 && $parsed[$p] eq 'protein') {
				$prot_ind = $p;
			    }
			    elsif($pep_ind == -1 && $parsed[$p] eq 'peptide sequence') {
				$pep_ind = $p;
			    }
			}
			print OUT "$_\n";
			$first = 0; # done
		    }
		    else {
			my $ok = 1;
			for(my $p = 0; $p <= $#parsed; $p++) {
			    for(my $s = 0; $s <= $#select_aas; $s++) {
				$ok &&= $pep_ind >= @parsed || $pep_ind == -1 || $p != $pep_ind || index($parsed[$p], $select_aas[$s]) >= 0;
			    }
			    $ok &&= $prot_ind >= @parsed || $prot_ind == -1 || $p != $prot_ind || index($parsed[$p], $text) >= 0; # =~ /$text/;
			}
			print OUT "$_\n" if($ok);
		    } # not first
		}
		else {
		    print OUT;
		}
	    }
	}
	elsif(0 && ! $start && /^\<font color\=\"green\"\>(\S.*)\<\/font\>/) {
	    print OUT "$1\n";
	}
    }
    close(IN);
    close(OUT);
    unlink($tempfile) if(-e $tempfile);
    unlink($tempxslfile) if(-e $tempxslfile);

    chmod 0666, $outfile;
}

sub writeGaggleNameListData {
 (my $outfile, my $engine, my $xml) = @_;
 my $tempxslfile = $xmlfile . '.tmp.nl.ggl.xsl';
 my $tempfile = $xmlfile . '.tmp.nl.ggl';

 my $text = exists $box{'text1'} ? $box{'text1'} : '';
 my @select_aas = ();
 if(exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) {
     for(my $k = 0; $k < (length $box{'pep_aa'}); $k++) {
	 my $next = substr($box{'pep_aa'}, $k);
	 if($next =~ /^[a-z,A-Z](\d\d+)/) {
	     my $mass = $1;
	     push(@select_aas, substr($box{'pep_aa'}, $k, 1) . '[' . $mass . ']');
	     $k += (length $mass);
	 }
	 else {
	     push(@select_aas, substr($box{'pep_aa'}, $k, 1));
	 }
     }
 }


 unlink($tempxslfile) if(-e $tempxslfile);
 unlink($tempfile) if(-e $tempfile);
 writeGaggleNameListXSLFile($tempxslfile, \%box);

 if($xslt =~ /xsltproc/) {
     system("$engine $tempxslfile $xml > $tempfile");
 }
 else {
     system("$engine $xml $tempxslfile > $tempfile");
 }

 open(OUT, ">$outfile") or die "cannot open $outfile $!\n";
 open(IN, "$tempfile") or die "cannot open $tempfile $!\n";
 my $nrows = 0;
 my $start = 0;
 my $active = 0;
 my $prot_ind = -1;
 my $pep_ind = -1;
 my $first = 1;
 while(<IN>) {
     if(index($_, $start_string_comment) >= 0) {   
	 $start = 1;
     }
     elsif($start) {
	 if(/\S/) {
	     $active = 1;
	 }
	 if(/^\<\/form/ || /^\<\/table/) {
	     $start = 0;
	 }
	 elsif($active) {
	     if(@select_aas > 0 || ! ($text eq '')) {
		 chomp();
		 my @parsed = split('\t');
		 if($first) {
		     for(my $p = 0; $p <= $#parsed; $p++) {
			 if($prot_ind == -1 && $parsed[$p] eq 'protein') {
			     $prot_ind = $p;
			 }
			 elsif($pep_ind == -1 && $parsed[$p] eq 'peptide sequence') {
			     $pep_ind = $p;
			 }
		     }
		     print OUT "$_\n";
		     $first = 0; # done
		 }
		 else {
		     my $ok = 1;
		     for(my $p = 0; $p <= $#parsed; $p++) {
			 for(my $s = 0; $s <= $#select_aas; $s++) {
			     $ok &&= $pep_ind >= @parsed || $pep_ind == -1 || $p != $pep_ind || index($parsed[$p], $select_aas[$s]) >= 0;
			 }
			 $ok &&= $prot_ind >= @parsed || $prot_ind == -1 || $p != $prot_ind || index($parsed[$p], $text) >= 0; # =~ /$text/;
		     }
		     if( $ok ) { print OUT "$_\n"; } 
		 } # not first
	     } 
	     else {
		 print OUT;
		 $nrows++;
	     }
	 }
     }
     elsif(0 && ! $start && /^\<font color\=\"green\"\>(\S.*)\<\/font\>/) {
	 print OUT "$1\n";
     }
 }

 close(IN);
 close(OUT);
 unlink($tempfile) if(-e $tempfile);
 unlink($tempxslfile) if(-e $tempxslfile);

 chmod 0666, $outfile;
 return $nrows;
}

sub writeGaggleNameValueData {
 (my $outfile, my $engine, my $xml) = @_;
 my $tempxslfile = $xmlfile . '.tmp.nv.ggl.xsl';
 my $tempfile = $xmlfile . '.tmp.nv.ggl';

 my $text = exists $box{'text1'} ? $box{'text1'} : '';
 my @select_aas = ();
 if(exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) {
     for(my $k = 0; $k < (length $box{'pep_aa'}); $k++) {
	 my $next = substr($box{'pep_aa'}, $k);
	 if($next =~ /^[a-z,A-Z](\d\d+)/) {
	     my $mass = $1;
	     push(@select_aas, substr($box{'pep_aa'}, $k, 1) . '[' . $mass . ']');
	     $k += (length $mass);
	 }
	 else {
	     push(@select_aas, substr($box{'pep_aa'}, $k, 1));
	 }
     }
 }


 unlink($tempxslfile) if(-e $tempxslfile);
 unlink($tempfile) if(-e $tempfile);
 writeGaggleNameValueXSLFile($tempxslfile, \%box);

 if($xslt =~ /xsltproc/) {
     system("$engine $tempxslfile $xml > $tempfile");
 }
 else {
     system("$engine $xml $tempxslfile > $tempfile");
 }

 open(OUT, ">$outfile") or die "cannot open $outfile $!\n";
 open(IN, "$tempfile") or die "cannot open $tempfile $!\n";

 my $nrows = 0;
 my $start = 0;
 my $active = 0;
 my $prot_ind = -1;
 my $pep_ind = -1;
 my $first = 1;
 while(<IN>) {
     if(index($_, $start_string_comment) >= 0) {   
	 $start = 1;
     }
     elsif($start) {
	 if(/\S/) {
	     $active = 1;
	 }
	 if(/^\<\/form/ || /^\<\/table/) {
	     $start = 0;
	 }
	 elsif($active) {
	     if(@select_aas > 0 || ! ($text eq '')) {
		 chomp();
		 my @parsed = split('\t');
		 if($first) {
		     for(my $p = 0; $p <= $#parsed; $p++) {
			 if($prot_ind == -1 && $parsed[$p] eq 'protein') {
			     $prot_ind = $p;
			 }
			 elsif($pep_ind == -1 && $parsed[$p] eq 'peptide sequence') {
			     $pep_ind = $p;
			 }
		     }
		     print OUT "$_\n";
		     $first = 0; # done
		 }
		 else {
		     my $ok = 1;
		     for(my $p = 0; $p <= $#parsed; $p++) {
			 for(my $s = 0; $s <= $#select_aas; $s++) {
			     $ok &&= $pep_ind >= @parsed || $pep_ind == -1 || $p != $pep_ind || index($parsed[$p], $select_aas[$s]) >= 0;
			 }
			 $ok &&= $prot_ind >= @parsed || $prot_ind == -1 || $p != $prot_ind || index($parsed[$p], $text) >= 0; # =~ /$text/;
		     }
		     if($ok) { 
			 print OUT "$_\n"; 
		     } 
		 } # not first
	     } 
	     else {
		 print OUT;			
		 $nrows++; 
	     }
	 }
     }
     elsif(0 && ! $start && /^\<font color\=\"green\"\>(\S.*)\<\/font\>/) {
	 print OUT "$1\n";
     }
 }

 close(IN);
 close(OUT);
 unlink($tempfile) if(-e $tempfile);
 unlink($tempxslfile) if(-e $tempxslfile);

 chmod 0666, $outfile;
 return $nrows;

}

sub writeGaggleMatrixData {
 (my $outfile, my $engine, my $xml) = @_;
 my $tempxslfile = $xmlfile . '.tmp.mx.ggl.xsl';
 my $tempfile = $xmlfile . '.tmp.mx.ggl';

 my $text = exists $box{'text1'} ? $box{'text1'} : '';
 my @select_aas = ();
 if(exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) {
     for(my $k = 0; $k < (length $box{'pep_aa'}); $k++) {
	 my $next = substr($box{'pep_aa'}, $k);
	 if($next =~ /^[a-z,A-Z](\d\d+)/) {
	     my $mass = $1;
	     push(@select_aas, substr($box{'pep_aa'}, $k, 1) . '[' . $mass . ']');
	     $k += (length $mass);
	 }
	 else {
	     push(@select_aas, substr($box{'pep_aa'}, $k, 1));
	 }
     }
 }


 unlink($tempxslfile) if(-e $tempxslfile);
 unlink($tempfile) if(-e $tempfile);
 my $ncols = writeGaggleMatrixXSLFile($tempxslfile, \%box);

 if($xslt =~ /xsltproc/) {
     system("$engine $tempxslfile $xml > $tempfile");
 }
 else {
     system("$engine $xml $tempxslfile > $tempfile");
 }

 open(OUT, ">$outfile") or die "cannot open $outfile $!\n";
 open(IN, "$tempfile") or die "cannot open $tempfile $!\n";
 my $start = 0;
 my $active = 0;
 my $prot_ind = -1;
 my $pep_ind = -1;
 my $first = 1;
 while(<IN>) {
     if(index($_, $start_string_comment) >= 0) {   
	 $start = 1;
     }
     elsif($start) {
	 if(/\S/) {
	     $active = 1;
	 }
	 if(/^\<\/form/ || /^\<\/table/) {
	     $start = 0;
	 }
	 elsif($active) {
	     if(@select_aas > 0 || ! ($text eq '')) {
		 chomp();
		 my @parsed = split('\t');
		 if($first) {
		     for(my $p = 0; $p <= $#parsed; $p++) {
			 if($prot_ind == -1 && $parsed[$p] eq 'protein') {
			     $prot_ind = $p;
			 }
			 elsif($pep_ind == -1 && $parsed[$p] eq 'peptide sequence') {
			     $pep_ind = $p;
			 }
		     }
		     print OUT "$_\n";
		     $first = 0; # done
		 }
		 else {
		     my $ok = 1;
		     for(my $p = 0; $p <= $#parsed; $p++) {
			 for(my $s = 0; $s <= $#select_aas; $s++) {
			     $ok &&= $pep_ind >= @parsed || $pep_ind == -1 || $p != $pep_ind || index($parsed[$p], $select_aas[$s]) >= 0;
			 }
			 $ok &&= $prot_ind >= @parsed || $prot_ind == -1 || $p != $prot_ind || index($parsed[$p], $text) >= 0; # =~ /$text/;
		     }
		     print OUT "$_\n" if($ok);
		 } # not first
	     } 
	     else {
		 print OUT;
	     }
	 }
     }
     elsif(0 && ! $start && /^\<font color\=\"green\"\>(\S.*)\<\/font\>/) {
	 print OUT "$1\n";
     }
 }
 close(IN);
 close(OUT);
 unlink($tempfile) if(-e $tempfile);
 unlink($tempxslfile) if(-e $tempxslfile);

 chmod 0666, $outfile;

 return $ncols;
}


sub readXSLFile {
(my $xslfile) = @_;
die "cannot find $xslfile $!\n" if(! -e $xslfile);

my %output = ();
open(XSL, $xslfile);

while(<XSL>) {

    # avoid all checkboxes


    if(/name\=\"mark_aa\"\s+value\=\"(.*?)\"/) {
	$output{'mark_aa'} = $1;
    }
    elsif(/NAME\=\"mark_aa\"\s+VALUE\=\"(.*?)\"/) {
	$output{'mark_aa'} = $1;
    }
    if(/name\=\"text1\"\s+value\=\"(.*?)\"/) {
	$output{'text1'} = $1;
    }
    elsif(/NAME\=\"text1\"\s+VALUE\=\"(.*?)\"/) {
	$output{'text1'} = $1;
    }
    if(/name\=\"pep_aa\"\s+value\=\"(.*?)\"/) {
	$output{'pep_aa'} = $1;
    }
    elsif(/NAME\=\"pep_aa\"\s+VALUE\=\"(.*?)\"/) {
	$output{'pep_aa'} = $1;
    }
    if(/name\=\"go_level\".*?option value\=\"(\d+)\" selected\=\"yes\".*\<\/select\>/) {
	$output{'go_level'} = $1;
    }
    if(/name\=\"show\_groups\"\s+value\=\"(.*?)\"/) {
	$output{'show_groups'} = $1;
    }
    elsif(/NAME\=\"show\_groups\"\s+VALUE\=\"(.*?)\"/) {
	$output{'show_groups'} = $1;
    }
    if(/name\=\"icat\_mode\"\s+value\=\"(.*?)\"/) {
	$output{'icat_mode'} = $1;
    }
    elsif(/NAME\=\"icat\_mode\"\s+VALUE\=\"(.*?)\"/) {
	$output{'icat_mode'} = $1;
    }
    if(/hidden\"\s+name\=\"glyc\_mode\"\s+value\=\"(.*?)\"/) {
	$output{'glyc_mode'} = $1;
    }
    elsif(/HIDDEN\"\s+NAME\=\"glyc\_mode\"\s+VALUE\=\"(.*?)\"/) {
	$output{'glyc_mode'} = $1;
    }
    if(/hidden\"\s+name\=\"glyc\"\s+value\=\"(.*?)\"/) {
	$output{'glyc'} = $1;
    }
    elsif(/HIDDEN\"\s+NAME\=\"glyc\"\s+VALUE\=\"(.*?)\"/) {
	$output{'glyc'} = $1;
    }

}
close(XSL);

return \%output;
}


sub colorAAs2 {
(my $peptide, my $aasptr, my $glyc) = @_;

# old version (pre-TPP)
$peptide =~ s/(N[\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.]?[A-O,Q-Z][\#,\@,\*,0-9,\$,\!,\.]?[S,T])/\<font color\=\"\#FF00FF\"\>$1<\/font\>/g if($glyc);

# new version (TPP)
$peptide =~ s/(N\[?\d?\d?\d?\d?\]?[A-O,Q-Z]\[?\d?\d?\d?\d?\]?[S,T])/\<font color\=\"\#FF00FF\"\>$1<\/font\>/g if($glyc);



my $next;
my $next_alt;
for(my $k = 0; $k <= $#{$aasptr}; $k++) {
    $next = ${$aasptr}[$k];
    $next_alt = $next;
    if($next_alt =~ /^(\S)\[(\d+)\]$/) {
	$next_alt = $1 . '\[' . $2 . '\]';
    }

    if($next eq '1') {
	$peptide =~ s/^n([\@,\*,\#,0-9,\$,\!,\~,\^,\?,\',\[,\],\.]*)/\<font color\=\"red\"\>n$1\<\/font\>/g;
    }
    elsif($next eq '2') {
	$peptide =~ s/c([\@,\*,\#,0-9,\$,\!,\~,\^,\?,\',\[,\],\.]*\])$/\<font color\=\"red\"\>c$1\<\/font\>/g;
    }
    else {
	$peptide =~ s/$next_alt([\@,\*,\#,0-9,\$,\!,\~,\^,\?,\',\[,\],\.]*)/\<font color\=\"red\"\>$next$1\<\/font\>/g;
    }
}
if($DISPLAY_MODS) {
    $peptide =~ s/\[/\<font size\=\"\-2\"\>/g;
    $peptide =~ s/\]/\<\/font\>/g;
}
return $peptide;
}


sub getCustomizedSettings {
(my $xmlfile) = @_;
my %output = ();
my $settingsfile = getCustomizedSettingsFile($xmlfile);
if(! ($settingsfile eq '') && -e $settingsfile) {
    open(SF, $settingsfile);
    my @lines = <SF>;
    close(SF);
    if(@lines > 0) {
	chomp $lines[0];
	my @settings = split(' ', $lines[0]);
	for(my $s = 0; $s <= $#settings; $s++) {
	    if($settings[$s] =~ /^(\S+)\=\'(.*?)\'$/) {
		$output{$1} = $2;
	    }
	}
    }
}
if($ICAT) {
    $output{'icat_mode'} = 'yes';
    if(! exists $output{'mark_aa'} || $output{'mark_aa'} eq '') {
	$output{'mark_aa'} =  'C';
    }
    elsif($output{'mark_aa'} !~ /c/ && $output{'mark_aa'} !~ /C/) {
	$output{'mark_aa'} .= 'C';
    }
}
if($GLYC) {
    $output{'glyc'} = 'yes';
    $output{'glyc_mode'} = 'yes';
}
return \%output;
}

######################################################################################################
# this function must be customized to particular running environments
# given xmlfile name, parses out owner and returns a unique filename on system unique to that owner
sub getCustomizedSettingsFile {
(my $xmlfile) = @_;
my $default_settings = 'settings_prot.txt';
return $SERVER_ROOT . $default_settings if(! $ISB_VERSION);

# have default for non-ISB users
#if($DTD_FILE !~ /akeller/ && $DTD_FILE =~ /^(\S+)ProteinProphet\_v\d\.\d\.dtd/) {
#    return $1 . $default_settings;
#}

my $prefix = '/data3/search/akeller/PROTXML/'; # settings file prefix
my $suffix = '_' . $default_settings; # settings file suffix

# extract user name from xml file name
if($xmlfile =~ /^\/data2?\/search\/([^\/]+)\//) {
    if($1 eq 'guest') {
	if($xmlfile =~ /^\/data2?\/search\/guest\/([^\/]+)\//) {
	    return $prefix . 'g_' . $1 . $suffix;
	}
	else {
	    return '';
	}
    }
    elsif($1 eq 'samd') {
	if($xmlfile =~ /^\/data2?\/search\/samd\/([^\/]+)\//) {
	    return $prefix . 's_' . $1 . $suffix;
	}
	else {
	    return '';
	}
    }
    return $prefix . $1 . $suffix;
}
return ''; # error
}
####################################################################################################################

sub writeCustomizedSettings {
(my $xmlfile, my $use_default, my $boxptr) = @_;
my $settingsfile = getCustomizedSettingsFile($xmlfile);
if(! ($settingsfile eq '')) {
    if($use_default) {
	unlink($settingsfile) if(-e $settingsfile);
    }
    else {
	if(! open(SF, ">$settingsfile")) {
	    print "error: cannot open $settingsfile $!\n";
	    exit(1);
	}
	foreach(keys %{$boxptr}) {
	    print SF "$_='${$boxptr}{$_}' " if(! /^p?incl/ && ! /^p?excl/ && ! /menu/ && ! /custom\_settings/);
	}
	print SF "\n";
	close(SF);
    }
}
}

sub initialize {
(my $xslt, my $xmlfile, my $xslfile, my $boxptr, my $htmlfile, my $restore) = @_;
$boxptr = getCustomizedSettings($xmlfile);

if($SHTML) {
   initialize_shtml($xslt, $xmlfile, $xslfile, $boxptr, $htmlfile, $restore);
}
else {
    initialize_htm($xslt, $xmlfile, $xslfile, $boxptr, $htmlfile);
}
chmod 0666, $xslfile;
return $boxptr;
}

sub initialize_shtml {
    (my $xslt, my $xmlfile, my $xslfile, my $boxptr, my $htmlfile, my $restore) = @_;
    if($HTML_ORIENTATION) {
	writeXSLFile($xslfile, $boxptr, 0, 0, 0);
	return if($restore);

	open(OUT, ">$htmlfile") or die "cannot open $htmlfile $!\n";
	print OUT "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 3.0//EN\">\n";
        if ( $WEBSRVR eq "IIS" ) {
	    print OUT "<!--#exec cgi=\"" . $CGI_HOME . "protxml2html.pl?xmlfile=$xmlfile&restore_view=yes";
        }else {
	    print OUT "<!--#include virtual=\"" . $CGI_HOME . "protxml2html.pl?xmlfile=$xmlfile&restore_view=yes";
        }
	print OUT "\" -->\n";
	close(OUT);
	# permissions
    }
}

sub initialize_htm {
(my $xslt, my $xmlfile, my $xslfile, my $boxptr, my $htmlfile) = @_;
if($HTML_ORIENTATION) {
    $HTML = 1;
    writeXSLFile($xslfile, $boxptr, 0, 0, 0);

    if($xslt =~ /xsltproc/) {
	open HTML, "$xslt $xslfile $xmlfile |";
    }
    else {
	open HTML, "$xslt $xmlfile $xslfile |";
    }
    open(OUT, ">$htmlfile") or die "cannot open $htmlfile $!\n";
    my $index = 0;
    my $single = 0;
	   
    while(<HTML>) {
	chomp();
	($index, $single) = singleLine($index, $single, $_) if($index >= 0);
	if($single) {
	    print OUT "$_";
	}
	else {
	    print OUT "$_\n"; 
	}
    }
    close(OUT);
    close(HTML);
    $HTML = 0;
    writeXSLFile($xslfile, $boxptr, 0, 0, 0);
}
}

sub singleLine {
(my $index, my $position, my $entry) = @_;
my @starts = ('\<HEAD', '\<input type\=\"submit\" value\=\"Write Displayed Data', '\<\/table', '\<br', '\<br', '\<FONT'); #, '\<pre', '\<\!\-\-start\-\-');
my @stops = ('\<table', '\<input', '\<table', '\<\/table>', '\<\/pre', '\<table'); #, '\<FONT', '\<tr');
return if($index < 0 || $index >= @starts);
if($position) {
    if($entry =~ /$stops[$index]/) {
	if($index+1 >= @starts) { # done
	    return (-1, 0);
	}
	else {
	    return ($index+1, 0);
	}
    }
    else {
	return($index, 1); # still keep one line
    }
}
else {
    if($entry =~ /^$starts[$index]/) {
	return ($index, 1);
    }
    else {
	return ($index, 0);
    }
}
}


sub printHTML {
(my $xslt, my $xmlfile, my $xslfile, my $boxptr) = @_;
if(! -e $xmlfile) {
    print "cannot find xml file $xmlfile\n";
    exit(1);
}
if(! -e $xslfile) {
    print "cannot find xsl file $xslfile\n";
    exit(1);
}


my $xml_header = "xml version=";

if(0) {
    unlink("test.html") if(-e "test.html");
    system("$xslt $xmlfile $xslfile > test.html");
    exit(1);
}

if($xslt =~ /xsltproc/) {
    open HTML, "$xslt $xslfile $xmlfile |";
}
else {
    open HTML, "$xslt $xmlfile $xslfile |";
}
my $show_groups = ! exists ${$boxptr}{'show_groups'} || ${$boxptr}{'show_groups'} eq 'show';
my $last = '';
my $next = '';;
my $start = 0;
my $text1 = exists $box{'text1'} && ! ($box{'text1'} eq '');
my $text = $text1 ? $box{'text1'} : '';
my $printout = 0;
my $glyc = (exists $box{'glyc_mode'} && $box{'glyc_mode'} eq 'yes') || (exists $box{'glyc'} && $box{'glyc'} eq 'yes');
my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';

my %unique_peptides = ();
my %total_peptides = ();
my %glyc_peptides = ();
my $show_peps = ! exists ${$boxptr}{'show_peps'} || ${$boxptr}{'show_peps'} eq 'show';
my $discards = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes' ? $checked : '';


my $mark_aa = exists $box{'mark_aa'} ? $box{'mark_aa'} : '';
$mark_aa .= 'C' if($ICAT && ! exists $box{'icat_mode'});
$box{'mark_aa'} = $mark_aa;

my @select_aas = ();
if(exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) {
    for(my $k = 0; $k < (length $box{'pep_aa'}); $k++) {
	my $next = substr($box{'pep_aa'}, $k);
	if($next =~ /^[a-z,A-Z](\d\d+)/) {
	    my $mass = $1;
	    push(@select_aas, substr($box{'pep_aa'}, $k, 1) . '[' . $mass . ']');
	    $k += (length $mass);
	}
	else {
	    push(@select_aas, substr($box{'pep_aa'}, $k, 1));
	}
    }
}


my $color = (exists $box{'mark_aa'} && ! ($box{'mark_aa'} eq '')) || (exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) || $glyc; 
$color = 1;

my @color_aas = ();
if(exists $box{'mark_aa'} && ! ($box{'mark_aa'} eq '')) {
    for(my $k = 0; $k < (length $box{'mark_aa'}); $k++) {
	my $next = substr($box{'mark_aa'}, $k);
	if($next =~ /^[a-z,A-Z](\d\d+)/) {
	    my $mass = $1;
	    push(@color_aas, substr($box{'mark_aa'}, $k, 1) . '[' . $mass . ']');
	    $k += (length $mass);
	}
	else {
	    push(@color_aas, substr($box{'mark_aa'}, $k, 1));
	}
    }
}

my $space = ' ';
my $sort_index = 0;
my $DEBUG = 0;
open(OUT, ">temp.html") if($DEBUG);

my $counter = 0;
my $table_size = 200; #500; # write table subsets so can be displayed by browser as html is passed from xslt
my $table_specs = $RESULT_TABLE_SUF . $RESULT_TABLE_PRE . $RESULT_TABLE;

my $html_pepxml_view = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'PepXMLViewer.cgi?page=1';

my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';

if(useXMLFormatLinks($xmlfile)) {
    if($quant_light2heavy eq 'true') {
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?cgi-bin=' . $CGI_HOME . '&amp;Ref=';
    }
    else { # add ratioType
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?cgi-bin=' . $CGI_HOME . '&amp;ratioType=1&amp;Ref=';
    }
}

my $reject = 0;
my $reset;
my $reset_group;
my $xsltproc = $xslt =~ /xsltproc/; # special way to parse data
my $line_separator = '</tr>';
my $nextline = '';
my $complete_line = 0;


while(<HTML>) { 
    chomp(); 
    # hack to recover +/- sign after xsltproc transform (except for IPI link)
    $_ =~ s/[^MXG]\+\-/\&\#177\;/g if($xsltproc);

    if($xsltproc && $start) {
	$nextline .= $_; # concatenate line
	$complete_line = /$line_separator/;
    }

    if(! $xsltproc || ! $start || $complete_line) {
	$nextline = $_ if(! $start || ! $xsltproc);

	if($discards) {
	    $reset = index($nextline, "name=\"incl") >= 0;
	    $reset ||= index($nextline, "name=\"pincl") >= 0 if($show_groups eq '');
	}
	else {
	    $reset = index($nextline, "name=\"excl") >= 0;
	    $reset ||= index($nextline, "name=\"pexcl") >= 0 if($show_groups eq '');
	}


	if($CALCULATE_PIES && $go_level > 0) {
	    # first check if last line with </HTML>
	    if($nextline =~ /(.*)(\<\/BODY\>.*)/) {
		print $1;
		getGoOntology(\%go_prots, $go_level);
		$nextline = $2;
	    }
	    else {

		# here extract protein names for pies
		my @next_prot_entries = $nextline =~ /Ref\=(IPI.*?)\&amp/g;
		for(my $n = 0; $n <= $#next_prot_entries; $n++) {
		    $go_prots{$next_prot_entries[$n]} = sprintf("%0.2f", 1 / @next_prot_entries);
		}
	    }
	}

	$printout = ! $text1 if($reset);
	if($start && $reset && $text1) {

	    $nextline =~ s/\<\/td\>/$space/g;

	    my $line = $nextline;
	    while($line =~ /(.*)\<.*?\>(.*)/) {
		$line = $1 . $2;
	    }
	    if($text1) {
		$printout = index($line, $box{'text1'}) >= 0;
	    }
	}
	if(! useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?$CGI_HOME.*?PepXMLViewer\.cgi.*?\>\-?\-?\d?\_?)([A-Z,\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\],n,c]+)(.*?)$/) {
	    my $ok = 1;
	    my $first = $1;
	    my $second = $2;
	    my $third = $3;

	    if(@select_aas > 0) {
		for(my $s = 0; $s <= $#select_aas; $s++) {
		    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
		    $ok = 0 if(index($second, $alt) < 0);
		}
	    }
	    print $first, colorAAs2($second, \@color_aas, $glyc), $third if($ok); #$DISPLAY_MODS || $ok);
	    $reject = ! $ok;
	}
	elsif(useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?$CGI_HOME.*?PepXMLViewer\.cgi.*?\>\-?\-?\d?\_?)([A-Z,\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\],n,c]+)(.*?)$/) {
	    my $ok = 1;
	    my $first = $1;
	    my $second = $2;
	    my $third = $3;


	    if(@select_aas > 0) {
		for(my $s = 0; $s <= $#select_aas; $s++) {
#		    print "$second vs $select_aas[$s]\n";
		    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
#		    $ok = 0 if($second !~ /$alt/);
		    $ok = 0 if(index($second, $alt) < 0);
		}
	    }

	    if($show_peps) {
		$unique_peptides{strip($second)}++;
		$total_peptides{$second}++;
		$glyc_peptides{$second}++ if($glyc && $second =~ /(N\[?\d?\d?\d?\d?\]?[A-O,Q-Z]\[?\d?\d?\d?\d?\]?[S,T])/);
	    }

	    print $first, colorAAs2($second, \@color_aas, $glyc), $third if($ok); #DISPLAY_MODS || $ok);

	    $reject = ! $ok;
	}
	elsif($text1 && $printout) {
	    print $nextline;
	    print $start ? "\n" : '';  # otherwise top form + tables look bad (too many <pre> tags!!)
	}
	elsif(! $start || $printout) {
	    print $nextline;
	    print $start ? "\n" : '';  # otherwise top form + tables look bad (too many <pre> tags!!)
	}

	$nextline = '' if($xsltproc); # reset
    }
    elsif($reject && /^\<\/tr\>\s?$/) { }
    elsif($reject && /^\<tr\>\s?$/) {
	$reject = 0;
    }


    if(! $start && index($_, $start_string_comment) >= 0) {
	$start = 1;
    }

} # while 


# last entry
if($CALCULATE_PIES && $go_level > 0) {
    # first check if last line with </HTML>
    if($nextline =~ /(.*)(\<\/BODY\>.*)/) {
	print $1;
	getGoOntology(\%go_prots, $go_level);
	$nextline = $2;
    }
    else {

	# here extract protein names for pies
	my @next_prot_entries = $nextline =~ /Ref\=(IPI.*?)\&amp/g;
	for(my $n = 0; $n <= $#next_prot_entries; $n++) {
	    $go_prots{$next_prot_entries[$n]} = sprintf("%0.2f", 1 / @next_prot_entries);
	}
    }
}



# last entry
if(! useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?$CGI_HOME.*?PepXMLViewer\.cgi.*?\>\-?\-?\d?\_?)([A-Z,\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\],n,c]+)(.*?)$/) {
    my $ok = 1;
    my $first = $1;
    my $second = $2;
    my $third = $3;

    if(@select_aas > 0) {
	for(my $s = 0; $s <= $#select_aas; $s++) {
	    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
	    $ok = 0 if($second !~ /$alt/);
	}
    }
    print $first, colorAAs2($second, \@color_aas, $glyc), $third if($ok);
    $reject = ! $ok;
    if($show_peps) {
	$unique_peptides{strip($second)}++;
	$total_peptides{$second}++;
	$glyc_peptides{$second}++ if($glyc && $second =~ /(N\[?\d?\d?\d?\d?\]?[A-O,Q-Z]\[?\d?\d?\d?\d?\]?[S,T])/);
    }
}
elsif(useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?$CGI_HOME.*?PepXMLViewer\.cgi.*?\>\-?\-?\d?\_?)([A-Z,\#,\@,\*,\.,\[,\],n,c]+)(.*?)$/) {


    my $ok = 1;
    my $first = $1;
    my $second = $2;
    my $third = $3;

    if(@select_aas > 0) {
	for(my $s = 0; $s <= $#select_aas; $s++) {
	    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
	    $ok = 0 if(index($second, $alt) < 0);
	}
    }
    print $first, colorAAs2($second, \@color_aas, $glyc), $third if($ok);
    $reject = ! $ok;
}
elsif($text1 && $printout) {
    print "$nextline\n";
}
elsif(! $start || $printout) {
    print "$nextline\n";
}


close(HTML);

if($show_peps) {
    if($glyc) {
	printf "<HR/><center><font color=\"red\"><b>%d peptides with NXS/T motif, %d total peptides, %d unique stripped peptides</b></font></center>", scalar keys %glyc_peptides, scalar keys %total_peptides, scalar keys %unique_peptides;
    }
    else {
	printf "<HR/><center><font color=\"red\"><b>%d total peptides, %d unique stripped peptides</b></font></center>", scalar keys %total_peptides, scalar keys %unique_peptides;
    }

}


close(OUT) if($DEBUG);

}


# checkes whether new version with xml input needing new cgi links
sub isXML_INPUT {
(my $xmlfile) = @_;
open(XML, $xmlfile) or die "cannot open $xmlfile $!\n";
while(<XML>) {
    if(/\<protein\_summary\_header/) {
	close(XML);
	if(/XML\_INPUT/) {
	    return 1;
	}
	else {
	    return 0;
	}
    }
}
close(XML);
die "error: no protein_summary_header found in $xmlfile\n";
}


sub writeXSLFile {
(my $xfile, my $boxptr, my $tab_delim, my $nrows, my $ncols) = @_;

if(! open(XSL, ">$xfile")) {
    print " cannot open $xfile: $!\n";
    exit(1);
}
print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">', "\n";
my $tab = '<xsl:value-of select="$tab"/>';
my $newline = '<br/><xsl:value-of select="$newline"/>';
$newline = '<xsl:value-of select="$newline"/>' if($tab_delim >= 1);
my $nonbreakline = '<xsl:value-of select="$newline"/>';
my $newlinespace = '<p/>';
my $doubleline = $newline . $newline;
my $space = '&#160';

my $num_cols = 3; # first & last


# just in case read recently from customized
$ICAT = 1 if(exists ${$boxptr}{'icat_mode'} && ${$boxptr}{'icat_mode'} eq 'yes');
$GLYC = 1 if(exists ${$boxptr}{'glyc_mode'} && ${$boxptr}{'glyc_mode'} eq 'yes');

# DEPRECATED: restore now fixed at 0 (taken care of up front)
my $restore = 0; 

my @minscore = (exists ${$boxptr}{'min_score1'} && ! (${$boxptr}{'min_score1'} eq '') ? ${$boxptr}{'min_score1'} : 0, 
		exists ${$boxptr}{'min_score2'} && ! (${$boxptr}{'min_score2'} eq '') ? ${$boxptr}{'min_score2'} : 0, 
		exists ${$boxptr}{'min_score3'} && ! (${$boxptr}{'min_score3'} eq '') ? ${$boxptr}{'min_score3'} : 0);

my $minprob = exists ${$boxptr}{'min_prob'} && ! (${$boxptr}{'min_prob'} eq '') ? ${$boxptr}{'min_prob'} : 0;
$minprob = $MIN_PROT_PROB if(! $HTML && $inital_xsl);

my $min_asap = exists ${$boxptr}{'min_asap'} && ! (${$boxptr}{'min_asap'} eq '') ? ${$boxptr}{'min_asap'} : 0;
my $max_asap = exists ${$boxptr}{'max_asap'} && ! (${$boxptr}{'max_asap'} eq '') ? ${$boxptr}{'max_asap'} : 0;
my $min_xpress = exists ${$boxptr}{'min_xpress'} && ! (${$boxptr}{'min_xpress'} eq '') ? ${$boxptr}{'min_xpress'} : 0;
my $max_xpress = exists ${$boxptr}{'max_xpress'} && ! (${$boxptr}{'max_xpress'} eq '') ? ${$boxptr}{'max_xpress'} : 0;

my $sort = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'yes';
${$boxptr}{'pep_aa'} = uc ${$boxptr}{'pep_aa'} if(exists ${$boxptr}{'pep_aa'});
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';
${$boxptr}{'mark_aa'} = uc ${$boxptr}{'mark_aa'} if(exists ${$boxptr}{'mark_aa'});
my $mark_aa = exists ${$boxptr}{'mark_aa'} && ! (${$boxptr}{'mark_aa'} eq '') ? ${$boxptr}{'mark_aa'} : '';
my $minntt = exists ${$boxptr}{'min_ntt'} && ! (${$boxptr}{'min_ntt'} eq '') ? ${$boxptr}{'min_ntt'} : 0;
my $min_pepprob = exists ${$boxptr}{'min_pep_prob'} && ! (${$boxptr}{'min_pep_prob'} eq '') ? ${$boxptr}{'min_pep_prob'} : 0;
my $maxnmc = exists ${$boxptr}{'max_nmc'} && ! (${$boxptr}{'max_nmc'} eq '') ? ${$boxptr}{'max_nmc'} : -1;

my @inclusions = exists ${$boxptr}{'inclusions'} ? split(' ', ${$boxptr}{'inclusions'}) : ();
my @exclusions = exists ${$boxptr}{'exclusions'} ? split(' ', ${$boxptr}{'exclusions'}) : ();
my @pinclusions = exists ${$boxptr}{'pinclusions'} ? split(' ', ${$boxptr}{'pinclusions'}) : ();
my @pexclusions = exists ${$boxptr}{'pexclusions'} ? split(' ', ${$boxptr}{'pexclusions'}) : ();
if(exists ${$boxptr}{'clear'} && ${$boxptr}{'clear'} eq 'yes') {
    @inclusions = ();
    @exclusions = ();
    @pinclusions = ();
    @pexclusions = ();
}

my $exclude_1 = exists ${$boxptr}{'ex1'} && ${$boxptr}{'ex1'} eq 'yes' ? $checked : '';
my $exclude_2 = exists ${$boxptr}{'ex2'} && ${$boxptr}{'ex2'} eq 'yes' ? $checked : '';
my $exclude_3 = exists ${$boxptr}{'ex3'} && ${$boxptr}{'ex3'} eq 'yes' ? $checked : '';

my $show_ggl = exists ${$boxptr}{'show_ggl'} && ${$boxptr}{'show_ggl'} eq 'yes' ? $checked : '';


my $peptide_prophet_check1 = 'count(protx:peptide_prophet_summary) &gt; \'0\'';
my $peptide_prophet_check2 = 'count(parent::node()/protx:peptide_prophet_summary) &gt; \'0\'';

my $discards_init = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes';
my $discards = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes' ? $checked : '';

my $table_space = '&#160;';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
    $table_spacer = '<xsl:text>     </xsl:text>';
}

my $asap_xpress = exists ${$boxptr}{'asap_xpress'} && ${$boxptr}{'asap_xpress'} eq 'yes' ? $checked : '';
my $show_groups = ! exists ${$boxptr}{'show_groups'} || ${$boxptr}{'show_groups'} eq 'show' ? $checked : '';
my $hide_groups = $show_groups eq '' ? $checked : '';

my $show_annot = ! exists ${$boxptr}{'show_annot'} || ${$boxptr}{'show_annot'} eq 'show' ? $checked : '';
my $hide_annot = $show_annot eq '' ? $checked : '';
my $show_peps = ! exists ${$boxptr}{'show_peps'} || ${$boxptr}{'show_peps'} eq 'show' ? $checked : '';
my $hide_peps = $show_peps eq '' ? $checked : '';

my $show_adjusted_asap = (! exists ${$boxptr}{'show_adjusted_asap'} && ! exists ${$boxptr}{'adj_asap'}) || (${$boxptr}{'show_adjusted_asap'} eq 'yes') ? $checked : '';

my $max_pvalue_display = exists ${$boxptr}{'max_pvalue'} && ! (${$boxptr}{'max_pvalue'} eq '') ? ${$boxptr}{'max_pvalue'} : 1.0;

my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';
my $glyc = exists ${$boxptr}{'glyc'} && ${$boxptr}{'glyc'} eq 'yes' ? $checked : '';

if(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'classic') {
    ${$boxptr}{'index'} = 1;
    ${$boxptr}{'prob'} = 2;
    ${$boxptr}{'spec_name'} = 3;
    ${$boxptr}{'neutral_mass'} = 4;
    ${$boxptr}{'massdiff'} = 5;
    ${$boxptr}{'sequest_xcorr'} = 6;
    ${$boxptr}{'sequest_delta'} = 7;
    ${$boxptr}{'sequest_spscore'} = -1;
    ${$boxptr}{'sequest_sprank'} = 8;
    ${$boxptr}{'matched_ions'} = 9;
    ${$boxptr}{'protein'} = 10;
    ${$boxptr}{'alt_prots'} = 11;
    ${$boxptr}{'peptide'} = 12;
    ${$boxptr}{'num_tol_term'} = -1;
    ${$boxptr}{'num_missed_cl'} = -1;
}
elsif(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'default') {
    ${$boxptr}{'weight'} = -1;
    ${$boxptr}{'peptide_sequence'} = -1;
    ${$boxptr}{'nsp_adjusted_probability'} = -1;
    ${$boxptr}{'initial_probability'} = -1;
    ${$boxptr}{'n_tryptic_termini'} = -1;
    ${$boxptr}{'n_sibling_peptides_bin'} = -1;
    ${$boxptr}{'n_instances'} = -1;
    ${$boxptr}{'peptide_group_designator'} = -1;
}
if(exists ${$boxptr}{'annot_order'} && ${$boxptr}{'annot_order'} eq 'default') {
    ${$boxptr}{'ensembl'} = -1;
    ${$boxptr}{'trembl'} = -1;
    ${$boxptr}{'swissprot'} = -1;
    ${$boxptr}{'refseq'} = -1;
    ${$boxptr}{'locus_link'} = -1;
}


# now add on new ones
foreach(keys %{$boxptr}) {
    if(/^excl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    if($inclusions[$i] == $1) {
		@inclusions = @inclusions[0..$i-1, $i+1..$#inclusions]; # delete it from inclusions
		$done = 1;
		$i = @inclusions;
		# cancel all previous pexclusions with same parent
		my $next_ex = $1;
		for(my $p = 0; $p <= $#pinclusions; $p++) {
		    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
			@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_ex = $1;
	push(@exclusions, $next_ex) if(! $done); # add to exclusions
	# cancel all previous pinclusions with same parent
	for(my $p = 0; $p <= $#pinclusions; $p++) {
	    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
		@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
	    }
	}

    }
    elsif(/^incl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    if($exclusions[$e] == $1) {
		@exclusions = @exclusions[0..$e-1, $e+1..$#exclusions]; # delete it from inclusions
		$done = 1;
		$e = @exclusions;
		# cancel all previous pexclusions with same parent
		my $next_in = $1;
		for(my $p = 0; $p <= $#pexclusions; $p++) {
		    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
			@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_in = $1;
	push(@inclusions, $next_in) if(! $done); # add to inclusions
	# cancel all previous pexclusions with same parent
	for(my $p = 0; $p <= $#pexclusions; $p++) {
	    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
		@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
	    }
	}
    }
}


# now add on new ones
foreach(keys %{$boxptr}) {

    if(/^pexcl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#pinclusions; $i++) {
	    if($pinclusions[$i] == $1) {
		@pinclusions = @pinclusions[0..$i-1, $i+1..$#pinclusions]; # delete it from inclusions
		$done = 1;
		$i = @pinclusions;
	    }
	}
	push(@pexclusions, $1) if(! $done); # add to exclusions
    }
    elsif(/^pincl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    if($pexclusions[$e] == $1) {
		@pexclusions = @pexclusions[0..$e-1, $e+1..$#pexclusions]; # delete it from inclusions
		$done = 1;
		$e = @pexclusions;
	    }
	}
	push(@pinclusions, $1) if(! $done); # add to inclusions
    }
}


my $exclusions = join(' ', @exclusions);
my $inclusions = join(' ', @inclusions);
my $pexclusions = join(' ', @pexclusions);
my $pinclusions = join(' ', @pinclusions);

my %parent_excls = ();
my %parent_incls = ();
foreach(@pexclusions) {
    if(/^(\d+)[a-z,A-Z]$/) {
	$parent_excls{$1}++;
    }
}
foreach(@pinclusions) {
    if(/^(\d+)([a-z,A-Z])$/) {
	if(exists $parent_incls{$1}) {
	    push(@{$parent_incls{$1}}, $2);
	}
	else {
	    my @next = ($2);
	    $parent_incls{$1} = \@next;
	}
    }
}

my $full_menu = (exists ${$boxptr}{'menu'} && ${$boxptr}{'menu'} eq 'full') || 
    (exists ${$boxptr}{'full_menu'} && ${$boxptr}{'full_menu'} eq 'yes');

my $short_menu = exists ${$boxptr}{'short_menu'} && ${$boxptr}{'short_menu'} eq 'yes';
$full_menu = 0 if($short_menu); # it takes precedence

my @minscore_display = ($minscore[0] > 0 ? $minscore[0] : '',$minscore[1] > 0 ? $minscore[1] : '',$minscore[2] > 0 ? $minscore[2] : '');
my $minprob_display = $minprob > 0 ? $minprob : '';
my $minntt_display = $minntt > 0 ? $minntt : '';
my $maxnmc_display = $maxnmc >= 0 ? $maxnmc : '';
my $min_asap_display = $min_asap > 0 ? $min_asap : '';
my $max_asap_display = $max_asap > 0 ? $max_asap : '';
my $min_xpress_display = $min_xpress > 0 ? $min_xpress : '';
my $max_xpress_display = $max_xpress > 0 ? $max_xpress : '';
my $min_pepprob_display = $min_pepprob > 0 ? $min_pepprob : '';

my $asap_display = $initiate || exists ${$boxptr}{'asap_display'} ? $checked : '';
my $xpress_display =  $initiate || exists ${$boxptr}{'xpress_display'} ? $checked : '';

print XSL '<xsl:variable name="tab"><xsl:text>&#x09;</xsl:text></xsl:variable>', "\n";
print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
print XSL '</xsl:text></xsl:variable>';

print XSL '<xsl:variable name="libra_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="ref_db" select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/>';
print XSL '<xsl:variable name="asap_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="xpress_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="source_files" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="source_files_alt" select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/>';
print XSL '<xsl:variable name="organism"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@organism"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@organism)">UNKNOWN</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="reference_isotope"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"><xsl:value-of select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope)">UNSET</xsl:if></xsl:variable>';


## JMT: global vars for protein name alphabetization
#

# case conversion
print XSL "\n\n";
print XSL '<xsl:variable name="lcletters">' . "\n";
print XSL '  abcdefghijklmnopqrstuvwxyz' . "\n";
print XSL '</xsl:variable>' . "\n";


print XSL '<xsl:variable name="ucletters">' . "\n";
print XSL '  ABCDEFGHIJKLMNOPQRSTUVWXYZ' . "\n";
print XSL '</xsl:variable>' . "\n";

# build fast lookups of main and indist. protein names, and for
# subsumed proteins, for each protein group
print XSL '<xsl:key name="proteinName"' . "\n";
print XSL '         match="/protx:protein_summary/protx:protein_group/protx:protein"' . "\n";
print XSL '	    use="../@group_number" />' . "\n";
  
print XSL '<xsl:key name="indistProteinNames"' . "\n";
print XSL '	   match="/protx:protein_summary/protx:protein_group/protx:protein/protx:indistinguishable_protein"' . "\n";
print XSL '	   use="../../@group_number" />' . "\n";

#query by a protein name, returns set of subsumed proteins
print XSL '<xsl:key name="subsumedProteinNames"' . "\n";
print XSL '  	    match="/protx:protein_summary/protx:protein_group/protx:protein"' . "\n";
print XSL '	    use="@subsuming_protein_entry" />' . "\n";

#
## end JMT


my %display = ();
my %display_order = ();
my %register_order = ();
my %reg_annot_order = ();
my %display_annot_order = ();
my %header = ();
my %default_order = ();
my %tab_display = ();
my %tab_header = ();
my %annot_display = ();
my %annot_order = ();
my %annot_tab_display = ();

my $header_pre = '<font color="brown"><i>';
my $header_post = '</i></font>';

$display{'protein'} = '<xsl:value-of select="@protein"/><xsl:for-each select="protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';
$header{'protein'} = 'protein';

## JMT
#
#
# alphabetize the list of protein and indistinguishable proteins

#$tab_display{'protein'} = '<xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text>,</xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';

$tab_display{'protein'} =  '';
$tab_display{'protein'} .= '<xsl:for-each select="parent::node() | parent::node()/protx:indistinguishable_protein">';
$tab_display{'protein'} .= '  <xsl:sort select="translate(@protein_name,$ucletters,$lcletters)" />';

$tab_display{'protein'} .= '  <xsl:if test="position() &gt; \'1\'">';
$tab_display{'protein'} .= '    <xsl:text>,</xsl:text>';
$tab_display{'protein'} .= '  </xsl:if>';
$tab_display{'protein'} .= '  <xsl:value-of select="@protein_name"/>';
$tab_display{'protein'} .= '</xsl:for-each>';
#
## JMT

$tab_display{'protein'} .= $tab . $TPPhostname . $CGI_HOME . 'comet-fastadb.cgi?Ref=<xsl:value-of select="parent::node()/@protein_name"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Db=<xsl:value-of select="$ref_db"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Pep=<xsl:value-of select="parent::node()/@unique_stripped_peptides"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,' . $TPPhostname . $CGI_HOME . 'comet-fastadb.cgi?Ref=<xsl:value-of select="@protein_name"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Db=<xsl:value-of select="$ref_db"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Pep=<xsl:value-of select="parent::node()/@unique_stripped_peptides"/></xsl:for-each>';
$header{'protein'} = 'protein' . $tab . 'protein link';


$default_order{'protein'} = -1;

$display{'coverage'} = '<xsl:value-of select="@percent_coverage"/>';
$header{'coverage'} = 'percent coverage';
$tab_display{'coverage'} = '<xsl:value-of select="parent::node()/@percent_coverage"/>';
$default_order{'coverage'} = -1;

$display{'molweight'} = '<xsl:value-of select="protx:parameter[@name=\'mol_weight\']/@value"/>';
$default_order{'molweight'} = 0.1;
$tab_header{'molweight'} = 'protein molecular weight';
$tab_display{'molweight'} = '<xsl:if test="parent::node()/protx:parameter[@name=\'mol_weight\']"><xsl:value-of select="parent::node()/protx:parameter[@name=\'mol_weight\']/@value"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">;<xsl:if test="protx:parameter[@name=\'mol_weight\']"><xsl:value-of select="protx:parameter[@name=\'mol_weight\']/@value"/></xsl:if></xsl:for-each>';


$display{'protlen'} = '<xsl:value-of select="protx:parameter[@name=\'prot_length\']/@value"/>';
$default_order{'protlen'} = 0.2;
$tab_header{'protlen'} = 'protein length';
$tab_display{'protlen'} = '<xsl:if test="parent::node()/protx:parameter[@name=\'prot_length\']"><xsl:value-of select="parent::node()/protx:parameter[@name=\'prot_length\']/@value"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">;<xsl:if test="protx:parameter[@name=\'prot_length\']"><xsl:value-of select="protx:parameter[@name=\'prot_length\']/@value"/></xsl:if></xsl:for-each>';

$display{'annotation'} = '<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$header{'annotation'} = 'annotation';
$tab_display{'annotation'} = '<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$default_order{'annotation'} = -1;


# add here the cgi info for peptide
my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
my $html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
if(useXMLFormatLinks($xmlfile)) {
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?xslt=' . $xslt . '&amp;gi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;Ref=';
	$html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';
	$html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;';
	$html_peptide_lead3 .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
	$html_peptide_lead3 .= 'glyc=Y&amp;' if($glyc);
	$html_peptide_lead3 .= 'libra={$libra_quant}&amp;';
	$html_peptide_lead3 .= 'Ref=';

	$html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep2}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';


}
my $html_peptide_mid = '&amp;Infile=';
my $html_pepxml_view = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'PepXMLViewer.cgi?page=1&amp;requiredPeptideText=';
my $html_pepxml_mid = '&amp;xmlFileName=';
my $html_pepxml_mid2 = '&amp;requiredSpectrumText=';



if($DISPLAY_MODS) {

    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_pepxml_view . '{$mypep}' . $html_pepxml_mid . '{$myinputfiles}"><xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_pepxml_view . '{$mypep}' . $html_pepxml_mid . '{$myinputfiles}"><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if></td>';


}
else {
    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_peptide_lead . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:value-of select="@peptide_sequence"/></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_peptide_lead . '{$mypep}' . $html_peptide_mid . '{$myinputfiles}"<xsl:value-of select="@peptide_sequence"/></A></xsl:if></td>';
}

my $display_ind_peptide_seq = '<td class="indist_pep">--' . $html_pepxml_view . '{$mypep}' . $html_pepxml_mid . '{$myinputfiles2}' . $html_pepxml_mid2 . '\.{$mycharge2}$">' . '<xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></td>';



$header{'peptide_sequence'} = '<td>' . $header_pre . 'peptide sequence' . $header_post . '</td>';
$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:value-of select="@peptide_sequence"/><xsl:for-each select="indistinguishable_peptide">,<xsl:value-of select="@peptide_sequence"/></xsl:for-each>';
$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence';

#print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';
#print XSL '<xsl:variable name="database" select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/>';

$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if>' . $tab . $TPPhostname . $CGI_HOME . 'peptidexml_html2.pl?PepMass=<xsl:value-of select="@calc_neutral_pep_mass"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>StdPep=<xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>MassError=' . $MOD_MASS_ERROR . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>xslt=' . $xslt . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>cgi-bin=' . $CGI_HOME . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>ratioType=<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>';

$tab_display{'peptide_sequence'} .= 'mark_aa=' . $mark_aa . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if(! ($mark_aa eq ''));
$tab_display{'peptide_sequence'} .= 'glyc=Y<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if($glyc);
$tab_display{'peptide_sequence'} .= 'Ref=<xsl:value-of select="@charge"/>_<xsl:value-of select="@peptide_sequence"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Infile=<xsl:value-of select="$source_files_alt"/>';

$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence' . $tab . 'peptide link';

$default_order{'peptide_sequence'} = 2;


my $wt_header_js = '<A HREF="javascript:wtlink(';
my $wt_suf = '</A>';


$tab_display{'is_nondegen_evidence'} = '<xsl:value-of select="@is_nondegenerate_evidence"/>';
$tab_header{'is_nondegen_evidence'} = 'is nondegenerate evidence';
$default_order{'is_nondegen_evidence'} = 0.5;

$display{'weight'} = '<td><xsl:if test="@is_nondegenerate_evidence = \'Y\'"><font color="#990000">*</font></xsl:if></td><td>' . $wt_header_js . '\'{$mypep}\',\'{$mycharge}\',\'{$PepMass}\',\'{$StdPep}\')">' . '<nobr>wt-<xsl:value-of select="@weight"/><xsl:text> </xsl:text></nobr>' . $wt_suf . '</td>';
$header{'weight'} = '<td></td><td>' . $header_pre . 'weight' . $header_post . '</td>';
$tab_display{'weight'} = '<xsl:value-of select="@weight"/>';
$default_order{'weight'} = 1;

$display{'nsp_adjusted_probability'} = '<td><xsl:if test="@is_contributing_evidence = \'Y\'"><font COLOR="#FF9933"><xsl:value-of select="@nsp_adjusted_probability"/></font></xsl:if><xsl:if test="@is_contributing_evidence = \'N\'"><xsl:value-of select="@nsp_adjusted_probability"/></xsl:if></td>';
$header{'nsp_adjusted_probability'} = '<td>' . $header_pre . 'nsp adj prob' . $header_post . '</td>';
$tab_display{'nsp_adjusted_probability'} = '<xsl:value-of select="@nsp_adjusted_probability"/>';
$tab_header{'nsp_adjusted_probability'} = 'nsp adjusted probability';
$default_order{'nsp_adjusted_probability'} = 3;

$display{'initial_probability'} = '<td><xsl:value-of select="@initial_probability"/></td>';
$header{'initial_probability'} = '<td>' . $header_pre . 'init prob' . $header_post . '</td>';
$tab_display{'initial_probability'} = '<xsl:value-of select="@initial_probability"/>';
$tab_header{'initial_probability'} = 'initial probability';
$default_order{'initial_probability'} = 4;

$display{'num_tol_term'} = '<td><xsl:value-of select="@n_enzymatic_termini"/></td>';
$header{'num_tol_term'} = '<td>' . $header_pre . 'ntt' . $header_post . '</td>';
$tab_display{'num_tol_term'} = '<xsl:value-of select="@n_enzymatic_termini"/>';
$tab_header{'num_tol_term'} = 'n tol termini';
$default_order{'num_tol_term'} = 5;

my $nsp_pre_js = '<A HREF="javascript:nsplink(\'{$mypep}\',\'{$mycharge}\',\'{$nspbin}\',\'{$nspval}\',\'{$myprots}\')">';
my $tempnsp = '<td><xsl:if test="@n_sibling_peptides">' . $nsp_pre_js . '<xsl:value-of select="@n_sibling_peptides"/></A></xsl:if>
<xsl:if test="not(@n_sibling_peptides)"><xsl:value-of select="@n_sibling_peptides_bin"/></xsl:if></td>';
$display{'n_sibling_peptides_bin'} = $tempnsp;

$header{'n_sibling_peptides_bin'} = '<td>' . $header_pre . 'nsp<xsl:if test="not(protx:peptide/@n_sibling_peptides)"> bin</xsl:if>' . $header_post . '</td>';
$tab_display{'n_sibling_peptides_bin'} = '<xsl:value-of select="@n_sibling_peptides_bin"/>';
$tab_header{'n_sibling_peptides_bin'} = 'n sibling peptides bin';
$default_order{'n_sibling_peptides_bin'} = 6;

$display{'peptide_group_designator'} = '<td><xsl:if test="@peptide_group_designator"><font color="#DD00DD"><xsl:value-of select="@peptide_group_designator"/>-<xsl:value-of select="@charge"/></font></xsl:if></td>';
$header{'peptide_group_designator'} = '<td>' . $header_pre . 'pep grp ind' . $header_post . '</td>';
$tab_display{'peptide_group_designator'} = '<xsl:value-of select="@peptide_group_designator"/>';
$tab_header{'peptide_group_designator'} = 'peptide group designator';
$default_order{'peptide_group_designator'} = 8;

# we only want this column in the tab-delimited output, so no need to define $display and $header...
$tab_display{'is_evidence'} = '<xsl:value-of select="@is_contributing_evidence"/>';
$tab_header{'is_evidence'} = 'is evidence';
$default_order{'is_evidence'} = 10;

$header{'group_number'} = 'entry no.';
$tab_display{'group_number'} = '<xsl:value-of select="parent::node()/parent::node()/@group_number"/><xsl:if test="count(parent::node()/parent::node()/protx:protein) &gt; \'1\'"><xsl:value-of select="parent::node()/@group_sibling_id"/></xsl:if>';
$default_order{'group_number'} = -1;
$display{'group_number'} = '';
$tab_header{'group_number'} = 'entry no.';

$annot_display{'description'} = '<xsl:if test="protx:annotation/@protein_description"><xsl:if test="$organism = \'UNKNOWN\'"><font color="green"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if><xsl:if test="not($organism = \'UNKNOWN\')"><font color="#008080"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if>' . $table_space . ' </xsl:if>';

$annot_order{'description'} = -1;
$annot_tab_display{'description'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@protein_description"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if></xsl:for-each>';

$header{'description'} = 'description';

my $flybase_header = '<a TARGET="Win1" href="http://flybase.bio.indiana.edu/.bin/fbidq.html?';

$tab_header{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'">flybase' . $tab . '</xsl:if>';
$annot_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="protx:annotation/@flybase"><xsl:variable name="flybase" select="protx:annotation/@flybase"/>' . $flybase_header . '{$flybase}"><font color="green">Flybase:<xsl:value-of select="$flybase"/></font></a>' . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'flybase'} = 9;
$annot_tab_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@flybase"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@flybase"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';

my $ipi_header_js = '<A HREF="javascript:ipi(\'';
my $ipi_mid = '\')">';
my $ipi_suf = '</A>';
$annot_display{'ipi'} = '<font color="green">&gt;</font><xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="protx:annotation/@ipi_name"><xsl:variable name="ipi" select="protx:annotation/@ipi_name"/>' . $ipi_header_js . '{$ipi}' . $ipi_mid . '<font color="green">IPI:<xsl:value-of select="$ipi"/></font>' . $ipi_suf . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'ipi'} = -1;
$annot_tab_display{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ipi_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ipi_name"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';

$header{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')">ipi' . $tab . '</xsl:if>';

my $embl_header_js = '<A HREF="javascript:embl(';
my $embl_suf = '</A>';

$annot_display{'ensembl'} = '<xsl:if test="protx:annotation/@ensembl_name"><xsl:variable name="org" select="$organism"/><xsl:variable name="ensembl" select="protx:annotation/@ensembl_name"/>' . $embl_header_js . '\'{$ensembl}\',\'{$org}\')"><font color="green">E:<xsl:value-of select="$ensembl"/></font>' . $embl_suf . $table_space . ' </xsl:if>';
$annot_order{'ensembl'} = 3;
$annot_tab_display{'ensembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ensembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ensembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'ensembl'} = 'ensembl' . $tab;

$annot_display{'trembl'} = '<xsl:if test="protx:annotation/@trembl_name"><font color="green">Tr:<xsl:value-of select="protx:annotation/@trembl_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'trembl'} = 4;
$annot_tab_display{'trembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@trembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@trembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'trembl'} = 'trembl' . $tab;

$annot_display{'swissprot'} = '<xsl:if test="protx:annotation/@swissprot_name"><font color="green">Sw:<xsl:value-of select="protx:annotation/@swissprot_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'swissprot'} = 5;
$annot_tab_display{'swissprot'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@swissprot_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@swissprot_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'swissprot'} = 'swissprot' . $tab;

$annot_display{'refseq'} = '<xsl:if test="protx:annotation/@refseq_name"><font color="green">Ref:<xsl:value-of select="protx:annotation/@refseq_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'refseq'} = 6;
$annot_tab_display{'refseq'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@refseq_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@refseq_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'refseq'} = 'refseq' . $tab;

my $locus_header = '<a TARGET="Win1" href="http://www.ncbi.nlm.nih.gov/LocusLink/LocRpt.cgi?l=';
my $locus_suf = '</a>';

$annot_display{'locus_link'} = '<xsl:if test="protx:annotation/@locus_link_name"><xsl:variable name="loc" select="protx:annotation/@locus_link_name"/>' . $locus_header . '{$loc}' . '"><font color="green">LL:<xsl:value-of select="$loc"/></font>' . $locus_suf . $table_space . ' </xsl:if>';
$annot_order{'locus_link'} = 7;
$annot_tab_display{'locus_link'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@locus_link_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@locus_link_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'locus_link'} = 'locus link' . $tab;




my $asap_file_pre = '';
my $asap_proph_suf = '_prophet.bof';
my $asap_pep_suf = '_peptide.bof';
my $asap_proph = '';
my $asap_pep = '';
if($xfile =~ /^(\S+\/)/) { # assume in same directory
    $asap_proph = $1 . 'ASAPRatio_prophet.bof';
    $asap_pep = $1 . 'ASAPRatio_peptide.bof';
    $asap_file_pre = $1 . 'ASAPRatio';
}


my $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . 'xli/ASAPRatio_lstProRatio.cgi?orgFile=';
my $asap_header_mid = '.orig&amp;proBofFile=' . $asap_file_pre . '{$filextn}' . $asap_proph_suf . '&amp;pepBofFile=' . $asap_file_pre . '{$filextn}' . $asap_pep_suf . '&amp;proIndx=';


my $plusmn = '&#177;';
$plusmn = ' +-' if($xslt =~ /xsltproc/);

my $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'xpress-prophet.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light={$heavy2light}">';

my $xpress_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $xpress_link;
if(useXMLFormatLinks($xmlfile)) {
    $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light=' . $xpress_ratio_type;
    $xpress_pre .= '&amp;mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
    $xpress_pre .= '">'; #{$heavy2light}">';

    $xpress_link = $TPPhostname . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>protein=<xsl:value-of select="$mult_prot"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>peptide_string=<xsl:value-of select="$peptide_string"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>ratio=<xsl:value-of select="$xratio"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>stddev=<xsl:value-of select="$xstd"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>num=<xsl:value-of select="$xnum"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlfile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>min_pep_prob=<xsl:value-of select="$min_pep_prob"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>source_files=<xsl:value-of select="$source_files"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>heavy2light=' . $xpress_ratio_type;
    $xpress_link .= '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
}

my $xpress_suf = '</a>';

$num_cols = 3;



$display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'"><td width="250"><xsl:if test="protx:analysis_result[@analysis=\'libra\']">LIBRA<xsl:text> </xsl:text>(<xsl:value-of select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>)<xsl:for-each select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><br/><xsl:value-of select="@mz"/>:<xsl:text> </xsl:text><xsl:value-of select="@ratio"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="@error"/><xsl:text> </xsl:text></xsl:for-each></xsl:if></td></xsl:if>';

$tab_display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\' and parent::node()/protx:analysis_result[@analysis=\'libra\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>' . $tab . '<xsl:for-each select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><xsl:value-of select="@ratio"/>' . $tab . '<xsl:value-of select="@error"/>' . $tab . '</xsl:for-each></xsl:if>';

$header{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'">LIBRA number peptides' . $tab . '<xsl:for-each select="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']/protx:libra_summary/protx:fragment_masses">LIBRA <xsl:value-of select="@mz"/> ratio' . $tab . 'LIBRA <xsl:value-of select="@mz"/> error' . $tab . '</xsl:for-each></xsl:if>';



if($xpress_display eq $checked) {

    $display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'xpress\']">XPRESS';
    if(! ($quant_light2heavy eq 'true')) {
	$display{'xpress'} .= '(H/L)';
    }
    $display{'xpress'} .= ': ' . $xpress_pre . '<xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>(<xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>)</xsl:if>' . $xpress_suf . '</xsl:if><xsl:if test="not(protx:analysis_summary[@analysis=\'xpress\'])">' . $table_spacer . '</xsl:if></td></xsl:if>';

    $tab_display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'xpress\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>' . $tab . $xpress_link . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'xpress\'])">'  . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';

    $header{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'">xpress';
    if(! ($quant_light2heavy eq 'true')) {
	$header{'xpress'} .= '(H/L)';
    }
    $header{'xpress'} .= ' ratio mean' . $tab . 'xpress<xsl:if test="not($reference_isotope = \'UNSET\')"><xsl:if test="$reference_isotope=\'light\'"> (H/L)</xsl:if></xsl:if> stdev' . $tab . 'xpress num peptides' . $tab . 'xpress link' . $tab. '</xsl:if>';
} # if display xpress

my $NEW_ASAP_CGI = 1;

my $asap_display_cgi = 'asap-prophet-display.cgi';
if(useXMLFormatLinks($xmlfile)) {
    $asap_display_cgi = 'ASAPCGIDisplay.cgi';
}
my $asap_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $asap_link;
if($NEW_ASAP_CGI) {
    $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '&amp;xmlFile=' . $xmlfile . '&amp;group_no={$prot_number}&amp;';
    $asap_header_pre .= 'markAA=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
    $asap_header_pre .= 'protein=';

    $asap_link = $TPPhostname . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlFile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>';
    $asap_link .= 'markAA=' . $mark_aa . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>' if(! ($mark_aa eq ''));
    $asap_link .= 'protein=<xsl:value-of select="$mult_prot"/>';
}



my $asap_header_mid2 = '&amp;ratioType=0">';

my $asap_header_suf = '</A>';
my $pvalue_header_pre = '<a target="Win1" href="';

my $pvalue_header_suf = '</a>';
if($asap_display eq $checked) {
    if($NEW_ASAP_CGI) {

	# first display regular ratio no matter what
	$display{'asapratio'} = '<xsl:if test="$asap_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']">ASAPRatio';
	if(! ($quant_light2heavy eq 'true')) {
	    $display{'asapratio'} .= '(H/L)';
	}
	$display{'asapratio'} .= ': ' . $asap_header_pre . '{$mult_prot}">';
	$display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/></xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	# now add on the adjusted only if desired and present
	if ($show_adjusted_asap ne '') {
	    $display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\']">[<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_standard_dev"/>]</xsl:if>';
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><td width="200"><xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if>';

  
	}
	else {
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if>';

	}

    }
    else { # old format
	if($show_adjusted_asap eq '') {
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/></xsl:if><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	    }
	    else { # display adjsuted
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="400"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . ' [<xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_standard_dev"/>]';
	    }
	    $display{'asapratio'} .= '</xsl:if></td><td width="200"><xsl:if test="protx:ASAPRatio">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:ASAPRatio/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if><xsl:if test="not(/protx:protein_summary/protx:ASAP_pvalue_analysis_summary)"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . '</xsl:if></td></xsl:if></xsl:if>';
    } # if old version
 

    $tab_display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>' . $tab . $asap_link . $tab . '</xsl:if><xsl:if test="count(parent::node()/protx:analysis_result[@analysis=\'asapratio\'])= \'0\'">' . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab .'N_A' . $tab . '</xsl:if></xsl:if>';
    
    if(! ($show_adjusted_asap eq '')) { # display adjusted
	$tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev"/>' . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\'])">' .'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';
    }
    $tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></xsl:if>' . $tab . $TPPhostname . '<xsl:value-of select="$pvalpngfile"/>' . $tab .'</xsl:if>';

    $header{'asapratio'} = '<xsl:if test="count(protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">ratio mean' . $tab . 'ratio stdev' . $tab . 'ratio num peps' . $tab . 'asap ratio link'. $tab;
    if(! ($show_adjusted_asap eq '')) {
	$header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">adjusted ratio mean' . $tab . 'adjusted ratio stdev' . $tab . '</xsl:if>';
    }
    $header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">pvalue' . $tab . 'pvalue link'. $tab. '</xsl:if></xsl:if>';
} # if display asapratio info

my $reference = '$group_number' ; #$show_groups eq '' ? '$parental_group_number' : '$group_number';
my $gn = $show_groups eq '' ? '<xsl:value-of select="parent::node()/@group_number"/>' : '<xsl:value-of select="@group_number"/>';
if($discards) {

    $display{'group_number'} .= '<input type="checkbox" name="incl{' . $reference . '}" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@exclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
else {

    $display{'group_number'} .= '<input type="checkbox" name="excl';
    if($show_groups eq '') {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    else {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    $display{'group_number'} .= '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@inclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
$display{'group_number'} .= '<a name="{' . $reference . '}"/>' if($HTML);
$display{'group_number'} .= '<xsl:text> </xsl:text>';

$display{'prot_number'} = '';
if($discards) {

    $display{'prot_number'} .= '<input type="checkbox" name="pincl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pexclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pexclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pexclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
else {

    $display{'prot_number'} .= '<input type="checkbox" name="pexcl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pinclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pinclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pinclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
$display{'prot_number'} .= '<xsl:text> </xsl:text>';

$display{'n_instances'} = '<td><xsl:value-of select="@n_instances"/></td>';
$header{'n_instances'} = '<td>' . $header_pre . 'total' . $header_post . '</td>';
$tab_display{'n_instances'} = '<xsl:value-of select="@n_instances"/>';
$default_order{'n_instances'} = 7;
$tab_header{'n_instances'} = 'n instances';


foreach(keys %display) {
    $display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}

foreach(keys %annot_display) {
    $display_annot_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $reg_annot_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}


# test it out privately
if(0 && scalar keys %display_order > 0) {
    open(TEMP, ">temp.out");
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	print TEMP $display{$_}, "\n";
    }
    close(TEMP);
}

print XSL "\n";
# define tab and newline here


# protein_summary template
print XSL "\n\n\n";
print XSL '<xsl:template match="protx:protein_summary">', "\n";

    printCountProtsXSL($boxptr);
print XSL '<xsl:variable name="database3" select="protx:protein_summary_header/@reference_database"/>';
 
print XSL '<HTML>', "\n";
print XSL '<HEAD><TITLE>ProteinProphet protXML Viewer (' . $TPPVersionInfo . ')</TITLE>', "\n";
print XSL '<STYLE TYPE="text/css" media="screen">', "\n";
print XSL '    div.visible {', "\n";
print XSL '    display: inline;', "\n";
print XSL '    white-space: nowrap;', "\n";
print XSL '    }', "\n";
print XSL '    div.hidden {', "\n";
print XSL '    display: none;', "\n";
print XSL '    }', "\n";
print XSL '    results {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.peptide {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.indist_pep {', "\n";
print XSL '	font-size: 10pt;', "\n";
print XSL '    }', "\n";
print XSL '    indist_pep_mod {', "\n";
print XSL '	font-size: 8pt;', "\n";
print XSL '    }', "\n";
print XSL '</STYLE>', "\n";

# ToDo: get rid of xslt and xmlinput in url vars?
print XSL '<SCRIPT TYPE="text/javascript">', "\n";
print XSL '    var cgihome = "' . $CGI_HOME . '";', "\n";
print XSL '    var xmlfile = "' . $xmlfile  . '";', "\n";

print XSL '    function protlink( protein, peptides ) {', "\n";
print XSL '       var baselink = cgihome + "comet-fastadb.cgi?Db=<xsl:value-of select="$database3"/>";', "\n";
print XSL '	  baselink = baselink + "&amp;N-Glyco=1";', "\n" if ($glyc);
print XSL '	  baselink = baselink + "&amp;MarkAA=' . $mark_aa . '";', "\n" if(! ($mark_aa eq ''));
print XSL '	  baselink = baselink + "&amp;Ref=" + protein;', "\n";
print XSL '	  baselink = baselink + "&amp;Pep=" + peptides;', "\n";
print XSL '	  var link = encodeURI(baselink);', "\n";
print XSL '	  window.open(link,"Win1");', "\n";
print XSL '      }', "\n";

print XSL '    function wtlink( peptide, charge, pepmass, modpep ) {', "\n";
print XSL '       var baselink = "' . $CGI_HOME . 'prot_wt_xml.pl?xmlfile=" + xmlfile;', "\n";
print XSL '	  baselink = baselink + "&amp;cgi-home=" + cgihome;', "\n";
print XSL '	  baselink = baselink + "&amp;quant_light2heavy=' . $quant_light2heavy . '";', "\n";
print XSL '	  baselink = baselink + "&amp;modpep=" + modpep;', "\n";
print XSL '	  baselink = baselink + "&amp;pepmass=" + pepmass;', "\n";
print XSL '       baselink = baselink + "&amp;xml_input=1";', "\n" if(! $DISTR_VERSION && $NEW_XML_FORMAT);
print XSL '	  baselink = baselink + "&amp;glyc=1";', "\n" if ($glyc);
print XSL '	  baselink = baselink + "&amp;mark_aa=' . $mark_aa . '";', "\n" if(! ($mark_aa eq ''));
print XSL '	  baselink = baselink + "&amp;peptide=" + peptide;', "\n";
print XSL '	  baselink = baselink + "&amp;charge=" + charge;', "\n";
print XSL '	  var link = encodeURI(baselink);', "\n";
print XSL '	  window.open(link,"Win1");', "\n";
print XSL '      }', "\n";

print XSL '    function nsplink( peptide, charge, nspbin, nspval, prots ) {', "\n";
print XSL '       var baselink = "' . $CGI_HOME . 'show_nspbin.pl?xmlfile=" + xmlfile;', "\n";
print XSL '	  baselink = baselink + "&amp;nsp_bin=" + nspbin;', "\n";
print XSL '	  baselink = baselink + "&amp;nsp_val=" + nspval;', "\n";
print XSL '	  baselink = baselink + "&amp;charge=" + charge;', "\n";
print XSL '	  baselink = baselink + "&amp;pep=" + peptide;', "\n";
print XSL '	  baselink = baselink + "&amp;prot=" + prots;', "\n";
print XSL '	  var link = encodeURI(baselink);', "\n";
print XSL '	  window.open(link,"Win1");', "\n";
print XSL '      }', "\n";

print XSL '    function embl( ensemblname, organism ) {', "\n";
print XSL '       var baselink = "http://www.ensembl.org/";', "\n";
print XSL '       baselink = baselink + organism;', "\n";
print XSL '       baselink = baselink + "/protview?peptide=";', "\n";
print XSL '	  baselink = baselink + ensemblname;', "\n";
print XSL '	  var link = encodeURI(baselink);', "\n";
print XSL '	  window.open(link,"Win1");', "\n";
print XSL '      }', "\n";

print XSL '    function ipi( ipiaccession ) {', "\n";
print XSL '       var baselink = "http://srs.ebi.ac.uk/cgi-bin/wgetz?-id+m_RJ1KrMXG+-e+[IPI-acc:";', "\n";
print XSL '	  baselink = baselink + ipiaccession + "]";', "\n";
print XSL '	  var link = encodeURI(baselink);', "\n";
print XSL '	  window.open(link,"Win1");', "\n";
print XSL '      }', "\n";

print XSL '</SCRIPT>', "\n";

print XSL '</HEAD>', "\n";
print XSL '<BODY BGCOLOR="white" OnLoad="self.focus()"><PRE>', "\n";

print XSL '<table width="100%" border="3" BGCOLOR="#AAAAFF" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;"><tr><td align="center">', "\n";
print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";
if($HTML) {
    print XSL '<input type="submit" value="Restore Last View" style="background-color:#FFFF88;"/>', "\n";
    print XSL '<input type="hidden" name="restore_view" value="yes"/>', "\n";
}
else {
    print XSL '<input type="submit" value="Restore Original"/>', "\n";
    print XSL '<input type="hidden" name="restore" value="yes"/>', "\n";
}
    print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
    print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
    print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);



    print XSL '</form>';
    print XSL '</td><td align="center">';
    print XSL '<pre>ProteinProphet<sup><font size="3">&#xAE;</font></sup> protXML Viewer (' . $TPPVersionInfo . ')</pre>A.Keller   2.23.05</td>';

my $sort_none = ! exists ${$boxptr}{'sort'} || ${$boxptr}{'sort'} eq 'none' ?  $checked : '';
my $sort_xcorr = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xcorr' ? $checked : '';
my $sort_prob = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'prob' ? $checked : '';
my $sort_spec = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spec' ? $checked : '';
my $sort_pep = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'peptide' ? $checked : '';
my $sort_prot = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'protein' ? $checked : '';
my $sort_cov = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'coverage' ? $checked : '';
my $sort_peps = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'numpeps' ? $checked : '';
my $sort_spec_ids = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spectrum_ids' ? $checked : '';

my $sort_pvalue = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'pvalue' ? $checked : '';
my $text1 = exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq '') ? ${$boxptr}{'text1'} : '';
my $sort_asap_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_desc' ? $checked : '';
my $sort_asap_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_asc' ? $checked : '';
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ${$boxptr}{'filter_asap'} eq 'yes' ? $checked : '';
my $filter_xpress = exists ${$boxptr}{'filter_xpress'} && ${$boxptr}{'filter_xpress'} eq 'yes' ? $checked : '';
my $sort_xpress_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_desc' ? $checked : '';
my $sort_xpress_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_asc' ? $checked : '';
my $exclude_degens = exists ${$boxptr}{'no_degens'} && ${$boxptr}{'no_degens'} eq 'yes' ? $checked : '';

# show sens error info (still relevant for filtered dataset)
my $show_sens = exists ${$boxptr}{'senserr'} && ${$boxptr}{'senserr'} eq 'show' ? $checked : '';
my $eligible = ($filter_asap eq '' && $min_asap == 0 && $max_asap == 0 && (! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') && @exclusions == 0 && @inclusions == 0 && @pexclusions == 0 && @pexclusions == 0 && $filter_xpress eq '' && $min_xpress == 0 && $max_xpress == 0 && $asap_xpress eq '');
my $show_tot_num_peps = ! exists ${$boxptr}{'tot_num_peps'} || ${$boxptr}{'tot_num_peps'} eq 'show' ? $checked : '';
my $show_num_unique_peps = ! exists ${$boxptr}{'num_unique_peps'} || ${$boxptr}{'num_unique_peps'} eq 'show' ? $checked : '';
my $show_pct_spectrum_ids = ! exists ${$boxptr}{'pct_spectrum_ids'} || ${$boxptr}{'pct_spectrum_ids'} eq 'show' ? $checked : '';

my $suffix = $HTML_ORIENTATION ? '.htm' : '.xml';
$suffix = '.shtml' if($SHTML);

# write output xml
print XSL '<td align="center"><form method="GET" target="Win1" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";

print XSL '<pre>';
print XSL '<input type="submit" value="Write Displayed Data Subset to File" /><pre>' . $nonbreakline . '</pre>';
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";

print XSL '<input type="hidden" name="ex1" value="yes"/>', "\n" if(! ($exclude_1 eq ''));
print XSL '<input type="hidden" name="ex2" value="yes"/>', "\n" if(! ($exclude_2 eq ''));
print XSL '<input type="hidden" name="ex3" value="yes"/>', "\n" if(! ($exclude_3 eq ''));
print XSL '<input type="hidden" name="show_ggl" value="yes"/>', "\n" if(! ($show_ggl eq ''));
print XSL '<input type="hidden" name="text1" value="' . ${$boxptr}{'text1'} . '"/>' if(exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq ''));
print XSL '<input type="hidden" name="min_prob" value="' . $minprob . '"/>' if($minprob > 0);
print XSL '<input type="hidden" name="min_score1" value="' . $minscore[0] . '"/>' if($minscore[0] > 0);
print XSL '<input type="hidden" name="min_score2" value="' . $minscore[1] . '"/>' if($minscore[1] > 0);
print XSL '<input type="hidden" name="min_score3" value="' . $minscore[2] . '"/>' if($minscore[2] > 0);
print XSL '<input type="hidden" name="min_ntt" value="' . $minntt . '"/>' if($minntt > 0);
print XSL '<input type="hidden" name="max_nmc" value="' . $maxnmc . '"/>' if($maxnmc >= 0);
print XSL '<input type="hidden" name="pep_aa" value="' . $pep_aa . '"/>' if(! ($pep_aa eq ''));
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>' if(! ($inclusions eq ''));
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>' if(! ($exclusions eq ''));
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>' if(! ($pinclusions eq ''));
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>' if(! ($pexclusions eq ''));
print XSL '<input type="hidden" name="filter_asap" value="yes"/>' if(! ($filter_asap eq ''));
print XSL '<input type="hidden" name="filter_xpress" value="yes"/>' if(! ($filter_xpress eq ''));
print XSL '<input type="hidden" name="min_pepprob" value="' . $min_pepprob . '"/>' if(! ($min_pepprob eq ''));
print XSL '<input type="hidden" name="show_groups" value="yes"/>' if(! ($show_groups eq ''));
print XSL '<input type="hidden" name="min_xpress" value="' . $min_xpress . '"/>' if($min_xpress > 0);
print XSL '<input type="hidden" name="max_xpress" value="' . $max_xpress . '"/>' if($max_xpress > 0);
print XSL '<input type="hidden" name="min_asap" value="' . $min_asap . '"/>' if($min_asap > 0);
print XSL '<input type="hidden" name="max_asap" value="' . $max_asap . '"/>' if($max_asap > 0);
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="show_adjusted_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="adj_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="max_pvalue" value="' . $max_pvalue_display . '"/>' if($max_pvalue_display < 1.0);
print XSL '<input type="hidden" name="asap_xpress" value="yes"/>' if(! ($asap_xpress eq ''));
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

print XSL 'file name: <input type="text" name="outfile" value="" size="20" maxlength="100"/>' . $suffix . '</pre>', "\n";
print XSL '</form></td></tr></table>';

print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl"><table width="100%" border="3" BGCOLOR="#AAAAFF">';
print XSL '<tr><td align="left" valign="center"><pre><input type="checkbox" name="show_ggl" value="yes" ' . $show_ggl . '/><b>Enable Gaggle Broadcast</b><A TARGET="Win1" HREF="http://tools.proteomecenter.org/wiki/index.php?title=Software:Firegoose%2C_Gaggle%2C_and_PIPE"><IMG BORDER="0" SRC="'. $HELP_DIR. 'images/qMark.png"/></A></pre></td></tr><tr><td><pre>';

if($discards) {
    print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
}
else {
    print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
}

print XSL $table_spacer . '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_sens_err.pl?xmlfile=' . $xmlfile;
print XSL '&amp;' if(! $DISTR_VERSION);

print XSL '">Sensitivity/Error Info</a></xsl:if><xsl:if test="protx:dataset_derivation/@generation_no &gt;\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_dataset_derivation.pl?xmlfile=' . $xmlfile . '">Dataset Derivation Info</a></xsl:if>';;

print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'more_anal.pl?xmlfile=' . $xmlfile ;
print XSL '&amp;shtml=yes' if($SHTML);
print XSL '&amp;helpdir=' . $HELP_DIR;
print XSL '">More Analysis Info</a>';
print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'show_help.pl?help_dir=' . $HELP_DIR . '">Help</a>';
print XSL $newline;



print XSL '<xsl:text> </xsl:text>';
print XSL 'sort by: <input type="radio" name="sort" value="none" ' . $sort_none . '/>index';
print XSL '<input type="radio" name="sort" value="prob" ' . $sort_prob, '/>probability';
print XSL ' <input type="radio" name="sort" value="protein" ' . $sort_prot, '/>protein';
print XSL ' <input type="radio" name="sort" value="coverage" ' . $sort_cov, '/>coverage';

print XSL '<xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids)"> <input type="radio" name="sort" value="numpeps" ' . $sort_peps, '/>num peps</xsl:if>';
print XSL '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids"> <input type="radio" name="sort" value="spectrum_ids" ' . $sort_spec_ids, '/>share of spectrum ids</xsl:if>';
print XSL '<xsl:if test="$xpress_quant &gt; \'0\' and count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"> ';
print XSL $newline . '<xsl:text>          </xsl:text>';
print XSL '</xsl:if>';

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"> ';
print XSL ' <input type="radio" name="sort" value="xpress_desc" ' . $sort_xpress_desc, '/>xpress desc';
print XSL ' <input type="radio" name="sort" value="xpress_asc" ' . $sort_xpress_asc, '/>xpress asc';
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';
print XSL ' <input type="radio" name="sort" value="asap_desc" ' . $sort_asap_desc, '/>asap desc';
print XSL ' <input type="radio" name="sort" value="asap_asc" ' . $sort_asap_asc, '/>asap asc';
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"> <input type="radio" name="sort" value="pvalue" ' . $sort_pvalue, '/>pvalue</xsl:if>';
print XSL '</xsl:if>';

print XSL $newline;

print XSL '<xsl:text> </xsl:text>min probability: <INPUT TYPE="text" NAME="min_prob" VALUE="' . $minprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
# pick one of the following
print XSL $newline;
print XSL '<xsl:text> </xsl:text>protein groups: <input type="radio" name="show_groups" value="show" ' . $show_groups . '/>show  ';
print XSL '<input type="radio" name="show_groups" value="hide" ' . $hide_groups . '/>hide  ';

print XSL '   annotation: <input type="radio" name="show_annot" value="show" ' . $show_annot . '/>show  ';
print XSL '<input type="radio" name="show_annot" value="hide" ' . $hide_annot . '/>hide  ';
print XSL '   peptides: <input type="radio" name="show_peps" value="show" ' . $show_peps . '/>show  ';
print XSL '<input type="radio" name="show_peps" value="hide" ' . $hide_peps . '/>hide  ';
print XSL $newline;

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o XPRESS Ratio: <input type="checkbox" name="filter_xpress" value="yes" ' . $filter_xpress . '/>';
print XSL '  min XPRESS Ratio: <INPUT TYPE="text" NAME="min_xpress" VALUE="', $min_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max XPRESS Ratio: <INPUT TYPE="text" NAME="max_xpress" VALUE="', $max_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'">  ASAPRatio consistent: <input type="checkbox" name="asap_xpress" value="yes" ' . $asap_xpress . '/></xsl:if>';

print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o ASAPRatio: <input type="checkbox" name="filter_asap" value="yes" ' . $filter_asap . '/>';
print XSL '  min ASAPRatio: <INPUT TYPE="text" NAME="min_asap" VALUE="', $min_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max ASAPRatio: <INPUT TYPE="text" NAME="max_asap" VALUE="', $max_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
my $alt_max = $max_pvalue_display < 1.0 ? $max_pvalue_display : '';

print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">  max pvalue: <INPUT TYPE="text" NAME="max_pvalue" VALUE="', $alt_max, '" SIZE="3" MAXLENGTH="8"/>  adjusted: <input type="checkbox" name="show_adjusted_asap" value="yes" ' . $show_adjusted_asap . '/><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<xsl:text> </xsl:text><input type="submit" name="action" value="Recompute p-values"/>';
print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:text> </xsl:text>exclude degen peps: <input type="checkbox" name="no_degens" value="yes" ' . $exclude_degens . '/>';
print XSL '  exclude charge: <input type="checkbox" name="ex1" value="yes" ' . $exclude_1 . '/>1+';
print XSL '<input type="checkbox" name="ex2" value="yes" ' . $exclude_2 . '/>2+';
print XSL '<input type="checkbox" name="ex3" value="yes" ' . $exclude_3 . '/>3+' . '<xsl:text>   </xsl:text>';

print XSL 'min pep prob: <INPUT TYPE="text" NAME="min_pep_prob" VALUE="' . $min_pepprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
print XSL ' min num tol term: <INPUT TYPE="text" NAME="min_ntt" VALUE="', $minntt_display, '" SIZE="1" MAXLENGTH="1"/><xsl:text> </xsl:text>';
print XSL $newline;


print XSL '<xsl:text> </xsl:text>include aa: <INPUT TYPE="text" NAME="pep_aa" VALUE="', $pep_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   mark aa: <INPUT TYPE="text" NAME="mark_aa" VALUE="', $mark_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   NxS/T: <input type="checkbox" name="glyc" value="yes" ' . $glyc . '/><xsl:text>   </xsl:text>';
print XSL 'protein text: <input type="text" name="text1" value="', $text1, '" size="12" maxlength="24"/><xsl:text>   </xsl:text>';
print XSL 'export to excel: <input type="checkbox" name="excel" value="yes" />', "\n";
print XSL '<input type="hidden" name="restore" value="no"/>', "\n";
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>', "\n";
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>', "\n";
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="glyc" value="yes"/>' if($glyc);
print XSL '<input type="hidden" name="xml_input" value="1"/>' if(! $DISTR_VERSION && $NEW_XML_FORMAT);
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="asapratio_pvalue" value="yes"/></xsl:if>';



if($full_menu) {
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL '<xsl:text> </xsl:text>sensitivity/error information: <input type="radio" name="senserr" value="show" ';
    print XSL $checked if(! ($show_sens eq ''));
    print XSL '/>show<input type="radio" name="senserr" value="hide" ';
    print XSL $checked if($show_sens eq '');
    print XSL '/>hide' . $newline;
    
    print XSL '<pre>' . $newline . '</pre>';


    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><xsl:text> </xsl:text>Quantitation Ratio: <input type="radio" name="quant_light2heavy" value="true" ';
	print XSL $checked if(! ($quant_light2heavy eq 'false'));
	print XSL '/>light/heavy<input type="radio" name="quant_light2heavy" value="false" ';
	print XSL $checked if($quant_light2heavy eq 'false');
	print XSL '/>heavy/light';
	print XSL '<pre>' . $newline . '</pre></xsl:if>';
    } # only for xml version

    print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:text> </xsl:text>XPRESS information: <input type="radio" name="xpress_display" value="show" ';
    print XSL $checked if($xpress_display eq $checked);
    print XSL '/>show<input type="radio" name="xpress_display" value="hide" ';
    print XSL $checked if($xpress_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:if test="$asap_quant &gt; \'0\'"><xsl:text> </xsl:text>ASAPRatio information: <input type="radio" name="asap_display" value="show" ';
    print XSL $checked if($asap_display eq $checked);
    print XSL '/>show<input type="radio" name="asap_display" value="hide" ';
    print XSL $checked if($asap_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:text> </xsl:text>protein display  ' . $newline;
    print XSL '<xsl:text> </xsl:text>number unique peptides: <input type="radio" name="num_unique_peps" value="show" ';
    print XSL $checked if(! ($show_num_unique_peps eq ''));
    print XSL '/>show<input type="radio" name="num_unique_peps" value="hide" ';
    print XSL $checked if($show_num_unique_peps eq '');
    print XSL '/>hide';
    print XSL '   total number peptides: <input type="radio" name="tot_num_peps" value="show" ';
    print XSL $checked if(! ($show_tot_num_peps eq ''));
    print XSL '/>show<input type="radio" name="tot_num_peps" value="hide" ';
    print XSL $checked if($show_tot_num_peps eq '');
    print XSL '/>hide';

    print XSL '   share of spectrum ids: <input type="radio" name="pct_spectrum_ids" value="show" ';
    print XSL $checked if(! ($show_pct_spectrum_ids eq ''));
    print XSL '/>show<input type="radio" name="pct_spectrum_ids" value="hide" ';
    print XSL $checked if($show_pct_spectrum_ids eq '');
    print XSL '/>hide';

    print XSL $newline;
    print XSL '<pre>' . $newline . '</pre>';

    print XSL '<xsl:text> </xsl:text>peptide column display   ' . $newline;

    print XSL '<input type="radio" name="order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;


    print XSL '<xsl:text> </xsl:text>weight <input type="text" name="weight" value="' . $register_order{'weight'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>peptide sequence <input type="text" name="peptide_sequence" value="' . $register_order{'peptide_sequence'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>nsp adjusted probability <input type="text" name="nsp_adjusted_probability" value="' . $register_order{'nsp_adjusted_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>initial probability <input type="text" name="initial_probability" value="' . $register_order{'initial_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>number tolerable termini <input type="text" name="num_tol_term" value="' . $register_order{'num_tol_term'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>nsp bin <input type="text" name="n_sibling_peptides_bin" value="' . $register_order{'n_sibling_peptides_bin'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>total number peptide instances <input type="text" name="n_instances" value="' . $register_order{'n_instances'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>peptide group index <input type="text" name="peptide_group_designator" value="' . $register_order{'peptide_group_designator'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and not($organism=\'Drosophila\')">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline . 'annotation column display   ' . $newline;
    print XSL '<input type="radio" name="annot_order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="annot_order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;
    print XSL '<xsl:text> </xsl:text>ensembl <input type="text" name="ensembl" value="' . $reg_annot_order{'ensembl'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>trembl <input type="text" name="trembl" value="' . $reg_annot_order{'trembl'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>swissprot <input type="text" name="swissprot" value="' . $reg_annot_order{'swissprot'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>refseq <input type="text" name="refseq" value="' . $reg_annot_order{'refseq'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>locuslink <input type="text" name="locus_link" value="' . $reg_annot_order{'locus_link'} . '" size="2" maxlength="3"/>', $newline;

    print XSL '</xsl:if>';
    print XSL '<pre>' . $newline . '</pre>';
    print XSL "---------------------------------------------------------------------------------------------------------";    
    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>set customized data view: ';
    print XSL '<input type="radio" name="custom_settings" value="prev" ' . $checked . '/>no change ';
    print XSL '<input type="radio" name="custom_settings" value="current"/>current ';
    print XSL '<input type="radio" name="custom_settings" value="default"/>default';

    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>short menu <input type="checkbox" name="short_menu" value="yes"/>';
    print XSL '<input type="hidden" name="menu" value="full"/>';


} # if full menu
else { # short menu case
    print XSL '<pre>' . $nonbreakline . '</pre>'. $newline;
    print XSL ' full menu <input type="checkbox" name="full_menu" value="yes"/>  '; 
    print XSL '    show discarded entries <input type="checkbox" name="discards" value="yes" ' . $discards . '/>    clear manual discards/restores <input type="checkbox" name="clear" value="yes"/>';

    # hidden information

    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><input type="hidden" name="quant_light2heavy" value="';
	if($quant_light2heavy eq 'false') {
	    print XSL 'false';
	}
	else {
	    print XSL 'true';
	}
	print XSL '"/></xsl:if>';
    } # only for xml version


    foreach(keys %register_order) {
	print XSL '<input type="hidden" name="' . $_ . '" value="' . $register_order{$_} . '"/>';
    }
    print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

# more here

    print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
    print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    
    print XSL '<input type="hidden" name="xpress_display" value="';
    if($xpress_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";
    print XSL '<input type="hidden" name="asap_display" value="';
    if($asap_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";

}
if($CALCULATE_PIES) {
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and $organism=\'Homo_sapiens\'">    go ontology level <select name="go_level"><option value="0"/><option value="1"';
    print XSL ' selected="yes"' if($go_level == 1);
    print XSL '>1</option><option value="101"';
    print XSL ' selected="yes"' if($go_level == 101);

    print XSL '>1H</option><option value="2"';
    print XSL ' selected="yes"' if($go_level == 2);
    print XSL '>2</option><option value="102"';
    print XSL ' selected="yes"' if($go_level == 102);


    print XSL '>2H</option><option value="3"';
    print XSL ' selected="yes"' if($go_level == 3);
    print XSL '>3</option>';
    print XSL '<option value="103"';
    print XSL ' selected="yes"' if($go_level == 103);
    print XSL '>3H</option>'; 



    print XSL '<option value="4"';
    print XSL ' selected="yes"' if($go_level == 4);
    print XSL '>4</option>'; 
    print XSL '<option value="104"';
    print XSL ' selected="yes"' if($go_level == 104);
    print XSL '>4H</option>'; 


    print XSL '<option value="5"';
    print XSL ' selected="yes"' if($go_level == 5);
    print XSL '>5</option>';
    print XSL '<option value="105"';
    print XSL ' selected="yes"' if($go_level == 105);
    print XSL '>5H</option>'; 

    print XSL '<option value="6"';
    print XSL ' selected="yes"' if($go_level == 6);
    print XSL '>6</option>';
    print XSL '<option value="106"';
    print XSL ' selected="yes"' if($go_level == 106);
    print XSL '>6H</option>'; 

    print XSL '<option value="7"';
    print XSL ' selected="yes"' if($go_level == 7);
    print XSL '>7</option>';
    print XSL '<option value="107"';
    print XSL ' selected="yes"' if($go_level == 107);
    print XSL '>7H</option>'; 

    print XSL '<option value="8"';
    print XSL ' selected="yes"' if($go_level == 8);
    print XSL '>8</option>';
    print XSL '<option value="108"';
    print XSL ' selected="yes"' if($go_level == 108);
    print XSL '>8H</option>'; 


    print XSL '<option value="9"';
    print XSL ' selected="yes"' if($go_level == 9);
    print XSL '>9</option>';
    print XSL '<option value="109"';
    print XSL ' selected="yes"' if($go_level == 109);
    print XSL '>9H</option>'; 

    print XSL '</select></xsl:if>';
} # if calc pies
print XSL $nonbreakline ;

if($full_menu) {
    print XSL '<pre>' . $newline . '</pre>';
    if($discards) {
	print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
    }
    else {
	print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
    }
    print XSL $newline;
}

print XSL '</pre></td></tr></table>', "\n";

# make local reference
if(exists ${$boxptr}{'excel'} && ${$boxptr}{'excel'} eq 'yes') {
    my $local_excelfile = $excelfile;
    if((length $SERVER_ROOT) <= (length $local_excelfile) && 
       index((lc $local_excelfile), ($LC_SERVER_ROOT)) == 0) {
	$local_excelfile =  substr($local_excelfile, (length $SERVER_ROOT));
	if (substr($local_excelfile, 0, 1) ne '/') {
	    $local_excelfile = '/' . $local_excelfile;
	}
    }
    else {
	die "problem (pr7): $local_excelfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    my $windows_excelfile = $excelfile;
    if($WINDOWS_CYGWIN) {
	$windows_excelfile =  `cygpath -w '$excelfile'`;
	if($windows_excelfile =~ /^(\S+)\s?/) {
	    $windows_excelfile = $1;
	}
    }
    print XSL 'excel file: <a target="Win1" href="' . $local_excelfile . '">' . $windows_excelfile . '</a>'  . $newline;
    
}
if((! ($show_sens eq '') && $eligible)) {

  # make local reference
  my $local_pngfile = $pngfile;
  if(! $ISB_VERSION) {
      if((length $SERVER_ROOT) <= (length $local_pngfile) && 
	 index((lc $local_pngfile), ($LC_SERVER_ROOT)) == 0) {
	  $local_pngfile = '/' . substr($local_pngfile, (length $SERVER_ROOT));
      }
      else {
#	  die "problem (pr8): $local_pngfile is not mounted under webserver root: $SERVER_ROOT\n";
      }
  } # if iis & cygwin

    print XSL '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'">';
    print XSL '<font color="blue"> Predicted Total Number of Correct Entries: <xsl:value-of select="protx:protein_summary_header/@num_predicted_correct_prots"/></font>';
    print XSL "\n\n";
    print XSL "<TABLE><TR height=\"100\"><TD>";

    print XSL "<IMG SRC=\"$local_pngfile\"/>";
    print XSL "</TD><TD><PRE>";

    print XSL "<font color=\"red\">sensitivity</font>\tfraction of all correct proteins" . $newline . $tab . $tab . " with probs &gt;= min_prob" . $newline;
    print XSL "<font color=\"green\">error</font>\t\tfraction of all proteins with probs" . $newline . $tab . $tab . " &gt;= min_prob that are incorrect" . $newline . '<pre>' . $newline . '</pre>';

    print XSL 'minprob' . $tab . '<font color="red">sens</font>' . $tab . '<font color="green">err</font>' . $tab . '<font color="red"># corr</font>' . $tab . '<font color ="green"># incorr</font>' . $newline;
    print XSL '<xsl:apply-templates select="protx:protein_summary_header/protx:protein_summary_data_filter">';
    print XSL '<xsl:sort select="@min_probability" order="descending" data-type="number"/>';
    print XSL '</xsl:apply-templates>';

    print XSL '</PRE></TD></TR></TABLE>';
    print XSL $newline . '<pre>' . $newline . '</pre>';
    print XSL '</xsl:if>';
}


########################## COUNT ENTRIES  #################################

my $local_xmlfile = $xmlfile;
my $windows_xmlfile = $xmlfile;
if(! $ISB_VERSION) {
    if((length $SERVER_ROOT) <= (length $local_xmlfile) && 
       index((lc $local_xmlfile), ($LC_SERVER_ROOT)) == 0) {
	$local_xmlfile = '/' . substr($local_xmlfile, (length $SERVER_ROOT));
    }
    else {
	die "problem (pr9): $local_xmlfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    if($WINDOWS_CYGWIN) {
	$windows_xmlfile = `cygpath -w '$windows_xmlfile'`;
	if($windows_xmlfile =~ /^(\S+)\s?/) {
	    $windows_xmlfile = $1;
	}
    }
} # if iis & cygwin

my $MAX_XMLFILE_LEN = 80;
my $format_choice = ($WINDOWS_CYGWIN && (length $windows_xmlfile) > $MAX_XMLFILE_LEN) || 
	(! $WINDOWS_CYGWIN && (length $local_xmlfile) > $MAX_XMLFILE_LEN) ? '<br/>' : '';


if(! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') {
   

    print XSL '<font color="red">';
    print XSL '<xsl:value-of select="$prot_group_count"/>';

    print XSL '<font color="black"><i> discarded</i></font>' if($discards);

    print XSL ' entries (';

    print XSL '<xsl:value-of select="$single_hits_count"/>';

    print XSL ' single hits)';

    if(! $ISB_VERSION) {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; 
}
    else {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; 
    }
} # if count
else {

    print XSL '<font color="black"><i>discarded</i></font> ' if($discards);
    if(! $ISB_VERSION) {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }
    else {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }

}


###################################################

print XSL $newline . '<pre>' . $newline . '</pre>';
print XSL '<FONT COLOR="990000">* corresponds to peptide is_nondegenerate_evidence flag</FONT>' . $newline;


# calculate how many columns, and header line here

$num_cols += 8;
my $extra_column = '<td>' . $table_spacer . '</td>';

my $HEADER = '<td><!-- header --></td>';

if($tab_delim >= 1) {
    $HEADER = $header{'group_number'} . $tab; # cancel it
    $HEADER .= 'group probability' . $tab if(! ($show_groups eq ''));
    $HEADER .= $header{'protein'} . $tab;
    $HEADER .= 'protein probability' . $tab;
    $HEADER .= $header{'coverage'} . $tab;
    $HEADER .= $header{'libra'};
    $HEADER .= $header{'xpress'} if $header{'xpress'};
    $HEADER .= $header{'asapratio'} if $header{'asapratio'}; # tab is built in 

    if(! ($show_num_unique_peps eq '')) {
	$HEADER .= 'num unique peps' . $tab;
    }
    if(! ($show_tot_num_peps eq '')) {
	$HEADER .= 'tot indep spectra' . $tab;
    }
    if(! ($show_pct_spectrum_ids eq '')) {
	$HEADER .= 'percent share of spectrum ids' . $tab;
    }

    # now annotation
    if(! ($show_annot eq '')) {
	my $annot_header = $header{'ipi'};
	$annot_header .= $header{'description'} . $tab;

	$annot_header .= '<xsl:if test="not($organism = \'UNKNOWN\')">';

	if(scalar keys %display_annot_order > 0) {
	    foreach(sort {$display_annot_order{$a} <=> $display_annot_order{$b}} keys %display_annot_order) {
		$annot_header .= $header{$_}; # . '<xsl:text>  </xsl:text>';
	    }
	}
	else {
	    foreach(sort {$annot_order{$a} <=> $annot_order{$b}} keys %annot_order) {
		if($annot_order{$_} >= 0) {
		    $annot_header .= $header{$_} if $header{$_};
		}
	    }
	}
	$annot_header .= $tab_header{'flybase'};
	$annot_header .= '</xsl:if>';
	$HEADER .= $annot_header if(! ($show_annot eq ''));

    }

} # if tab delim
# now comes peptide info....

if(scalar keys %display_order > 0) {
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	if($tab_delim >= 1) {
	    if(exists $tab_header{$_}) {
		$HEADER .= $tab_header{$_} . $tab if(! ($show_peps eq ''));
	    }
	    else {
		$HEADER .= $_ . $tab if(! ($show_peps eq ''));
	    }
	}
	else {
	    $HEADER .= $header{$_}; # . '<xsl:text>  </xsl:text>';
	}
    }
}
else {
    foreach(sort {$default_order{$a} <=> $default_order{$b}} keys %default_order) {
	if($default_order{$_} >= 0) {
	    if($tab_delim >= 1) {
		if(exists $tab_header{$_}) {
		    $HEADER .= $tab_header{$_} . $tab if(! ($show_peps eq ''));
		}
		else {
		    $HEADER .= $_ . $tab if(! ($show_peps eq ''));
		}
	    }
	    else {
		$HEADER .= $header{$_} if $header{$_}; # . '<xsl:text>  </xsl:text>';
	    }
	}
    }
}

$HEADER .= $extra_column if(! $tab_delim);


my $annotation = $annot_display{'ipi'} . $annot_display{'description'}; 


$annotation .= 	'<xsl:if test="not($organism = \'UNKNOWN\')">';


if(scalar keys %display_annot_order > 0) {
    foreach(sort {$display_annot_order{$a} <=> $display_annot_order{$b}} keys %display_annot_order) {
	    $annotation .= $annot_display{$_}; 
    }
}
else {
    foreach(sort {$annot_order{$a} <=> $annot_order{$b}} keys %annot_order) {
	if($annot_order{$_} >= 0) {
		$annotation .= $annot_display{$_}; 
	}
    }
}
$annotation .= '</xsl:if>';
my $prot_header = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'comet-fastadb.cgi?';
$prot_header .= 'N-Glyco=1&amp;' if ($glyc);
$prot_header .= 'MarkAA=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
$prot_header .= 'Ref=';

my $prot_header_js = '<A HREF="javascript:protlink(';

my $prot_suf = '</A>';

print XSL $RESULT_TABLE_PRE . $RESULT_TABLE, "\n";


print XSL '<xsl:comment>' . $start_string . '</xsl:comment>' . $newline . "\n";;
print XSL $HEADER . $newline if($tab_delim >= 1);

# bypass protein groups altogether for no groups mode.....
if(! ($show_groups eq '')) {
    print XSL "\n", '<xsl:apply-templates select="protx:protein_group">', "\n";
}
else {
    print XSL '<xsl:apply-templates select="protx:protein_group/protx:protein">', "\n";
}

if(! ($sort_pvalue) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_xpress_desc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	print XSL 'sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean' . "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_xpress_asc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
    else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_asap_desc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
}
elsif(! ($sort_asap_asc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
}
elsif(! ($sort_prob eq '')) {
	print XSL '<xsl:sort select="@probability" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_prot eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@protein_name"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@protein_name"/>', "\n";

    }
}
elsif(! ($sort_cov eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@percent_coverage" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@percent_coverage" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_peps eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@total_number_peptides" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@total_number_peptides" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_spec_ids eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";

    }
}
else {
    if($USE_INDEX) {
	if($show_groups eq '') {
	    print XSL '<xsl:sort select="parent::node()/@group_number" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="@group_sibling_id"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="@group_number" data-type="number"/>', "\n";
	}
    }

}

print XSL '</xsl:apply-templates>', "\n";

print XSL $RESULT_TABLE_SUF, "\n";
print XSL '</form>';

if ($show_ggl eq $checked && $nrows > 0 && $ncols > 0) {
    my $webSV = $ENV{'WEBSERVER_ROOT'}; 
    
    $webSV =~ s/\\/\//g; 
    $gaggleNameValueFile =~ s/\\/\//g;
    $gaggleNameListFile =~ s/\\/\//g;
    $gaggleMatrixFile =~ s/\\/\//g;
    
    $gaggleNameValueFile =~ s%^$webSV%%gi;
    $gaggleNameListFile =~ s%^$webSV%%gi;
    $gaggleMatrixFile =~ s%^$webSV%%gi;

    if ($gaggleNameValueFile =~ /^\//) {
	$gaggleNameValueFile = substr($gaggleNameValueFile, 1)
    }
        
    if ($gaggleNameListFile =~ /^\//) {
	$gaggleNameListFile = substr($gaggleNameListFile, 1)
    }

    if ($gaggleMatrixFile =~ /^\//) {
	$gaggleMatrixFile = substr($gaggleMatrixFile, 1)
    }
    

    my $URL = $ENV{'WEBSERVER_URL'};

    if ($URL !~ /^http/ ) {
	$URL = "http://" . $ENV{'HTTP_HOST'} . "/";
    }

    print XSL '<DIV name="gaggle_xml" class="hidden">' . "\n";
    print XSL '<gaggleData version="0.1">' . "\n";
    print XSL '<nameValue type="indirect" name="Protein Probabilities" url="' . $URL . $gaggleNameValueFile . '"><xsl:attribute name="organism"><xsl:value-of select="$organism"/></xsl:attribute><xsl:attribute name="size">'. $nrows . '</xsl:attribute></nameValue>' . "\n";
    print XSL '<nameList type="indirect" name="Protein Names" url="' . $URL . $gaggleNameListFile . '"><xsl:attribute name="organism"><xsl:value-of select="$organism"/></xsl:attribute><xsl:attribute name="size">'. $nrows . '</xsl:attribute></nameList>' . "\n";
    print XSL '<dataMatrix type="indirect" name="Protein Data Matrix" url="' . $URL . $gaggleMatrixFile . '"><xsl:attribute name="organism"><xsl:value-of select="$organism"/></xsl:attribute><xsl:attribute name="size">'. $nrows . ',' . $ncols . '</xsl:attribute></dataMatrix>' . "\n";
    print XSL '</gaggleData></DIV>' . "\n";
    
}
print XSL '</PRE></BODY></HTML>', "\n";
print XSL '</xsl:template>', "\n\n\n";

if(! ($show_groups eq '')) {
# protein group
print XSL "\n\n\n";
print XSL '<xsl:template match="protx:protein_group">', "\n";

my $suffix = '';
if(@inclusions > 0) {
    $suffix = ' or @group_number=\'';
    for(my $i = 0; $i <= $#inclusions; $i++) {
	$suffix .= $inclusions[$i] . '\'';
	$suffix .= ' or @group_number=\'' if($i <= $#inclusions - 1);
    }
}

foreach(keys %parent_incls) {
    $suffix .= ' or @group_number=\'' . $_ . '\'';
}    


if($discards) {

    if(! ($show_groups eq '')) {
	# see if fails criteria
	print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	} # show adjusted
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
	# check for all exclusions
	if(@exclusions > 0) {
	    for(my $k = 0; $k <= $#exclusions; $k++) {
		print XSL ' or (@group_number = \'' . $exclusions[$k] . '\')';
	    }
 	}
	# check for excluded children of this parent
	if(@pexclusions > 0) {
	    foreach(keys %parent_excls) {
		print XSL ' or (@group_number = \'' . $_ . '\')';
	    }
	}
	print XSL '">';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '<xsl:if test="not(@group_number=\'' . $inclusions[$i] . '\')">', "\n";
	}

    } # groups
    else {  # hide groups...want to make sure no singletons pass by default
	print XSL '<xsl:if test="count(protx:protein) &gt;\'1\' or protx:protein[@group_sibling_id=\'a\']/@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	foreach(@exclusions) {
	    print XSL ' or @group_number=\'' . $_ . '\'';
	}
	print XSL '">';
	foreach(@inclusions) {
	    print XSL '<xsl:if test="not(count(protx:protein) =\'1\' and @group_number=\'' . $_ . '\')">';
	}

    }

} # discards
else { # conventional view

    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '<xsl:if test="not(@group_number=\'' . $exclusions[$e] . '\')">', "\n";
    }
    if(! ($show_groups eq '')) {

	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));

	}
	else { # adjusted asapratios
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));
	}
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' . $suffix . '">' if($max_pvalue_display < 1.0);

	print XSL '<xsl:if test="(@probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
    }
    else { # hide groups
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or @probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'))' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'))' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	else {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'))' . $suffix . '">' if($max_pvalue_display < 1.0);

    }

} # normal mode


print XSL '<xsl:variable name="group_member" select="count(protx:protein)"/>';
print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));
print XSL '<xsl:variable name="parental_group_number" select="parent::node()/@group_number"/>';
print XSL '<xsl:variable name="sole_prot" select="protx:protein/@protein_name"/>';
print XSL '<xsl:variable name="database" select="$ref_db"/>';
print XSL '<xsl:variable name="peps1" select="protx:protein/@unique_stripped_peptides"/>';



if($tab_delim >= 1) {
    if(! ($show_groups eq '') && ! ($show_peps eq '')) {
	print XSL $newline;
    }
}
else {

    if(! ($show_groups eq '')) {
	print XSL '<table cellpadding="0" bgcolor="white" class="results">';
	print XSL '<tr><td height="' . $entry_delimiter . '" colspan="10">' . $table_spacer . '</td></tr>';

	print XSL '<tr><td><nobr>';
	print XSL $display{'group_number'} . '</nobr></td>';
	print XSL '<td colspan="' . ($num_cols-1) . '">';

	print XSL '<xsl:if test="$group_member &gt;\'1\'">PROTEIN GROUP: <xsl:value-of select="@pseudo_name"/></xsl:if>';

	# display alphabetized list of all proteins (main and
	# indistinguishable proteins)
	#alphabetized list of all proteins (main and indistinguishable proteins)
	#print XSL '<xsl:if test="$group_member=\'1\'">' . $prot_header . '{$sole_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$sole_prot"/>' . $prot_suf . '<xsl:for-each select="protx:protein/protx:indistinguishable_protein"><xsl:variable name="indist_prot" select="@protein_name"/>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$indist_prot"/><xsl:if test="(position() + 2) mod 6 = \'1\'">' . $newline . '</xsl:if>' . $prot_suf . '</xsl:for-each></xsl:if>' . $table_space . $table_space;

	print XSL '<xsl:if test="$group_member=\'1\'">' . "\n";
	#linked protein names
	print XSL '  <xsl:for-each select="protx:protein | protx:protein/protx:indistinguishable_protein">' . "\n";
	print XSL '    <xsl:sort select="translate(@protein_name,$ucletters,$lcletters)" />' . "\n";
	print XSL "    $prot_header_js" . '\'{@protein_name}\', \'{$peps1}\')">'; 
	print XSL '    	<xsl:value-of select="@protein_name"/>' . "\n";
	print XSL "    $prot_suf" . "\n"; # "</A>"
	print XSL "    $table_space\n";
	print XSL "    $table_space\n";
	
	print XSL '    <xsl:if test="(position() + 1) mod 6 = \'1\'">' . "\n";
	print XSL '      <br/>' . "\n";
	print XSL '      <xsl:value-of select="$newline"/>' . "\n";
	print XSL '    </xsl:if>' . "\n";
	print XSL '  </xsl:for-each>' . "\n";
	print XSL '</xsl:if>' . "\n";
	
	print XSL "$table_space\n $table_space\n";


	# display probability
	print XSL '<font color="red"><b><xsl:value-of select="@probability"/></b></font>';

	print XSL '</td></tr>';
    }
    else { # case hiding groups
	print XSL '<xsl:if test="$group_member = \'1\'"><tr><td><nobr>' . $display{'group_number'} . '</nobr></td><td colspan="' . ($num_cols-1) . '">';
	print XSL '<xsl:if test="$group_member=\'1\'">' . $prot_header . '{$sole_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$sole_prot"/>' . $prot_suf . '<xsl:for-each select="protx:protein/protx:indistinguishable_protein"><xsl:variable name="indist_prot" select="@protein_name"/>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each></xsl:if>' . $table_space . $table_space;
;
	print XSL '<font color="red"><b><xsl:value-of select="@probability"/></b></font>';
	print XSL '</td></tr></xsl:if>';
    }
} # if not tab


print XSL '<xsl:apply-templates select="protx:protein">';
print XSL '</xsl:apply-templates>';


print XSL '<tr><td>' . $table_spacer . '</td></tr></table>' if(! $tab_delim);


if($discards) {
    if(! ($show_groups eq '')) {
	print XSL '</xsl:if>';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '</xsl:if>';
	}
    }
    else { # hide groups
	print XSL '</xsl:if>';
	foreach(@inclusions) {
	    print XSL '</xsl:if>';
	}
    }
}
else {
    ############################ 10/7/03
    print XSL '</xsl:if>' if(! ($asap_xpress eq ''));  # agree
    print XSL '</xsl:if>' if($minprob > 0);
    print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
    print XSL '</xsl:if>' if(! ($filter_asap eq ''));
    print XSL '</xsl:if>' if($min_xpress > 0);
    print XSL '</xsl:if>' if($max_xpress > 0);
    print XSL '</xsl:if>' if($min_asap > 0);
    print XSL '</xsl:if>' if($max_asap > 0);
    print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '</xsl:if>', "\n";
    }
}

print XSL '</xsl:template>', "\n\n\n";

} # only if show groups

############ PROTEIN ########################
print XSL "\n\n\n";
print XSL '<xsl:template match="protx:protein">';
my $num_pincl = 0;

print XSL '<xsl:variable name="group_number" select="parent::node()/@group_number"/>' if($show_groups eq '');
print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));

print XSL "\n" . '<xsl:variable name="realGroupNumber" select="../@group_number" />' . "\n";


# integrate inclusions....
if($discards) {
    
    print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';

    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'' . $min_xpress . '\')' if($min_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;=\'' . $max_xpress . '\')' if($max_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));
    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));

    }
    else {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));
    }
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
    if(@exclusions > 0) {
	foreach(@exclusions) {
	    print XSL ' or parent::node()/@group_number=\'' . $_ . '\'';
	}
    }
    if(@pexclusions > 0) {
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or (parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';

	    }
	}
    }
    print XSL '">';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '<xsl:if test="not(parent::node()/@group_number=\'' . $inclusions[$i] . '\')">', "\n";
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')">', "\n";
	}
    }
}
else { # conventional

    # need suffix
    my $prot_suffix = '';
    if(@pinclusions > 0) {
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		$prot_suffix .= ' or(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }    
	}
    }

    if($show_groups eq '') {
	foreach(@exclusions) {
	    print XSL '<xsl:if test="not(count(parent::node()/protx:protein)=\'1\' and parent::node()/@group_number=\'' . $_ . '\')' . $prot_suffix . '">';
	}
    }


    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')' . $prot_suffix . '">', "\n";
	}
    }
    if($show_groups eq '') {
	print XSL '<xsl:if test="@probability &gt;= \'' . $minprob . '\'' . $prot_suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_asap eq ''));

	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'' . $prot_suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'' . $prot_suffix . '">' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {

	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);


	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));

	}
    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'' . $prot_suffix . '">' if($max_pvalue_display < 1.0);

    }

    foreach(keys %parent_incls) {
	my @members = @{$parent_incls{$_}};
	if(@members > 0) {
	    $num_pincl++;
	    print XSL '<xsl:if test="not(parent::node()/@group_number=\'' . $_ . '\')';
	    for(my $m = 0; $m <= $#members; $m++) {
		print XSL ' or @group_sibling_id=\'' . $members[$m] . '\'';
	    }
	    print XSL '">';
	}

    }
#####################
    print XSL '<xsl:if test="count(protx:peptide)=\'1\'">' if($SINGLE_HITS);

} # convent


# check whether part of group
print XSL '<xsl:variable name="mult_prot" select="@protein_name"/>';
print XSL '<xsl:variable name="database2" select="$ref_db"/>';
print XSL '<xsl:variable name="peps2" select="@unique_stripped_peptides"/>';
print XSL '<xsl:variable name="filextn"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn">_<xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="asap_ind" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@index"/>';
print XSL '<xsl:variable name="prot_number"><xsl:value-of select="parent::node()/@group_number"/><xsl:if test="count(parent::node()/protx:protein) &gt;\'1\'"><xsl:value-of select="@group_sibling_id"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="pvalpngfile" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']/protx:ASAP_pvalue_analysis_summary/@analysis_distribution_file"/>';

# more variables here
print XSL '<xsl:variable name="peptide_string" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@peptide_string"/>';
if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev"/>';
}
else { # reverse
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_standard_dev"/>';
}
print XSL '<xsl:variable name="xnum" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>';
print XSL '<xsl:variable name="min_pep_prob" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@min_peptide_probability"/>';
# print XSL '<xsl:variable name="source" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="heavy2light"><xsl:if test="$reference_isotope=\'heavy\'">0</xsl:if><xsl:if test="$reference_isotope=\'light\'">1</xsl:if></xsl:variable>';
if($tab_delim >= 1) {
    if($show_groups eq '' && ! ($show_peps eq '')) {
	print XSL $newline;
    }

}
else {
    print XSL '<xsl:if test="count(parent::node()/protx:protein) &gt; \'1\'">' if(! ($show_groups eq ''));

    print XSL '<xsl:if test="not(@group_sibling_id=\'a\')"><tr><td>' . $table_spacer . '</td></tr></xsl:if>' if(! ($show_groups eq ''));

    print XSL '<tr><td height="' . $entry_delimiter . '" colspan="10">' . $table_spacer . '</td></tr>';



    print XSL '<tr><td><nobr>';
    print XSL '<xsl:if test="count(parent::node()/protx:protein) = \'1\'">' . $display{'group_number'} . '</xsl:if><xsl:if test="count(parent::node()/protx:protein) &gt; \'1\'">' . $display{'prot_number'} . '</xsl:if>';


    ## JMT
    # alphabetizing protein names for main and indistinguishable proteins
    print XSL '</nobr></td>' . "\n";
    print XSL '<td colspan="' . ($num_cols-1) . '">' . "\n";
    print XSL '  <xsl:for-each select=". | protx:indistinguishable_protein">' . "\n";
    #print XSL '  <xsl:for-each select="key(\'proteinName\', $realGroupNumber) | key(\'indistProteinNames\', $realGroupNumber)" >' . "\n";
    print XSL '    <xsl:sort select="translate(@protein_name,$ucletters,$lcletters)" />' . "\n";
    print XSL "    $prot_header_js" . '\'{@protein_name}\', \'{$peps2}\')">'; 
    print XSL '      <xsl:value-of select="@protein_name" />';
    print XSL "    $prot_suf"; # </A>
    print XSL "    $table_space\n $table_space\n";
    print XSL '    <xsl:if test="(position() + 1) mod 6 = \'1\'">' . "\n";
    print XSL '      <br/>' . "\n";
    print XSL '      <xsl:value-of select="$newline"/>' . "\n";
    print XSL '    </xsl:if>' . "\n";    
    print XSL "    $table_space\n $table_space\n";
    print XSL '  </xsl:for-each>' . "\n";

    print XSL "  $table_space\n $table_space\n";
    #
    ## JMT


    if($show_groups eq '') {
	print XSL ' <font color="red"><b><xsl:value-of select="@probability"/></b></font>';
    }
    else {
	print XSL ' <font color="#B40000"><xsl:value-of select="@probability"/></font>';
    }
    print XSL '</td></tr>';
    print XSL '</xsl:if>' if(! ($show_groups eq ''));

    print XSL '<tr>';
    print XSL '<td>' . $table_spacer . '</td>' . '<td colspan="11">' . "\n";

    print XSL '<table ' . $RESULT_TABLE . '<tr>';

#    print XSL '<td width="150"><xsl:if test="@confidence &gt;=\'0\'">confidence: <xsl:value-of select="@confidence"/></xsl:if></td>';

    print XSL '<td width="300"><xsl:if test="@percent_coverage &gt;\'0\'"><xsl:if test="@n_indistinguishable_proteins &gt; \'1\'">max<xsl:text> </xsl:text></xsl:if>coverage: <xsl:value-of select="@percent_coverage"/>%</xsl:if></td>';

    print XSL $display{'libra'};
    print XSL $display{'xpress'};
    print XSL $display{'asapratio'};

    if(! ($show_num_unique_peps eq '')) {
	print XSL '<td width="225">num unique peps: <xsl:value-of select="count(protx:peptide[@is_contributing_evidence=\'Y\'])"/></td>';
    }
    if(! ($show_tot_num_peps eq '')) {
	#print XSL '<td width="275">tot indep spectra: <xsl:value-of select="@total_number_peptides"/></td>';
	print XSL '<td width="225">tot indep spectra: <xsl:value-of select="@total_number_peptides"/></td>';
    }
    if(! ($show_pct_spectrum_ids eq '')) {
	print XSL '<td width="225"><xsl:if test="@pct_spectrum_ids">share of spectrum ids: <xsl:value-of select="@pct_spectrum_ids"/>%</xsl:if></td>';
    }

    print XSL '<xsl:variable name="myprotein3" select="@protein_name"/>';



    ## JMT:
    # this step took a long time; switched to hash lookup
    #print XSL '<xsl:variable name="mychildren" select="count(/protx:protein_summary/protx:protein_group/protx:protein[@group_sibling_id = \'a\' and @subsuming_protein_entry and @subsuming_protein_entry=$myprotein3])"/>';
    print XSL '<xsl:variable name="mychildren" select="count(key(\'subsumedProteinNames\',$myprotein3) )"/>' . "\n";
    #
    ## JMT


    print XSL '<td><xsl:if test="$mychildren &gt; \'0\'"><A Target="Win1" HREF="' . $CGI_HOME . 'findsubsets.pl?Protein={$myprotein3}&amp;Xmlfile=' . $xmlfile;
    print XSL '&amp;xml_input=1' if($NEW_XML_FORMAT);
    print XSL '&amp;show_annot=hide' if($hide_annot);
    print XSL '&amp;icat_mode=yes' if($ICAT);
    print XSL '&amp;glyc_mode=yes' if($GLYC);
    print XSL '&amp;mark_aa=' . $mark_aa if($mark_aa);
    print XSL '&amp;glyc=yes' if($glyc);
    print XSL '&amp;quant_light2heavy=false' if($quant_light2heavy eq 'false');
    print XSL '&amp;weight=' .  $register_order{'weight'} if($register_order{'weight'});
    print XSL '&amp;peptide_sequence=' . $register_order{'peptide_sequence'} if($register_order{'peptide_sequence'});
    print XSL '&amp;nsp_adjusted_probability=' . $register_order{'nsp_adjusted_probability'} if($register_order{'nsp_adjusted_probability'});
    print XSL '&amp;initial_probability=' . $register_order{'initial_probability'} if($register_order{'initial_probability'});
    print XSL '&amp;num_tol_term=' . $register_order{'num_tol_term'} if($register_order{'num_tol_term'});
    print XSL '&amp;n_sibling_peptides_bin=' . $register_order{'n_sibling_peptides_bin'} if($register_order{'n_sibling_peptides_bin'});
    print XSL '&amp;n_instances=' . $register_order{'n_instances'} if($register_order{'n_instances'});
    print XSL '&amp;peptide_group_designator=' . $register_order{'peptide_group_designator'} if($register_order{'peptide_group_designator'});
    print XSL '&amp;ensembl=' . $reg_annot_order{'ensembl'} if($reg_annot_order{'ensembl'});
    print XSL '&amp;trembl=' . $reg_annot_order{'trembl'} if($reg_annot_order{'trembl'});
    print XSL '&amp;swissprot=' . $reg_annot_order{'swissprot'} if($reg_annot_order{'swissprot'});
    print XSL '&amp;refseq=' . $reg_annot_order{'refseq'} if($reg_annot_order{'refseq'});
    print XSL '&amp;locus_link=' . $reg_annot_order{'locus_link'} if($reg_annot_order{'locus_link'});

    print XSL '">subsumed entries: <xsl:value-of select="$mychildren"/></A></xsl:if></td>';
    print XSL '</tr></table></td>';

    print XSL '</tr>';

    if(! ($show_annot eq '')) {

	## JMT
	# sort descriptions in same order as alphabetized protein list,
	# and use hashes for speed

	# ALL PROTEIN annotations (main AND indistinguishable proteins)

	#print XSL '<xsl:for-each select="key(\'proteinName\', $realGroupNumber) | key(\'indistProteinNames\', $realGroupNumber)">' . "\n";
	print XSL '<xsl:for-each select=". | protx:indistinguishable_protein">' . "\n";
	print XSL '  <xsl:sort select="translate(@protein_name,$ucletters,$lcletters)" />' . "\n";

	print XSL '  <xsl:if test="protx:annotation|protx:parameter[@name=\'mol_weight\']|protx:parameter[@name=\'prot_length\']">' . "\n";;
	print XSL '    <tr>' . "\n";
	print XSL '      <td>' . $table_spacer . '</td>' . "\n";
	print XSL ' <xsl:if test="protx:annotation">' . "\n";
	print XSL '      <td  bgcolor="E0E0E0" width="675" colspan="' . ($num_cols-3) . '">' . "\n";
	# massive annotation block
	print XSL $annotation;
	print XSL '      </td>' . "\n";
	print XSL '  </xsl:if>' . "\n";
	print XSL '<xsl:if test="protx:parameter[@name=\'mol_weight\']"><td><xsl:attribute name="width">150</xsl:attribute>MolWt: <xsl:value-of select="protx:parameter[@name=\'mol_weight\']/@value"/>Da</td></xsl:if>';
	print XSL '<xsl:if test="protx:parameter[@name=\'prot_length\']"><td><xsl:attribute name="width">150</xsl:attribute>Length: <xsl:value-of select="protx:parameter[@name=\'prot_length\']/@value"/>aa</td></xsl:if>';
	print XSL '    </tr>' . "\n";
	print XSL '  </xsl:if>' . "\n";
	print XSL '</xsl:for-each>' . "\n"

    }

    print XSL '<tr height="50">' . $HEADER . '</tr>' if(! ($show_peps eq ''));

} # not tab display

print XSL '<xsl:apply-templates select="protx:peptide">';
print XSL '<xsl:sort select = "@nsp_adjusted_probability" order="descending" data-type="number"/>';
print XSL '<xsl:with-param name="pvalpngfile" select="$pvalpngfile"/>';
print XSL '<xsl:with-param name="mult_prot" select="$mult_prot"/>';
print XSL '<xsl:with-param name="peptide_string" select="$peptide_string"/>';
print XSL '<xsl:with-param name="xratio" select="$xratio"/>';
print XSL '<xsl:with-param name="xstd" select="$xstd"/>';
print XSL '<xsl:with-param name="xnum" select="$xnum"/>';
print XSL '<xsl:with-param name="min_pep_prob" select="$min_pep_prob"/>';
# print XSL '<xsl:with-param name="source" select="$source"/>';
print XSL '</xsl:apply-templates>';
print XSL '<tr><td>' . $table_spacer . '</td></tr>' if($show_groups eq '' && ! $tab_delim);

###########
print XSL '</xsl:if>' if($SINGLE_HITS);

if($discards) {
    
    print XSL '</xsl:if>';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '</xsl:if>';
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }

}
else { # conve
    if($show_groups eq '') {
	print XSL '</xsl:if>' if(! ($asap_xpress eq ''));
	print XSL '</xsl:if>' if($minprob > 0);
	print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
	print XSL '</xsl:if>' if(! ($filter_asap eq ''));
	print XSL '</xsl:if>' if($min_xpress > 0);
	print XSL '</xsl:if>' if($max_xpress > 0);
	print XSL '</xsl:if>' if($min_asap > 0);
	print XSL '</xsl:if>' if($max_asap > 0);
	print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
	foreach(@exclusions) {
	    print XSL '</xsl:if>';
	}
    }
    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }
    if(! ($show_groups eq '')) {
	for(my $k = 0; $k < $num_pincl; $k++) {
	    print XSL '</xsl:if>';

	}
    }
} # conv


print XSL '</xsl:template>' . "\n\n\n";


################### PEPTIDE  ###################################
print XSL "\n\n\n";
print XSL '<xsl:template match="protx:peptide">';
print XSL '<xsl:param name="pvalpngfile"/>';
print XSL '<xsl:param name="mult_prot"/>';
print XSL '<xsl:param name="peptide_string"/>';
print XSL '<xsl:param name="xratio"/>';
print XSL '<xsl:param name="xstd"/>';
print XSL '<xsl:param name="xnum"/>';
print XSL '<xsl:param name="min_pep_prob"/>';
# print XSL '<xsl:param name="source"/>';
print XSL '<xsl:variable name="mypep"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="mycharge" select="@charge"/>';
print XSL '<xsl:variable name="PepMass"><xsl:if test="@calc_neutral_pep_mass"><xsl:value-of select="@calc_neutral_pep_mass"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="StdPep"><xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if></xsl:variable>';

print XSL '<xsl:variable name="myinputfiles" select="$source_files_alt"/>';
print XSL '<xsl:variable name="myprots"><xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each></xsl:variable>';

print XSL '<xsl:variable name="nspbin" select="@n_sibling_peptides_bin"/>';
print XSL '<xsl:variable name="nspval" select="@n_sibling_peptides"/>';

if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">1</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}


print XSL '<xsl:if test="@nsp_adjusted_probability &gt;=\''. $min_pepprob . '\'">' if($min_pepprob > 0);
print XSL '<xsl:if test="@n_enzymatic_termini &gt;=\''. $minntt . '\'">' if($minntt > 0);
print XSL '<xsl:if test="not(@charge=\'1\')">' if(! ($exclude_1 eq ''));
print XSL '<xsl:if test="not(@charge=\'2\')">' if(! ($exclude_2 eq ''));
print XSL '<xsl:if test="not(@charge=\'3\')">' if(! ($exclude_3 eq ''));
print XSL '<xsl:if test="@is_nondegenerate_evidence=\'Y\'">' if(! ($exclude_degens eq ''));

print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';
if($tab_delim >= 1) {

    print XSL '<xsl:if test="position()=\'1\'">' if($show_peps eq '');
    print XSL $tab_display{'group_number'} . $tab;

    # here need group prob (when show groups)
    if(! ($show_groups eq '')) {
	print XSL '<xsl:value-of select="parent::node()/parent::node()/@probability"/>' . $tab;
    }

    print XSL $tab_display{'protein'} . $tab;

    # here need prot prob
    print XSL '<xsl:value-of select="parent::node()/@probability"/>' . $tab;

    # for HUPO, put unique_stripped_peptides
    print XSL $tab_display{'coverage'} . $tab;

    print XSL $tab_display{'libra'};
    print XSL $tab_display{'xpress'} if $tab_display{'xpress'};
    print XSL $tab_display{'asapratio'} if $tab_display{'asapratio'}; # tab built in

    if(! ($show_num_unique_peps eq '')) {
	print XSL '<xsl:value-of select="count(parent::node()/protx:peptide[@is_contributing_evidence=\'Y\'])"/>' . $tab;
    }
    if(! ($show_tot_num_peps eq '')) {
	print XSL '<xsl:value-of select="parent::node()/@total_number_peptides"/>' . $tab;
    }
    if(! ($show_pct_spectrum_ids eq '')) {
	print XSL '<xsl:value-of select="parent::node()/@pct_spectrum_ids"/>' . $tab;
    }


    # now the annotation
    if(! ($show_annot eq '')) {
	print XSL $annot_tab_display{'ipi'};
	print XSL $annot_tab_display{'description'} . $tab;

	print XSL '<xsl:if test="not($organism = \'UNKNOWN\')">';


	if(scalar keys %display_annot_order > 0) {
	    foreach(sort {$display_annot_order{$a} <=> $display_annot_order{$b}} keys %display_annot_order) {
		print XSL $annot_tab_display{$_}; # . '<xsl:text>  </xsl:text>';
	    }
	}
	else {
	    foreach(sort {$annot_order{$a} <=> $annot_order{$b}} keys %annot_order) {
		if($annot_order{$_} >= 0) {
		    print XSL $annot_tab_display{$_};
		}
	    }
	}
	print XSL '</xsl:if>';
    } # if show annot

} # if tab

print XSL '<tr><td>' . $table_spacer . '</td>' if(! $tab_delim && ! ($show_peps eq ''));
if(scalar keys %display_order > 0) {
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	if($tab_delim >= 1) {
	    print XSL $tab_display{$_} . $tab if(! ($show_peps eq ''));
	}
	else {
	    print XSL $display{$_} if(! ($show_peps eq ''));
	}
    }
}
else { # use default
    foreach(sort {$default_order{$a} <=> $default_order{$b}} keys %default_order) {
	if($tab_delim >= 1) {
	    print XSL $tab_display{$_} . $tab  if(! ($show_peps eq '') && $default_order{$_} >= 0);
	}
	else {
	    print XSL $display{$_} if($display{$_} && $default_order{$_} >= 0 && ! ($show_peps eq ''));
	}
    }

}

print XSL '</tr>' if(! $tab_delim && ! ($show_peps eq ''));
print XSL $newline if($tab_delim >= 1);

if(! $tab_delim && ! ($show_peps eq '')) { # make extra entry(s)
    print XSL '<xsl:apply-templates select="protx:indistinguishable_peptide">' ;
    print XSL '<xsl:with-param name="mypep" select="$mypep"/>';
    print XSL '</xsl:apply-templates>';
}

print XSL '</xsl:if>' if($min_pepprob > 0);
print XSL '</xsl:if>' if($minntt > 0);
print XSL '</xsl:if>' if(! ($exclude_1 eq ''));
print XSL '</xsl:if>' if(! ($exclude_2 eq ''));
print XSL '</xsl:if>' if(! ($exclude_3 eq ''));
print XSL '</xsl:if>' if(! ($exclude_degens eq ''));
if($tab_delim && $show_peps eq '') {
    print XSL '</xsl:if>';
}

print XSL '</xsl:template>' . "\n\n\n";

# indistinguishable_peptide
if(! $tab_delim && ! ($show_peps eq '')) {
    print XSL "\n\n\n";
    print XSL '<xsl:template match="protx:indistinguishable_peptide">';
    print XSL '<xsl:param name="mypep"/>';
    print XSL '<xsl:variable name="PepMass"><xsl:if test="@calc_neutral_pep_mass"><xsl:value-of select="@calc_neutral_pep_mass"/></xsl:if></xsl:variable>';
    print XSL '<xsl:variable name="StdPep2">';
    print XSL '<xsl:choose>';
    print XSL '<xsl:when test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/>';
    print XSL '</xsl:when>';
      print XSL '<xsl:otherwise><xsl:value-of select="@peptide_sequence"/></xsl:otherwise>';
    print XSL '</xsl:choose>';
    print XSL '</xsl:variable>';

    print XSL '<xsl:variable name="mycharge2"><xsl:choose><xsl:when test="@charge"><xsl:value-of select="@charge"/></xsl:when><xsl:otherwise><xsl:value-of select="parent::node()/@charge"/></xsl:otherwise></xsl:choose></xsl:variable>';
    print XSL '<xsl:variable name="mypep2"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
    print XSL '<xsl:variable name="myinputfiles2" select="$source_files_alt"/>';
if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype2"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype2"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">1</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}

    print XSL '<tr><td>' . $table_spacer . '</td>';
    if(scalar keys %display_order > 0) {
	foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	    if($_ eq 'peptide_sequence') {
		print XSL $display_ind_peptide_seq;
	    }
	    else {
		print XSL '<td></td>' if(! ($show_peps eq ''));
	    }
	}
    }
    else { # use default
	foreach(sort {$default_order{$a} <=> $default_order{$b}} keys %default_order) {
	    if($_ eq 'peptide_sequence') {
		print XSL  $display_ind_peptide_seq if($default_order{$_} >= 0);
	    }
	    elsif ( $default_order{$_} >= 0.5) {
		#print XSL '<td>' . $table_space . '</td>' if($default_order{$_} >= 0);
		print XSL '<td></td>' if($default_order{$_} >= 0);
	    }
	}
    }

    print XSL '</tr>';
    print XSL '</xsl:template>' . "\n\n\n";
} # if not tab delim


if((! ($show_sens eq '') && $eligible)) {
    print XSL "\n\n\n";
    print XSL '<xsl:template match="protx:protein_summary_data_filter">';
    print XSL '<xsl:value-of select="@min_probability"/>' . $tab . '<font color="red"><xsl:value-of select="@sensitivity"/></font>' . $tab . '<font color="green"><xsl:value-of select="@false_positive_error_rate"/></font>' . $tab . '<font color="red"><xsl:value-of select="@predicted_num_correct"/></font>' . $tab . '<font color="green"><xsl:value-of select="@predicted_num_incorrect"/></font>' . $newline;

    print XSL '</xsl:template>' . "\n\n\n";
}

print XSL '</xsl:stylesheet>', "\n";

print XSL "\n";

close(XSL);

}


sub writeGaggleNameValueXSLFile {
(my $xfile, my $boxptr) = @_;

if(! open(XSL, ">$xfile")) {
    print " cannot open $xfile: $!\n";
    exit(1);
}
print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">', "\n";
my $tab = '<xsl:value-of select="$tab"/>';
my $newline = '<xsl:value-of select="$newline"/>';
my $nonbreakline = '<xsl:value-of select="$newline"/>';
my $newlinespace = '<p/>';
my $doubleline = $newline . $newline;
my $space = '&#160';

my $num_cols = 3; # first & last


# just in case read recently from customized
$ICAT = 1 if(exists ${$boxptr}{'icat_mode'} && ${$boxptr}{'icat_mode'} eq 'yes');
$GLYC = 1 if(exists ${$boxptr}{'glyc_mode'} && ${$boxptr}{'glyc_mode'} eq 'yes');

# DEPRECATED: restore now fixed at 0 (taken care of up front)
my $restore = 0; 

my @minscore = (exists ${$boxptr}{'min_score1'} && ! (${$boxptr}{'min_score1'} eq '') ? ${$boxptr}{'min_score1'} : 0, 
		exists ${$boxptr}{'min_score2'} && ! (${$boxptr}{'min_score2'} eq '') ? ${$boxptr}{'min_score2'} : 0, 
		exists ${$boxptr}{'min_score3'} && ! (${$boxptr}{'min_score3'} eq '') ? ${$boxptr}{'min_score3'} : 0);

my $minprob = exists ${$boxptr}{'min_prob'} && ! (${$boxptr}{'min_prob'} eq '') ? ${$boxptr}{'min_prob'} : 0;
$minprob = $MIN_PROT_PROB if(! $HTML && $inital_xsl);

my $min_asap = exists ${$boxptr}{'min_asap'} && ! (${$boxptr}{'min_asap'} eq '') ? ${$boxptr}{'min_asap'} : 0;
my $max_asap = exists ${$boxptr}{'max_asap'} && ! (${$boxptr}{'max_asap'} eq '') ? ${$boxptr}{'max_asap'} : 0;
my $min_xpress = exists ${$boxptr}{'min_xpress'} && ! (${$boxptr}{'min_xpress'} eq '') ? ${$boxptr}{'min_xpress'} : 0;
my $max_xpress = exists ${$boxptr}{'max_xpress'} && ! (${$boxptr}{'max_xpress'} eq '') ? ${$boxptr}{'max_xpress'} : 0;

my $sort = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'yes';
${$boxptr}{'pep_aa'} = uc ${$boxptr}{'pep_aa'} if(exists ${$boxptr}{'pep_aa'});
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';
${$boxptr}{'mark_aa'} = uc ${$boxptr}{'mark_aa'} if(exists ${$boxptr}{'mark_aa'});
my $mark_aa = exists ${$boxptr}{'mark_aa'} && ! (${$boxptr}{'mark_aa'} eq '') ? ${$boxptr}{'mark_aa'} : '';
my $minntt = exists ${$boxptr}{'min_ntt'} && ! (${$boxptr}{'min_ntt'} eq '') ? ${$boxptr}{'min_ntt'} : 0;
my $min_pepprob = exists ${$boxptr}{'min_pep_prob'} && ! (${$boxptr}{'min_pep_prob'} eq '') ? ${$boxptr}{'min_pep_prob'} : 0;
my $maxnmc = exists ${$boxptr}{'max_nmc'} && ! (${$boxptr}{'max_nmc'} eq '') ? ${$boxptr}{'max_nmc'} : -1;

my @inclusions = exists ${$boxptr}{'inclusions'} ? split(' ', ${$boxptr}{'inclusions'}) : ();
my @exclusions = exists ${$boxptr}{'exclusions'} ? split(' ', ${$boxptr}{'exclusions'}) : ();
my @pinclusions = exists ${$boxptr}{'pinclusions'} ? split(' ', ${$boxptr}{'pinclusions'}) : ();
my @pexclusions = exists ${$boxptr}{'pexclusions'} ? split(' ', ${$boxptr}{'pexclusions'}) : ();
if(exists ${$boxptr}{'clear'} && ${$boxptr}{'clear'} eq 'yes') {
    @inclusions = ();
    @exclusions = ();
    @pinclusions = ();
    @pexclusions = ();
}

my $exclude_1 = exists ${$boxptr}{'ex1'} && ${$boxptr}{'ex1'} eq 'yes' ? $checked : '';
my $exclude_2 = exists ${$boxptr}{'ex2'} && ${$boxptr}{'ex2'} eq 'yes' ? $checked : '';
my $exclude_3 = exists ${$boxptr}{'ex3'} && ${$boxptr}{'ex3'} eq 'yes' ? $checked : '';


my $peptide_prophet_check1 = 'count(protx:peptide_prophet_summary) &gt; \'0\'';
my $peptide_prophet_check2 = 'count(parent::node()/protx:peptide_prophet_summary) &gt; \'0\'';

my $discards_init = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes';
my $discards = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes' ? $checked : '';

my $table_space = '&#160;';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
    $table_spacer = '<xsl:text>     </xsl:text>';
}

my $asap_xpress = exists ${$boxptr}{'asap_xpress'} && ${$boxptr}{'asap_xpress'} eq 'yes' ? $checked : '';
#my $show_groups = ! exists ${$boxptr}{'show_groups'} || ${$boxptr}{'show_groups'} eq 'show' ? $checked : '';
my $show_groups = '';
my $hide_groups = $show_groups eq '' ? $checked : '';

my $show_annot = ! exists ${$boxptr}{'show_annot'} || ${$boxptr}{'show_annot'} eq 'show' ? $checked : '';
my $hide_annot = $show_annot eq '' ? $checked : '';
my $show_peps = ! exists ${$boxptr}{'show_peps'} || ${$boxptr}{'show_peps'} eq 'show' ? $checked : '';
my $hide_peps = $show_peps eq '' ? $checked : '';

my $show_adjusted_asap = (! exists ${$boxptr}{'show_adjusted_asap'} && ! exists ${$boxptr}{'adj_asap'}) || (${$boxptr}{'show_adjusted_asap'} eq 'yes') ? $checked : '';

my $max_pvalue_display = exists ${$boxptr}{'max_pvalue'} && ! (${$boxptr}{'max_pvalue'} eq '') ? ${$boxptr}{'max_pvalue'} : 1.0;

my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';
my $glyc = exists ${$boxptr}{'glyc'} && ${$boxptr}{'glyc'} eq 'yes' ? $checked : '';

if(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'classic') {
    ${$boxptr}{'index'} = 1;
    ${$boxptr}{'prob'} = 2;
    ${$boxptr}{'spec_name'} = 3;
    ${$boxptr}{'neutral_mass'} = 4;
    ${$boxptr}{'massdiff'} = 5;
    ${$boxptr}{'sequest_xcorr'} = 6;
    ${$boxptr}{'sequest_delta'} = 7;
    ${$boxptr}{'sequest_spscore'} = -1;
    ${$boxptr}{'sequest_sprank'} = 8;
    ${$boxptr}{'matched_ions'} = 9;
    ${$boxptr}{'protein'} = 10;
    ${$boxptr}{'alt_prots'} = 11;
    ${$boxptr}{'peptide'} = 12;
    ${$boxptr}{'num_tol_term'} = -1;
    ${$boxptr}{'num_missed_cl'} = -1;
}
elsif(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'default') {
    ${$boxptr}{'weight'} = -1;
    ${$boxptr}{'peptide_sequence'} = -1;
    ${$boxptr}{'nsp_adjusted_probability'} = -1;
    ${$boxptr}{'initial_probability'} = -1;
    ${$boxptr}{'n_tryptic_termini'} = -1;
    ${$boxptr}{'n_sibling_peptides_bin'} = -1;
    ${$boxptr}{'n_instances'} = -1;
    ${$boxptr}{'peptide_group_designator'} = -1;
}
if(exists ${$boxptr}{'annot_order'} && ${$boxptr}{'annot_order'} eq 'default') {
    ${$boxptr}{'ensembl'} = -1;
    ${$boxptr}{'trembl'} = -1;
    ${$boxptr}{'swissprot'} = -1;
    ${$boxptr}{'refseq'} = -1;
    ${$boxptr}{'locus_link'} = -1;
}


# now add on new ones
foreach(keys %{$boxptr}) {
    if(/^excl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    if($inclusions[$i] == $1) {
		@inclusions = @inclusions[0..$i-1, $i+1..$#inclusions]; # delete it from inclusions
		$done = 1;
		$i = @inclusions;
		# cancel all previous pexclusions with same parent
		my $next_ex = $1;
		for(my $p = 0; $p <= $#pinclusions; $p++) {
		    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
			@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_ex = $1;
	push(@exclusions, $next_ex) if(! $done); # add to exclusions
	# cancel all previous pinclusions with same parent
	for(my $p = 0; $p <= $#pinclusions; $p++) {
	    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
		@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
	    }
	}

    }
    elsif(/^incl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    if($exclusions[$e] == $1) {
		@exclusions = @exclusions[0..$e-1, $e+1..$#exclusions]; # delete it from inclusions
		$done = 1;
		$e = @exclusions;
		# cancel all previous pexclusions with same parent
		my $next_in = $1;
		for(my $p = 0; $p <= $#pexclusions; $p++) {
		    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
			@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_in = $1;
	push(@inclusions, $next_in) if(! $done); # add to inclusions
	# cancel all previous pexclusions with same parent
	for(my $p = 0; $p <= $#pexclusions; $p++) {
	    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
		@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
	    }
	}
    }
}


# now add on new ones
foreach(keys %{$boxptr}) {

    if(/^pexcl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#pinclusions; $i++) {
	    if($pinclusions[$i] == $1) {
		@pinclusions = @pinclusions[0..$i-1, $i+1..$#pinclusions]; # delete it from inclusions
		$done = 1;
		$i = @pinclusions;
	    }
	}
	push(@pexclusions, $1) if(! $done); # add to exclusions
    }
    elsif(/^pincl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    if($pexclusions[$e] == $1) {
		@pexclusions = @pexclusions[0..$e-1, $e+1..$#pexclusions]; # delete it from inclusions
		$done = 1;
		$e = @pexclusions;
	    }
	}
	push(@pinclusions, $1) if(! $done); # add to inclusions
    }
}


my $exclusions = join(' ', @exclusions);
my $inclusions = join(' ', @inclusions);
my $pexclusions = join(' ', @pexclusions);
my $pinclusions = join(' ', @pinclusions);

my %parent_excls = ();
my %parent_incls = ();
foreach(@pexclusions) {
    if(/^(\d+)[a-z,A-Z]$/) {
	$parent_excls{$1}++;
    }
}
foreach(@pinclusions) {
    if(/^(\d+)([a-z,A-Z])$/) {
	if(exists $parent_incls{$1}) {
	    push(@{$parent_incls{$1}}, $2);
	}
	else {
	    my @next = ($2);
	    $parent_incls{$1} = \@next;
	}
    }
}

my $full_menu = (exists ${$boxptr}{'menu'} && ${$boxptr}{'menu'} eq 'full') || 
    (exists ${$boxptr}{'full_menu'} && ${$boxptr}{'full_menu'} eq 'yes');

my $short_menu = exists ${$boxptr}{'short_menu'} && ${$boxptr}{'short_menu'} eq 'yes';
$full_menu = 0 if($short_menu); # it takes precedence

my @minscore_display = ($minscore[0] > 0 ? $minscore[0] : '',$minscore[1] > 0 ? $minscore[1] : '',$minscore[2] > 0 ? $minscore[2] : '');
my $minprob_display = $minprob > 0 ? $minprob : '';
my $minntt_display = $minntt > 0 ? $minntt : '';
my $maxnmc_display = $maxnmc >= 0 ? $maxnmc : '';
my $min_asap_display = $min_asap > 0 ? $min_asap : '';
my $max_asap_display = $max_asap > 0 ? $max_asap : '';
my $min_xpress_display = $min_xpress > 0 ? $min_xpress : '';
my $max_xpress_display = $max_xpress > 0 ? $max_xpress : '';
my $min_pepprob_display = $min_pepprob > 0 ? $min_pepprob : '';

my $asap_display = exists ${$boxptr}{'asap_display'} && ${$boxptr}{'asap_display'} eq 'show' ? $checked : '';
my $xpress_display = exists ${$boxptr}{'xpress_display'} && ${$boxptr}{'xpress_display'} eq 'show' ? $checked : '';

my $show_ggl = exists ${$boxptr}{'show_ggl'} && ${$boxptr}{'show_ggl'} eq 'yes' ? $checked : '';

print XSL '<xsl:variable name="tab"><xsl:text>&#x09;</xsl:text></xsl:variable>', "\n";
print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
print XSL '</xsl:text></xsl:variable>';

print XSL '<xsl:variable name="libra_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="ref_db" select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/>';
print XSL '<xsl:variable name="asap_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="xpress_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="source_files" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="source_files_alt" select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/>';
print XSL '<xsl:variable name="organism"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@organism"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@organism)">UNKNOWN</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="reference_isotope"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"><xsl:value-of select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope)">UNSET</xsl:if></xsl:variable>';

my %display = ();
my %display_order = ();
my %register_order = ();
my %reg_annot_order = ();
my %display_annot_order = ();
my %header = ();
my %default_order = ();
my %tab_display = ();
my %tab_header = ();
my %annot_display = ();
my %annot_order = ();
my %annot_tab_display = ();

my $header_pre = '<font color="brown"><i>';
my $header_post = '</i></font>';

$display{'protein'} = '<xsl:value-of select="@protein"/><xsl:for-each select="protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';
$header{'protein'} = 'protein';
$tab_display{'protein'} = '<xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text>,</xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';

$default_order{'protein'} = -1;

$display{'coverage'} = '<xsl:value-of select="@percent_coverage"/>';
$header{'coverage'} = 'percent coverage';
$tab_display{'coverage'} = '<xsl:value-of select="parent::node()/@percent_coverage"/>';
$default_order{'coverage'} = -1;

$display{'annotation'} = '<xsl:if test="annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$header{'annotation'} = 'annotation';
$tab_display{'annotation'} = '<xsl:if test="annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$default_order{'annotation'} = -1;


# add here the cgi info for peptide
my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
my $html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
if(useXMLFormatLinks($xmlfile)) {
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;Ref=';
	$html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';
	$html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;';
	$html_peptide_lead3 .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
	$html_peptide_lead3 .= 'glyc=Y&amp;' if($glyc);
	$html_peptide_lead3 .= 'libra={$libra_quant}&amp;';
	$html_peptide_lead3 .= 'Ref=';

	$html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep2}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';

}
my $html_peptide_mid = '&amp;Infile=';


if($DISPLAY_MODS) {
    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_peptide_lead3 . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_peptide_lead3 . '{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if></td>';
}
else {
    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_peptide_lead . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . '<xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:value-of select="@peptide_sequence"/></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_peptide_lead . '{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . '<xsl:value-of select="@peptide_sequence"/></A></xsl:if></td>';
}
my $display_ind_peptide_seq = '<td class="indist_pep">--' . $html_peptide_lead4 . '{$mycharge2}_{$mypep}' . $html_peptide_mid . '{$myinputfiles2}">' . '<xsl:value-of select="parent::node()/@charge"/>_<xsl:value-of select="@peptide_sequence"/></A></td>';

$header{'peptide_sequence'} = '<td>' . $header_pre . 'peptide sequence' . $header_post . '</td>';
$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:value-of select="@peptide_sequence"/><xsl:for-each select="indistinguishable_peptide">,<xsl:value-of select="@peptide_sequence"/></xsl:for-each>';
$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence';

#print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';
#print XSL '<xsl:variable name="database" select="$ref_db"/>';

$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if>' . $tab . $TPPhostname . $CGI_HOME . 'peptidexml_html2.pl?PepMass=<xsl:value-of select="@calc_neutral_pep_mass"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>StdPep=<xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>MassError=' . $MOD_MASS_ERROR . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>xslt=' . $xslt . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>cgi-bin=' . $CGI_HOME . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>ratioType=<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>';

$tab_display{'peptide_sequence'} .= 'mark_aa=' . $mark_aa . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if(! ($mark_aa eq ''));
$tab_display{'peptide_sequence'} .= 'glyc=Y<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if($glyc);
$tab_display{'peptide_sequence'} .= 'Ref=<xsl:value-of select="@charge"/>_<xsl:value-of select="@peptide_sequence"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Infile=<xsl:value-of select="$source_files_alt"/>';

$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence' . $tab . 'peptide link';

$default_order{'peptide_sequence'} = 2;


my $wt_header = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'prot_wt_xml.pl?xmlfile=' . $xmlfile . '&amp;cgi-home=' . $CGI_HOME . '&amp;quant_light2heavy=' . $quant_light2heavy . '&amp;modpep={$StdPep}&amp;pepmass={$PepMass}&amp;';
$wt_header .= 'xml_input=1&amp;' if(! $DISTR_VERSION && $NEW_XML_FORMAT);
$wt_header .= 'glyc=1&amp;' if($glyc);
$wt_header .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
$wt_header .= 'peptide=';
my $wt_suf = '</A>';


$tab_display{'is_nondegen_evidence'} = '<xsl:value-of select="@is_nondegenerate_evidence"/>';
$tab_header{'is_nondegen_evidence'} = 'is nondegenerate evidence';
$default_order{'is_nondegen_evidence'} = 0.5;


$display{'weight'} = '<td><xsl:if test="@is_nondegenerate_evidence = \'Y\'"><font color="#990000">*</font></xsl:if></td><td>' . $wt_header . '{$mypep}&amp;charge={$mycharge}&amp;">' . '<nobr>wt-<xsl:value-of select="@weight"/><xsl:text> </xsl:text></nobr>' . $wt_suf . '&nbsp;&nbsp;</td>';
$header{'weight'} = '<td></td><td>' . $header_pre . 'weight' . $header_post . '</td>';
$tab_display{'weight'} = '<xsl:value-of select="@weight"/>';
$default_order{'weight'} = 1;

$display{'nsp_adjusted_probability'} = '<td><xsl:if test="@is_contributing_evidence = \'Y\'"><font COLOR="#FF9933"><xsl:value-of select="@nsp_adjusted_probability"/></font></xsl:if><xsl:if test="@is_contributing_evidence = \'N\'"><xsl:value-of select="@nsp_adjusted_probability"/></xsl:if></td>';
$header{'nsp_adjusted_probability'} = '<td>' . $header_pre . 'nsp adj prob' . $header_post . '</td>';
$tab_display{'nsp_adjusted_probability'} = '<xsl:value-of select="@nsp_adjusted_probability"/>';
$tab_header{'nsp_adjusted_probability'} = 'nsp adjusted probability';
$default_order{'nsp_adjusted_probability'} = 3;

$display{'initial_probability'} = '<td><xsl:value-of select="@initial_probability"/></td>';
$header{'initial_probability'} = '<td>' . $header_pre . 'init prob' . $header_post . '</td>';
$tab_display{'initial_probability'} = '<xsl:value-of select="@initial_probability"/>';
$tab_header{'initial_probability'} = 'initial probability';
$default_order{'initial_probability'} = 4;

$display{'num_tol_term'} = '<td><xsl:value-of select="@n_enzymatic_termini"/></td>';
$header{'num_tol_term'} = '<td>' . $header_pre . 'ntt' . $header_post . '</td>';
$tab_display{'num_tol_term'} = '<xsl:value-of select="@n_enzymatic_termini"/>';
$tab_header{'num_tol_term'} = 'n tol termini';
$default_order{'num_tol_term'} = 5;

my $nsp_pre = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'show_nspbin.pl?xmlfile=' . $xmlfile . '&amp;nsp_bin={$nspbin}&amp;nsp_val={$nspval}&amp;charge={$mycharge}&amp;pep={$mypep}&amp;prot={$myprots}">';
my $tempnsp = '<td><xsl:if test="@n_sibling_peptides">' . $nsp_pre . '<xsl:value-of select="@n_sibling_peptides"/></A></xsl:if>
<xsl:if test="not(@n_sibling_peptides)"><xsl:value-of select="@n_sibling_peptides_bin"/></xsl:if></td>';
$display{'n_sibling_peptides_bin'} = $tempnsp;

$header{'n_sibling_peptides_bin'} = '<td>' . $header_pre . 'nsp<xsl:if test="not(protx:peptide/@n_sibling_peptides)"> bin</xsl:if>' . $header_post . '</td>';
$tab_display{'n_sibling_peptides_bin'} = '<xsl:value-of select="@n_sibling_peptides_bin"/>';
$tab_header{'n_sibling_peptides_bin'} = 'n sibling peptides bin';
$default_order{'n_sibling_peptides_bin'} = 6;

$display{'peptide_group_designator'} = '<td><xsl:if test="@peptide_group_designator"><font color="#DD00DD"><xsl:value-of select="@peptide_group_designator"/>-<xsl:value-of select="@charge"/></font></xsl:if></td>';
$header{'peptide_group_designator'} = '<td>' . $header_pre . 'pep grp ind' . $header_post . '</td>';
$tab_display{'peptide_group_designator'} = '<xsl:value-of select="@peptide_group_designator"/>';
$tab_header{'peptide_group_designator'} = 'peptide group designator';
$default_order{'peptide_group_designator'} = 8;


$header{'group_number'} = 'entry no.';
$tab_display{'group_number'} = '<xsl:value-of select="parent::node()/parent::node()/@group_number"/><xsl:if test="count(parent::node()/parent::node()/protx:protein) &gt; \'1\'"><xsl:value-of select="parent::node()/@group_sibling_id"/></xsl:if>';
$default_order{'group_number'} = -1;
$display{'group_number'} = '';
$tab_header{'group_number'} = 'entry no.';

$annot_display{'description'} = '<xsl:if test="protx:annotation/@protein_description"><xsl:if test="$organism = \'UNKNOWN\'"><font color="green"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if><xsl:if test="not($organism = \'UNKNOWN\')"><font color="#008080"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if>' . $table_space . ' </xsl:if>';

$annot_order{'description'} = -1;
$annot_tab_display{'description'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@protein_description"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if></xsl:for-each>';

$header{'description'} = 'description';

my $flybase_header = '<a TARGET="Win1" href="http://flybase.bio.indiana.edu/.bin/fbidq.html?';

$tab_header{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'">flybase' . $tab . '</xsl:if>';
$annot_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="protx:annotation/@flybase"><xsl:variable name="flybase" select="protx:annotation/@flybase"/>' . $flybase_header . '{$flybase}"><font color="green">Flybase:<xsl:value-of select="$flybase"/></font></a>' . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'flybase'} = 9;
$annot_tab_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@flybase"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@flybase"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';

my $ipi_header = '<a TARGET="Win1" href="http://srs.ebi.ac.uk/cgi-bin/wgetz?-id+m_RJ1KrMXG+-e+[IPI-acc:';
my $ipi_mid = ']">';
my $ipi_suf = '</a>';
$annot_display{'ipi'} = '<font color="green">&gt;</font><xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="annotation/@ipi_name"><xsl:variable name="ipi" select="annotation/@ipi_name"/>' . $ipi_header . '{$ipi}' . $ipi_mid . '<font color="green">IPI:<xsl:value-of select="$ipi"/></font>' . $ipi_suf . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'ipi'} = -1;
$annot_tab_display{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ipi_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ipi_name"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';


$header{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')">ipi' . $tab . '</xsl:if>';

my $embl_header = '<a TARGET="Win1" href="http://www.ensembl.org/';
my $embl_mid = '/protview?peptide=';
my $embl_suf = '</a>';
$annot_display{'ensembl'} = '<xsl:if test="protx:annotation/@ensembl_name"><xsl:variable name="org" select="$organism"/><xsl:variable name="ensembl" select="protx:annotation/@ensembl_name"/>' . $embl_header . '{$org}' . $embl_mid . '{$ensembl}"><font color="green">E:<xsl:value-of select="$ensembl"/></font>' . $embl_suf . $table_space . ' </xsl:if>';
$annot_order{'ensembl'} = 3;
$annot_tab_display{'ensembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ensembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ensembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'ensembl'} = 'ensembl' . $tab;

$annot_display{'trembl'} = '<xsl:if test="protx:annotation/@trembl_name"><font color="green">Tr:<xsl:value-of select="protx:annotation/@trembl_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'trembl'} = 4;
$annot_tab_display{'trembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@trembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@trembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'trembl'} = 'trembl' . $tab;

$annot_display{'swissprot'} = '<xsl:if test="protx:annotation/@swissprot_name"><font color="green">Sw:<xsl:value-of select="protx:annotation/@swissprot_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'swissprot'} = 5;
$annot_tab_display{'swissprot'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@swissprot_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@swissprot_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'swissprot'} = 'swissprot' . $tab;

$annot_display{'refseq'} = '<xsl:if test="protx:annotation/@refseq_name"><font color="green">Ref:<xsl:value-of select="protx:annotation/@refseq_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'refseq'} = 6;
$annot_tab_display{'refseq'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@refseq_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@refseq_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'refseq'} = 'refseq' . $tab;

my $locus_header = '<a TARGET="Win1" href="http://www.ncbi.nlm.nih.gov/LocusLink/LocRpt.cgi?l=';
my $locus_suf = '</a>';

$annot_display{'locus_link'} = '<xsl:if test="protx:annotation/@locus_link_name"><xsl:variable name="loc" select="protx:annotation/@locus_link_name"/>' . $locus_header . '{$loc}' . '"><font color="green">LL:<xsl:value-of select="$loc"/></font>' . $locus_suf . $table_space . ' </xsl:if>';
$annot_order{'locus_link'} = 7;
$annot_tab_display{'locus_link'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@locus_link_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@locus_link_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'locus_link'} = 'locus link' . $tab;




my $asap_file_pre = '';
my $asap_proph_suf = '_prophet.bof';
my $asap_pep_suf = '_peptide.bof';
my $asap_proph = '';
my $asap_pep = '';
if($xfile =~ /^(\S+\/)/) { # assume in same directory
    $asap_proph = $1 . 'ASAPRatio_prophet.bof';
    $asap_pep = $1 . 'ASAPRatio_peptide.bof';
    $asap_file_pre = $1 . 'ASAPRatio';
}


my $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . 'xli/ASAPRatio_lstProRatio.cgi?orgFile=';
my $asap_header_mid = '.orig&amp;proBofFile=' . $asap_file_pre . '{$filextn}' . $asap_proph_suf . '&amp;pepBofFile=' . $asap_file_pre . '{$filextn}' . $asap_pep_suf . '&amp;proIndx=';


my $plusmn = '&#177;';
$plusmn = ' +-' if($xslt =~ /xsltproc/);

my $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'xpress-prophet.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light={$heavy2light}">';

my $xpress_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $xpress_link;
if(useXMLFormatLinks($xmlfile)) {
    $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light=' . $xpress_ratio_type;
    $xpress_pre .= '&amp;mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
    $xpress_pre .= '">'; #{$heavy2light}">';

    $xpress_link = $TPPhostname . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>protein=<xsl:value-of select="$mult_prot"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>peptide_string=<xsl:value-of select="$peptide_string"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>ratio=<xsl:value-of select="$xratio"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>stddev=<xsl:value-of select="$xstd"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>num=<xsl:value-of select="$xnum"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlfile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>min_pep_prob=<xsl:value-of select="$min_pep_prob"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>source_files=<xsl:value-of select="$source_files"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>heavy2light=' . $xpress_ratio_type;
    $xpress_link .= '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
}

my $xpress_suf = '</a>';

$num_cols = 3;



$display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'"><td width="250"><xsl:if test="protx:analysis_result[@analysis=\'libra\']">LIBRA<xsl:text> </xsl:text>(<xsl:value-of select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>)<xsl:for-each select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><br/><xsl:value-of select="@mz"/>:<xsl:text> </xsl:text><xsl:value-of select="@ratio"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="@error"/><xsl:text> </xsl:text></xsl:for-each></xsl:if></td></xsl:if>';

$tab_display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\' and parent::node()/protx:analysis_result[@analysis=\'libra\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>' . $tab . '<xsl:for-each select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><xsl:value-of select="@ratio"/>' . $tab . '<xsl:value-of select="@error"/>' . $tab . '</xsl:for-each></xsl:if>';

$header{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'">LIBRA number peptides' . $tab . '<xsl:for-each select="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']/protx:libra_summary/protx:fragment_masses">LIBRA <xsl:value-of select="@mz"/> ratio' . $tab . 'LIBRA <xsl:value-of select="@mz"/> error' . $tab . '</xsl:for-each></xsl:if>';



if($xpress_display eq $checked) {

    $display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'xpress\']">XPRESS';
    if(! ($quant_light2heavy eq 'true')) {
	$display{'xpress'} .= '(H/L)';
    }
    $display{'xpress'} .= ': ' . $xpress_pre . '<xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>(<xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>)</xsl:if>' . $xpress_suf . '</xsl:if><xsl:if test="not(protx:analysis_summary[@analysis=\'xpress\'])">' . $table_spacer . '</xsl:if></td></xsl:if>';

    $tab_display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'xpress\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>' . $tab . $xpress_link . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'xpress\'])">'  . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';

    $header{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'">xpress';
    if(! ($quant_light2heavy eq 'true')) {
	$header{'xpress'} .= '(H/L)';
    }
    $header{'xpress'} .= ' ratio mean' . $tab . 'xpress<xsl:if test="not($reference_isotope = \'UNSET\')"><xsl:if test="$reference_isotope=\'light\'"> (H/L)</xsl:if></xsl:if> stdev' . $tab . 'xpress num peptides' . $tab . 'xpress link' . $tab. '</xsl:if>';
} # if display xpress

my $NEW_ASAP_CGI = 1;

my $asap_display_cgi = 'asap-prophet-display.cgi';
if(useXMLFormatLinks($xmlfile)) {
    $asap_display_cgi = 'ASAPCGIDisplay.cgi';
}
my $asap_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $asap_link;
if($NEW_ASAP_CGI) {
    $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '&amp;xmlFile=' . $xmlfile . '&amp;';
    $asap_header_pre .= 'markAA=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
    $asap_header_pre .= 'protein=';

    $asap_link = $TPPhostname . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlFile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>';
    $asap_link .= 'markAA=' . $mark_aa . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>' if(! ($mark_aa eq ''));
    $asap_link .= 'protein=<xsl:value-of select="$mult_prot"/>';
}



my $asap_header_mid2 = '&amp;ratioType=0">';

my $asap_header_suf = '</A>';
my $pvalue_header_pre = '<a target="Win1" href="';

my $pvalue_header_suf = '</a>';
if($asap_display eq $checked) {
    if($NEW_ASAP_CGI) {

	# first display regular ratio no matter what
	$display{'asapratio'} = '<xsl:if test="$asap_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']">ASAPRatio';
	if(! ($quant_light2heavy eq 'true')) {
	    $display{'asapratio'} .= '(H/L)';
	}
	$display{'asapratio'} .= ': ' . $asap_header_pre . '{$mult_prot}">';
	$display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/></xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	# now add on the adjusted only if desired and present
	if ($show_adjusted_asap ne '') {
	    $display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\']">[<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_standard_dev"/>]</xsl:if>';
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><td width="200"><xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if>';

  
	}
	else {
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if>';

	}

    }
    else { # old format
	if($show_adjusted_asap eq '') {
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/></xsl:if><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	    }
	    else { # display adjsuted
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="400"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . ' [<xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_standard_dev"/>]';
	    }
	    $display{'asapratio'} .= '</xsl:if></td><td width="200"><xsl:if test="protx:ASAPRatio">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:ASAPRatio/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if><xsl:if test="not(/protx:protein_summary/protx:ASAP_pvalue_analysis_summary)"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . '</xsl:if></td></xsl:if></xsl:if>';
    } # if old version
 

    $tab_display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>' . $tab . $asap_link . $tab . '</xsl:if><xsl:if test="count(parent::node()/protx:analysis_result[@analysis=\'asapratio\'])= \'0\'">' . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab .'N_A' . $tab . '</xsl:if></xsl:if>';
    
    if(! ($show_adjusted_asap eq '')) { # display adjusted
	$tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev"/>' . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\'])">' .'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';
    }
    $tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></xsl:if>' . $tab . $TPPhostname . '<xsl:value-of select="$pvalpngfile"/>' . $tab .'</xsl:if>';

    $header{'asapratio'} = '<xsl:if test="count(protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">ratio mean' . $tab . 'ratio stdev' . $tab . 'ratio num peps' . $tab . 'asap ratio link'. $tab;
    if(! ($show_adjusted_asap eq '')) {
	$header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">adjusted ratio mean' . $tab . 'adjusted ratio stdev' . $tab . '</xsl:if>';
    }
    $header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">pvalue' . $tab . 'pvalue link'. $tab. '</xsl:if></xsl:if>';
} # if display asapratio info

my $reference = '$group_number' ; #$show_groups eq '' ? '$parental_group_number' : '$group_number';
my $gn = $show_groups eq '' ? '<xsl:value-of select="parent::node()/@group_number"/>' : '<xsl:value-of select="@group_number"/>';
if($discards) {

    $display{'group_number'} .= '<input type="checkbox" name="incl{' . $reference . '}" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@exclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
else {

    $display{'group_number'} .= '<input type="checkbox" name="excl';
    if($show_groups eq '') {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    else {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    $display{'group_number'} .= '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@inclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
$display{'group_number'} .= '<a name="{' . $reference . '}"/>' if($HTML);
$display{'group_number'} .= '<xsl:text> </xsl:text>';

$display{'prot_number'} = '';
if($discards) {

    $display{'prot_number'} .= '<input type="checkbox" name="pincl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pexclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pexclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pexclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
else {

    $display{'prot_number'} .= '<input type="checkbox" name="pexcl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pinclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pinclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pinclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
$display{'prot_number'} .= '<xsl:text> </xsl:text>';

$display{'n_instances'} = '<td><xsl:value-of select="@n_instances"/></td>';
$header{'n_instances'} = '<td>' . $header_pre . 'total' . $header_post . '</td>';
$tab_display{'n_instances'} = '<xsl:value-of select="@n_instances"/>';
$default_order{'n_instances'} = 7;
$tab_header{'n_instances'} = 'n instances';


foreach(keys %display) {
    $display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}

foreach(keys %annot_display) {
    $display_annot_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $reg_annot_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}


# test it out privately
if(0 && scalar keys %display_order > 0) {
    open(TEMP, ">temp.out");
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	print TEMP $display{$_}, "\n";
    }
    close(TEMP);
}

print XSL "\n";
# define tab and newline here


print XSL '<xsl:template match="protx:protein_summary">', "\n";
    printCountProtsXSL($boxptr);

print XSL '<HTML><BODY BGCOLOR="white" OnLoad="self.focus()"><PRE>', "\n";
print XSL '<HEAD><TITLE>ProteinProphet protXML Viewer (' . $TPPVersionInfo . ')</TITLE>', "\n";
print XSL '<STYLE TYPE="text/css" media="screen">', "\n";
print XSL '    div.visible {', "\n";
print XSL '    display: inline;', "\n";
print XSL '    white-space: nowrap;', "\n";
print XSL '    }', "\n";
print XSL '    div.hidden {', "\n";
print XSL '    display: none;', "\n";
print XSL '    }', "\n";
print XSL '    results {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.peptide {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.indist_pep {', "\n";
print XSL '	font-size: 10pt;', "\n";
print XSL '    }', "\n";
print XSL '    indist_pep_mod {', "\n";
print XSL '	font-size: 8pt;', "\n";
print XSL '    }', "\n";
print XSL '</STYLE></HEAD>', "\n";
print XSL '<table width="100%" border="3" BGCOLOR="#AAAAFF" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;"><tr><td align="center">', "\n";
print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";
if($HTML) {
    print XSL '<input type="submit" value="Restore Last View" style="background-color:#FFFF88;"/>', "\n";
    print XSL '<input type="hidden" name="restore_view" value="yes"/>', "\n";
}
else {
    print XSL '<input type="submit" value="Restore Original"/>', "\n";
    print XSL '<input type="hidden" name="restore" value="yes"/>', "\n";
}
    print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
    print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
    print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);



    print XSL '</form>';
    print XSL '</td><td align="center">';
    print XSL '<pre>ProteinProphet<sup><font size="3">&#xAE;</font></sup> protXML Viewer</pre>A.Keller   2.23.05</td>';

my $sort_none = ! exists ${$boxptr}{'sort'} || ${$boxptr}{'sort'} eq 'none' ?  $checked : '';
my $sort_xcorr = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xcorr' ? $checked : '';
my $sort_prob = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'prob' ? $checked : '';
my $sort_spec = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spec' ? $checked : '';
my $sort_pep = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'peptide' ? $checked : '';
my $sort_prot = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'protein' ? $checked : '';
my $sort_cov = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'coverage' ? $checked : '';
my $sort_peps = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'numpeps' ? $checked : '';
my $sort_spec_ids = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spectrum_ids' ? $checked : '';

my $sort_pvalue = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'pvalue' ? $checked : '';
my $text1 = exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq '') ? ${$boxptr}{'text1'} : '';
my $sort_asap_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_desc' ? $checked : '';
my $sort_asap_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_asc' ? $checked : '';
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ${$boxptr}{'filter_asap'} eq 'yes' ? $checked : '';
my $filter_xpress = exists ${$boxptr}{'filter_xpress'} && ${$boxptr}{'filter_xpress'} eq 'yes' ? $checked : '';
my $sort_xpress_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_desc' ? $checked : '';
my $sort_xpress_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_asc' ? $checked : '';
my $exclude_degens = exists ${$boxptr}{'no_degens'} && ${$boxptr}{'no_degens'} eq 'yes' ? $checked : '';

# show sens error info (still relevant for filtered dataset)
my $show_sens = exists ${$boxptr}{'senserr'} && ${$boxptr}{'senserr'} eq 'show' ? $checked : '';
my $eligible = ($filter_asap eq '' && $min_asap == 0 && $max_asap == 0 && (! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') && @exclusions == 0 && @inclusions == 0 && @pexclusions == 0 && @pexclusions == 0 && $filter_xpress eq '' && $min_xpress == 0 && $max_xpress == 0 && $asap_xpress eq '');
my $show_tot_num_peps = ! exists ${$boxptr}{'tot_num_peps'} || ${$boxptr}{'tot_num_peps'} eq 'show' ? $checked : '';
my $show_num_unique_peps = ! exists ${$boxptr}{'num_unique_peps'} || ${$boxptr}{'num_unique_peps'} eq 'show' ? $checked : '';
my $show_pct_spectrum_ids = ! exists ${$boxptr}{'pct_spectrum_ids'} || ${$boxptr}{'pct_spectrum_ids'} eq 'show' ? $checked : '';

my $suffix = $HTML_ORIENTATION ? '.htm' : '.xml';
$suffix = '.shtml' if($SHTML);

# write output xml
print XSL '<td align="center"><form method="GET" target="Win1" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";

print XSL '<pre>';
print XSL '<input type="submit" value="Write Displayed Data Subset to File" /><pre>' . $nonbreakline . '</pre>';
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";

print XSL '<input type="hidden" name="ex1" value="yes"/>', "\n" if(! ($exclude_1 eq ''));
print XSL '<input type="hidden" name="ex2" value="yes"/>', "\n" if(! ($exclude_2 eq ''));
print XSL '<input type="hidden" name="ex3" value="yes"/>', "\n" if(! ($exclude_3 eq ''));
print XSL '<input type="hidden" name="text1" value="' . ${$boxptr}{'text1'} . '"/>' if(exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq ''));
print XSL '<input type="hidden" name="min_prob" value="' . $minprob . '"/>' if($minprob > 0);
print XSL '<input type="hidden" name="min_score1" value="' . $minscore[0] . '"/>' if($minscore[0] > 0);
print XSL '<input type="hidden" name="min_score2" value="' . $minscore[1] . '"/>' if($minscore[1] > 0);
print XSL '<input type="hidden" name="min_score3" value="' . $minscore[2] . '"/>' if($minscore[2] > 0);
print XSL '<input type="hidden" name="min_ntt" value="' . $minntt . '"/>' if($minntt > 0);
print XSL '<input type="hidden" name="max_nmc" value="' . $maxnmc . '"/>' if($maxnmc >= 0);
print XSL '<input type="hidden" name="pep_aa" value="' . $pep_aa . '"/>' if(! ($pep_aa eq ''));
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>' if(! ($inclusions eq ''));
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>' if(! ($exclusions eq ''));
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>' if(! ($pinclusions eq ''));
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>' if(! ($pexclusions eq ''));
print XSL '<input type="hidden" name="filter_asap" value="yes"/>' if(! ($filter_asap eq ''));
print XSL '<input type="hidden" name="filter_xpress" value="yes"/>' if(! ($filter_xpress eq ''));
print XSL '<input type="hidden" name="min_pepprob" value="' . $min_pepprob . '"/>' if(! ($min_pepprob eq ''));
print XSL '<input type="hidden" name="show_groups" value="yes"/>' if(! ($show_groups eq ''));
print XSL '<input type="hidden" name="min_xpress" value="' . $min_xpress . '"/>' if($min_xpress > 0);
print XSL '<input type="hidden" name="max_xpress" value="' . $max_xpress . '"/>' if($max_xpress > 0);
print XSL '<input type="hidden" name="min_asap" value="' . $min_asap . '"/>' if($min_asap > 0);
print XSL '<input type="hidden" name="max_asap" value="' . $max_asap . '"/>' if($max_asap > 0);
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="show_adjusted_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="adj_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="max_pvalue" value="' . $max_pvalue_display . '"/>' if($max_pvalue_display < 1.0);
print XSL '<input type="hidden" name="asap_xpress" value="yes"/>' if(! ($asap_xpress eq ''));
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

print XSL 'file name: <input type="text" name="outfile" value="" size="20" maxlength="100"/>' . $suffix . '</pre>', "\n";
print XSL '</form></td></tr></table>';

print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl"><table width="100%" border="3" BGCOLOR="#AAAAFF">';
print XSL '<tr><td align="left" valign="center"><pre><input type="checkbox" name="show_ggl" value="yes" ' . $show_ggl . '/><b>Enable Gaggle Broadcast</b><A TARGET="Win1" HREF="http://tools.proteomecenter.org/wiki/index.php?title=Software:Firegoose%2C_Gaggle%2C_and_PIPE"><IMG BORDER="0" SRC="'. $HELP_DIR. 'images/qMark.png"/></A></pre></td></tr><tr><td><pre>';

if($discards) {
    print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
}
else {
    print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
}

print XSL $table_spacer . '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_sens_err.pl?xmlfile=' . $xmlfile;
print XSL '&amp;' if(! $DISTR_VERSION);

print XSL '">Sensitivity/Error Info</a></xsl:if><xsl:if test="protx:dataset_derivation/@generation_no &gt;\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_dataset_derivation.pl?xmlfile=' . $xmlfile . '">Dataset Derivation Info</a></xsl:if>';;

print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'more_anal.pl?xmlfile=' . $xmlfile;
print XSL '&amp;shtml=yes' if($SHTML);
print XSL '&amp;helpdir=' . $HELP_DIR;
print XSL '">More Analysis Info</a>';
print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'show_help.pl?help_dir=' . $HELP_DIR . '">Help</a>';
print XSL $newline;



print XSL '<xsl:text> </xsl:text>';
print XSL 'sort by: <input type="radio" name="sort" value="none" ' . $sort_none . '/>index';
print XSL '<input type="radio" name="sort" value="prob" ' . $sort_prob, '/>probability';
print XSL ' <input type="radio" name="sort" value="protein" ' . $sort_prot, '/>protein';
print XSL ' <input type="radio" name="sort" value="coverage" ' . $sort_cov, '/>coverage';

print XSL '<xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids)"> <input type="radio" name="sort" value="numpeps" ' . $sort_peps, '/>num peps</xsl:if>';
print XSL '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids"> <input type="radio" name="sort" value="spectrum_ids" ' . $sort_spec_ids, '/>share of spectrum ids</xsl:if>';
print XSL '<xsl:if test="$xpress_quant &gt; \'0\' and count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"> ';
print XSL $newline . '<xsl:text>          </xsl:text>';
print XSL '</xsl:if>';

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"> ';
print XSL ' <input type="radio" name="sort" value="xpress_desc" ' . $sort_xpress_desc, '/>xpress desc';
print XSL ' <input type="radio" name="sort" value="xpress_asc" ' . $sort_xpress_asc, '/>xpress asc';
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';
print XSL ' <input type="radio" name="sort" value="asap_desc" ' . $sort_asap_desc, '/>asap desc';
print XSL ' <input type="radio" name="sort" value="asap_asc" ' . $sort_asap_asc, '/>asap asc';
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"> <input type="radio" name="sort" value="pvalue" ' . $sort_pvalue, '/>pvalue</xsl:if>';
print XSL '</xsl:if>';

print XSL $newline;

print XSL '<xsl:text> </xsl:text>min probability: <INPUT TYPE="text" NAME="min_prob" VALUE="' . $minprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
# pick one of the following
print XSL $newline;
print XSL '<xsl:text> </xsl:text>protein groups: <input type="radio" name="show_groups" value="show" ' . $show_groups . '/>show  ';
print XSL '<input type="radio" name="show_groups" value="hide" ' . $hide_groups . '/>hide  ';

print XSL '   annotation: <input type="radio" name="show_annot" value="show" ' . $show_annot . '/>show  ';
print XSL '<input type="radio" name="show_annot" value="hide" ' . $hide_annot . '/>hide  ';
print XSL '   peptides: <input type="radio" name="show_peps" value="show" ' . $show_peps . '/>show  ';
print XSL '<input type="radio" name="show_peps" value="hide" ' . $hide_peps . '/>hide  ';
print XSL $newline;

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o XPRESS Ratio: <input type="checkbox" name="filter_xpress" value="yes" ' . $filter_xpress . '/>';
print XSL '  min XPRESS Ratio: <INPUT TYPE="text" NAME="min_xpress" VALUE="', $min_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max XPRESS Ratio: <INPUT TYPE="text" NAME="max_xpress" VALUE="', $max_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'">  ASAPRatio consistent: <input type="checkbox" name="asap_xpress" value="yes" ' . $asap_xpress . '/></xsl:if>';

print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o ASAPRatio: <input type="checkbox" name="filter_asap" value="yes" ' . $filter_asap . '/>';
print XSL '  min ASAPRatio: <INPUT TYPE="text" NAME="min_asap" VALUE="', $min_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max ASAPRatio: <INPUT TYPE="text" NAME="max_asap" VALUE="', $max_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
my $alt_max = $max_pvalue_display < 1.0 ? $max_pvalue_display : '';

print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">  max pvalue: <INPUT TYPE="text" NAME="max_pvalue" VALUE="', $alt_max, '" SIZE="3" MAXLENGTH="8"/>  adjusted: <input type="checkbox" name="show_adjusted_asap" value="yes" ' . $show_adjusted_asap . '/><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<xsl:text> </xsl:text><input type="submit" name="action" value="Recompute p-values"/>';
print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:text> </xsl:text>exclude degen peps: <input type="checkbox" name="no_degens" value="yes" ' . $exclude_degens . '/>';
print XSL '  exclude charge: <input type="checkbox" name="ex1" value="yes" ' . $exclude_1 . '/>1+';
print XSL '<input type="checkbox" name="ex2" value="yes" ' . $exclude_2 . '/>2+';
print XSL '<input type="checkbox" name="ex3" value="yes" ' . $exclude_3 . '/>3+' . '<xsl:text>   </xsl:text>';

print XSL 'min pep prob: <INPUT TYPE="text" NAME="min_pep_prob" VALUE="' . $min_pepprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
print XSL ' min num tol term: <INPUT TYPE="text" NAME="min_ntt" VALUE="', $minntt_display, '" SIZE="1" MAXLENGTH="1"/><xsl:text> </xsl:text>';
print XSL $newline;


print XSL '<xsl:text> </xsl:text>include aa: <INPUT TYPE="text" NAME="pep_aa" VALUE="', $pep_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   mark aa: <INPUT TYPE="text" NAME="mark_aa" VALUE="', $mark_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   NxS/T: <input type="checkbox" name="glyc" value="yes" ' . $glyc . '/><xsl:text>   </xsl:text>';
print XSL 'protein text: <input type="text" name="text1" value="', $text1, '" size="12" maxlength="24"/><xsl:text>   </xsl:text>';
print XSL 'export to excel: <input type="checkbox" name="excel" value="yes" />', "\n";
print XSL '<input type="hidden" name="restore" value="no"/>', "\n";
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>', "\n";
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>', "\n";
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="glyc" value="yes"/>' if($glyc);
print XSL '<input type="hidden" name="xml_input" value="1"/>' if(! $DISTR_VERSION && $NEW_XML_FORMAT);
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="asapratio_pvalue" value="yes"/></xsl:if>';



if($full_menu) {
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL '<xsl:text> </xsl:text>sensitivity/error information: <input type="radio" name="senserr" value="show" ';
    print XSL $checked if(! ($show_sens eq ''));
    print XSL '/>show<input type="radio" name="senserr" value="hide" ';
    print XSL $checked if($show_sens eq '');
    print XSL '/>hide' . $newline;
    
    print XSL '<pre>' . $newline . '</pre>';


    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><xsl:text> </xsl:text>Quantitation Ratio: <input type="radio" name="quant_light2heavy" value="true" ';
	print XSL $checked if(! ($quant_light2heavy eq 'false'));
	print XSL '/>light/heavy<input type="radio" name="quant_light2heavy" value="false" ';
	print XSL $checked if($quant_light2heavy eq 'false');
	print XSL '/>heavy/light';
	print XSL '<pre>' . $newline . '</pre></xsl:if>';
    } # only for xml version

    print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:text> </xsl:text>XPRESS information: <input type="radio" name="xpress_display" value="show" ';
    print XSL $checked if($xpress_display eq $checked);
    print XSL '/>show<input type="radio" name="xpress_display" value="hide" ';
    print XSL $checked if($xpress_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:if test="$asap_quant &gt; \'0\'"><xsl:text> </xsl:text>ASAPRatio information: <input type="radio" name="asap_display" value="show" ';
    print XSL $checked if($asap_display eq $checked);
    print XSL '/>show<input type="radio" name="asap_display" value="hide" ';
    print XSL $checked if($asap_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:text> </xsl:text>protein display  ' . $newline;
    print XSL '<xsl:text> </xsl:text>number unique peptides: <input type="radio" name="num_unique_peps" value="show" ';
    print XSL $checked if(! ($show_num_unique_peps eq ''));
    print XSL '/>show<input type="radio" name="num_unique_peps" value="hide" ';
    print XSL $checked if($show_num_unique_peps eq '');
    print XSL '/>hide';
    print XSL '   total number peptides: <input type="radio" name="tot_num_peps" value="show" ';
    print XSL $checked if(! ($show_tot_num_peps eq ''));
    print XSL '/>show<input type="radio" name="tot_num_peps" value="hide" ';
    print XSL $checked if($show_tot_num_peps eq '');
    print XSL '/>hide';

    print XSL '   share of spectrum ids: <input type="radio" name="pct_spectrum_ids" value="show" ';
    print XSL $checked if(! ($show_pct_spectrum_ids eq ''));
    print XSL '/>show<input type="radio" name="pct_spectrum_ids" value="hide" ';
    print XSL $checked if($show_pct_spectrum_ids eq '');
    print XSL '/>hide';

    print XSL $newline;
    print XSL '<pre>' . $newline . '</pre>';

    print XSL '<xsl:text> </xsl:text>peptide column display   ' . $newline;

    print XSL '<input type="radio" name="order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;


    print XSL '<xsl:text> </xsl:text>weight <input type="text" name="weight" value="' . $register_order{'weight'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>peptide sequence <input type="text" name="peptide_sequence" value="' . $register_order{'peptide_sequence'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>nsp adjusted probability <input type="text" name="nsp_adjusted_probability" value="' . $register_order{'nsp_adjusted_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>initial probability <input type="text" name="initial_probability" value="' . $register_order{'initial_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>number tolerable termini <input type="text" name="num_tol_term" value="' . $register_order{'num_tol_term'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>nsp bin <input type="text" name="n_sibling_peptides_bin" value="' . $register_order{'n_sibling_peptides_bin'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>total number peptide instances <input type="text" name="n_instances" value="' . $register_order{'n_instances'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>peptide group index <input type="text" name="peptide_group_designator" value="' . $register_order{'peptide_group_designator'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and not($organism=\'Drosophila\')">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline . 'annotation column display   ' . $newline;
    print XSL '<input type="radio" name="annot_order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="annot_order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;
    print XSL '<xsl:text> </xsl:text>ensembl <input type="text" name="ensembl" value="' . $reg_annot_order{'ensembl'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>trembl <input type="text" name="trembl" value="' . $reg_annot_order{'trembl'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>swissprot <input type="text" name="swissprot" value="' . $reg_annot_order{'swissprot'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>refseq <input type="text" name="refseq" value="' . $reg_annot_order{'refseq'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>locuslink <input type="text" name="locus_link" value="' . $reg_annot_order{'locus_link'} . '" size="2" maxlength="3"/>', $newline;

    print XSL '</xsl:if>';
    print XSL '<pre>' . $newline . '</pre>';
    print XSL "---------------------------------------------------------------------------------------------------------";    
    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>set customized data view: ';
    print XSL '<input type="radio" name="custom_settings" value="prev" ' . $checked . '/>no change ';
    print XSL '<input type="radio" name="custom_settings" value="current"/>current ';
    print XSL '<input type="radio" name="custom_settings" value="default"/>default';

    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>short menu <input type="checkbox" name="short_menu" value="yes"/>';
    print XSL '<input type="hidden" name="menu" value="full"/>';


} # if full menu
else { # short menu case
    print XSL '<pre>' . $nonbreakline . '</pre>'. $newline;
    print XSL ' full menu <input type="checkbox" name="full_menu" value="yes"/>  '; 
    print XSL '    show discarded entries <input type="checkbox" name="discards" value="yes" ' . $discards . '/>    clear manual discards/restores <input type="checkbox" name="clear" value="yes"/>';

    # hidden information

    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><input type="hidden" name="quant_light2heavy" value="';
	if($quant_light2heavy eq 'false') {
	    print XSL 'false';
	}
	else {
	    print XSL 'true';
	}
	print XSL '"/></xsl:if>';
    } # only for xml version


    foreach(keys %register_order) {
	print XSL '<input type="hidden" name="' . $_ . '" value="' . $register_order{$_} . '"/>';
    }
    print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

# more here

    print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
    print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="xpress_display" value="';
    if($xpress_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";
    print XSL '<input type="hidden" name="asap_display" value="';
    if($asap_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";

}
if($CALCULATE_PIES) {
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and $organism=\'Homo_sapiens\'">    go ontology level <select name="go_level"><option value="0"/><option value="1"';
    print XSL ' selected="yes"' if($go_level == 1);
    print XSL '>1</option><option value="101"';
    print XSL ' selected="yes"' if($go_level == 101);

    print XSL '>1H</option><option value="2"';
    print XSL ' selected="yes"' if($go_level == 2);
    print XSL '>2</option><option value="102"';
    print XSL ' selected="yes"' if($go_level == 102);


    print XSL '>2H</option><option value="3"';
    print XSL ' selected="yes"' if($go_level == 3);
    print XSL '>3</option>';
    print XSL '<option value="103"';
    print XSL ' selected="yes"' if($go_level == 103);
    print XSL '>3H</option>'; 



    print XSL '<option value="4"';
    print XSL ' selected="yes"' if($go_level == 4);
    print XSL '>4</option>'; 
    print XSL '<option value="104"';
    print XSL ' selected="yes"' if($go_level == 104);
    print XSL '>4H</option>'; 


    print XSL '<option value="5"';
    print XSL ' selected="yes"' if($go_level == 5);
    print XSL '>5</option>';
    print XSL '<option value="105"';
    print XSL ' selected="yes"' if($go_level == 105);
    print XSL '>5H</option>'; 

    print XSL '<option value="6"';
    print XSL ' selected="yes"' if($go_level == 6);
    print XSL '>6</option>';
    print XSL '<option value="106"';
    print XSL ' selected="yes"' if($go_level == 106);
    print XSL '>6H</option>'; 

    print XSL '<option value="7"';
    print XSL ' selected="yes"' if($go_level == 7);
    print XSL '>7</option>';
    print XSL '<option value="107"';
    print XSL ' selected="yes"' if($go_level == 107);
    print XSL '>7H</option>'; 

    print XSL '<option value="8"';
    print XSL ' selected="yes"' if($go_level == 8);
    print XSL '>8</option>';
    print XSL '<option value="108"';
    print XSL ' selected="yes"' if($go_level == 108);
    print XSL '>8H</option>'; 


    print XSL '<option value="9"';
    print XSL ' selected="yes"' if($go_level == 9);
    print XSL '>9</option>';
    print XSL '<option value="109"';
    print XSL ' selected="yes"' if($go_level == 109);
    print XSL '>9H</option>'; 

    print XSL '</select></xsl:if>';
} # if calc pies
print XSL $nonbreakline ;

if($full_menu) {
    print XSL '<pre>' . $newline . '</pre>';
    if($discards) {
	print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
    }
    else {
	print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
    }
    print XSL $newline;
}

print XSL '</pre></td></tr></table>', "\n";

# make local reference
if(exists ${$boxptr}{'excel'} && ${$boxptr}{'excel'} eq 'yes') {
    my $local_excelfile = $excelfile;
    if((length $SERVER_ROOT) <= (length $local_excelfile) && 
       index((lc $local_excelfile), ($LC_SERVER_ROOT)) == 0) {
	$local_excelfile =  substr($local_excelfile, (length $SERVER_ROOT));
	if (substr($local_excelfile, 0, 1) ne '/') {
	    $local_excelfile = '/' . $local_excelfile;
	}
    }
    else {
	die "problem: $local_excelfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    my $windows_excelfile = $excelfile;
    if($WINDOWS_CYGWIN) {
	$windows_excelfile =  `cygpath -w '$excelfile'`;
	if($windows_excelfile =~ /^(\S+)\s?/) {
	    $windows_excelfile = $1;
	}
    }
    print XSL 'excel file: <a target="Win1" href="' . $local_excelfile . '">' . $windows_excelfile . '</a>'  . $newline;
    
}
if((! ($show_sens eq '') && $eligible)) {

  # make local reference
  my $local_pngfile = $pngfile;
  if(! $ISB_VERSION) {
      if((length $SERVER_ROOT) <= (length $local_pngfile) && 
	 index((lc $local_pngfile), ($LC_SERVER_ROOT)) == 0) {
	  $local_pngfile = '/' . substr($local_pngfile, (length $SERVER_ROOT));
      }
      else {
	  die "problem: $local_pngfile is not mounted under webserver root: $SERVER_ROOT\n";
      }
  } # if iis & cygwin

    print XSL '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'">';
    print XSL '<font color="blue"> Predicted Total Number of Correct Entries: <xsl:value-of select="protx:protein_summary_header/@num_predicted_correct_prots"/></font>';
    print XSL "\n\n";
    print XSL "<TABLE><TR><TD>";

    print XSL "<IMG SRC=\"$local_pngfile\"/>";
    print XSL "</TD><TD><PRE>";

    print XSL "<font color=\"red\">sensitivity</font>\tfraction of all correct proteins" . $newline . $tab . $tab . " with probs &gt;= min_prob" . $newline;
    print XSL "<font color=\"green\">error</font>\t\tfraction of all proteins with probs" . $newline . $tab . $tab . " &gt;= min_prob that are incorrect" . $newline . '<pre>' . $newline . '</pre>';

    print XSL 'minprob' . $tab . '<font color="red">sens</font>' . $tab . '<font color="green">err</font>' . $tab . '<font color="red"># corr</font>' . $tab . '<font color ="green"># incorr</font>' . $newline;
    print XSL '<xsl:apply-templates select="protx:protein_summary_header/protx:protein_summary_data_filter">';
    print XSL '<xsl:sort select="@min_probability" order="descending" data-type="number"/>';
    print XSL '</xsl:apply-templates>';

    print XSL '</PRE></TD></TR></TABLE>';
    print XSL $newline . '<pre>' . $newline . '</pre>';
    print XSL '</xsl:if>';
}


########################## COUNT ENTRIES  #################################

my $local_xmlfile = $xmlfile;
my $windows_xmlfile = $xmlfile;
if(! $ISB_VERSION) {
    if((length $SERVER_ROOT) <= (length $local_xmlfile) && 
       index((lc $local_xmlfile), ($LC_SERVER_ROOT)) == 0) {
	$local_xmlfile = '/' . substr($local_xmlfile, (length $SERVER_ROOT));
    }
    else {
	die "problem: $local_xmlfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    if($WINDOWS_CYGWIN) {
	$windows_xmlfile = `cygpath -w '$windows_xmlfile'`;
	if($windows_xmlfile =~ /^(\S+)\s?/) {
	    $windows_xmlfile = $1;
	}
    }
} # if iis & cygwin

my $MAX_XMLFILE_LEN = 80;
my $format_choice = ($WINDOWS_CYGWIN && (length $windows_xmlfile) > $MAX_XMLFILE_LEN) || 
	(! $WINDOWS_CYGWIN && (length $local_xmlfile) > $MAX_XMLFILE_LEN) ? '<br/>' : '';


if(! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') {


    print XSL '<font color="red">';
    print XSL '<xsl:value-of select="$prot_group_count"/>';

    print XSL '<font color="black"><i> discarded</i></font>' if($discards);

    print XSL ' entries (';

    print XSL '<xsl:value-of select="$single_hits_count"/>';

    print XSL ' single hits)';

    if(! $ISB_VERSION) {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; 
}
    else {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; 
    }
} # if count
else {

    print XSL '<font color="black"><i>discarded</i></font> ' if($discards);
    if(! $ISB_VERSION) {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }
    else {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }

}


###################################################

print XSL $newline . '<pre>' . $newline . '</pre>';
print XSL '<FONT COLOR="990000">* indicates peptide corresponding to unique protein entry</FONT>' . $newline;


# calculate how many columns, and header line here

$num_cols += 8;
my $extra_column = '<td>' . $table_spacer . '</td>';

my $HEADER = $header{'protein'} . $tab;
$HEADER .= 'protein_probability' . $newline;

print XSL $RESULT_TABLE_PRE . $RESULT_TABLE, "\n";


print XSL '<xsl:comment>' . $start_string . '</xsl:comment>' . $newline . "\n";

#print XSL $HEADER . $newline ;

# bypass protein groups altogether for no groups mode.....
if(! ($show_groups eq '')) {
    print XSL "\n", '<xsl:apply-templates select="protx:protein_group">', "\n";
}
else {
    print XSL '<xsl:apply-templates select="protx:protein_group/protx:protein">', "\n";
}

if(! ($sort_pvalue) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_xpress_desc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	print XSL 'sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean' . "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_xpress_asc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
    else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_asap_desc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
}
elsif(! ($sort_asap_asc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
}
elsif(! ($sort_prob eq '')) {
	print XSL '<xsl:sort select="@probability" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_prot eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@protein_name"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@protein_name"/>', "\n";

    }
}
elsif(! ($sort_cov eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@percent_coverage" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@percent_coverage" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_peps eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@total_number_peptides" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@total_number_peptides" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_spec_ids eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";

    }
}
else {
    if($USE_INDEX) {
	if($show_groups eq '') {
	    print XSL '<xsl:sort select="parent::node()/@group_number" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="@group_sibling_id"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="@group_number" data-type="number"/>', "\n";
	}
    }

}

print XSL '</xsl:apply-templates>', "\n";

print XSL $RESULT_TABLE_SUF, "\n";
print XSL '</form>';
print XSL '</PRE></BODY></HTML>', "\n";
print XSL '</xsl:template>', "\n";

if(! ($show_groups eq '')) {
print XSL '<xsl:template match="protx:protein_group">', "\n";

my $suffix = '';
if(@inclusions > 0) {
    $suffix = ' or @group_number=\'';
    for(my $i = 0; $i <= $#inclusions; $i++) {
	$suffix .= $inclusions[$i] . '\'';
	$suffix .= ' or @group_number=\'' if($i <= $#inclusions - 1);
    }
}

foreach(keys %parent_incls) {
    $suffix .= ' or @group_number=\'' . $_ . '\'';
}    


if($discards) {

    if(! ($show_groups eq '')) {
	# see if fails criteria
	print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	} # show adjusted
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
	# check for all exclusions
	if(@exclusions > 0) {
	    for(my $k = 0; $k <= $#exclusions; $k++) {
		print XSL ' or (@group_number = \'' . $exclusions[$k] . '\')';
	    }
 	}
	# check for excluded children of this parent
	if(@pexclusions > 0) {
	    foreach(keys %parent_excls) {
		print XSL ' or (@group_number = \'' . $_ . '\')';
	    }
	}
	print XSL '">';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '<xsl:if test="not(@group_number=\'' . $inclusions[$i] . '\')">', "\n";
	}

    } # groups
    else {  # hide groups...want to make sure no singletons pass by default
	print XSL '<xsl:if test="count(protx:protein) &gt;\'1\' or protx:protein[@group_sibling_id=\'a\']/@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	foreach(@exclusions) {
	    print XSL ' or @group_number=\'' . $_ . '\'';
	}
	print XSL '">';
	foreach(@inclusions) {
	    print XSL '<xsl:if test="not(count(protx:protein) =\'1\' and @group_number=\'' . $_ . '\')">';
	}

    }

} # discards
else { # conventional view

    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '<xsl:if test="not(@group_number=\'' . $exclusions[$e] . '\')">', "\n";
    }
    if(! ($show_groups eq '')) {

	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));

	}
	else { # adjusted asapratios
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));
	}
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' . $suffix . '">' if($max_pvalue_display < 1.0);

	print XSL '<xsl:if test="(@probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
    }
    else { # hide groups
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or @probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'))' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'))' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	else {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'))' . $suffix . '">' if($max_pvalue_display < 1.0);

    }

} # normal mode

print XSL '<xsl:variable name="group_member" select="count(protx:protein)"/>';
print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));
print XSL '<xsl:variable name="parental_group_number" select="parent::node()/@group_number"/>';
print XSL '<xsl:variable name="sole_prot" select="protx:protein/@protein_name"/>';
print XSL '<xsl:variable name="database" select="$ref_db"/>';
print XSL '<xsl:variable name="peps1" select="protx:protein/@unique_stripped_peptides"/>';

print XSL '<xsl:apply-templates select="protx:protein">';
print XSL '<xsl:with-param name="group_member" select="$group_member"/>';
print XSL '<xsl:with-param name="group_number" select="$group_number"/>' if(! ($show_groups eq ''));
print XSL '<xsl:with-param name="parental_group_number" select="$parental_group_number"/>';
print XSL '<xsl:with-param name="sole_prot" select="$sole_prot"/>';
print XSL '<xsl:with-param name="database" select="$database"/>';
print XSL '<xsl:with-param name="peps1" select="$peps1"/>';

print XSL '</xsl:apply-templates>';

if($discards) {
    if(! ($show_groups eq '')) {
	print XSL '</xsl:if>';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '</xsl:if>';
	}
    }
    else { # hide groups
	print XSL '</xsl:if>';
	foreach(@inclusions) {
	    print XSL '</xsl:if>';
	}
    }
}
else {
    ############################ 10/7/03
    print XSL '</xsl:if>' if(! ($asap_xpress eq ''));  # agree
    print XSL '</xsl:if>' if($minprob > 0);
    print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
    print XSL '</xsl:if>' if(! ($filter_asap eq ''));
    print XSL '</xsl:if>' if($min_xpress > 0);
    print XSL '</xsl:if>' if($max_xpress > 0);
    print XSL '</xsl:if>' if($min_asap > 0);
    print XSL '</xsl:if>' if($max_asap > 0);
    print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '</xsl:if>', "\n";
    }
}

print XSL '</xsl:template>', "\n";

} # only if show groups

############ PROTEIN ########################
print XSL '<xsl:template match="protx:protein">';
print XSL '<xsl:param name="group_member" />';
print XSL '<xsl:param name="group_number" />' if(! ($show_groups eq ''));
print XSL '<xsl:param name="parental_group_number" />';
print XSL '<xsl:param name="sole_prot"/>';
print XSL '<xsl:param name="database"/>';
print XSL '<xsl:param name="peps1"/>';

my $num_pincl = 0;

#print XSL '<xsl:variable name="group_number" select="parent::node()/@group_number"/>' if($show_groups eq '');
#print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));



# integrate inclusions....
if($discards) {
    
    print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';

    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'' . $min_xpress . '\')' if($min_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;=\'' . $max_xpress . '\')' if($max_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));
    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));

    }
    else {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));
    }
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
    if(@exclusions > 0) {
	foreach(@exclusions) {
	    print XSL ' or $group_number=\'' . $_ . '\'';
	}
    }
    if(@pexclusions > 0) {
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or ($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';

	    }
	}
    }
    print XSL '">';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '<xsl:if test="not($group_number=\'' . $inclusions[$i] . '\')">', "\n";
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')">', "\n";
	}
    }
}
else { # conventional

    # need suffix
    my $prot_suffix = '';
    if(@pinclusions > 0) {
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		$prot_suffix .= ' or($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }    
	}
    }

    if($show_groups eq '') {
	foreach(@exclusions) {
	    print XSL '<xsl:if test="not(count(parent::node()/protx:protein)=\'1\' and $group_number=\'' . $_ . '\')' . $prot_suffix . '">';
	}
    }


    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')' . $prot_suffix . '">', "\n";
	}
    }
    if($show_groups eq '') {
	print XSL '<xsl:if test="@probability &gt;= \'' . $minprob . '\'' . $prot_suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_asap eq ''));

	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'' . $prot_suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'' . $prot_suffix . '">' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {

	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);


	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));

	}
    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'' . $prot_suffix . '">' if($max_pvalue_display < 1.0);

    }

    foreach(keys %parent_incls) {
	my @members = @{$parent_incls{$_}};
	if(@members > 0) {
	    $num_pincl++;
	    print XSL '<xsl:if test="not($group_number=\'' . $_ . '\')';
	    for(my $m = 0; $m <= $#members; $m++) {
		print XSL ' or @group_sibling_id=\'' . $members[$m] . '\'';
	    }
	    print XSL '">';
	}

    }
#####################
    print XSL '<xsl:if test="count(protx:peptide)=\'1\'">' if($SINGLE_HITS);

} # convent


# check whether part of group
print XSL '<xsl:variable name="mult_prot" select="@protein_name"/>';
print XSL '<xsl:variable name="database2" select="$ref_db"/>';
print XSL '<xsl:variable name="peps2" select="@unique_stripped_peptides"/>';
print XSL '<xsl:variable name="filextn"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn">_<xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="asap_ind" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@index"/>';
print XSL '<xsl:variable name="prot_number"><xsl:value-of select="parent::node()/@group_number"/><xsl:if test="count(parent::node()/protx:protein) &gt;\'1\'"><xsl:value-of select="@group_sibling_id"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="pvalpngfile" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']/protx:ASAP_pvalue_analysis_summary/@analysis_distribution_file"/>';

# more variables here
print XSL '<xsl:variable name="peptide_string" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@peptide_string"/>';
if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev"/>';
}
else { # reverse
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_standard_dev"/>';
}
print XSL '<xsl:variable name="xnum" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>';
print XSL '<xsl:variable name="min_pep_prob" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@min_peptide_probability"/>';
# print XSL '<xsl:variable name="source" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="heavy2light"><xsl:if test="$reference_isotope=\'heavy\'">0</xsl:if><xsl:if test="$reference_isotope=\'light\'">1</xsl:if></xsl:variable>';

#if($show_groups eq '' && ! ($show_peps eq '')) {
#    print XSL $newline;
#}

#print XSL '<xsl:apply-templates select="protx:peptide">';
#print XSL '<xsl:sort select = "@nsp_adjusted_probability" order="descending" data-type="number"/>';
#print XSL '<xsl:with-param name="pvalpngfile" select="$pvalpngfile"/>';
#print XSL '<xsl:with-param name="mult_prot" select="$mult_prot"/>';
#print XSL '<xsl:with-param name="peptide_string" select="$peptide_string"/>';
#print XSL '<xsl:with-param name="xratio" select="$xratio"/>';
#print XSL '<xsl:with-param name="xstd" select="$xstd"/>';
#print XSL '<xsl:with-param name="xnum" select="$xnum"/>';
#print XSL '<xsl:with-param name="min_pep_prob" select="$min_pep_prob"/>';
#print XSL '<xsl:with-param name="source" select="$source"/>';
#print XSL '</xsl:apply-templates>';
$tab_display{'protein'} = '<xsl:value-of select="@protein_name"/><xsl:for-each select="protx:indistinguishable_protein"><xsl:text>,</xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';
print XSL $tab_display{'protein'} . $tab;

    # here need prot prob
print XSL '<xsl:value-of select="@probability"/>' . $newline;

###########333
print XSL '</xsl:if>' if($SINGLE_HITS);

if($discards) {
    
    print XSL '</xsl:if>';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '</xsl:if>';
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }

}
else { # conve
    if($show_groups eq '') {
	print XSL '</xsl:if>' if(! ($asap_xpress eq ''));
	print XSL '</xsl:if>' if($minprob > 0);
	print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
	print XSL '</xsl:if>' if(! ($filter_asap eq ''));
	print XSL '</xsl:if>' if($min_xpress > 0);
	print XSL '</xsl:if>' if($max_xpress > 0);
	print XSL '</xsl:if>' if($min_asap > 0);
	print XSL '</xsl:if>' if($max_asap > 0);
	print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
	foreach(@exclusions) {
	    print XSL '</xsl:if>';
	}
    }
    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }
    if(! ($show_groups eq '')) {
	for(my $k = 0; $k < $num_pincl; $k++) {
	    print XSL '</xsl:if>';

	}
    }
} # conv


print XSL '</xsl:template>';


################### PEPTIDE  ###################################
print XSL '<xsl:template match="protx:peptide">';
print XSL '<xsl:param name="pvalpngfile"/>';
print XSL '<xsl:param name="mult_prot"/>';
print XSL '<xsl:param name="peptide_string"/>';
print XSL '<xsl:param name="xratio"/>';
print XSL '<xsl:param name="xstd"/>';
print XSL '<xsl:param name="xnum"/>';
print XSL '<xsl:param name="min_pep_prob"/>';
# print XSL '<xsl:param name="source"/>';
print XSL '<xsl:variable name="mypep"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="mycharge" select="@charge"/>';
print XSL '<xsl:variable name="PepMass"><xsl:if test="@calc_neutral_pep_mass"><xsl:value-of select="@calc_neutral_pep_mass"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="StdPep"><xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if></xsl:variable>';

print XSL '<xsl:variable name="myinputfiles" select="$source_files_alt"/>';
print XSL '<xsl:variable name="myprots"><xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each></xsl:variable>';

print XSL '<xsl:variable name="nspbin" select="@n_sibling_peptides_bin"/>';
print XSL '<xsl:variable name="nspval" select="@n_sibling_peptides"/>';

if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">1</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}


print XSL '<xsl:if test="@nsp_adjusted_probability &gt;=\''. $min_pepprob . '\'">' if($min_pepprob > 0);
print XSL '<xsl:if test="@n_enzymatic_termini &gt;=\''. $minntt . '\'">' if($minntt > 0);
print XSL '<xsl:if test="not(@charge=\'1\')">' if(! ($exclude_1 eq ''));
print XSL '<xsl:if test="not(@charge=\'2\')">' if(! ($exclude_2 eq ''));
print XSL '<xsl:if test="not(@charge=\'3\')">' if(! ($exclude_3 eq ''));
print XSL '<xsl:if test="@is_nondegenerate_evidence=\'Y\'">' if(! ($exclude_degens eq ''));

print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';


print XSL '<xsl:if test="position()=\'1\'">' if($show_peps eq '');


    print XSL $tab_display{'protein'} . $tab;

    # here need prot prob
    print XSL '<xsl:value-of select="parent::node()/@probability"/>' . $newline;



print XSL '</xsl:if>' if($min_pepprob > 0);
print XSL '</xsl:if>' if($minntt > 0);
print XSL '</xsl:if>' if(! ($exclude_1 eq ''));
print XSL '</xsl:if>' if(! ($exclude_2 eq ''));
print XSL '</xsl:if>' if(! ($exclude_3 eq ''));
print XSL '</xsl:if>' if(! ($exclude_degens eq ''));

#print XSL '</xsl:if>';


print XSL '</xsl:template>';



if((! ($show_sens eq '') && $eligible)) {
    print XSL '<xsl:template match="protx:protein_summary_data_filter">';
    print XSL '<xsl:value-of select="@min_probability"/>' . $tab . '<font color="red"><xsl:value-of select="@sensitivity"/></font>' . $tab . '<font color="green"><xsl:value-of select="@false_positive_error_rate"/></font>' . $tab . '<font color="red"><xsl:value-of select="@predicted_num_correct"/></font>' . $tab . '<font color="green"><xsl:value-of select="@predicted_num_incorrect"/></font>' . $newline;

    print XSL '</xsl:template>';
}

print XSL '</xsl:stylesheet>', "\n";

print XSL "\n";

close(XSL);


}

sub writeGaggleNameListXSLFile {
(my $xfile, my $boxptr) = @_;

if(! open(XSL, ">$xfile")) {
    print " cannot open $xfile: $!\n";
    exit(1);
}
print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">', "\n";
my $tab = '<xsl:value-of select="$tab"/>';
my $newline = '<xsl:value-of select="$newline"/>';
my $nonbreakline = '<xsl:value-of select="$newline"/>';
my $newlinespace = '<p/>';
my $doubleline = $newline . $newline;
my $space = '&#160';

my $num_cols = 3; # first & last


# just in case read recently from customized
$ICAT = 1 if(exists ${$boxptr}{'icat_mode'} && ${$boxptr}{'icat_mode'} eq 'yes');
$GLYC = 1 if(exists ${$boxptr}{'glyc_mode'} && ${$boxptr}{'glyc_mode'} eq 'yes');

# DEPRECATED: restore now fixed at 0 (taken care of up front)
my $restore = 0; 

my @minscore = (exists ${$boxptr}{'min_score1'} && ! (${$boxptr}{'min_score1'} eq '') ? ${$boxptr}{'min_score1'} : 0, 
		exists ${$boxptr}{'min_score2'} && ! (${$boxptr}{'min_score2'} eq '') ? ${$boxptr}{'min_score2'} : 0, 
		exists ${$boxptr}{'min_score3'} && ! (${$boxptr}{'min_score3'} eq '') ? ${$boxptr}{'min_score3'} : 0);

my $minprob = exists ${$boxptr}{'min_prob'} && ! (${$boxptr}{'min_prob'} eq '') ? ${$boxptr}{'min_prob'} : 0;
$minprob = $MIN_PROT_PROB if(! $HTML && $inital_xsl);

my $min_asap = exists ${$boxptr}{'min_asap'} && ! (${$boxptr}{'min_asap'} eq '') ? ${$boxptr}{'min_asap'} : 0;
my $max_asap = exists ${$boxptr}{'max_asap'} && ! (${$boxptr}{'max_asap'} eq '') ? ${$boxptr}{'max_asap'} : 0;
my $min_xpress = exists ${$boxptr}{'min_xpress'} && ! (${$boxptr}{'min_xpress'} eq '') ? ${$boxptr}{'min_xpress'} : 0;
my $max_xpress = exists ${$boxptr}{'max_xpress'} && ! (${$boxptr}{'max_xpress'} eq '') ? ${$boxptr}{'max_xpress'} : 0;

my $sort = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'yes';
${$boxptr}{'pep_aa'} = uc ${$boxptr}{'pep_aa'} if(exists ${$boxptr}{'pep_aa'});
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';
${$boxptr}{'mark_aa'} = uc ${$boxptr}{'mark_aa'} if(exists ${$boxptr}{'mark_aa'});
my $mark_aa = exists ${$boxptr}{'mark_aa'} && ! (${$boxptr}{'mark_aa'} eq '') ? ${$boxptr}{'mark_aa'} : '';
my $minntt = exists ${$boxptr}{'min_ntt'} && ! (${$boxptr}{'min_ntt'} eq '') ? ${$boxptr}{'min_ntt'} : 0;
my $min_pepprob = exists ${$boxptr}{'min_pep_prob'} && ! (${$boxptr}{'min_pep_prob'} eq '') ? ${$boxptr}{'min_pep_prob'} : 0;
my $maxnmc = exists ${$boxptr}{'max_nmc'} && ! (${$boxptr}{'max_nmc'} eq '') ? ${$boxptr}{'max_nmc'} : -1;

my @inclusions = exists ${$boxptr}{'inclusions'} ? split(' ', ${$boxptr}{'inclusions'}) : ();
my @exclusions = exists ${$boxptr}{'exclusions'} ? split(' ', ${$boxptr}{'exclusions'}) : ();
my @pinclusions = exists ${$boxptr}{'pinclusions'} ? split(' ', ${$boxptr}{'pinclusions'}) : ();
my @pexclusions = exists ${$boxptr}{'pexclusions'} ? split(' ', ${$boxptr}{'pexclusions'}) : ();
if(exists ${$boxptr}{'clear'} && ${$boxptr}{'clear'} eq 'yes') {
    @inclusions = ();
    @exclusions = ();
    @pinclusions = ();
    @pexclusions = ();
}

my $exclude_1 = exists ${$boxptr}{'ex1'} && ${$boxptr}{'ex1'} eq 'yes' ? $checked : '';
my $exclude_2 = exists ${$boxptr}{'ex2'} && ${$boxptr}{'ex2'} eq 'yes' ? $checked : '';
my $exclude_3 = exists ${$boxptr}{'ex3'} && ${$boxptr}{'ex3'} eq 'yes' ? $checked : '';


my $peptide_prophet_check1 = 'count(protx:peptide_prophet_summary) &gt; \'0\'';
my $peptide_prophet_check2 = 'count(parent::node()/protx:peptide_prophet_summary) &gt; \'0\'';

my $discards_init = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes';
my $discards = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes' ? $checked : '';

my $table_space = '&#160;';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
    $table_spacer = '<xsl:text>     </xsl:text>';
}

my $asap_xpress = exists ${$boxptr}{'asap_xpress'} && ${$boxptr}{'asap_xpress'} eq 'yes' ? $checked : '';
#my $show_groups = ! exists ${$boxptr}{'show_groups'} || ${$boxptr}{'show_groups'} eq 'show' ? $checked : '';
my $show_groups = '';
my $hide_groups = $show_groups eq '' ? $checked : '';

my $show_annot = ! exists ${$boxptr}{'show_annot'} || ${$boxptr}{'show_annot'} eq 'show' ? $checked : '';
my $hide_annot = $show_annot eq '' ? $checked : '';
my $show_peps = ! exists ${$boxptr}{'show_peps'} || ${$boxptr}{'show_peps'} eq 'show' ? $checked : '';
my $hide_peps = $show_peps eq '' ? $checked : '';

my $show_adjusted_asap = (! exists ${$boxptr}{'show_adjusted_asap'} && ! exists ${$boxptr}{'adj_asap'}) || (${$boxptr}{'show_adjusted_asap'} eq 'yes') ? $checked : '';

my $max_pvalue_display = exists ${$boxptr}{'max_pvalue'} && ! (${$boxptr}{'max_pvalue'} eq '') ? ${$boxptr}{'max_pvalue'} : 1.0;

my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';
my $glyc = exists ${$boxptr}{'glyc'} && ${$boxptr}{'glyc'} eq 'yes' ? $checked : '';

if(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'classic') {
    ${$boxptr}{'index'} = 1;
    ${$boxptr}{'prob'} = 2;
    ${$boxptr}{'spec_name'} = 3;
    ${$boxptr}{'neutral_mass'} = 4;
    ${$boxptr}{'massdiff'} = 5;
    ${$boxptr}{'sequest_xcorr'} = 6;
    ${$boxptr}{'sequest_delta'} = 7;
    ${$boxptr}{'sequest_spscore'} = -1;
    ${$boxptr}{'sequest_sprank'} = 8;
    ${$boxptr}{'matched_ions'} = 9;
    ${$boxptr}{'protein'} = 10;
    ${$boxptr}{'alt_prots'} = 11;
    ${$boxptr}{'peptide'} = 12;
    ${$boxptr}{'num_tol_term'} = -1;
    ${$boxptr}{'num_missed_cl'} = -1;
}
elsif(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'default') {
    ${$boxptr}{'weight'} = -1;
    ${$boxptr}{'peptide_sequence'} = -1;
    ${$boxptr}{'nsp_adjusted_probability'} = -1;
    ${$boxptr}{'initial_probability'} = -1;
    ${$boxptr}{'n_tryptic_termini'} = -1;
    ${$boxptr}{'n_sibling_peptides_bin'} = -1;
    ${$boxptr}{'n_instances'} = -1;
    ${$boxptr}{'peptide_group_designator'} = -1;
}
if(exists ${$boxptr}{'annot_order'} && ${$boxptr}{'annot_order'} eq 'default') {
    ${$boxptr}{'ensembl'} = -1;
    ${$boxptr}{'trembl'} = -1;
    ${$boxptr}{'swissprot'} = -1;
    ${$boxptr}{'refseq'} = -1;
    ${$boxptr}{'locus_link'} = -1;
}


# now add on new ones
foreach(keys %{$boxptr}) {
    if(/^excl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    if($inclusions[$i] == $1) {
		@inclusions = @inclusions[0..$i-1, $i+1..$#inclusions]; # delete it from inclusions
		$done = 1;
		$i = @inclusions;
		# cancel all previous pexclusions with same parent
		my $next_ex = $1;
		for(my $p = 0; $p <= $#pinclusions; $p++) {
		    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
			@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_ex = $1;
	push(@exclusions, $next_ex) if(! $done); # add to exclusions
	# cancel all previous pinclusions with same parent
	for(my $p = 0; $p <= $#pinclusions; $p++) {
	    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
		@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
	    }
	}

    }
    elsif(/^incl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    if($exclusions[$e] == $1) {
		@exclusions = @exclusions[0..$e-1, $e+1..$#exclusions]; # delete it from inclusions
		$done = 1;
		$e = @exclusions;
		# cancel all previous pexclusions with same parent
		my $next_in = $1;
		for(my $p = 0; $p <= $#pexclusions; $p++) {
		    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
			@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_in = $1;
	push(@inclusions, $next_in) if(! $done); # add to inclusions
	# cancel all previous pexclusions with same parent
	for(my $p = 0; $p <= $#pexclusions; $p++) {
	    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
		@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
	    }
	}
    }
}


# now add on new ones
foreach(keys %{$boxptr}) {

    if(/^pexcl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#pinclusions; $i++) {
	    if($pinclusions[$i] == $1) {
		@pinclusions = @pinclusions[0..$i-1, $i+1..$#pinclusions]; # delete it from inclusions
		$done = 1;
		$i = @pinclusions;
	    }
	}
	push(@pexclusions, $1) if(! $done); # add to exclusions
    }
    elsif(/^pincl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    if($pexclusions[$e] == $1) {
		@pexclusions = @pexclusions[0..$e-1, $e+1..$#pexclusions]; # delete it from inclusions
		$done = 1;
		$e = @pexclusions;
	    }
	}
	push(@pinclusions, $1) if(! $done); # add to inclusions
    }
}


my $exclusions = join(' ', @exclusions);
my $inclusions = join(' ', @inclusions);
my $pexclusions = join(' ', @pexclusions);
my $pinclusions = join(' ', @pinclusions);

my %parent_excls = ();
my %parent_incls = ();
foreach(@pexclusions) {
    if(/^(\d+)[a-z,A-Z]$/) {
	$parent_excls{$1}++;
    }
}
foreach(@pinclusions) {
    if(/^(\d+)([a-z,A-Z])$/) {
	if(exists $parent_incls{$1}) {
	    push(@{$parent_incls{$1}}, $2);
	}
	else {
	    my @next = ($2);
	    $parent_incls{$1} = \@next;
	}
    }
}

my $full_menu = (exists ${$boxptr}{'menu'} && ${$boxptr}{'menu'} eq 'full') || 
    (exists ${$boxptr}{'full_menu'} && ${$boxptr}{'full_menu'} eq 'yes');

my $short_menu = exists ${$boxptr}{'short_menu'} && ${$boxptr}{'short_menu'} eq 'yes';
$full_menu = 0 if($short_menu); # it takes precedence

my @minscore_display = ($minscore[0] > 0 ? $minscore[0] : '',$minscore[1] > 0 ? $minscore[1] : '',$minscore[2] > 0 ? $minscore[2] : '');
my $minprob_display = $minprob > 0 ? $minprob : '';
my $minntt_display = $minntt > 0 ? $minntt : '';
my $maxnmc_display = $maxnmc >= 0 ? $maxnmc : '';
my $min_asap_display = $min_asap > 0 ? $min_asap : '';
my $max_asap_display = $max_asap > 0 ? $max_asap : '';
my $min_xpress_display = $min_xpress > 0 ? $min_xpress : '';
my $max_xpress_display = $max_xpress > 0 ? $max_xpress : '';
my $min_pepprob_display = $min_pepprob > 0 ? $min_pepprob : '';

my $asap_display = exists ${$boxptr}{'asap_display'} && ${$boxptr}{'asap_display'} eq 'show' ? $checked : '';
my $xpress_display = exists ${$boxptr}{'xpress_display'} && ${$boxptr}{'xpress_display'} eq 'show' ? $checked : '';

my $show_ggl = exists ${$boxptr}{'show_ggl'} && ${$boxptr}{'show_ggl'} eq 'yes' ? $checked : '';

print XSL '<xsl:variable name="tab"><xsl:text>&#x09;</xsl:text></xsl:variable>', "\n";
print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
print XSL '</xsl:text></xsl:variable>';

print XSL '<xsl:variable name="libra_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="ref_db" select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/>';
print XSL '<xsl:variable name="asap_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="xpress_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="source_files" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="source_files_alt" select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/>';
print XSL '<xsl:variable name="organism"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@organism"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@organism)">UNKNOWN</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="reference_isotope"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"><xsl:value-of select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope)">UNSET</xsl:if></xsl:variable>';


my %display = ();
my %display_order = ();
my %register_order = ();
my %reg_annot_order = ();
my %display_annot_order = ();
my %header = ();
my %default_order = ();
my %tab_display = ();
my %tab_header = ();
my %annot_display = ();
my %annot_order = ();
my %annot_tab_display = ();

my $header_pre = '<font color="brown"><i>';
my $header_post = '</i></font>';

$display{'protein'} = '<xsl:value-of select="@protein"/><xsl:for-each select="protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';
$header{'protein'} = 'protein';
$tab_display{'protein'} = '<xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text>,</xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';

$default_order{'protein'} = -1;

$display{'coverage'} = '<xsl:value-of select="@percent_coverage"/>';
$header{'coverage'} = 'percent coverage';
$tab_display{'coverage'} = '<xsl:value-of select="parent::node()/@percent_coverage"/>';
$default_order{'coverage'} = -1;

$display{'annotation'} = '<xsl:if test="annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$header{'annotation'} = 'annotation';
$tab_display{'annotation'} = '<xsl:if test="annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$default_order{'annotation'} = -1;


# add here the cgi info for peptide
my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
my $html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
if(useXMLFormatLinks($xmlfile)) {
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;Ref=';
	$html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';
	$html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;';
	$html_peptide_lead3 .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
	$html_peptide_lead3 .= 'glyc=Y&amp;' if($glyc);
	$html_peptide_lead3 .= 'libra={$libra_quant}&amp;';
	$html_peptide_lead3 .= 'Ref=';

	$html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep2}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';

}
my $html_peptide_mid = '&amp;Infile=';


if($DISPLAY_MODS) {
    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_peptide_lead3 . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_peptide_lead3 . '{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if></td>';
}
else {
    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_peptide_lead . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . '<xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:value-of select="@peptide_sequence"/></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_peptide_lead . '{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . '<xsl:value-of select="@peptide_sequence"/></A></xsl:if></td>';
}
my $display_ind_peptide_seq = '<td class="indist_pep">--' . $html_peptide_lead4 . '{$mycharge2}_{$mypep}' . $html_peptide_mid . '{$myinputfiles2}">' . '<xsl:value-of select="parent::node()/@charge"/>_<xsl:value-of select="@peptide_sequence"/></A></td>';

$header{'peptide_sequence'} = '<td>' . $header_pre . 'peptide sequence' . $header_post . '</td>';
$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:value-of select="@peptide_sequence"/><xsl:for-each select="indistinguishable_peptide">,<xsl:value-of select="@peptide_sequence"/></xsl:for-each>';
$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence';

#print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';
#print XSL '<xsl:variable name="database" select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/>';

$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if>' . $tab . $TPPhostname . $CGI_HOME . 'peptidexml_html2.pl?PepMass=<xsl:value-of select="@calc_neutral_pep_mass"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>StdPep=<xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>MassError=' . $MOD_MASS_ERROR . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>cgi-bin=' . $CGI_HOME . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>ratioType=<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>';

$tab_display{'peptide_sequence'} .= 'mark_aa=' . $mark_aa . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if(! ($mark_aa eq ''));
$tab_display{'peptide_sequence'} .= 'glyc=Y<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if($glyc);
$tab_display{'peptide_sequence'} .= 'Ref=<xsl:value-of select="@charge"/>_<xsl:value-of select="@peptide_sequence"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Infile=<xsl:value-of select="$source_files_alt"/>';

$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence' . $tab . 'peptide link';

$default_order{'peptide_sequence'} = 2;


my $wt_header = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'prot_wt_xml.pl?xmlfile=' . $xmlfile . '&amp;cgi-home=' . $CGI_HOME . '&amp;xslt=' . $xslt . '&amp;quant_light2heavy=' . $quant_light2heavy . '&amp;modpep={$StdPep}&amp;pepmass={$PepMass}&amp;';
$wt_header .= 'xml_input=1&amp;' if(! $DISTR_VERSION && $NEW_XML_FORMAT);
$wt_header .= 'glyc=1&amp;' if($glyc);
$wt_header .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
$wt_header .= 'peptide=';
my $wt_suf = '</A>';


$tab_display{'is_nondegen_evidence'} = '<xsl:value-of select="@is_nondegenerate_evidence"/>';
$tab_header{'is_nondegen_evidence'} = 'is nondegenerate evidence';
$default_order{'is_nondegen_evidence'} = 0.5;


$display{'weight'} = '<td><xsl:if test="@is_nondegenerate_evidence = \'Y\'"><font color="#990000">*</font></xsl:if></td><td>' . $wt_header . '{$mypep}&amp;charge={$mycharge}&amp;">' . '<nobr>wt-<xsl:value-of select="@weight"/><xsl:text> </xsl:text></nobr>' . $wt_suf . '&nbsp;&nbsp;</td>';
$header{'weight'} = '<td></td><td>' . $header_pre . 'weight' . $header_post . '</td>';
$tab_display{'weight'} = '<xsl:value-of select="@weight"/>';
$default_order{'weight'} = 1;

$display{'nsp_adjusted_probability'} = '<td><xsl:if test="@is_contributing_evidence = \'Y\'"><font COLOR="#FF9933"><xsl:value-of select="@nsp_adjusted_probability"/></font></xsl:if><xsl:if test="@is_contributing_evidence = \'N\'"><xsl:value-of select="@nsp_adjusted_probability"/></xsl:if></td>';
$header{'nsp_adjusted_probability'} = '<td>' . $header_pre . 'nsp adj prob' . $header_post . '</td>';
$tab_display{'nsp_adjusted_probability'} = '<xsl:value-of select="@nsp_adjusted_probability"/>';
$tab_header{'nsp_adjusted_probability'} = 'nsp adjusted probability';
$default_order{'nsp_adjusted_probability'} = 3;

$display{'initial_probability'} = '<td><xsl:value-of select="@initial_probability"/></td>';
$header{'initial_probability'} = '<td>' . $header_pre . 'init prob' . $header_post . '</td>';
$tab_display{'initial_probability'} = '<xsl:value-of select="@initial_probability"/>';
$tab_header{'initial_probability'} = 'initial probability';
$default_order{'initial_probability'} = 4;

$display{'num_tol_term'} = '<td><xsl:value-of select="@n_enzymatic_termini"/></td>';
$header{'num_tol_term'} = '<td>' . $header_pre . 'ntt' . $header_post . '</td>';
$tab_display{'num_tol_term'} = '<xsl:value-of select="@n_enzymatic_termini"/>';
$tab_header{'num_tol_term'} = 'n tol termini';
$default_order{'num_tol_term'} = 5;

my $nsp_pre = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'show_nspbin.pl?xmlfile=' . $xmlfile . '&amp;nsp_bin={$nspbin}&amp;nsp_val={$nspval}&amp;charge={$mycharge}&amp;pep={$mypep}&amp;prot={$myprots}">';
my $tempnsp = '<td><xsl:if test="@n_sibling_peptides">' . $nsp_pre . '<xsl:value-of select="@n_sibling_peptides"/></A></xsl:if>
<xsl:if test="not(@n_sibling_peptides)"><xsl:value-of select="@n_sibling_peptides_bin"/></xsl:if></td>';
$display{'n_sibling_peptides_bin'} = $tempnsp;

$header{'n_sibling_peptides_bin'} = '<td>' . $header_pre . 'nsp<xsl:if test="not(protx:peptide/@n_sibling_peptides)"> bin</xsl:if>' . $header_post . '</td>';
$tab_display{'n_sibling_peptides_bin'} = '<xsl:value-of select="@n_sibling_peptides_bin"/>';
$tab_header{'n_sibling_peptides_bin'} = 'n sibling peptides bin';
$default_order{'n_sibling_peptides_bin'} = 6;

$display{'peptide_group_designator'} = '<td><xsl:if test="@peptide_group_designator"><font color="#DD00DD"><xsl:value-of select="@peptide_group_designator"/>-<xsl:value-of select="@charge"/></font></xsl:if></td>';
$header{'peptide_group_designator'} = '<td>' . $header_pre . 'pep grp ind' . $header_post . '</td>';
$tab_display{'peptide_group_designator'} = '<xsl:value-of select="@peptide_group_designator"/>';
$tab_header{'peptide_group_designator'} = 'peptide group designator';
$default_order{'peptide_group_designator'} = 8;


$header{'group_number'} = 'entry no.';
$tab_display{'group_number'} = '<xsl:value-of select="parent::node()/parent::node()/@group_number"/><xsl:if test="count(parent::node()/parent::node()/protx:protein) &gt; \'1\'"><xsl:value-of select="parent::node()/@group_sibling_id"/></xsl:if>';
$default_order{'group_number'} = -1;
$display{'group_number'} = '';
$tab_header{'group_number'} = 'entry no.';

$annot_display{'description'} = '<xsl:if test="protx:annotation/@protein_description"><xsl:if test="$organism = \'UNKNOWN\'"><font color="green"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if><xsl:if test="not($organism = \'UNKNOWN\')"><font color="#008080"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if>' . $table_space . ' </xsl:if>';

$annot_order{'description'} = -1;
$annot_tab_display{'description'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@protein_description"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if></xsl:for-each>';

$header{'description'} = 'description';

my $flybase_header = '<a TARGET="Win1" href="http://flybase.bio.indiana.edu/.bin/fbidq.html?';

$tab_header{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'">flybase' . $tab . '</xsl:if>';
$annot_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="protx:annotation/@flybase"><xsl:variable name="flybase" select="protx:annotation/@flybase"/>' . $flybase_header . '{$flybase}"><font color="green">Flybase:<xsl:value-of select="$flybase"/></font></a>' . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'flybase'} = 9;
$annot_tab_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@flybase"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@flybase"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';

my $ipi_header = '<a TARGET="Win1" href="http://srs.ebi.ac.uk/cgi-bin/wgetz?-id+m_RJ1KrMXG+-e+[IPI-acc:';
my $ipi_mid = ']">';
my $ipi_suf = '</a>';
$annot_display{'ipi'} = '<font color="green">&gt;</font><xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="annotation/@ipi_name"><xsl:variable name="ipi" select="annotation/@ipi_name"/>' . $ipi_header . '{$ipi}' . $ipi_mid . '<font color="green">IPI:<xsl:value-of select="$ipi"/></font>' . $ipi_suf . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'ipi'} = -1;
$annot_tab_display{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ipi_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ipi_name"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';


$header{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')">ipi' . $tab . '</xsl:if>';

my $embl_header = '<a TARGET="Win1" href="http://www.ensembl.org/';
my $embl_mid = '/protview?peptide=';
my $embl_suf = '</a>';
$annot_display{'ensembl'} = '<xsl:if test="protx:annotation/@ensembl_name"><xsl:variable name="org" select="$organism"/><xsl:variable name="ensembl" select="protx:annotation/@ensembl_name"/>' . $embl_header . '{$org}' . $embl_mid . '{$ensembl}"><font color="green">E:<xsl:value-of select="$ensembl"/></font>' . $embl_suf . $table_space . ' </xsl:if>';
$annot_order{'ensembl'} = 3;
$annot_tab_display{'ensembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ensembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ensembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'ensembl'} = 'ensembl' . $tab;

$annot_display{'trembl'} = '<xsl:if test="protx:annotation/@trembl_name"><font color="green">Tr:<xsl:value-of select="protx:annotation/@trembl_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'trembl'} = 4;
$annot_tab_display{'trembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@trembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@trembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'trembl'} = 'trembl' . $tab;

$annot_display{'swissprot'} = '<xsl:if test="protx:annotation/@swissprot_name"><font color="green">Sw:<xsl:value-of select="protx:annotation/@swissprot_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'swissprot'} = 5;
$annot_tab_display{'swissprot'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@swissprot_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@swissprot_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'swissprot'} = 'swissprot' . $tab;

$annot_display{'refseq'} = '<xsl:if test="protx:annotation/@refseq_name"><font color="green">Ref:<xsl:value-of select="protx:annotation/@refseq_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'refseq'} = 6;
$annot_tab_display{'refseq'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@refseq_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@refseq_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'refseq'} = 'refseq' . $tab;

my $locus_header = '<a TARGET="Win1" href="http://www.ncbi.nlm.nih.gov/LocusLink/LocRpt.cgi?l=';
my $locus_suf = '</a>';

$annot_display{'locus_link'} = '<xsl:if test="protx:annotation/@locus_link_name"><xsl:variable name="loc" select="protx:annotation/@locus_link_name"/>' . $locus_header . '{$loc}' . '"><font color="green">LL:<xsl:value-of select="$loc"/></font>' . $locus_suf . $table_space . ' </xsl:if>';
$annot_order{'locus_link'} = 7;
$annot_tab_display{'locus_link'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@locus_link_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@locus_link_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'locus_link'} = 'locus link' . $tab;




my $asap_file_pre = '';
my $asap_proph_suf = '_prophet.bof';
my $asap_pep_suf = '_peptide.bof';
my $asap_proph = '';
my $asap_pep = '';
if($xfile =~ /^(\S+\/)/) { # assume in same directory
    $asap_proph = $1 . 'ASAPRatio_prophet.bof';
    $asap_pep = $1 . 'ASAPRatio_peptide.bof';
    $asap_file_pre = $1 . 'ASAPRatio';
}


my $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . 'xli/ASAPRatio_lstProRatio.cgi?orgFile=';
my $asap_header_mid = '.orig&amp;proBofFile=' . $asap_file_pre . '{$filextn}' . $asap_proph_suf . '&amp;pepBofFile=' . $asap_file_pre . '{$filextn}' . $asap_pep_suf . '&amp;proIndx=';


my $plusmn = '&#177;';
$plusmn = ' +-' if($xslt =~ /xsltproc/);

my $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'xpress-prophet.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light={$heavy2light}">';

my $xpress_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $xpress_link;
if(useXMLFormatLinks($xmlfile)) {
    $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light=' . $xpress_ratio_type;
    $xpress_pre .= '&amp;mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
    $xpress_pre .= '">'; #{$heavy2light}">';

    $xpress_link = $TPPhostname . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>protein=<xsl:value-of select="$mult_prot"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>peptide_string=<xsl:value-of select="$peptide_string"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>ratio=<xsl:value-of select="$xratio"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>stddev=<xsl:value-of select="$xstd"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>num=<xsl:value-of select="$xnum"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlfile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>min_pep_prob=<xsl:value-of select="$min_pep_prob"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>source_files=<xsl:value-of select="$source_files"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>heavy2light=' . $xpress_ratio_type;
    $xpress_link .= '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
}

my $xpress_suf = '</a>';

$num_cols = 3;



$display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'"><td width="250"><xsl:if test="protx:analysis_result[@analysis=\'libra\']">LIBRA<xsl:text> </xsl:text>(<xsl:value-of select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>)<xsl:for-each select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><br/><xsl:value-of select="@mz"/>:<xsl:text> </xsl:text><xsl:value-of select="@ratio"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="@error"/><xsl:text> </xsl:text></xsl:for-each></xsl:if></td></xsl:if>';

$tab_display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\' and parent::node()/protx:analysis_result[@analysis=\'libra\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>' . $tab . '<xsl:for-each select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><xsl:value-of select="@ratio"/>' . $tab . '<xsl:value-of select="@error"/>' . $tab . '</xsl:for-each></xsl:if>';

$header{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'">LIBRA number peptides' . $tab . '<xsl:for-each select="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']/protx:libra_summary/protx:fragment_masses">LIBRA <xsl:value-of select="@mz"/> ratio' . $tab . 'LIBRA <xsl:value-of select="@mz"/> error' . $tab . '</xsl:for-each></xsl:if>';



if($xpress_display eq $checked) {

    $display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'xpress\']">XPRESS';
    if(! ($quant_light2heavy eq 'true')) {
	$display{'xpress'} .= '(H/L)';
    }
    $display{'xpress'} .= ': ' . $xpress_pre . '<xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>(<xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>)</xsl:if>' . $xpress_suf . '</xsl:if><xsl:if test="not(protx:analysis_summary[@analysis=\'xpress\'])">' . $table_spacer . '</xsl:if></td></xsl:if>';

    $tab_display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'xpress\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>' . $tab . $xpress_link . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'xpress\'])">'  . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';

    $header{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'">xpress';
    if(! ($quant_light2heavy eq 'true')) {
	$header{'xpress'} .= '(H/L)';
    }
    $header{'xpress'} .= ' ratio mean' . $tab . 'xpress<xsl:if test="not($reference_isotope = \'UNSET\')"><xsl:if test="$reference_isotope=\'light\'"> (H/L)</xsl:if></xsl:if> stdev' . $tab . 'xpress num peptides' . $tab . 'xpress link' . $tab. '</xsl:if>';
} # if display xpress

my $NEW_ASAP_CGI = 1;

my $asap_display_cgi = 'asap-prophet-display.cgi';
if(useXMLFormatLinks($xmlfile)) {
    $asap_display_cgi = 'ASAPCGIDisplay.cgi';
}
my $asap_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $asap_link;
if($NEW_ASAP_CGI) {
    $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '&amp;xmlFile=' . $xmlfile . '&amp;';
    $asap_header_pre .= 'markAA=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
    $asap_header_pre .= 'protein=';

    $asap_link = $TPPhostname . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlFile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>';
    $asap_link .= 'markAA=' . $mark_aa . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>' if(! ($mark_aa eq ''));
    $asap_link .= 'protein=<xsl:value-of select="$mult_prot"/>';
}



my $asap_header_mid2 = '&amp;ratioType=0">';

my $asap_header_suf = '</A>';
my $pvalue_header_pre = '<a target="Win1" href="';

my $pvalue_header_suf = '</a>';
if($asap_display eq $checked) {
    if($NEW_ASAP_CGI) {

	# first display regular ratio no matter what
	$display{'asapratio'} = '<xsl:if test="$asap_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']">ASAPRatio';
	if(! ($quant_light2heavy eq 'true')) {
	    $display{'asapratio'} .= '(H/L)';
	}
	$display{'asapratio'} .= ': ' . $asap_header_pre . '{$mult_prot}">';
	$display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/></xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	# now add on the adjusted only if desired and present
	if ($show_adjusted_asap ne '') {
	    $display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\']">[<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_standard_dev"/>]</xsl:if>';
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><td width="200"><xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if>';

  
	}
	else {
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if>';

	}

    }
    else { # old format
	if($show_adjusted_asap eq '') {
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/></xsl:if><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	    }
	    else { # display adjsuted
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="400"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . ' [<xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_standard_dev"/>]';
	    }
	    $display{'asapratio'} .= '</xsl:if></td><td width="200"><xsl:if test="protx:ASAPRatio">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:ASAPRatio/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if><xsl:if test="not(/protx:protein_summary/protx:ASAP_pvalue_analysis_summary)"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . '</xsl:if></td></xsl:if></xsl:if>';
    } # if old version
 

    $tab_display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>' . $tab . $asap_link . $tab . '</xsl:if><xsl:if test="count(parent::node()/protx:analysis_result[@analysis=\'asapratio\'])= \'0\'">' . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab .'N_A' . $tab . '</xsl:if></xsl:if>';
    
    if(! ($show_adjusted_asap eq '')) { # display adjusted
	$tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev"/>' . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\'])">' .'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';
    }
    $tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></xsl:if>' . $tab . $TPPhostname . '<xsl:value-of select="$pvalpngfile"/>' . $tab .'</xsl:if>';

    $header{'asapratio'} = '<xsl:if test="count(protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">ratio mean' . $tab . 'ratio stdev' . $tab . 'ratio num peps' . $tab . 'asap ratio link'. $tab;
    if(! ($show_adjusted_asap eq '')) {
	$header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">adjusted ratio mean' . $tab . 'adjusted ratio stdev' . $tab . '</xsl:if>';
    }
    $header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">pvalue' . $tab . 'pvalue link'. $tab. '</xsl:if></xsl:if>';
} # if display asapratio info

my $reference = '$group_number' ; #$show_groups eq '' ? '$parental_group_number' : '$group_number';
my $gn = $show_groups eq '' ? '<xsl:value-of select="parent::node()/@group_number"/>' : '<xsl:value-of select="@group_number"/>';
if($discards) {

    $display{'group_number'} .= '<input type="checkbox" name="incl{' . $reference . '}" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@exclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
else {

    $display{'group_number'} .= '<input type="checkbox" name="excl';
    if($show_groups eq '') {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    else {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    $display{'group_number'} .= '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@inclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
$display{'group_number'} .= '<a name="{' . $reference . '}"/>' if($HTML);
$display{'group_number'} .= '<xsl:text> </xsl:text>';

$display{'prot_number'} = '';
if($discards) {

    $display{'prot_number'} .= '<input type="checkbox" name="pincl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pexclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pexclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pexclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
else {

    $display{'prot_number'} .= '<input type="checkbox" name="pexcl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pinclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pinclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pinclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
$display{'prot_number'} .= '<xsl:text> </xsl:text>';

$display{'n_instances'} = '<td><xsl:value-of select="@n_instances"/></td>';
$header{'n_instances'} = '<td>' . $header_pre . 'total' . $header_post . '</td>';
$tab_display{'n_instances'} = '<xsl:value-of select="@n_instances"/>';
$default_order{'n_instances'} = 7;
$tab_header{'n_instances'} = 'n instances';


foreach(keys %display) {
    $display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}

foreach(keys %annot_display) {
    $display_annot_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $reg_annot_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}


# test it out privately
if(0 && scalar keys %display_order > 0) {
    open(TEMP, ">temp.out");
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	print TEMP $display{$_}, "\n";
    }
    close(TEMP);
}

print XSL "\n";
# define tab and newline here


print XSL '<xsl:template match="protx:protein_summary">', "\n";
    printCountProtsXSL($boxptr);

print XSL '<HTML><BODY BGCOLOR="white" OnLoad="self.focus()"><PRE>', "\n";
print XSL '<HEAD><TITLE>ProteinProphet protXML Viewer (' . $TPPVersionInfo . ')</TITLE>', "\n";
print XSL '<STYLE TYPE="text/css" media="screen">', "\n";
print XSL '    div.visible {', "\n";
print XSL '    display: inline;', "\n";
print XSL '    white-space: nowrap;', "\n";
print XSL '    }', "\n";
print XSL '    div.hidden {', "\n";
print XSL '    display: none;', "\n";
print XSL '    }', "\n";
print XSL '    results {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.peptide {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.indist_pep {', "\n";
print XSL '	font-size: 10pt;', "\n";
print XSL '    }', "\n";
print XSL '    indist_pep_mod {', "\n";
print XSL '	font-size: 8pt;', "\n";
print XSL '    }', "\n";
print XSL '</STYLE></HEAD>', "\n";
print XSL '<table width="100%" border="3" BGCOLOR="#AAAAFF" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;"><tr><td align="center">', "\n";
print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";
if($HTML) {
    print XSL '<input type="submit" value="Restore Last View" style="background-color:#FFFF88;"/>', "\n";
    print XSL '<input type="hidden" name="restore_view" value="yes"/>', "\n";
}
else {
    print XSL '<input type="submit" value="Restore Original"/>', "\n";
    print XSL '<input type="hidden" name="restore" value="yes"/>', "\n";
}
    print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
    print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
    print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);



    print XSL '</form>';
    print XSL '</td><td align="center">';
    print XSL '<pre>ProteinProphet<sup><font size="3">&#xAE;</font></sup> protXML Viewer</pre>A.Keller   2.23.05</td>';

my $sort_none = ! exists ${$boxptr}{'sort'} || ${$boxptr}{'sort'} eq 'none' ?  $checked : '';
my $sort_xcorr = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xcorr' ? $checked : '';
my $sort_prob = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'prob' ? $checked : '';
my $sort_spec = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spec' ? $checked : '';
my $sort_pep = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'peptide' ? $checked : '';
my $sort_prot = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'protein' ? $checked : '';
my $sort_cov = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'coverage' ? $checked : '';
my $sort_peps = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'numpeps' ? $checked : '';
my $sort_spec_ids = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spectrum_ids' ? $checked : '';

my $sort_pvalue = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'pvalue' ? $checked : '';
my $text1 = exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq '') ? ${$boxptr}{'text1'} : '';
my $sort_asap_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_desc' ? $checked : '';
my $sort_asap_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_asc' ? $checked : '';
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ${$boxptr}{'filter_asap'} eq 'yes' ? $checked : '';
my $filter_xpress = exists ${$boxptr}{'filter_xpress'} && ${$boxptr}{'filter_xpress'} eq 'yes' ? $checked : '';
my $sort_xpress_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_desc' ? $checked : '';
my $sort_xpress_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_asc' ? $checked : '';
my $exclude_degens = exists ${$boxptr}{'no_degens'} && ${$boxptr}{'no_degens'} eq 'yes' ? $checked : '';

# show sens error info (still relevant for filtered dataset)
my $show_sens = exists ${$boxptr}{'senserr'} && ${$boxptr}{'senserr'} eq 'show' ? $checked : '';
my $eligible = ($filter_asap eq '' && $min_asap == 0 && $max_asap == 0 && (! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') && @exclusions == 0 && @inclusions == 0 && @pexclusions == 0 && @pexclusions == 0 && $filter_xpress eq '' && $min_xpress == 0 && $max_xpress == 0 && $asap_xpress eq '');
my $show_tot_num_peps = ! exists ${$boxptr}{'tot_num_peps'} || ${$boxptr}{'tot_num_peps'} eq 'show' ? $checked : '';
my $show_num_unique_peps = ! exists ${$boxptr}{'num_unique_peps'} || ${$boxptr}{'num_unique_peps'} eq 'show' ? $checked : '';
my $show_pct_spectrum_ids = ! exists ${$boxptr}{'pct_spectrum_ids'} || ${$boxptr}{'pct_spectrum_ids'} eq 'show' ? $checked : '';

my $suffix = $HTML_ORIENTATION ? '.htm' : '.xml';
$suffix = '.shtml' if($SHTML);

# write output xml
print XSL '<td align="center"><form method="GET" target="Win1" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";

print XSL '<pre>';
print XSL '<input type="submit" value="Write Displayed Data Subset to File" /><pre>' . $nonbreakline . '</pre>';
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";

print XSL '<input type="hidden" name="ex1" value="yes"/>', "\n" if(! ($exclude_1 eq ''));
print XSL '<input type="hidden" name="ex2" value="yes"/>', "\n" if(! ($exclude_2 eq ''));
print XSL '<input type="hidden" name="ex3" value="yes"/>', "\n" if(! ($exclude_3 eq ''));
print XSL '<input type="hidden" name="text1" value="' . ${$boxptr}{'text1'} . '"/>' if(exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq ''));
print XSL '<input type="hidden" name="min_prob" value="' . $minprob . '"/>' if($minprob > 0);
print XSL '<input type="hidden" name="min_score1" value="' . $minscore[0] . '"/>' if($minscore[0] > 0);
print XSL '<input type="hidden" name="min_score2" value="' . $minscore[1] . '"/>' if($minscore[1] > 0);
print XSL '<input type="hidden" name="min_score3" value="' . $minscore[2] . '"/>' if($minscore[2] > 0);
print XSL '<input type="hidden" name="min_ntt" value="' . $minntt . '"/>' if($minntt > 0);
print XSL '<input type="hidden" name="max_nmc" value="' . $maxnmc . '"/>' if($maxnmc >= 0);
print XSL '<input type="hidden" name="pep_aa" value="' . $pep_aa . '"/>' if(! ($pep_aa eq ''));
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>' if(! ($inclusions eq ''));
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>' if(! ($exclusions eq ''));
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>' if(! ($pinclusions eq ''));
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>' if(! ($pexclusions eq ''));
print XSL '<input type="hidden" name="filter_asap" value="yes"/>' if(! ($filter_asap eq ''));
print XSL '<input type="hidden" name="filter_xpress" value="yes"/>' if(! ($filter_xpress eq ''));
print XSL '<input type="hidden" name="min_pepprob" value="' . $min_pepprob . '"/>' if(! ($min_pepprob eq ''));
print XSL '<input type="hidden" name="show_groups" value="yes"/>' if(! ($show_groups eq ''));
print XSL '<input type="hidden" name="min_xpress" value="' . $min_xpress . '"/>' if($min_xpress > 0);
print XSL '<input type="hidden" name="max_xpress" value="' . $max_xpress . '"/>' if($max_xpress > 0);
print XSL '<input type="hidden" name="min_asap" value="' . $min_asap . '"/>' if($min_asap > 0);
print XSL '<input type="hidden" name="max_asap" value="' . $max_asap . '"/>' if($max_asap > 0);
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="show_adjusted_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="adj_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="max_pvalue" value="' . $max_pvalue_display . '"/>' if($max_pvalue_display < 1.0);
print XSL '<input type="hidden" name="asap_xpress" value="yes"/>' if(! ($asap_xpress eq ''));
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

print XSL 'file name: <input type="text" name="outfile" value="" size="20" maxlength="100"/>' . $suffix . '</pre>', "\n";
print XSL '</form></td></tr></table>';

print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl"><table width="100%" border="3" BGCOLOR="#AAAAFF">';
print XSL '<tr><td align="left" valign="center"><pre><input type="checkbox" name="show_ggl" value="yes" ' . $show_ggl . '/><b>Enable Gaggle Broadcast</b><A TARGET="Win1" HREF="http://tools.proteomecenter.org/wiki/index.php?title=Software:Firegoose%2C_Gaggle%2C_and_PIPE"><IMG BORDER="0" SRC="'. $HELP_DIR. 'images/qMark.png"/></A></pre></td></tr><tr><td><pre>';

if($discards) {
    print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
}
else {
    print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
}

print XSL $table_spacer . '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_sens_err.pl?xmlfile=' . $xmlfile;
print XSL '&amp;' if(! $DISTR_VERSION);

print XSL '">Sensitivity/Error Info</a></xsl:if><xsl:if test="protx:dataset_derivation/@generation_no &gt;\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_dataset_derivation.pl?xmlfile=' . $xmlfile . '">Dataset Derivation Info</a></xsl:if>';;

print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'more_anal.pl?xmlfile=' . $xmlfile;
print XSL '&amp;shtml=yes' if($SHTML);
print XSL '&amp;helpdir=' . $HELP_DIR;
print XSL '">More Analysis Info</a>';
print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'show_help.pl?help_dir=' . $HELP_DIR . '">Help</a>';
print XSL $newline;



print XSL '<xsl:text> </xsl:text>';
print XSL 'sort by: <input type="radio" name="sort" value="none" ' . $sort_none . '/>index';
print XSL '<input type="radio" name="sort" value="prob" ' . $sort_prob, '/>probability';
print XSL ' <input type="radio" name="sort" value="protein" ' . $sort_prot, '/>protein';
print XSL ' <input type="radio" name="sort" value="coverage" ' . $sort_cov, '/>coverage';

print XSL '<xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids)"> <input type="radio" name="sort" value="numpeps" ' . $sort_peps, '/>num peps</xsl:if>';
print XSL '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids"> <input type="radio" name="sort" value="spectrum_ids" ' . $sort_spec_ids, '/>share of spectrum ids</xsl:if>';
print XSL '<xsl:if test="$xpress_quant &gt; \'0\' and count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"> ';
print XSL $newline . '<xsl:text>          </xsl:text>';
print XSL '</xsl:if>';

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"> ';
print XSL ' <input type="radio" name="sort" value="xpress_desc" ' . $sort_xpress_desc, '/>xpress desc';
print XSL ' <input type="radio" name="sort" value="xpress_asc" ' . $sort_xpress_asc, '/>xpress asc';
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';
print XSL ' <input type="radio" name="sort" value="asap_desc" ' . $sort_asap_desc, '/>asap desc';
print XSL ' <input type="radio" name="sort" value="asap_asc" ' . $sort_asap_asc, '/>asap asc';
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"> <input type="radio" name="sort" value="pvalue" ' . $sort_pvalue, '/>pvalue</xsl:if>';
print XSL '</xsl:if>';

print XSL $newline;

print XSL '<xsl:text> </xsl:text>min probability: <INPUT TYPE="text" NAME="min_prob" VALUE="' . $minprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
# pick one of the following
print XSL $newline;
print XSL '<xsl:text> </xsl:text>protein groups: <input type="radio" name="show_groups" value="show" ' . $show_groups . '/>show  ';
print XSL '<input type="radio" name="show_groups" value="hide" ' . $hide_groups . '/>hide  ';

print XSL '   annotation: <input type="radio" name="show_annot" value="show" ' . $show_annot . '/>show  ';
print XSL '<input type="radio" name="show_annot" value="hide" ' . $hide_annot . '/>hide  ';
print XSL '   peptides: <input type="radio" name="show_peps" value="show" ' . $show_peps . '/>show  ';
print XSL '<input type="radio" name="show_peps" value="hide" ' . $hide_peps . '/>hide  ';
print XSL $newline;

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o XPRESS Ratio: <input type="checkbox" name="filter_xpress" value="yes" ' . $filter_xpress . '/>';
print XSL '  min XPRESS Ratio: <INPUT TYPE="text" NAME="min_xpress" VALUE="', $min_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max XPRESS Ratio: <INPUT TYPE="text" NAME="max_xpress" VALUE="', $max_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'">  ASAPRatio consistent: <input type="checkbox" name="asap_xpress" value="yes" ' . $asap_xpress . '/></xsl:if>';

print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o ASAPRatio: <input type="checkbox" name="filter_asap" value="yes" ' . $filter_asap . '/>';
print XSL '  min ASAPRatio: <INPUT TYPE="text" NAME="min_asap" VALUE="', $min_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max ASAPRatio: <INPUT TYPE="text" NAME="max_asap" VALUE="', $max_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
my $alt_max = $max_pvalue_display < 1.0 ? $max_pvalue_display : '';

print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">  max pvalue: <INPUT TYPE="text" NAME="max_pvalue" VALUE="', $alt_max, '" SIZE="3" MAXLENGTH="8"/>  adjusted: <input type="checkbox" name="show_adjusted_asap" value="yes" ' . $show_adjusted_asap . '/><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<xsl:text> </xsl:text><input type="submit" name="action" value="Recompute p-values"/>';
print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:text> </xsl:text>exclude degen peps: <input type="checkbox" name="no_degens" value="yes" ' . $exclude_degens . '/>';
print XSL '  exclude charge: <input type="checkbox" name="ex1" value="yes" ' . $exclude_1 . '/>1+';
print XSL '<input type="checkbox" name="ex2" value="yes" ' . $exclude_2 . '/>2+';
print XSL '<input type="checkbox" name="ex3" value="yes" ' . $exclude_3 . '/>3+' . '<xsl:text>   </xsl:text>';

print XSL 'min pep prob: <INPUT TYPE="text" NAME="min_pep_prob" VALUE="' . $min_pepprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
print XSL ' min num tol term: <INPUT TYPE="text" NAME="min_ntt" VALUE="', $minntt_display, '" SIZE="1" MAXLENGTH="1"/><xsl:text> </xsl:text>';
print XSL $newline;


print XSL '<xsl:text> </xsl:text>include aa: <INPUT TYPE="text" NAME="pep_aa" VALUE="', $pep_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   mark aa: <INPUT TYPE="text" NAME="mark_aa" VALUE="', $mark_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   NxS/T: <input type="checkbox" name="glyc" value="yes" ' . $glyc . '/><xsl:text>   </xsl:text>';
print XSL 'protein text: <input type="text" name="text1" value="', $text1, '" size="12" maxlength="24"/><xsl:text>   </xsl:text>';
print XSL 'export to excel: <input type="checkbox" name="excel" value="yes" />', "\n";
print XSL '<input type="hidden" name="restore" value="no"/>', "\n";
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>', "\n";
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>', "\n";
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="glyc" value="yes"/>' if($glyc);
print XSL '<input type="hidden" name="xml_input" value="1"/>' if(! $DISTR_VERSION && $NEW_XML_FORMAT);
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="asapratio_pvalue" value="yes"/></xsl:if>';



if($full_menu) {
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL '<xsl:text> </xsl:text>sensitivity/error information: <input type="radio" name="senserr" value="show" ';
    print XSL $checked if(! ($show_sens eq ''));
    print XSL '/>show<input type="radio" name="senserr" value="hide" ';
    print XSL $checked if($show_sens eq '');
    print XSL '/>hide' . $newline;
    
    print XSL '<pre>' . $newline . '</pre>';


    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><xsl:text> </xsl:text>Quantitation Ratio: <input type="radio" name="quant_light2heavy" value="true" ';
	print XSL $checked if(! ($quant_light2heavy eq 'false'));
	print XSL '/>light/heavy<input type="radio" name="quant_light2heavy" value="false" ';
	print XSL $checked if($quant_light2heavy eq 'false');
	print XSL '/>heavy/light';
	print XSL '<pre>' . $newline . '</pre></xsl:if>';
    } # only for xml version

    print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:text> </xsl:text>XPRESS information: <input type="radio" name="xpress_display" value="show" ';
    print XSL $checked if($xpress_display eq $checked);
    print XSL '/>show<input type="radio" name="xpress_display" value="hide" ';
    print XSL $checked if($xpress_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:if test="$asap_quant &gt; \'0\'"><xsl:text> </xsl:text>ASAPRatio information: <input type="radio" name="asap_display" value="show" ';
    print XSL $checked if($asap_display eq $checked);
    print XSL '/>show<input type="radio" name="asap_display" value="hide" ';
    print XSL $checked if($asap_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:text> </xsl:text>protein display  ' . $newline;
    print XSL '<xsl:text> </xsl:text>number unique peptides: <input type="radio" name="num_unique_peps" value="show" ';
    print XSL $checked if(! ($show_num_unique_peps eq ''));
    print XSL '/>show<input type="radio" name="num_unique_peps" value="hide" ';
    print XSL $checked if($show_num_unique_peps eq '');
    print XSL '/>hide';
    print XSL '   total number peptides: <input type="radio" name="tot_num_peps" value="show" ';
    print XSL $checked if(! ($show_tot_num_peps eq ''));
    print XSL '/>show<input type="radio" name="tot_num_peps" value="hide" ';
    print XSL $checked if($show_tot_num_peps eq '');
    print XSL '/>hide';

    print XSL '   share of spectrum ids: <input type="radio" name="pct_spectrum_ids" value="show" ';
    print XSL $checked if(! ($show_pct_spectrum_ids eq ''));
    print XSL '/>show<input type="radio" name="pct_spectrum_ids" value="hide" ';
    print XSL $checked if($show_pct_spectrum_ids eq '');
    print XSL '/>hide';

    print XSL $newline;
    print XSL '<pre>' . $newline . '</pre>';

    print XSL '<xsl:text> </xsl:text>peptide column display   ' . $newline;

    print XSL '<input type="radio" name="order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;


    print XSL '<xsl:text> </xsl:text>weight <input type="text" name="weight" value="' . $register_order{'weight'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>peptide sequence <input type="text" name="peptide_sequence" value="' . $register_order{'peptide_sequence'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>nsp adjusted probability <input type="text" name="nsp_adjusted_probability" value="' . $register_order{'nsp_adjusted_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>initial probability <input type="text" name="initial_probability" value="' . $register_order{'initial_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>number tolerable termini <input type="text" name="num_tol_term" value="' . $register_order{'num_tol_term'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>nsp bin <input type="text" name="n_sibling_peptides_bin" value="' . $register_order{'n_sibling_peptides_bin'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>total number peptide instances <input type="text" name="n_instances" value="' . $register_order{'n_instances'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>peptide group index <input type="text" name="peptide_group_designator" value="' . $register_order{'peptide_group_designator'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and not($organism=\'Drosophila\')">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline . 'annotation column display   ' . $newline;
    print XSL '<input type="radio" name="annot_order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="annot_order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;
    print XSL '<xsl:text> </xsl:text>ensembl <input type="text" name="ensembl" value="' . $reg_annot_order{'ensembl'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>trembl <input type="text" name="trembl" value="' . $reg_annot_order{'trembl'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>swissprot <input type="text" name="swissprot" value="' . $reg_annot_order{'swissprot'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>refseq <input type="text" name="refseq" value="' . $reg_annot_order{'refseq'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>locuslink <input type="text" name="locus_link" value="' . $reg_annot_order{'locus_link'} . '" size="2" maxlength="3"/>', $newline;

    print XSL '</xsl:if>';
    print XSL '<pre>' . $newline . '</pre>';
    print XSL "---------------------------------------------------------------------------------------------------------";    
    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>set customized data view: ';
    print XSL '<input type="radio" name="custom_settings" value="prev" ' . $checked . '/>no change ';
    print XSL '<input type="radio" name="custom_settings" value="current"/>current ';
    print XSL '<input type="radio" name="custom_settings" value="default"/>default';

    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>short menu <input type="checkbox" name="short_menu" value="yes"/>';
    print XSL '<input type="hidden" name="menu" value="full"/>';


} # if full menu
else { # short menu case
    print XSL '<pre>' . $nonbreakline . '</pre>'. $newline;
    print XSL ' full menu <input type="checkbox" name="full_menu" value="yes"/>  '; 
    print XSL '    show discarded entries <input type="checkbox" name="discards" value="yes" ' . $discards . '/>    clear manual discards/restores <input type="checkbox" name="clear" value="yes"/>';

    # hidden information

    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><input type="hidden" name="quant_light2heavy" value="';
	if($quant_light2heavy eq 'false') {
	    print XSL 'false';
	}
	else {
	    print XSL 'true';
	}
	print XSL '"/></xsl:if>';
    } # only for xml version


    foreach(keys %register_order) {
	print XSL '<input type="hidden" name="' . $_ . '" value="' . $register_order{$_} . '"/>';
    }
    print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

# more here

    print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
    print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="xpress_display" value="';
    if($xpress_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";
    print XSL '<input type="hidden" name="asap_display" value="';
    if($asap_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";

}
if($CALCULATE_PIES) {
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and $organism=\'Homo_sapiens\'">    go ontology level <select name="go_level"><option value="0"/><option value="1"';
    print XSL ' selected="yes"' if($go_level == 1);
    print XSL '>1</option><option value="101"';
    print XSL ' selected="yes"' if($go_level == 101);

    print XSL '>1H</option><option value="2"';
    print XSL ' selected="yes"' if($go_level == 2);
    print XSL '>2</option><option value="102"';
    print XSL ' selected="yes"' if($go_level == 102);


    print XSL '>2H</option><option value="3"';
    print XSL ' selected="yes"' if($go_level == 3);
    print XSL '>3</option>';
    print XSL '<option value="103"';
    print XSL ' selected="yes"' if($go_level == 103);
    print XSL '>3H</option>'; 



    print XSL '<option value="4"';
    print XSL ' selected="yes"' if($go_level == 4);
    print XSL '>4</option>'; 
    print XSL '<option value="104"';
    print XSL ' selected="yes"' if($go_level == 104);
    print XSL '>4H</option>'; 


    print XSL '<option value="5"';
    print XSL ' selected="yes"' if($go_level == 5);
    print XSL '>5</option>';
    print XSL '<option value="105"';
    print XSL ' selected="yes"' if($go_level == 105);
    print XSL '>5H</option>'; 

    print XSL '<option value="6"';
    print XSL ' selected="yes"' if($go_level == 6);
    print XSL '>6</option>';
    print XSL '<option value="106"';
    print XSL ' selected="yes"' if($go_level == 106);
    print XSL '>6H</option>'; 

    print XSL '<option value="7"';
    print XSL ' selected="yes"' if($go_level == 7);
    print XSL '>7</option>';
    print XSL '<option value="107"';
    print XSL ' selected="yes"' if($go_level == 107);
    print XSL '>7H</option>'; 

    print XSL '<option value="8"';
    print XSL ' selected="yes"' if($go_level == 8);
    print XSL '>8</option>';
    print XSL '<option value="108"';
    print XSL ' selected="yes"' if($go_level == 108);
    print XSL '>8H</option>'; 


    print XSL '<option value="9"';
    print XSL ' selected="yes"' if($go_level == 9);
    print XSL '>9</option>';
    print XSL '<option value="109"';
    print XSL ' selected="yes"' if($go_level == 109);
    print XSL '>9H</option>'; 

    print XSL '</select></xsl:if>';
} # if calc pies
print XSL $nonbreakline ;

if($full_menu) {
    print XSL '<pre>' . $newline . '</pre>';
    if($discards) {
	print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
    }
    else {
	print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
    }
    print XSL $newline;
}

print XSL '</pre></td></tr></table>', "\n";

# make local reference
if(exists ${$boxptr}{'excel'} && ${$boxptr}{'excel'} eq 'yes') {
    my $local_excelfile = $excelfile;
    if((length $SERVER_ROOT) <= (length $local_excelfile) && 
       index((lc $local_excelfile), ($LC_SERVER_ROOT)) == 0) {
	$local_excelfile =  substr($local_excelfile, (length $SERVER_ROOT));
	if (substr($local_excelfile, 0, 1) ne '/') {
	    $local_excelfile = '/' . $local_excelfile;
	}
    }
    else {
	die "problem: $local_excelfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    my $windows_excelfile = $excelfile;
    if($WINDOWS_CYGWIN) {
	$windows_excelfile =  `cygpath -w '$excelfile'`;
	if($windows_excelfile =~ /^(\S+)\s?/) {
	    $windows_excelfile = $1;
	}
    }
    print XSL 'excel file: <a target="Win1" href="' . $local_excelfile . '">' . $windows_excelfile . '</a>'  . $newline;
    
}
if((! ($show_sens eq '') && $eligible)) {

  # make local reference
  my $local_pngfile = $pngfile;
  if(! $ISB_VERSION) {
      if((length $SERVER_ROOT) <= (length $local_pngfile) && 
	 index((lc $local_pngfile), ($LC_SERVER_ROOT)) == 0) {
	  $local_pngfile = '/' . substr($local_pngfile, (length $SERVER_ROOT));
      }
      else {
	  die "problem: $local_pngfile is not mounted under webserver root: $SERVER_ROOT\n";
      }
  } # if iis & cygwin

    print XSL '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'">';
    print XSL '<font color="blue"> Predicted Total Number of Correct Entries: <xsl:value-of select="protx:protein_summary_header/@num_predicted_correct_prots"/></font>';
    print XSL "\n\n";
    print XSL "<TABLE><TR><TD>";

    print XSL "<IMG SRC=\"$local_pngfile\"/>";
    print XSL "</TD><TD><PRE>";

    print XSL "<font color=\"red\">sensitivity</font>\tfraction of all correct proteins" . $newline . $tab . $tab . " with probs &gt;= min_prob" . $newline;
    print XSL "<font color=\"green\">error</font>\t\tfraction of all proteins with probs" . $newline . $tab . $tab . " &gt;= min_prob that are incorrect" . $newline . '<pre>' . $newline . '</pre>';

    print XSL 'minprob' . $tab . '<font color="red">sens</font>' . $tab . '<font color="green">err</font>' . $tab . '<font color="red"># corr</font>' . $tab . '<font color ="green"># incorr</font>' . $newline;
    print XSL '<xsl:apply-templates select="protx:protein_summary_header/protx:protein_summary_data_filter">';
    print XSL '<xsl:sort select="@min_probability" order="descending" data-type="number"/>';
    print XSL '</xsl:apply-templates>';

    print XSL '</PRE></TD></TR></TABLE>';
    print XSL $newline . '<pre>' . $newline . '</pre>';
    print XSL '</xsl:if>';
}


########################## COUNT ENTRIES  #################################

my $local_xmlfile = $xmlfile;
my $windows_xmlfile = $xmlfile;
if(! $ISB_VERSION) {
    if((length $SERVER_ROOT) <= (length $local_xmlfile) && 
       index((lc $local_xmlfile), ($LC_SERVER_ROOT)) == 0) {
	$local_xmlfile = '/' . substr($local_xmlfile, (length $SERVER_ROOT));
    }
    else {
	die "problem: $local_xmlfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    if($WINDOWS_CYGWIN) {
	$windows_xmlfile = `cygpath -w '$windows_xmlfile'`;
	if($windows_xmlfile =~ /^(\S+)\s?/) {
	    $windows_xmlfile = $1;
	}
    }
} # if iis & cygwin

my $MAX_XMLFILE_LEN = 80;
my $format_choice = ($WINDOWS_CYGWIN && (length $windows_xmlfile) > $MAX_XMLFILE_LEN) || 
	(! $WINDOWS_CYGWIN && (length $local_xmlfile) > $MAX_XMLFILE_LEN) ? '<br/>' : '';


if(! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') {


    print XSL '<font color="red">';
    print XSL '<xsl:value-of select="$prot_group_count"/>';

    print XSL '<font color="black"><i> discarded</i></font>' if($discards);

    print XSL ' entries (';

    print XSL '<xsl:value-of select="$single_hits_count"/>';

    print XSL ' single hits)';

    if(! $ISB_VERSION) {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; 
}
    else {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; 
    }
} # if count
else {

    print XSL '<font color="black"><i>discarded</i></font> ' if($discards);
    if(! $ISB_VERSION) {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }
    else {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }

}


###################################################

print XSL $newline . '<pre>' . $newline . '</pre>';
print XSL '<FONT COLOR="990000">* indicates peptide corresponding to unique protein entry</FONT>' . $newline;


# calculate how many columns, and header line here

$num_cols += 8;
my $extra_column = '<td>' . $table_spacer . '</td>';

my $HEADER = $header{'protein'} . $newline;


print XSL $RESULT_TABLE_PRE . $RESULT_TABLE, "\n";


print XSL '<xsl:comment>' . $start_string . '</xsl:comment>' . $newline . "\n";

#print XSL $HEADER . $newline ;

# bypass protein groups altogether for no groups mode.....
if(! ($show_groups eq '')) {
    print XSL "\n", '<xsl:apply-templates select="protx:protein_group">', "\n";
}
else {
    print XSL '<xsl:apply-templates select="protx:protein_group/protx:protein">', "\n";
}

if(! ($sort_pvalue) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_xpress_desc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	print XSL 'sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean' . "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_xpress_asc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
    else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_asap_desc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
}
elsif(! ($sort_asap_asc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
}
elsif(! ($sort_prob eq '')) {
	print XSL '<xsl:sort select="@probability" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_prot eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@protein_name"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@protein_name"/>', "\n";

    }
}
elsif(! ($sort_cov eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@percent_coverage" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@percent_coverage" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_peps eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@total_number_peptides" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@total_number_peptides" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_spec_ids eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";

    }
}
else {
    if($USE_INDEX) {
	if($show_groups eq '') {
	    print XSL '<xsl:sort select="parent::node()/@group_number" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="@group_sibling_id"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="@group_number" data-type="number"/>', "\n";
	}
    }

}

print XSL '</xsl:apply-templates>', "\n";

print XSL $RESULT_TABLE_SUF, "\n";
print XSL '</form>';
print XSL '</PRE></BODY></HTML>', "\n";
print XSL '</xsl:template>', "\n";

if(! ($show_groups eq '')) {
print XSL '<xsl:template match="protx:protein_group">', "\n";

my $suffix = '';
if(@inclusions > 0) {
    $suffix = ' or @group_number=\'';
    for(my $i = 0; $i <= $#inclusions; $i++) {
	$suffix .= $inclusions[$i] . '\'';
	$suffix .= ' or @group_number=\'' if($i <= $#inclusions - 1);
    }
}

foreach(keys %parent_incls) {
    $suffix .= ' or @group_number=\'' . $_ . '\'';
}    


if($discards) {

    if(! ($show_groups eq '')) {
	# see if fails criteria
	print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	} # show adjusted
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
	# check for all exclusions
	if(@exclusions > 0) {
	    for(my $k = 0; $k <= $#exclusions; $k++) {
		print XSL ' or (@group_number = \'' . $exclusions[$k] . '\')';
	    }
 	}
	# check for excluded children of this parent
	if(@pexclusions > 0) {
	    foreach(keys %parent_excls) {
		print XSL ' or (@group_number = \'' . $_ . '\')';
	    }
	}
	print XSL '">';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '<xsl:if test="not(@group_number=\'' . $inclusions[$i] . '\')">', "\n";
	}

    } # groups
    else {  # hide groups...want to make sure no singletons pass by default
	print XSL '<xsl:if test="count(protx:protein) &gt;\'1\' or protx:protein[@group_sibling_id=\'a\']/@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	foreach(@exclusions) {
	    print XSL ' or @group_number=\'' . $_ . '\'';
	}
	print XSL '">';
	foreach(@inclusions) {
	    print XSL '<xsl:if test="not(count(protx:protein) =\'1\' and @group_number=\'' . $_ . '\')">';
	}

    }

} # discards
else { # conventional view

    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '<xsl:if test="not(@group_number=\'' . $exclusions[$e] . '\')">', "\n";
    }
    if(! ($show_groups eq '')) {

	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));

	}
	else { # adjusted asapratios
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));
	}
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' . $suffix . '">' if($max_pvalue_display < 1.0);

	print XSL '<xsl:if test="(@probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
    }
    else { # hide groups
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or @probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'))' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'))' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	else {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'))' . $suffix . '">' if($max_pvalue_display < 1.0);

    }

} # normal mode

print XSL '<xsl:variable name="group_member" select="count(protx:protein)"/>';
print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));
print XSL '<xsl:variable name="parental_group_number" select="parent::node()/@group_number"/>';
print XSL '<xsl:variable name="sole_prot" select="protx:protein/@protein_name"/>';
print XSL '<xsl:variable name="database" select="$ref_db"/>';
print XSL '<xsl:variable name="peps1" select="protx:protein/@unique_stripped_peptides"/>';

print XSL '<xsl:apply-templates select="protx:protein">';
print XSL '<xsl:with-param name="group_member" select="$group_member"/>';
print XSL '<xsl:with-param name="group_number" select="$group_number"/>' if(! ($show_groups eq ''));
print XSL '<xsl:with-param name="parental_group_number" select="$parental_group_number"/>';
print XSL '<xsl:with-param name="sole_prot" select="$sole_prot"/>';
print XSL '<xsl:with-param name="database" select="$database"/>';
print XSL '<xsl:with-param name="peps1" select="$peps1"/>';

print XSL '</xsl:apply-templates>';


if($discards) {
    if(! ($show_groups eq '')) {
	print XSL '</xsl:if>';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '</xsl:if>';
	}
    }
    else { # hide groups
	print XSL '</xsl:if>';
	foreach(@inclusions) {
	    print XSL '</xsl:if>';
	}
    }
}
else {
    ############################ 10/7/03
    print XSL '</xsl:if>' if(! ($asap_xpress eq ''));  # agree
    print XSL '</xsl:if>' if($minprob > 0);
    print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
    print XSL '</xsl:if>' if(! ($filter_asap eq ''));
    print XSL '</xsl:if>' if($min_xpress > 0);
    print XSL '</xsl:if>' if($max_xpress > 0);
    print XSL '</xsl:if>' if($min_asap > 0);
    print XSL '</xsl:if>' if($max_asap > 0);
    print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '</xsl:if>', "\n";
    }
}

print XSL '</xsl:template>', "\n";

} # only if show groups

############ PROTEIN ########################
print XSL '<xsl:template match="protx:protein">';
print XSL '<xsl:param name="group_member" />';
print XSL '<xsl:param name="group_number" />' if(! ($show_groups eq ''));
print XSL '<xsl:param name="parental_group_number" />';
print XSL '<xsl:param name="sole_prot"/>';
print XSL '<xsl:param name="database"/>';
print XSL '<xsl:param name="peps1"/>';

my $num_pincl = 0;

#print XSL '<xsl:variable name="group_number" select="parent::node()/@group_number"/>' if($show_groups eq '');
#print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));



# integrate inclusions....
if($discards) {
    
    print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';

    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'' . $min_xpress . '\')' if($min_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;=\'' . $max_xpress . '\')' if($max_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));
    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));

    }
    else {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));
    }
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
    if(@exclusions > 0) {
	foreach(@exclusions) {
	    print XSL ' or $group_number=\'' . $_ . '\'';
	}
    }
    if(@pexclusions > 0) {
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or ($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';

	    }
	}
    }
    print XSL '">';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '<xsl:if test="not($group_number=\'' . $inclusions[$i] . '\')">', "\n";
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')">', "\n";
	}
    }
}
else { # conventional

    # need suffix
    my $prot_suffix = '';
    if(@pinclusions > 0) {
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		$prot_suffix .= ' or($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }    
	}
    }

    if($show_groups eq '') {
	foreach(@exclusions) {
	    print XSL '<xsl:if test="not(count(parent::node()/protx:protein)=\'1\' and $group_number=\'' . $_ . '\')' . $prot_suffix . '">';
	}
    }


    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')' . $prot_suffix . '">', "\n";
	}
    }
    if($show_groups eq '') {
	print XSL '<xsl:if test="@probability &gt;= \'' . $minprob . '\'' . $prot_suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_asap eq ''));

	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'' . $prot_suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'' . $prot_suffix . '">' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {

	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);


	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));

	}
    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'' . $prot_suffix . '">' if($max_pvalue_display < 1.0);

    }

    foreach(keys %parent_incls) {
	my @members = @{$parent_incls{$_}};
	if(@members > 0) {
	    $num_pincl++;
	    print XSL '<xsl:if test="not($group_number=\'' . $_ . '\')';
	    for(my $m = 0; $m <= $#members; $m++) {
		print XSL ' or @group_sibling_id=\'' . $members[$m] . '\'';
	    }
	    print XSL '">';
	}

    }
#####################
    print XSL '<xsl:if test="count(protx:peptide)=\'1\'">' if($SINGLE_HITS);

} # convent


# check whether part of group
print XSL '<xsl:variable name="mult_prot" select="@protein_name"/>';
print XSL '<xsl:variable name="database2" select="$ref_db"/>';
print XSL '<xsl:variable name="peps2" select="@unique_stripped_peptides"/>';
print XSL '<xsl:variable name="filextn"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn">_<xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="asap_ind" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@index"/>';
print XSL '<xsl:variable name="prot_number"><xsl:value-of select="parent::node()/@group_number"/><xsl:if test="count(parent::node()/protx:protein) &gt;\'1\'"><xsl:value-of select="@group_sibling_id"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="pvalpngfile" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']/protx:ASAP_pvalue_analysis_summary/@analysis_distribution_file"/>';

# more variables here
print XSL '<xsl:variable name="peptide_string" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@peptide_string"/>';
if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev"/>';
}
else { # reverse
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_standard_dev"/>';
}
print XSL '<xsl:variable name="xnum" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>';
print XSL '<xsl:variable name="min_pep_prob" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@min_peptide_probability"/>';
# print XSL '<xsl:variable name="source" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="heavy2light"><xsl:if test="$reference_isotope=\'heavy\'">0</xsl:if><xsl:if test="$reference_isotope=\'light\'">1</xsl:if></xsl:variable>';

#if($show_groups eq '' && ! ($show_peps eq '')) {
#    print XSL $newline;
#}

#print XSL '<xsl:apply-templates select="protx:peptide">';
#print XSL '<xsl:sort select = "@nsp_adjusted_probability" order="descending" data-type="number"/>';
#print XSL '<xsl:with-param name="pvalpngfile" select="$pvalpngfile"/>';
#print XSL '<xsl:with-param name="mult_prot" select="$mult_prot"/>';
#print XSL '<xsl:with-param name="peptide_string" select="$peptide_string"/>';
#print XSL '<xsl:with-param name="xratio" select="$xratio"/>';
#print XSL '<xsl:with-param name="xstd" select="$xstd"/>';
#print XSL '<xsl:with-param name="xnum" select="$xnum"/>';
#print XSL '<xsl:with-param name="min_pep_prob" select="$min_pep_prob"/>';
#print XSL '<xsl:with-param name="source" select="$source"/>';
#print XSL '</xsl:apply-templates>';
$tab_display{'protein'} = '<xsl:value-of select="@protein_name"/><xsl:for-each select="protx:indistinguishable_protein"><xsl:text>,</xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';
print XSL $tab_display{'protein'} . $newline;

    # here need prot prob
#print XSL '<xsl:value-of select="@probability"/>' . $newline;

###########333
print XSL '</xsl:if>' if($SINGLE_HITS);

if($discards) {
    
    print XSL '</xsl:if>';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '</xsl:if>';
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }

}
else { # conve
    if($show_groups eq '') {
	print XSL '</xsl:if>' if(! ($asap_xpress eq ''));
	print XSL '</xsl:if>' if($minprob > 0);
	print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
	print XSL '</xsl:if>' if(! ($filter_asap eq ''));
	print XSL '</xsl:if>' if($min_xpress > 0);
	print XSL '</xsl:if>' if($max_xpress > 0);
	print XSL '</xsl:if>' if($min_asap > 0);
	print XSL '</xsl:if>' if($max_asap > 0);
	print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
	foreach(@exclusions) {
	    print XSL '</xsl:if>';
	}
    }
    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }
    if(! ($show_groups eq '')) {
	for(my $k = 0; $k < $num_pincl; $k++) {
	    print XSL '</xsl:if>';

	}
    }
} # conv


print XSL '</xsl:template>';


################### PEPTIDE  ###################################
print XSL '<xsl:template match="protx:peptide">';
print XSL '<xsl:param name="pvalpngfile"/>';
print XSL '<xsl:param name="mult_prot"/>';
print XSL '<xsl:param name="peptide_string"/>';
print XSL '<xsl:param name="xratio"/>';
print XSL '<xsl:param name="xstd"/>';
print XSL '<xsl:param name="xnum"/>';
print XSL '<xsl:param name="min_pep_prob"/>';
# print XSL '<xsl:param name="source"/>';
print XSL '<xsl:variable name="mypep"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="mycharge" select="@charge"/>';
print XSL '<xsl:variable name="PepMass"><xsl:if test="@calc_neutral_pep_mass"><xsl:value-of select="@calc_neutral_pep_mass"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="StdPep"><xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if></xsl:variable>';

print XSL '<xsl:variable name="myinputfiles" select="$source_files_alt"/>';
print XSL '<xsl:variable name="myprots"><xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each></xsl:variable>';

print XSL '<xsl:variable name="nspbin" select="@n_sibling_peptides_bin"/>';
print XSL '<xsl:variable name="nspval" select="@n_sibling_peptides"/>';

if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">1</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}


print XSL '<xsl:if test="@nsp_adjusted_probability &gt;=\''. $min_pepprob . '\'">' if($min_pepprob > 0);
print XSL '<xsl:if test="@n_enzymatic_termini &gt;=\''. $minntt . '\'">' if($minntt > 0);
print XSL '<xsl:if test="not(@charge=\'1\')">' if(! ($exclude_1 eq ''));
print XSL '<xsl:if test="not(@charge=\'2\')">' if(! ($exclude_2 eq ''));
print XSL '<xsl:if test="not(@charge=\'3\')">' if(! ($exclude_3 eq ''));
print XSL '<xsl:if test="@is_nondegenerate_evidence=\'Y\'">' if(! ($exclude_degens eq ''));

print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';


print XSL '<xsl:if test="position()=\'1\'">' if($show_peps eq '');

    print XSL $tab_display{'protein'} . $tab;

    # here need prot prob
    print XSL '<xsl:value-of select="parent::node()/@probability"/>' . $newline;



print XSL '</xsl:if>' if($min_pepprob > 0);
print XSL '</xsl:if>' if($minntt > 0);
print XSL '</xsl:if>' if(! ($exclude_1 eq ''));
print XSL '</xsl:if>' if(! ($exclude_2 eq ''));
print XSL '</xsl:if>' if(! ($exclude_3 eq ''));
print XSL '</xsl:if>' if(! ($exclude_degens eq ''));

#print XSL '</xsl:if>';


print XSL '</xsl:template>';



if((! ($show_sens eq '') && $eligible)) {
    print XSL '<xsl:template match="protx:protein_summary_data_filter">';
    print XSL '<xsl:value-of select="@min_probability"/>' . $tab . '<font color="red"><xsl:value-of select="@sensitivity"/></font>' . $tab . '<font color="green"><xsl:value-of select="@false_positive_error_rate"/></font>' . $tab . '<font color="red"><xsl:value-of select="@predicted_num_correct"/></font>' . $tab . '<font color="green"><xsl:value-of select="@predicted_num_incorrect"/></font>' . $newline;

    print XSL '</xsl:template>';
}

print XSL '</xsl:stylesheet>', "\n";

print XSL "\n";

close(XSL);


}


sub printCountProtsXSL {
(my $boxptr) = @_;
my @minscore = (exists ${$boxptr}{'min_score1'} && ! (${$boxptr}{'min_score1'} eq '') ? ${$boxptr}{'min_score1'} : 0, 
		exists ${$boxptr}{'min_score2'} && ! (${$boxptr}{'min_score2'} eq '') ? ${$boxptr}{'min_score2'} : 0, 
		exists ${$boxptr}{'min_score3'} && ! (${$boxptr}{'min_score3'} eq '') ? ${$boxptr}{'min_score3'} : 0);

my $minprob = exists ${$boxptr}{'min_prob'} && ! (${$boxptr}{'min_prob'} eq '') ? ${$boxptr}{'min_prob'} : 0;
$minprob = $MIN_PROT_PROB if(! $HTML && $inital_xsl);

my $min_asap = exists ${$boxptr}{'min_asap'} && ! (${$boxptr}{'min_asap'} eq '') ? ${$boxptr}{'min_asap'} : 0;
my $max_asap = exists ${$boxptr}{'max_asap'} && ! (${$boxptr}{'max_asap'} eq '') ? ${$boxptr}{'max_asap'} : 0;
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ! ${$boxptr}{'filter_asap'} eq '' ? ${$boxptr}{'filter_asap'} : '';
my $min_xpress = exists ${$boxptr}{'min_xpress'} && ! (${$boxptr}{'min_xpress'} eq '') ? ${$boxptr}{'min_xpress'} : 0;
my $max_xpress = exists ${$boxptr}{'max_xpress'} && ! (${$boxptr}{'max_xpress'} eq '') ? ${$boxptr}{'max_xpress'} : 0;
my $filter_xpress = exists ${$boxptr}{'filter_xpress'} ? ${$boxptr}{'filter_xpress'} : '';

my $sort = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'yes';
${$boxptr}{'pep_aa'} = uc ${$boxptr}{'pep_aa'} if(exists ${$boxptr}{'pep_aa'});
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';
${$boxptr}{'mark_aa'} = uc ${$boxptr}{'mark_aa'} if(exists ${$boxptr}{'mark_aa'});
my $mark_aa = exists ${$boxptr}{'mark_aa'} && ! (${$boxptr}{'mark_aa'} eq '') ? ${$boxptr}{'mark_aa'} : '';
my $minntt = exists ${$boxptr}{'min_ntt'} && ! (${$boxptr}{'min_ntt'} eq '') ? ${$boxptr}{'min_ntt'} : 0;
my $min_pepprob = exists ${$boxptr}{'min_pep_prob'} && ! (${$boxptr}{'min_pep_prob'} eq '') ? ${$boxptr}{'min_pep_prob'} : 0;
my $maxnmc = exists ${$boxptr}{'max_nmc'} && ! (${$boxptr}{'max_nmc'} eq '') ? ${$boxptr}{'max_nmc'} : -1;

my @inclusions = exists ${$boxptr}{'inclusions'} ? split(' ', ${$boxptr}{'inclusions'}) : ();
my @exclusions = exists ${$boxptr}{'exclusions'} ? split(' ', ${$boxptr}{'exclusions'}) : ();

my @pinclusions = exists ${$boxptr}{'pinclusions'} ? split(' ', ${$boxptr}{'pinclusions'}) : ();
my @pexclusions = exists ${$boxptr}{'pexclusions'} ? split(' ', ${$boxptr}{'pexclusions'}) : ();
if(exists ${$boxptr}{'clear'} && ${$boxptr}{'clear'} eq 'yes') {
    @inclusions = ();
    @exclusions = ();
    @pinclusions = ();
    @pexclusions = ();
}

my $exclude_1 = exists ${$boxptr}{'ex1'} && ${$boxptr}{'ex1'} eq 'yes' ? $checked : '';
my $exclude_2 = exists ${$boxptr}{'ex2'} && ${$boxptr}{'ex2'} eq 'yes' ? $checked : '';
my $exclude_3 = exists ${$boxptr}{'ex3'} && ${$boxptr}{'ex3'} eq 'yes' ? $checked : '';

my $show_ggl = exists ${$boxptr}{'show_ggl'} && ${$boxptr}{'show_ggl'} eq 'yes' ? $checked : '';

my $peptide_prophet_check1 = 'count(protx:peptide_prophet_summary) &gt; \'0\'';
my $peptide_prophet_check2 = 'count(parent::node()/protx:peptide_prophet_summary) &gt; \'0\'';

my $discards_init = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes';
my $discards = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes' ? $checked : '';

my $table_space = '&#160;';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
    $table_spacer = '<xsl:text>     </xsl:text>';
}

my $asap_xpress = exists ${$boxptr}{'asap_xpress'} && ${$boxptr}{'asap_xpress'} eq 'yes' ? $checked : '';
my $show_groups = ! exists ${$boxptr}{'show_groups'} || ${$boxptr}{'show_groups'} eq 'show' ? $checked : '';
my $hide_groups = $show_groups eq '' ? $checked : '';

my $show_annot = ! exists ${$boxptr}{'show_annot'} || ${$boxptr}{'show_annot'} eq 'show' ? $checked : '';
my $hide_annot = $show_annot eq '' ? $checked : '';
my $show_peps = ! exists ${$boxptr}{'show_peps'} || ${$boxptr}{'show_peps'} eq 'show' ? $checked : '';
my $hide_peps = $show_peps eq '' ? $checked : '';

my $show_adjusted_asap = (! exists ${$boxptr}{'show_adjusted_asap'} && ! exists ${$boxptr}{'adj_asap'}) || (${$boxptr}{'show_adjusted_asap'} eq 'yes') ? $checked : '';

my $max_pvalue_display = exists ${$boxptr}{'max_pvalue'} && ! (${$boxptr}{'max_pvalue'} eq '') ? ${$boxptr}{'max_pvalue'} : 1.0;

my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';
my $glyc = exists ${$boxptr}{'glyc'} && ${$boxptr}{'glyc'} eq 'yes' ? $checked : '';
# DEFINE Variable prot_group_count to store number of protein groups displayed   print XSL '<font color="red"><xsl:value-of select="';

    print XSL '<xsl:variable name="prot_group_count" select="';
 
    if($show_groups eq '') { # count prots
	if($discards) {
	    print XSL 'count(protx:protein_group/protx:protein)-';
	}
	print XSL 'count(protx:protein_group[(@probability &gt;= \'' . $minprob . '\'';


	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}
	else { # show adjusted
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or(protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}

	print XSL ' and(protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	for(my $e = 0; $e <= $#exclusions; $e++) {
	    print XSL ' and not(parent::node()/@group_number=\'' . $exclusions[$e] . '\')';
	}
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

	print XSL ')';

	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL ' or (parent::node()/@group_number=\'' . $inclusions[$i] . '\'';
	    foreach(@pexclusions) {
		if(/^(\d+)([a-z,A-Z])$/) {
		    print XSL ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
		}
	    }
	    print XSL ')';
	}
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or (parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

    } # hide groups eq ''
    else { # count groups
	if($discards) {
	    print XSL 'count(protx:protein_group)-';
	}
	print XSL 'count(protx:protein_group[(@probability &gt;= \'' . $minprob . '\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    print XSL ' and not(@group_number=\'' . $exclusions[$e] . '\')';
	}
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	else {
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}

	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	print XSL ')';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL ' or @group_number=\'' . $inclusions[$i] . '\'';
	}

    } # hide groups
    print XSL '])"/>';

# END of group_prot_count DEFINITION 

# DEFINE Variable tot_prot_count to store number of all proteins displayed  

    print XSL '<xsl:variable name="tot_prot_count" select="';
 
    if($show_groups eq '') { # count prots
	if($discards) {
	    print XSL 'count(protx:protein_group/protx:protein)-';
	}
	print XSL 'count(protx:protein_group/protx:protein[(@probability &gt;= \'' . $minprob . '\'';


	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}
	else { # show adjusted
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or(protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}

	print XSL ' and(protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	for(my $e = 0; $e <= $#exclusions; $e++) {
	    print XSL ' and not(parent::node()/@group_number=\'' . $exclusions[$e] . '\')';
	}
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

	print XSL ')';

	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL ' or (parent::node()/@group_number=\'' . $inclusions[$i] . '\'';
	    foreach(@pexclusions) {
		if(/^(\d+)([a-z,A-Z])$/) {
		    print XSL ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
		}
	    }
	    print XSL ')';
	}
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or (parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

    } # hide groups eq ''
    else { # count groups
	if($discards) {
	    print XSL 'count(protx:protein_group)-';
	}
	print XSL 'count(protx:protein_group[(@probability &gt;= \'' . $minprob . '\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    print XSL ' and not(@group_number=\'' . $exclusions[$e] . '\')';
	}
	print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'xpress\'] and protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'xpress\'] and protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'xpress\'] and protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein/protx:analysis_result[@analysis=\'asapratio\']) or(protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:protein/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	else {
	    print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or(protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}

	print XSL ' and(protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	print XSL ')';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL ' or @group_number=\'' . $inclusions[$i] . '\'';
	}

    } # hide groups
    print XSL '])"/>';

# END of tot_prot_count DEFINITION 

# DEFINE single_hits_count DEFINITION
    # now count the number of single hits.....
    print XSL '<xsl:variable name="single_hits_count" select="';
    if($show_groups eq '') { # count prots
	if($discards) {
	    print XSL 'count(protx:protein_group/protx:protein)-';
	}
	print XSL 'count(protx:protein_group/protx:protein[(@probability &gt;= \'' . $minprob . '\'';

	print XSL ' and (count(protx:peptide[@is_contributing_evidence=\'Y\'])&lt;=\'1\')';
	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}
	else {
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}

	print XSL ' and(protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	for(my $e = 0; $e <= $#exclusions; $e++) {
	    print XSL ' and not(parent::node()/@group_number=\'' . $exclusions[$e] . '\')';
	}
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

	print XSL ')';

	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL ' or (parent::node()/@group_number=\'' . $inclusions[$i] . '\'';
	    print XSL ' and (count(protx:peptide[@is_contributing_evidence=\'Y\'])&lt;=\'1\')';
	    foreach(@pexclusions) {
		if(/^(\d+)([a-z,A-Z])$/) {
		    print XSL ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
		}
	    }
	    print XSL ')';
	}
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or (parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\'';
		print XSL ' and (count(protx:peptide[@is_contributing_evidence=\'Y\'])&lt;=\'1\'))';
	    }
	}

    } # hide groups eq ''
    else { # show groups
	if($discards) {
	    print XSL 'count(protx:protein_group)-';
	}
	print XSL 'count(protx:protein_group[(@probability &gt;= \'' . $minprob . '\'';


	print XSL ' and (count(protx:protein[@group_sibling_id = \'a\']/protx:peptide[@is_contributing_evidence=\'Y\'])&lt;=\'1\')';

	for(my $e = 0; $e <= $#exclusions; $e++) {
	    print XSL ' and not(@group_number=\'' . $exclusions[$e] . '\')';
	}
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	else {
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}

	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	print XSL ')';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL ' or (@group_number=\'' . $inclusions[$i] . '\'';
	    print XSL ' and (count(protx:protein[@group_sibling_id = \'a\']/protx:peptide[@is_contributing_evidence=\'Y\'])&lt;=\'1\'))';
	}

    } # hide groups
    print XSL '])"/>';



# END of tot_prot_count DEFINITION 


}

sub writeGaggleMatrixXSLFile {
(my $xfile, my $boxptr) = @_;

if(! open(XSL, ">$xfile")) {
    print " cannot open $xfile: $!\n";
    exit(1);
}
print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">', "\n";
my $tab = '<xsl:value-of select="$tab"/>';
my $newline = '<xsl:value-of select="$newline"/>';
my $nonbreakline = '<xsl:value-of select="$newline"/>';
my $newlinespace = '<p/>';
my $doubleline = $newline . $newline;
my $space = '&#160';

my $num_cols = 3; # first & last


# just in case read recently from customized
$ICAT = 1 if(exists ${$boxptr}{'icat_mode'} && ${$boxptr}{'icat_mode'} eq 'yes');
$GLYC = 1 if(exists ${$boxptr}{'glyc_mode'} && ${$boxptr}{'glyc_mode'} eq 'yes');

# DEPRECATED: restore now fixed at 0 (taken care of up front)
my $restore = 0; 

my @minscore = (exists ${$boxptr}{'min_score1'} && ! (${$boxptr}{'min_score1'} eq '') ? ${$boxptr}{'min_score1'} : 0, 
		exists ${$boxptr}{'min_score2'} && ! (${$boxptr}{'min_score2'} eq '') ? ${$boxptr}{'min_score2'} : 0, 
		exists ${$boxptr}{'min_score3'} && ! (${$boxptr}{'min_score3'} eq '') ? ${$boxptr}{'min_score3'} : 0);

my $minprob = exists ${$boxptr}{'min_prob'} && ! (${$boxptr}{'min_prob'} eq '') ? ${$boxptr}{'min_prob'} : 0;
$minprob = $MIN_PROT_PROB if(! $HTML && $inital_xsl);

my $min_asap = exists ${$boxptr}{'min_asap'} && ! (${$boxptr}{'min_asap'} eq '') ? ${$boxptr}{'min_asap'} : 0;
my $max_asap = exists ${$boxptr}{'max_asap'} && ! (${$boxptr}{'max_asap'} eq '') ? ${$boxptr}{'max_asap'} : 0;
my $min_xpress = exists ${$boxptr}{'min_xpress'} && ! (${$boxptr}{'min_xpress'} eq '') ? ${$boxptr}{'min_xpress'} : 0;
my $max_xpress = exists ${$boxptr}{'max_xpress'} && ! (${$boxptr}{'max_xpress'} eq '') ? ${$boxptr}{'max_xpress'} : 0;

my $sort = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'yes';
${$boxptr}{'pep_aa'} = uc ${$boxptr}{'pep_aa'} if(exists ${$boxptr}{'pep_aa'});
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';
${$boxptr}{'mark_aa'} = uc ${$boxptr}{'mark_aa'} if(exists ${$boxptr}{'mark_aa'});
my $mark_aa = exists ${$boxptr}{'mark_aa'} && ! (${$boxptr}{'mark_aa'} eq '') ? ${$boxptr}{'mark_aa'} : '';
my $minntt = exists ${$boxptr}{'min_ntt'} && ! (${$boxptr}{'min_ntt'} eq '') ? ${$boxptr}{'min_ntt'} : 0;
my $min_pepprob = exists ${$boxptr}{'min_pep_prob'} && ! (${$boxptr}{'min_pep_prob'} eq '') ? ${$boxptr}{'min_pep_prob'} : 0;
my $maxnmc = exists ${$boxptr}{'max_nmc'} && ! (${$boxptr}{'max_nmc'} eq '') ? ${$boxptr}{'max_nmc'} : -1;

my @inclusions = exists ${$boxptr}{'inclusions'} ? split(' ', ${$boxptr}{'inclusions'}) : ();
my @exclusions = exists ${$boxptr}{'exclusions'} ? split(' ', ${$boxptr}{'exclusions'}) : ();
my @pinclusions = exists ${$boxptr}{'pinclusions'} ? split(' ', ${$boxptr}{'pinclusions'}) : ();
my @pexclusions = exists ${$boxptr}{'pexclusions'} ? split(' ', ${$boxptr}{'pexclusions'}) : ();
if(exists ${$boxptr}{'clear'} && ${$boxptr}{'clear'} eq 'yes') {
    @inclusions = ();
    @exclusions = ();
    @pinclusions = ();
    @pexclusions = ();
}

my $exclude_1 = exists ${$boxptr}{'ex1'} && ${$boxptr}{'ex1'} eq 'yes' ? $checked : '';
my $exclude_2 = exists ${$boxptr}{'ex2'} && ${$boxptr}{'ex2'} eq 'yes' ? $checked : '';
my $exclude_3 = exists ${$boxptr}{'ex3'} && ${$boxptr}{'ex3'} eq 'yes' ? $checked : '';


my $peptide_prophet_check1 = 'count(protx:peptide_prophet_summary) &gt; \'0\'';
my $peptide_prophet_check2 = 'count(parent::node()/protx:peptide_prophet_summary) &gt; \'0\'';

my $discards_init = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes';
my $discards = exists ${$boxptr}{'discards'} && ${$boxptr}{'discards'} eq 'yes' ? $checked : '';

my $table_space = '&#160;';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
    $table_spacer = '<xsl:text>     </xsl:text>';
}

my $asap_xpress = exists ${$boxptr}{'asap_xpress'} && ${$boxptr}{'asap_xpress'} eq 'yes' ? $checked : '';
#my $show_groups = ! exists ${$boxptr}{'show_groups'} || ${$boxptr}{'show_groups'} eq 'show' ? $checked : '';
my $show_groups = '';
my $hide_groups = $show_groups eq '' ? $checked : '';

my $show_annot = ! exists ${$boxptr}{'show_annot'} || ${$boxptr}{'show_annot'} eq 'show' ? $checked : '';
my $hide_annot = $show_annot eq '' ? $checked : '';
my $show_peps = ! exists ${$boxptr}{'show_peps'} || ${$boxptr}{'show_peps'} eq 'show' ? $checked : '';
my $hide_peps = $show_peps eq '' ? $checked : '';

my $show_adjusted_asap = (! exists ${$boxptr}{'show_adjusted_asap'} && ! exists ${$boxptr}{'adj_asap'}) || (${$boxptr}{'show_adjusted_asap'} eq 'yes') ? $checked : '';

my $max_pvalue_display = exists ${$boxptr}{'max_pvalue'} && ! (${$boxptr}{'max_pvalue'} eq '') ? ${$boxptr}{'max_pvalue'} : 1.0;

my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';
my $glyc = exists ${$boxptr}{'glyc'} && ${$boxptr}{'glyc'} eq 'yes' ? $checked : '';

if(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'classic') {
    ${$boxptr}{'index'} = 1;
    ${$boxptr}{'prob'} = 2;
    ${$boxptr}{'spec_name'} = 3;
    ${$boxptr}{'neutral_mass'} = 4;
    ${$boxptr}{'massdiff'} = 5;
    ${$boxptr}{'sequest_xcorr'} = 6;
    ${$boxptr}{'sequest_delta'} = 7;
    ${$boxptr}{'sequest_spscore'} = -1;
    ${$boxptr}{'sequest_sprank'} = 8;
    ${$boxptr}{'matched_ions'} = 9;
    ${$boxptr}{'protein'} = 10;
    ${$boxptr}{'alt_prots'} = 11;
    ${$boxptr}{'peptide'} = 12;
    ${$boxptr}{'num_tol_term'} = -1;
    ${$boxptr}{'num_missed_cl'} = -1;
}
elsif(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'default') {
    ${$boxptr}{'weight'} = -1;
    ${$boxptr}{'peptide_sequence'} = -1;
    ${$boxptr}{'nsp_adjusted_probability'} = -1;
    ${$boxptr}{'initial_probability'} = -1;
    ${$boxptr}{'n_tryptic_termini'} = -1;
    ${$boxptr}{'n_sibling_peptides_bin'} = -1;
    ${$boxptr}{'n_instances'} = -1;
    ${$boxptr}{'peptide_group_designator'} = -1;
}
if(exists ${$boxptr}{'annot_order'} && ${$boxptr}{'annot_order'} eq 'default') {
    ${$boxptr}{'ensembl'} = -1;
    ${$boxptr}{'trembl'} = -1;
    ${$boxptr}{'swissprot'} = -1;
    ${$boxptr}{'refseq'} = -1;
    ${$boxptr}{'locus_link'} = -1;
}


# now add on new ones
foreach(keys %{$boxptr}) {
    if(/^excl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    if($inclusions[$i] == $1) {
		@inclusions = @inclusions[0..$i-1, $i+1..$#inclusions]; # delete it from inclusions
		$done = 1;
		$i = @inclusions;
		# cancel all previous pexclusions with same parent
		my $next_ex = $1;
		for(my $p = 0; $p <= $#pinclusions; $p++) {
		    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
			@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_ex = $1;
	push(@exclusions, $next_ex) if(! $done); # add to exclusions
	# cancel all previous pinclusions with same parent
	for(my $p = 0; $p <= $#pinclusions; $p++) {
	    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
		@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
	    }
	}

    }
    elsif(/^incl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    if($exclusions[$e] == $1) {
		@exclusions = @exclusions[0..$e-1, $e+1..$#exclusions]; # delete it from inclusions
		$done = 1;
		$e = @exclusions;
		# cancel all previous pexclusions with same parent
		my $next_in = $1;
		for(my $p = 0; $p <= $#pexclusions; $p++) {
		    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
			@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_in = $1;
	push(@inclusions, $next_in) if(! $done); # add to inclusions
	# cancel all previous pexclusions with same parent
	for(my $p = 0; $p <= $#pexclusions; $p++) {
	    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
		@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
	    }
	}
    }
}


# now add on new ones
foreach(keys %{$boxptr}) {

    if(/^pexcl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on inclusion list
	my $done = 0;
	for(my $i = 0; $i <= $#pinclusions; $i++) {
	    if($pinclusions[$i] == $1) {
		@pinclusions = @pinclusions[0..$i-1, $i+1..$#pinclusions]; # delete it from inclusions
		$done = 1;
		$i = @pinclusions;
	    }
	}
	push(@pexclusions, $1) if(! $done); # add to exclusions
    }
    elsif(/^pincl(\d+[a-z,A-Z])$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    if($pexclusions[$e] == $1) {
		@pexclusions = @pexclusions[0..$e-1, $e+1..$#pexclusions]; # delete it from inclusions
		$done = 1;
		$e = @pexclusions;
	    }
	}
	push(@pinclusions, $1) if(! $done); # add to inclusions
    }
}


my $exclusions = join(' ', @exclusions);
my $inclusions = join(' ', @inclusions);
my $pexclusions = join(' ', @pexclusions);
my $pinclusions = join(' ', @pinclusions);

my %parent_excls = ();
my %parent_incls = ();
foreach(@pexclusions) {
    if(/^(\d+)[a-z,A-Z]$/) {
	$parent_excls{$1}++;
    }
}
foreach(@pinclusions) {
    if(/^(\d+)([a-z,A-Z])$/) {
	if(exists $parent_incls{$1}) {
	    push(@{$parent_incls{$1}}, $2);
	}
	else {
	    my @next = ($2);
	    $parent_incls{$1} = \@next;
	}
    }
}

my $full_menu = (exists ${$boxptr}{'menu'} && ${$boxptr}{'menu'} eq 'full') || 
    (exists ${$boxptr}{'full_menu'} && ${$boxptr}{'full_menu'} eq 'yes');

my $short_menu = exists ${$boxptr}{'short_menu'} && ${$boxptr}{'short_menu'} eq 'yes';
$full_menu = 0 if($short_menu); # it takes precedence

my @minscore_display = ($minscore[0] > 0 ? $minscore[0] : '',$minscore[1] > 0 ? $minscore[1] : '',$minscore[2] > 0 ? $minscore[2] : '');
my $minprob_display = $minprob > 0 ? $minprob : '';
my $minntt_display = $minntt > 0 ? $minntt : '';
my $maxnmc_display = $maxnmc >= 0 ? $maxnmc : '';
my $min_asap_display = $min_asap > 0 ? $min_asap : '';
my $max_asap_display = $max_asap > 0 ? $max_asap : '';
my $min_xpress_display = $min_xpress > 0 ? $min_xpress : '';
my $max_xpress_display = $max_xpress > 0 ? $max_xpress : '';
my $min_pepprob_display = $min_pepprob > 0 ? $min_pepprob : '';

my $asap_display = exists ${$boxptr}{'asap_display'} && ${$boxptr}{'asap_display'} eq 'show' ? $checked : '';
my $xpress_display = exists ${$boxptr}{'xpress_display'} && ${$boxptr}{'xpress_display'} eq 'show' ? $checked : '';

my $show_ggl = exists ${$boxptr}{'show_ggl'} && ${$boxptr}{'show_ggl'} eq 'yes' ? $checked : '';

print XSL '<xsl:variable name="tab"><xsl:text>&#x09;</xsl:text></xsl:variable>', "\n";
print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
print XSL '</xsl:text></xsl:variable>';

print XSL '<xsl:variable name="libra_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="ref_db" select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/>';
print XSL '<xsl:variable name="asap_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="xpress_quant"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'])">0</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="source_files" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="source_files_alt" select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/>';
print XSL '<xsl:variable name="organism"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@organism"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@organism)">UNKNOWN</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="reference_isotope"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"><xsl:value-of select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"/></xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope)">UNSET</xsl:if></xsl:variable>';

my %display = ();
my %display_order = ();
my %register_order = ();
my %reg_annot_order = ();
my %display_annot_order = ();
my %header = ();
my %default_order = ();
my %tab_display = ();
my %tab_header = ();
my %annot_display = ();
my %annot_order = ();
my %annot_tab_display = ();

my $header_pre = '<font color="brown"><i>';
my $header_post = '</i></font>';

$display{'protein'} = '<xsl:value-of select="@protein"/><xsl:for-each select="protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';
$header{'protein'} = 'protein';
$tab_display{'protein'} = '<xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text>,</xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';


$default_order{'protein'} = -1;

$display{'coverage'} = '<xsl:value-of select="@percent_coverage"/>';
$header{'coverage'} = 'percent coverage';
$tab_display{'coverage'} = '<xsl:value-of select="parent::node()/@percent_coverage"/>';
$default_order{'coverage'} = -1;

$display{'annotation'} = '<xsl:if test="annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$header{'annotation'} = 'annotation';
$tab_display{'annotation'} = '<xsl:if test="annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$default_order{'annotation'} = -1;


# add here the cgi info for peptide
my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
my $html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;Ref=';
if(useXMLFormatLinks($xmlfile)) {
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;Ref=';
	$html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';
	$html_peptide_lead3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;';
	$html_peptide_lead3 .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
	$html_peptide_lead3 .= 'glyc=Y&amp;' if($glyc);
	$html_peptide_lead3 .= 'libra={$libra_quant}&amp;';
	$html_peptide_lead3 .= 'Ref=';

	$html_peptide_lead4 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep2}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';

}
my $html_peptide_mid = '&amp;Infile=';


if($DISPLAY_MODS) {
    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_peptide_lead3 . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_peptide_lead3 . '{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if></td>';
}
else {
    $display{'peptide_sequence'} = '<td class="peptide"><xsl:if test="$mycharge &gt; \'0\'">' . $html_peptide_lead . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . '<xsl:if test="@charge"><xsl:value-of select="@charge"/>_</xsl:if><xsl:value-of select="@peptide_sequence"/></A></xsl:if><xsl:if test="not($mycharge &gt; \'0\')">' . $html_peptide_lead . '{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . '<xsl:value-of select="@peptide_sequence"/></A></xsl:if></td>';
}
my $display_ind_peptide_seq = '<td class="indist_pep">--' . $html_peptide_lead4 . '{$mycharge2}_{$mypep}' . $html_peptide_mid . '{$myinputfiles2}">' . '<xsl:value-of select="parent::node()/@charge"/>_<xsl:value-of select="@peptide_sequence"/></A></td>';

$header{'peptide_sequence'} = '<td>' . $header_pre . 'peptide sequence' . $header_post . '</td>';
$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:value-of select="@peptide_sequence"/><xsl:for-each select="indistinguishable_peptide">,<xsl:value-of select="@peptide_sequence"/></xsl:for-each>';
$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence';

#print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';
#print XSL '<xsl:variable name="database" select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/>';

$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if>' . $tab . $TPPhostname . $CGI_HOME . 'peptidexml_html2.pl?PepMass=<xsl:value-of select="@calc_neutral_pep_mass"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>StdPep=<xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>MassError=' . $MOD_MASS_ERROR . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>cgi-bin=' . $CGI_HOME . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>ratioType=<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if><xsl:value-of disable-output-escaping="yes" select="$amp"/>';

$tab_display{'peptide_sequence'} .= 'mark_aa=' . $mark_aa . '<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if(! ($mark_aa eq ''));
$tab_display{'peptide_sequence'} .= 'glyc=Y<xsl:value-of disable-output-escaping="yes" select="$amp"/>' if($glyc);
$tab_display{'peptide_sequence'} .= 'Ref=<xsl:value-of select="@charge"/>_<xsl:value-of select="@peptide_sequence"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Infile=<xsl:value-of select="$source_files_alt"/>';

$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence' . $tab . 'peptide link';

$default_order{'peptide_sequence'} = 2;


my $wt_header = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'prot_wt_xml.pl?xmlfile=' . $xmlfile . '&amp;cgi-home=' . $CGI_HOME . '&amp;quant_light2heavy=' . $quant_light2heavy . '&amp;modpep={$StdPep}&amp;pepmass={$PepMass}&amp;';
$wt_header .= 'xml_input=1&amp;' if(! $DISTR_VERSION && $NEW_XML_FORMAT);
$wt_header .= 'glyc=1&amp;' if($glyc);
$wt_header .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
$wt_header .= 'peptide=';
my $wt_suf = '</A>';


$tab_display{'is_nondegen_evidence'} = '<xsl:value-of select="@is_nondegenerate_evidence"/>';
$tab_header{'is_nondegen_evidence'} = 'is nondegenerate evidence';
$default_order{'is_nondegen_evidence'} = 0.5;


$display{'weight'} = '<td><xsl:if test="@is_nondegenerate_evidence = \'Y\'"><font color="#990000">*</font></xsl:if></td><td>' . $wt_header . '{$mypep}&amp;charge={$mycharge}&amp;">' . '<nobr>wt-<xsl:value-of select="@weight"/><xsl:text> </xsl:text></nobr>' . $wt_suf . '</td>';
$header{'weight'} = '<td></td><td>' . $header_pre . 'weight' . $header_post . '</td>';
$tab_display{'weight'} = '<xsl:value-of select="@weight"/>';
$default_order{'weight'} = 1;

$display{'nsp_adjusted_probability'} = '<td><xsl:if test="@is_contributing_evidence = \'Y\'"><font COLOR="#FF9933"><xsl:value-of select="@nsp_adjusted_probability"/></font></xsl:if><xsl:if test="@is_contributing_evidence = \'N\'"><xsl:value-of select="@nsp_adjusted_probability"/></xsl:if></td>';
$header{'nsp_adjusted_probability'} = '<td>' . $header_pre . 'nsp adj prob' . $header_post . '</td>';
$tab_display{'nsp_adjusted_probability'} = '<xsl:value-of select="@nsp_adjusted_probability"/>';
$tab_header{'nsp_adjusted_probability'} = 'nsp adjusted probability';
$default_order{'nsp_adjusted_probability'} = 3;

$display{'initial_probability'} = '<td><xsl:value-of select="@initial_probability"/></td>';
$header{'initial_probability'} = '<td>' . $header_pre . 'init prob' . $header_post . '</td>';
$tab_display{'initial_probability'} = '<xsl:value-of select="@initial_probability"/>';
$tab_header{'initial_probability'} = 'initial probability';
$default_order{'initial_probability'} = 4;

$display{'num_tol_term'} = '<td><xsl:value-of select="@n_enzymatic_termini"/></td>';
$header{'num_tol_term'} = '<td>' . $header_pre . 'ntt' . $header_post . '</td>';
$tab_display{'num_tol_term'} = '<xsl:value-of select="@n_enzymatic_termini"/>';
$tab_header{'num_tol_term'} = 'n tol termini';
$default_order{'num_tol_term'} = 5;

my $nsp_pre = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'show_nspbin.pl?xmlfile=' . $xmlfile . '&amp;nsp_bin={$nspbin}&amp;nsp_val={$nspval}&amp;charge={$mycharge}&amp;pep={$mypep}&amp;prot={$myprots}">';
my $tempnsp = '<td><xsl:if test="@n_sibling_peptides">' . $nsp_pre . '<xsl:value-of select="@n_sibling_peptides"/></A></xsl:if>
<xsl:if test="not(@n_sibling_peptides)"><xsl:value-of select="@n_sibling_peptides_bin"/></xsl:if></td>';
$display{'n_sibling_peptides_bin'} = $tempnsp;

$header{'n_sibling_peptides_bin'} = '<td>' . $header_pre . 'nsp<xsl:if test="not(protx:peptide/@n_sibling_peptides)"> bin</xsl:if>' . $header_post . '</td>';
$tab_display{'n_sibling_peptides_bin'} = '<xsl:value-of select="@n_sibling_peptides_bin"/>';
$tab_header{'n_sibling_peptides_bin'} = 'n sibling peptides bin';
$default_order{'n_sibling_peptides_bin'} = 6;

$display{'peptide_group_designator'} = '<td><xsl:if test="@peptide_group_designator"><font color="#DD00DD"><xsl:value-of select="@peptide_group_designator"/>-<xsl:value-of select="@charge"/></font></xsl:if></td>';
$header{'peptide_group_designator'} = '<td>' . $header_pre . 'pep grp ind' . $header_post . '</td>';
$tab_display{'peptide_group_designator'} = '<xsl:value-of select="@peptide_group_designator"/>';
$tab_header{'peptide_group_designator'} = 'peptide group designator';
$default_order{'peptide_group_designator'} = 8;


$header{'group_number'} = 'entry no.';
$tab_display{'group_number'} = '<xsl:value-of select="parent::node()/parent::node()/@group_number"/><xsl:if test="count(parent::node()/parent::node()/protx:protein) &gt; \'1\'"><xsl:value-of select="parent::node()/@group_sibling_id"/></xsl:if>';
$default_order{'group_number'} = -1;
$display{'group_number'} = '';
$tab_header{'group_number'} = 'entry no.';

$annot_display{'description'} = '<xsl:if test="protx:annotation/@protein_description"><xsl:if test="$organism = \'UNKNOWN\'"><font color="green"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if><xsl:if test="not($organism = \'UNKNOWN\')"><font color="#008080"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if>' . $table_space . ' </xsl:if>';

$annot_order{'description'} = -1;
$annot_tab_display{'description'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@protein_description"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if></xsl:for-each>';

$header{'description'} = 'description';

my $flybase_header = '<a TARGET="Win1" href="http://flybase.bio.indiana.edu/.bin/fbidq.html?';

$tab_header{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'">flybase' . $tab . '</xsl:if>';
$annot_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="protx:annotation/@flybase"><xsl:variable name="flybase" select="protx:annotation/@flybase"/>' . $flybase_header . '{$flybase}"><font color="green">Flybase:<xsl:value-of select="$flybase"/></font></a>' . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'flybase'} = 9;
$annot_tab_display{'flybase'} = '<xsl:if test="$organism=\'Drosophila\'"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@flybase"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@flybase"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';

my $ipi_header = '<a TARGET="Win1" href="http://srs.ebi.ac.uk/cgi-bin/wgetz?-id+m_RJ1KrMXG+-e+[IPI-acc:';
my $ipi_mid = ']">';
my $ipi_suf = '</a>';
$annot_display{'ipi'} = '<font color="green">&gt;</font><xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="annotation/@ipi_name"><xsl:variable name="ipi" select="annotation/@ipi_name"/>' . $ipi_header . '{$ipi}' . $ipi_mid . '<font color="green">IPI:<xsl:value-of select="$ipi"/></font>' . $ipi_suf . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'ipi'} = -1;
$annot_tab_display{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ipi_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ipi_name"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';


$header{'ipi'} = '<xsl:if test="not($organism = \'UNKNOWN\')">ipi' . $tab . '</xsl:if>';

my $embl_header = '<a TARGET="Win1" href="http://www.ensembl.org/';
my $embl_mid = '/protview?peptide=';
my $embl_suf = '</a>';
$annot_display{'ensembl'} = '<xsl:if test="protx:annotation/@ensembl_name"><xsl:variable name="org" select="$organism"/><xsl:variable name="ensembl" select="protx:annotation/@ensembl_name"/>' . $embl_header . '{$org}' . $embl_mid . '{$ensembl}"><font color="green">E:<xsl:value-of select="$ensembl"/></font>' . $embl_suf . $table_space . ' </xsl:if>';
$annot_order{'ensembl'} = 3;
$annot_tab_display{'ensembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ensembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ensembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'ensembl'} = 'ensembl' . $tab;

$annot_display{'trembl'} = '<xsl:if test="protx:annotation/@trembl_name"><font color="green">Tr:<xsl:value-of select="protx:annotation/@trembl_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'trembl'} = 4;
$annot_tab_display{'trembl'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@trembl_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@trembl_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'trembl'} = 'trembl' . $tab;

$annot_display{'swissprot'} = '<xsl:if test="protx:annotation/@swissprot_name"><font color="green">Sw:<xsl:value-of select="protx:annotation/@swissprot_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'swissprot'} = 5;
$annot_tab_display{'swissprot'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@swissprot_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@swissprot_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'swissprot'} = 'swissprot' . $tab;

$annot_display{'refseq'} = '<xsl:if test="protx:annotation/@refseq_name"><font color="green">Ref:<xsl:value-of select="protx:annotation/@refseq_name"/></font>' . $table_space . ' </xsl:if>';
$annot_order{'refseq'} = 6;
$annot_tab_display{'refseq'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@refseq_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@refseq_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'refseq'} = 'refseq' . $tab;

my $locus_header = '<a TARGET="Win1" href="http://www.ncbi.nlm.nih.gov/LocusLink/LocRpt.cgi?l=';
my $locus_suf = '</a>';

$annot_display{'locus_link'} = '<xsl:if test="protx:annotation/@locus_link_name"><xsl:variable name="loc" select="protx:annotation/@locus_link_name"/>' . $locus_header . '{$loc}' . '"><font color="green">LL:<xsl:value-of select="$loc"/></font>' . $locus_suf . $table_space . ' </xsl:if>';
$annot_order{'locus_link'} = 7;
$annot_tab_display{'locus_link'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@locus_link_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@locus_link_name"/></xsl:if></xsl:for-each>' . $tab;
$header{'locus_link'} = 'locus link' . $tab;




my $asap_file_pre = '';
my $asap_proph_suf = '_prophet.bof';
my $asap_pep_suf = '_peptide.bof';
my $asap_proph = '';
my $asap_pep = '';
if($xfile =~ /^(\S+\/)/) { # assume in same directory
    $asap_proph = $1 . 'ASAPRatio_prophet.bof';
    $asap_pep = $1 . 'ASAPRatio_peptide.bof';
    $asap_file_pre = $1 . 'ASAPRatio';
}


my $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . 'xli/ASAPRatio_lstProRatio.cgi?orgFile=';
my $asap_header_mid = '.orig&amp;proBofFile=' . $asap_file_pre . '{$filextn}' . $asap_proph_suf . '&amp;pepBofFile=' . $asap_file_pre . '{$filextn}' . $asap_pep_suf . '&amp;proIndx=';


my $plusmn = '&#177;';
$plusmn = ' +-' if($xslt =~ /xsltproc/);

my $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'xpress-prophet.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light={$heavy2light}">';

my $xpress_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $xpress_link;
if(useXMLFormatLinks($xmlfile)) {
    $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source_files}&amp;heavy2light=' . $xpress_ratio_type;
    $xpress_pre .= '&amp;mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
    $xpress_pre .= '">'; #{$heavy2light}">';

    $xpress_link = $TPPhostname . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?cgihome=' . $CGI_HOME . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>protein=<xsl:value-of select="$mult_prot"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>peptide_string=<xsl:value-of select="$peptide_string"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>ratio=<xsl:value-of select="$xratio"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>stddev=<xsl:value-of select="$xstd"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>num=<xsl:value-of select="$xnum"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlfile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>min_pep_prob=<xsl:value-of select="$min_pep_prob"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>source_files=<xsl:value-of select="$source_files"/><xsl:text disable-output-escaping="yes">&amp;</xsl:text>heavy2light=' . $xpress_ratio_type;
    $xpress_link .= '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>mark_aa=' . $mark_aa if(! ($mark_aa eq ''));
}

my $xpress_suf = '</a>';

$num_cols = 3;



$display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'"><td width="250"><xsl:if test="protx:analysis_result[@analysis=\'libra\']">LIBRA<xsl:text> </xsl:text>(<xsl:value-of select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>)<xsl:for-each select="protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><br/><xsl:value-of select="@mz"/>:<xsl:text> </xsl:text><xsl:value-of select="@ratio"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="@error"/><xsl:text> </xsl:text></xsl:for-each></xsl:if></td></xsl:if>';

$tab_display{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\' and parent::node()/protx:analysis_result[@analysis=\'libra\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/@number"/>' . $tab . '<xsl:for-each select="parent::node()/protx:analysis_result[@analysis=\'libra\']/protx:libra_result/protx:intensity"><xsl:value-of select="@ratio"/>' . $tab . '<xsl:value-of select="@error"/>' . $tab . '</xsl:for-each></xsl:if>';

$header{'libra'} = '<xsl:if test="$libra_quant &gt; \'0\'">LIBRA number peptides' . $tab . '<xsl:for-each select="/protx:protein_summary/protx:analysis_summary[@analysis=\'libra\']/protx:libra_summary/protx:fragment_masses">LIBRA <xsl:value-of select="@mz"/> ratio' . $tab . 'LIBRA <xsl:value-of select="@mz"/> error' . $tab . '</xsl:for-each></xsl:if>';



if($xpress_display eq $checked) {

    $display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'xpress\']">XPRESS';
    if(! ($quant_light2heavy eq 'true')) {
	$display{'xpress'} .= '(H/L)';
    }
    $display{'xpress'} .= ': ' . $xpress_pre . '<xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>(<xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>)</xsl:if>' . $xpress_suf . '</xsl:if><xsl:if test="not(protx:analysis_summary[@analysis=\'xpress\'])">' . $table_spacer . '</xsl:if></td></xsl:if>';

    $tab_display{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'xpress\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>' . $tab . $xpress_link . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'xpress\'])">'  . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';

    $header{'xpress'} = '<xsl:if test="$xpress_quant &gt; \'0\'">xpress';
    if(! ($quant_light2heavy eq 'true')) {
	$header{'xpress'} .= '(H/L)';
    }
    $header{'xpress'} .= ' ratio mean' . $tab . 'xpress<xsl:if test="not($reference_isotope = \'UNSET\')"><xsl:if test="$reference_isotope=\'light\'"> (H/L)</xsl:if></xsl:if> stdev' . $tab . 'xpress num peptides' . $tab . 'xpress link' . $tab. '</xsl:if>';
} # if display xpress

my $NEW_ASAP_CGI = 1;

my $asap_display_cgi = 'asap-prophet-display.cgi';
if(useXMLFormatLinks($xmlfile)) {
    $asap_display_cgi = 'ASAPCGIDisplay.cgi';
}
my $asap_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $asap_link;
if($NEW_ASAP_CGI) {
    $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '&amp;xmlFile=' . $xmlfile . '&amp;';
    $asap_header_pre .= 'markAA=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
    $asap_header_pre .= 'protein=';

    $asap_link = $TPPhostname . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>xmlFile=' . $xmlfile . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>';
    $asap_link .= 'markAA=' . $mark_aa . '<xsl:text disable-output-escaping="yes">&amp;</xsl:text>' if(! ($mark_aa eq ''));
    $asap_link .= 'protein=<xsl:value-of select="$mult_prot"/>';
}

my $asap_header_mid2 = '&amp;ratioType=0">';

my $asap_header_suf = '</A>';
my $pvalue_header_pre = '<a target="Win1" href="';

my $pvalue_header_suf = '</a>';
if($asap_display eq $checked) {
    if($NEW_ASAP_CGI) {

	# first display regular ratio no matter what
	$display{'asapratio'} = '<xsl:if test="$asap_quant &gt; \'0\'"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']">ASAPRatio';
	if(! ($quant_light2heavy eq 'true')) {
	    $display{'asapratio'} .= '(H/L)';
	}
	$display{'asapratio'} .= ': ' . $asap_header_pre . '{$mult_prot}">';
	$display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/></xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	# now add on the adjusted only if desired and present
	if ($show_adjusted_asap ne '') {
	    $display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\']">[<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_standard_dev"/>]</xsl:if>';
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><td width="200"><xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if>';

  
	}
	else {
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if>';

	}

    }
    else { # old format
	if($show_adjusted_asap eq '') {
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/></xsl:if><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	    }
	    else { # display adjsuted
		$display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'"><xsl:if test="/protx:protein_summary/protx:ASAP_pvalue_analysis_summary"><td width="400"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . ' [<xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'adj_ratio_standard_dev"/>]';
	    }
	    $display{'asapratio'} .= '</xsl:if></td><td width="200"><xsl:if test="protx:ASAPRatio">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:ASAPRatio/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if><xsl:if test="not(/protx:protein_summary/protx:ASAP_pvalue_analysis_summary)"><td width="350"><xsl:if test="protx:ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$source_files}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/><xsl:if test="protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . '</xsl:if></td></xsl:if></xsl:if>';
    } # if old version
 

    $tab_display{'asapratio'} = '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>' . $tab . $asap_link . $tab . '</xsl:if><xsl:if test="count(parent::node()/protx:analysis_result[@analysis=\'asapratio\'])= \'0\'">' . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab .'N_A' . $tab . '</xsl:if></xsl:if>';
    
    if(! ($show_adjusted_asap eq '')) { # display adjusted
	$tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev"/>' . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\'])">' .'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';
    }
    $tab_display{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></xsl:if>' . $tab . $TPPhostname . '<xsl:value-of select="$pvalpngfile"/>' . $tab .'</xsl:if>';

    $header{'asapratio'} = '<xsl:if test="count(protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">ratio mean' . $tab . 'ratio stdev' . $tab . 'ratio num peps' . $tab . 'asap ratio link'. $tab;
    if(! ($show_adjusted_asap eq '')) {
	$header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">adjusted ratio mean' . $tab . 'adjusted ratio stdev' . $tab . '</xsl:if>';
    }
    $header{'asapratio'} .= '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">pvalue' . $tab . 'pvalue link'. $tab. '</xsl:if></xsl:if>';
} # if display asapratio info

my $reference = '$group_number' ; #$show_groups eq '' ? '$parental_group_number' : '$group_number';
my $gn = $show_groups eq '' ? '<xsl:value-of select="parent::node()/@group_number"/>' : '<xsl:value-of select="@group_number"/>';
if($discards) {

    $display{'group_number'} .= '<input type="checkbox" name="incl{' . $reference . '}" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@exclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#exclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
else {

    $display{'group_number'} .= '<input type="checkbox" name="excl';
    if($show_groups eq '') {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    else {
	$display{'group_number'} .= '{' . $reference . '}';
    }
    $display{'group_number'} .= '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@inclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e <= $#inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e <= $#inclusions - 1);
	}
	$display{'group_number'} .= '">' . $gn . '</xsl:if>';
    }
    else {
	$display{'group_number'} .= $gn;
    }
}
$display{'group_number'} .= '<a name="{' . $reference . '}"/>' if($HTML);
$display{'group_number'} .= '<xsl:text> </xsl:text>';

$display{'prot_number'} = '';
if($discards) {

    $display{'prot_number'} .= '<input type="checkbox" name="pincl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pexclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pexclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pexclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
else {

    $display{'prot_number'} .= '<input type="checkbox" name="pexcl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@pinclusions > 0) {
	$display{'prot_number'} .= '<xsl:if test="$prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e <= $#pinclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e <= $#pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e <= $#pinclusions - 1);
	}
	if($show_groups eq '') {
	    $display{'prot_number'} .= '"><xsl:value-of select="$prot_number"/></xsl:if>';
	}
	else {
	    $display{'prot_number'} .= '"><xsl:value-of select="@group_sibling_id"/></xsl:if>';
	}
    }
    else {
	if($show_groups eq '') {
	    $display{'prot_number'} .= '<xsl:value-of select="$prot_number"/>';
	}
	else {
	    $display{'prot_number'} .= '<xsl:value-of select="@group_sibling_id"/>';
	}
    }
}
$display{'prot_number'} .= '<xsl:text> </xsl:text>';

$display{'n_instances'} = '<td><xsl:value-of select="@n_instances"/></td>';
$header{'n_instances'} = '<td>' . $header_pre . 'total' . $header_post . '</td>';
$tab_display{'n_instances'} = '<xsl:value-of select="@n_instances"/>';
$default_order{'n_instances'} = 7;
$tab_header{'n_instances'} = 'n instances';


foreach(keys %display) {
    $display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}

foreach(keys %annot_display) {
    $display_annot_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $reg_annot_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}


# test it out privately
if(0 && scalar keys %display_order > 0) {
    open(TEMP, ">temp.out");
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	print TEMP $display{$_}, "\n";
    }
    close(TEMP);
}

print XSL "\n";
# define tab and newline here


print XSL '<xsl:template match="protx:protein_summary">', "\n";
    printCountProtsXSL($boxptr);

print XSL '<HTML><BODY BGCOLOR="white" OnLoad="self.focus()"><PRE>', "\n";
print XSL '<HEAD><TITLE>ProteinProphet protXML Viewer (' . $TPPVersionInfo . ')</TITLE>';
print XSL '<STYLE TYPE="text/css" media="screen">', "\n";
print XSL '    div.visible {', "\n";
print XSL '    display: inline;', "\n";
print XSL '    white-space: nowrap;', "\n";
print XSL '    }', "\n";
print XSL '    div.hidden {', "\n";
print XSL '    display: none;', "\n";
print XSL '    }', "\n";
print XSL '    results {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.peptide {', "\n";
print XSL '	font-size: 12pt;', "\n";
print XSL '    }', "\n";
print XSL '    td.indist_pep {', "\n";
print XSL '	font-size: 10pt;', "\n";
print XSL '    }', "\n";
print XSL '    indist_pep_mod {', "\n";
print XSL '	font-size: 8pt;', "\n";
print XSL '    }', "\n";
print XSL '</STYLE>', "\n";
print XSL '</HEAD>', "\n";
print XSL '<table width="100%" border="3" BGCOLOR="#AAAAFF" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;"><tr><td align="center">', "\n";
print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";
if($HTML) {
    print XSL '<input type="submit" value="Restore Last View" style="background-color:#FFFF88;"/>', "\n";
    print XSL '<input type="hidden" name="restore_view" value="yes"/>', "\n";
}
else {
    print XSL '<input type="submit" value="Restore Original"/>', "\n";
    print XSL '<input type="hidden" name="restore" value="yes"/>', "\n";
}
    print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
    print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
    print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);



    print XSL '</form>';
    print XSL '</td><td align="center">';
    print XSL '<pre>ProteinProphet<sup><font size="3">&#xAE;</font></sup> protXML Viewer</pre>A.Keller   2.23.05</td>';

my $sort_none = ! exists ${$boxptr}{'sort'} || ${$boxptr}{'sort'} eq 'none' ?  $checked : '';
my $sort_xcorr = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xcorr' ? $checked : '';
my $sort_prob = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'prob' ? $checked : '';
my $sort_spec = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spec' ? $checked : '';
my $sort_pep = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'peptide' ? $checked : '';
my $sort_prot = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'protein' ? $checked : '';
my $sort_cov = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'coverage' ? $checked : '';
my $sort_peps = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'numpeps' ? $checked : '';
my $sort_spec_ids = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spectrum_ids' ? $checked : '';

my $sort_pvalue = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'pvalue' ? $checked : '';
my $text1 = exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq '') ? ${$boxptr}{'text1'} : '';
my $sort_asap_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_desc' ? $checked : '';
my $sort_asap_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_asc' ? $checked : '';
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ${$boxptr}{'filter_asap'} eq 'yes' ? $checked : '';
my $filter_xpress = exists ${$boxptr}{'filter_xpress'} && ${$boxptr}{'filter_xpress'} eq 'yes' ? $checked : '';
my $sort_xpress_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_desc' ? $checked : '';
my $sort_xpress_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_asc' ? $checked : '';
my $exclude_degens = exists ${$boxptr}{'no_degens'} && ${$boxptr}{'no_degens'} eq 'yes' ? $checked : '';

# show sens error info (still relevant for filtered dataset)
my $show_sens = exists ${$boxptr}{'senserr'} && ${$boxptr}{'senserr'} eq 'show' ? $checked : '';
my $eligible = ($filter_asap eq '' && $min_asap == 0 && $max_asap == 0 && (! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') && @exclusions == 0 && @inclusions == 0 && @pexclusions == 0 && @pexclusions == 0 && $filter_xpress eq '' && $min_xpress == 0 && $max_xpress == 0 && $asap_xpress eq '');
my $show_tot_num_peps = ! exists ${$boxptr}{'tot_num_peps'} || ${$boxptr}{'tot_num_peps'} eq 'show' ? $checked : '';
my $show_num_unique_peps = ! exists ${$boxptr}{'num_unique_peps'} || ${$boxptr}{'num_unique_peps'} eq 'show' ? $checked : '';
my $show_pct_spectrum_ids = ! exists ${$boxptr}{'pct_spectrum_ids'} || ${$boxptr}{'pct_spectrum_ids'} eq 'show' ? $checked : '';

my $suffix = $HTML_ORIENTATION ? '.htm' : '.xml';
$suffix = '.shtml' if($SHTML);

# write output xml
print XSL '<td align="center"><form method="GET" target="Win1" action="' . $CGI_HOME . 'protxml2html.pl">', "\n";

print XSL '<pre>';
print XSL '<input type="submit" value="Write Displayed Data Subset to File" /><pre>' . $nonbreakline . '</pre>';
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";

print XSL '<input type="hidden" name="ex1" value="yes"/>', "\n" if(! ($exclude_1 eq ''));
print XSL '<input type="hidden" name="ex2" value="yes"/>', "\n" if(! ($exclude_2 eq ''));
print XSL '<input type="hidden" name="ex3" value="yes"/>', "\n" if(! ($exclude_3 eq ''));
print XSL '<input type="hidden" name="show_ggl" value="yes"/>', "\n" if(! ($show_ggl eq ''));
print XSL '<input type="hidden" name="text1" value="' . ${$boxptr}{'text1'} . '"/>' if(exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq ''));
print XSL '<input type="hidden" name="min_prob" value="' . $minprob . '"/>' if($minprob > 0);
print XSL '<input type="hidden" name="min_score1" value="' . $minscore[0] . '"/>' if($minscore[0] > 0);
print XSL '<input type="hidden" name="min_score2" value="' . $minscore[1] . '"/>' if($minscore[1] > 0);
print XSL '<input type="hidden" name="min_score3" value="' . $minscore[2] . '"/>' if($minscore[2] > 0);
print XSL '<input type="hidden" name="min_ntt" value="' . $minntt . '"/>' if($minntt > 0);
print XSL '<input type="hidden" name="max_nmc" value="' . $maxnmc . '"/>' if($maxnmc >= 0);
print XSL '<input type="hidden" name="pep_aa" value="' . $pep_aa . '"/>' if(! ($pep_aa eq ''));
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>' if(! ($inclusions eq ''));
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>' if(! ($exclusions eq ''));
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>' if(! ($pinclusions eq ''));
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>' if(! ($pexclusions eq ''));
print XSL '<input type="hidden" name="filter_asap" value="yes"/>' if(! ($filter_asap eq ''));
print XSL '<input type="hidden" name="filter_xpress" value="yes"/>' if(! ($filter_xpress eq ''));
print XSL '<input type="hidden" name="min_pepprob" value="' . $min_pepprob . '"/>' if(! ($min_pepprob eq ''));
print XSL '<input type="hidden" name="show_groups" value="yes"/>' if(! ($show_groups eq ''));
print XSL '<input type="hidden" name="min_xpress" value="' . $min_xpress . '"/>' if($min_xpress > 0);
print XSL '<input type="hidden" name="max_xpress" value="' . $max_xpress . '"/>' if($max_xpress > 0);
print XSL '<input type="hidden" name="min_asap" value="' . $min_asap . '"/>' if($min_asap > 0);
print XSL '<input type="hidden" name="max_asap" value="' . $max_asap . '"/>' if($max_asap > 0);
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
print XSL '<input type="hidden" name="show_adjusted_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="adj_asap" value="yes"/>' if(! ($show_adjusted_asap eq ''));
print XSL '<input type="hidden" name="max_pvalue" value="' . $max_pvalue_display . '"/>' if($max_pvalue_display < 1.0);
print XSL '<input type="hidden" name="asap_xpress" value="yes"/>' if(! ($asap_xpress eq ''));
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

print XSL 'file name: <input type="text" name="outfile" value="" size="20" maxlength="100"/>' . $suffix . '</pre>', "\n";
print XSL '</form></td></tr></table>';

print XSL '<form method="GET" action="' . $CGI_HOME . 'protxml2html.pl"><table width="100%" border="3" BGCOLOR="#AAAAFF">';
print XSL '<tr><td align="left" valign="center"><pre><input type="checkbox" name="show_ggl" value="yes" ' . $show_ggl . '/><b>Enable Gaggle Broadcast</b><A TARGET="Win1" HREF="http://tools.proteomecenter.org/wiki/index.php?title=Software:Firegoose%2C_Gaggle%2C_and_PIPE"><IMG BORDER="0" SRC="'. $HELP_DIR. 'images/qMark.png"/></A></pre></td></tr><tr><td><pre>';

if($discards) {
    print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
}
else {
    print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
}

print XSL $table_spacer . '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_sens_err.pl?xmlfile=' . $xmlfile;
print XSL '&amp;' if(! $DISTR_VERSION);

print XSL '">Sensitivity/Error Info</a></xsl:if><xsl:if test="protx:dataset_derivation/@generation_no &gt;\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_dataset_derivation.pl?xmlfile=' . $xmlfile . '">Dataset Derivation Info</a></xsl:if>';;

print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'more_anal.pl?xmlfile=' . $xmlfile;
print XSL '&amp;shtml=yes' if($SHTML);
print XSL '&amp;helpdir=' . $HELP_DIR;
print XSL '">More Analysis Info</a>';
print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'show_help.pl?help_dir=' . $HELP_DIR . '">Help</a>';
print XSL $newline;



print XSL '<xsl:text> </xsl:text>';
print XSL 'sort by: <input type="radio" name="sort" value="none" ' . $sort_none . '/>index';
print XSL '<input type="radio" name="sort" value="prob" ' . $sort_prob, '/>probability';
print XSL ' <input type="radio" name="sort" value="protein" ' . $sort_prot, '/>protein';
print XSL ' <input type="radio" name="sort" value="coverage" ' . $sort_cov, '/>coverage';

print XSL '<xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids)"> <input type="radio" name="sort" value="numpeps" ' . $sort_peps, '/>num peps</xsl:if>';
print XSL '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@total_no_spectrum_ids"> <input type="radio" name="sort" value="spectrum_ids" ' . $sort_spec_ids, '/>share of spectrum ids</xsl:if>';
print XSL '<xsl:if test="$xpress_quant &gt; \'0\' and count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'"> ';
print XSL $newline . '<xsl:text>          </xsl:text>';
print XSL '</xsl:if>';

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"> ';
print XSL ' <input type="radio" name="sort" value="xpress_desc" ' . $sort_xpress_desc, '/>xpress desc';
print XSL ' <input type="radio" name="sort" value="xpress_asc" ' . $sort_xpress_asc, '/>xpress asc';
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';
print XSL ' <input type="radio" name="sort" value="asap_desc" ' . $sort_asap_desc, '/>asap desc';
print XSL ' <input type="radio" name="sort" value="asap_asc" ' . $sort_asap_asc, '/>asap asc';
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"> <input type="radio" name="sort" value="pvalue" ' . $sort_pvalue, '/>pvalue</xsl:if>';
print XSL '</xsl:if>';

print XSL $newline;

print XSL '<xsl:text> </xsl:text>min probability: <INPUT TYPE="text" NAME="min_prob" VALUE="' . $minprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
# pick one of the following
print XSL $newline;
print XSL '<xsl:text> </xsl:text>protein groups: <input type="radio" name="show_groups" value="show" ' . $show_groups . '/>show  ';
print XSL '<input type="radio" name="show_groups" value="hide" ' . $hide_groups . '/>hide  ';

print XSL '   annotation: <input type="radio" name="show_annot" value="show" ' . $show_annot . '/>show  ';
print XSL '<input type="radio" name="show_annot" value="hide" ' . $hide_annot . '/>hide  ';
print XSL '   peptides: <input type="radio" name="show_peps" value="show" ' . $show_peps . '/>show  ';
print XSL '<input type="radio" name="show_peps" value="hide" ' . $hide_peps . '/>hide  ';
print XSL $newline;

print XSL '<xsl:if test="$xpress_quant &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o XPRESS Ratio: <input type="checkbox" name="filter_xpress" value="yes" ' . $filter_xpress . '/>';
print XSL '  min XPRESS Ratio: <INPUT TYPE="text" NAME="min_xpress" VALUE="', $min_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max XPRESS Ratio: <INPUT TYPE="text" NAME="max_xpress" VALUE="', $max_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:ASAPRatio) &gt; \'0\'">  ASAPRatio consistent: <input type="checkbox" name="asap_xpress" value="yes" ' . $asap_xpress . '/></xsl:if>';

print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:if test="count(/protx:protein_summary/protx:protein_group/protx:protein/protx:analysis_result[@analysis=\'asapratio\']) &gt; \'0\'">';

print XSL '<xsl:text> </xsl:text>exclude w/o ASAPRatio: <input type="checkbox" name="filter_asap" value="yes" ' . $filter_asap . '/>';
print XSL '  min ASAPRatio: <INPUT TYPE="text" NAME="min_asap" VALUE="', $min_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max ASAPRatio: <INPUT TYPE="text" NAME="max_asap" VALUE="', $max_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
my $alt_max = $max_pvalue_display < 1.0 ? $max_pvalue_display : '';

print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']">  max pvalue: <INPUT TYPE="text" NAME="max_pvalue" VALUE="', $alt_max, '" SIZE="3" MAXLENGTH="8"/>  adjusted: <input type="checkbox" name="show_adjusted_asap" value="yes" ' . $show_adjusted_asap . '/><input type="hidden" name="adj_asap" value="yes"/></xsl:if>';
print XSL '<xsl:text> </xsl:text><input type="submit" name="action" value="Recompute p-values"/>';
print XSL $newline;
print XSL '</xsl:if>';

print XSL '<xsl:text> </xsl:text>exclude degen peps: <input type="checkbox" name="no_degens" value="yes" ' . $exclude_degens . '/>';
print XSL '  exclude charge: <input type="checkbox" name="ex1" value="yes" ' . $exclude_1 . '/>1+';
print XSL '<input type="checkbox" name="ex2" value="yes" ' . $exclude_2 . '/>2+';
print XSL '<input type="checkbox" name="ex3" value="yes" ' . $exclude_3 . '/>3+' . '<xsl:text>   </xsl:text>';

print XSL 'min pep prob: <INPUT TYPE="text" NAME="min_pep_prob" VALUE="' . $min_pepprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text>';
print XSL ' min num tol term: <INPUT TYPE="text" NAME="min_ntt" VALUE="', $minntt_display, '" SIZE="1" MAXLENGTH="1"/><xsl:text> </xsl:text>';
print XSL $newline;


print XSL '<xsl:text> </xsl:text>include aa: <INPUT TYPE="text" NAME="pep_aa" VALUE="', $pep_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   mark aa: <INPUT TYPE="text" NAME="mark_aa" VALUE="', $mark_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   NxS/T: <input type="checkbox" name="glyc" value="yes" ' . $glyc . '/><xsl:text>   </xsl:text>';
print XSL 'protein text: <input type="text" name="text1" value="', $text1, '" size="12" maxlength="24"/><xsl:text>   </xsl:text>';
print XSL 'export to excel: <input type="checkbox" name="excel" value="yes" />', "\n";
print XSL '<input type="hidden" name="restore" value="no"/>', "\n";
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>', "\n";
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>', "\n";
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);
print XSL '<input type="hidden" name="glyc" value="yes"/>' if($glyc);
print XSL '<input type="hidden" name="xml_input" value="1"/>' if(! $DISTR_VERSION && $NEW_XML_FORMAT);
print XSL '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><input type="hidden" name="asapratio_pvalue" value="yes"/></xsl:if>';



if($full_menu) {
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL '<xsl:text> </xsl:text>sensitivity/error information: <input type="radio" name="senserr" value="show" ';
    print XSL $checked if(! ($show_sens eq ''));
    print XSL '/>show<input type="radio" name="senserr" value="hide" ';
    print XSL $checked if($show_sens eq '');
    print XSL '/>hide' . $newline;
    
    print XSL '<pre>' . $newline . '</pre>';


    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><xsl:text> </xsl:text>Quantitation Ratio: <input type="radio" name="quant_light2heavy" value="true" ';
	print XSL $checked if(! ($quant_light2heavy eq 'false'));
	print XSL '/>light/heavy<input type="radio" name="quant_light2heavy" value="false" ';
	print XSL $checked if($quant_light2heavy eq 'false');
	print XSL '/>heavy/light';
	print XSL '<pre>' . $newline . '</pre></xsl:if>';
    } # only for xml version

    print XSL '<xsl:if test="$xpress_quant &gt; \'0\'"><xsl:text> </xsl:text>XPRESS information: <input type="radio" name="xpress_display" value="show" ';
    print XSL $checked if($xpress_display eq $checked);
    print XSL '/>show<input type="radio" name="xpress_display" value="hide" ';
    print XSL $checked if($xpress_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:if test="$asap_quant &gt; \'0\'"><xsl:text> </xsl:text>ASAPRatio information: <input type="radio" name="asap_display" value="show" ';
    print XSL $checked if($asap_display eq $checked);
    print XSL '/>show<input type="radio" name="asap_display" value="hide" ';
    print XSL $checked if($asap_display ne $checked);
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:text> </xsl:text>protein display  ' . $newline;
    print XSL '<xsl:text> </xsl:text>number unique peptides: <input type="radio" name="num_unique_peps" value="show" ';
    print XSL $checked if(! ($show_num_unique_peps eq ''));
    print XSL '/>show<input type="radio" name="num_unique_peps" value="hide" ';
    print XSL $checked if($show_num_unique_peps eq '');
    print XSL '/>hide';
    print XSL '   total number peptides: <input type="radio" name="tot_num_peps" value="show" ';
    print XSL $checked if(! ($show_tot_num_peps eq ''));
    print XSL '/>show<input type="radio" name="tot_num_peps" value="hide" ';
    print XSL $checked if($show_tot_num_peps eq '');
    print XSL '/>hide';

    print XSL '   share of spectrum ids: <input type="radio" name="pct_spectrum_ids" value="show" ';
    print XSL $checked if(! ($show_pct_spectrum_ids eq ''));
    print XSL '/>show<input type="radio" name="pct_spectrum_ids" value="hide" ';
    print XSL $checked if($show_pct_spectrum_ids eq '');
    print XSL '/>hide';

    print XSL $newline;
    print XSL '<pre>' . $newline . '</pre>';

    print XSL '<xsl:text> </xsl:text>peptide column display   ' . $newline;

    print XSL '<input type="radio" name="order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;


    print XSL '<xsl:text> </xsl:text>weight <input type="text" name="weight" value="' . $register_order{'weight'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>peptide sequence <input type="text" name="peptide_sequence" value="' . $register_order{'peptide_sequence'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>nsp adjusted probability <input type="text" name="nsp_adjusted_probability" value="' . $register_order{'nsp_adjusted_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>initial probability <input type="text" name="initial_probability" value="' . $register_order{'initial_probability'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>number tolerable termini <input type="text" name="num_tol_term" value="' . $register_order{'num_tol_term'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>nsp bin <input type="text" name="n_sibling_peptides_bin" value="' . $register_order{'n_sibling_peptides_bin'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>total number peptide instances <input type="text" name="n_instances" value="' . $register_order{'n_instances'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>peptide group index <input type="text" name="peptide_group_designator" value="' . $register_order{'peptide_group_designator'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and not($organism=\'Drosophila\')">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline . 'annotation column display   ' . $newline;
    print XSL '<input type="radio" name="annot_order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="annot_order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;
    print XSL '<xsl:text> </xsl:text>ensembl <input type="text" name="ensembl" value="' . $reg_annot_order{'ensembl'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>trembl <input type="text" name="trembl" value="' . $reg_annot_order{'trembl'} . '" size="2" maxlength="3"/>', $newline;
   print XSL '<xsl:text> </xsl:text>swissprot <input type="text" name="swissprot" value="' . $reg_annot_order{'swissprot'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>refseq <input type="text" name="refseq" value="' . $reg_annot_order{'refseq'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>locuslink <input type="text" name="locus_link" value="' . $reg_annot_order{'locus_link'} . '" size="2" maxlength="3"/>', $newline;

    print XSL '</xsl:if>';
    print XSL '<pre>' . $newline . '</pre>';
    print XSL "---------------------------------------------------------------------------------------------------------";    
    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>set customized data view: ';
    print XSL '<input type="radio" name="custom_settings" value="prev" ' . $checked . '/>no change ';
    print XSL '<input type="radio" name="custom_settings" value="current"/>current ';
    print XSL '<input type="radio" name="custom_settings" value="default"/>default';

    print XSL '<pre>' . $newline . '</pre>';
    print XSL '<xsl:text> </xsl:text>short menu <input type="checkbox" name="short_menu" value="yes"/>';
    print XSL '<input type="hidden" name="menu" value="full"/>';


} # if full menu
else { # short menu case
    print XSL '<pre>' . $nonbreakline . '</pre>'. $newline;
    print XSL ' full menu <input type="checkbox" name="full_menu" value="yes"/>  '; 
    print XSL '    show discarded entries <input type="checkbox" name="discards" value="yes" ' . $discards . '/>    clear manual discards/restores <input type="checkbox" name="clear" value="yes"/>';

    # hidden information

    # quantitation info
    if(useXMLFormatLinks($xmlfile)) {
	print XSL '<xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'"><input type="hidden" name="quant_light2heavy" value="';
	if($quant_light2heavy eq 'false') {
	    print XSL 'false';
	}
	else {
	    print XSL 'true';
	}
	print XSL '"/></xsl:if>';
    } # only for xml version


    foreach(keys %register_order) {
	print XSL '<input type="hidden" name="' . $_ . '" value="' . $register_order{$_} . '"/>';
    }
    print XSL '<input type="hidden" name="quant_light2heavy" value="' . $quant_light2heavy . '"/>';

# more here

    print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
    print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="xpress_display" value="';
    if($xpress_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";
    print XSL '<input type="hidden" name="asap_display" value="';
    if($asap_display ne $checked) {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";

}
if($CALCULATE_PIES) {
    print XSL '<xsl:if test="not($organism = \'UNKNOWN\') and $organism=\'Homo_sapiens\'">    go ontology level <select name="go_level"><option value="0"/><option value="1"';
    print XSL ' selected="yes"' if($go_level == 1);
    print XSL '>1</option><option value="101"';
    print XSL ' selected="yes"' if($go_level == 101);

    print XSL '>1H</option><option value="2"';
    print XSL ' selected="yes"' if($go_level == 2);
    print XSL '>2</option><option value="102"';
    print XSL ' selected="yes"' if($go_level == 102);


    print XSL '>2H</option><option value="3"';
    print XSL ' selected="yes"' if($go_level == 3);
    print XSL '>3</option>';
    print XSL '<option value="103"';
    print XSL ' selected="yes"' if($go_level == 103);
    print XSL '>3H</option>'; 



    print XSL '<option value="4"';
    print XSL ' selected="yes"' if($go_level == 4);
    print XSL '>4</option>'; 
    print XSL '<option value="104"';
    print XSL ' selected="yes"' if($go_level == 104);
    print XSL '>4H</option>'; 


    print XSL '<option value="5"';
    print XSL ' selected="yes"' if($go_level == 5);
    print XSL '>5</option>';
    print XSL '<option value="105"';
    print XSL ' selected="yes"' if($go_level == 105);
    print XSL '>5H</option>'; 

    print XSL '<option value="6"';
    print XSL ' selected="yes"' if($go_level == 6);
    print XSL '>6</option>';
    print XSL '<option value="106"';
    print XSL ' selected="yes"' if($go_level == 106);
    print XSL '>6H</option>'; 

    print XSL '<option value="7"';
    print XSL ' selected="yes"' if($go_level == 7);
    print XSL '>7</option>';
    print XSL '<option value="107"';
    print XSL ' selected="yes"' if($go_level == 107);
    print XSL '>7H</option>'; 

    print XSL '<option value="8"';
    print XSL ' selected="yes"' if($go_level == 8);
    print XSL '>8</option>';
    print XSL '<option value="108"';
    print XSL ' selected="yes"' if($go_level == 108);
    print XSL '>8H</option>'; 


    print XSL '<option value="9"';
    print XSL ' selected="yes"' if($go_level == 9);
    print XSL '>9</option>';
    print XSL '<option value="109"';
    print XSL ' selected="yes"' if($go_level == 109);
    print XSL '>9H</option>'; 

    print XSL '</select></xsl:if>';
} # if calc pies
print XSL $nonbreakline ;

if($full_menu) {
    print XSL '<pre>' . $newline . '</pre>';
    if($discards) {
	print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
    }
    else {
	print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
    }
    print XSL $newline;
}

print XSL '</pre></td></tr></table>', "\n";

# make local reference
if(exists ${$boxptr}{'excel'} && ${$boxptr}{'excel'} eq 'yes') {
    my $local_excelfile = $excelfile;
    if((length $SERVER_ROOT) <= (length $local_excelfile) && 
       index((lc $local_excelfile), ($LC_SERVER_ROOT)) == 0) {
	$local_excelfile =  substr($local_excelfile, (length $SERVER_ROOT));
	if (substr($local_excelfile, 0, 1) ne '/') {
	    $local_excelfile = '/' . $local_excelfile;
	}
    }
    else {
	die "problem: $local_excelfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    my $windows_excelfile = $excelfile;
    if($WINDOWS_CYGWIN) {
	$windows_excelfile =  `cygpath -w '$excelfile'`;
	if($windows_excelfile =~ /^(\S+)\s?/) {
	    $windows_excelfile = $1;
	}
    }
    print XSL 'excel file: <a target="Win1" href="' . $local_excelfile . '">' . $windows_excelfile . '</a>'  . $newline;
    
}
if((! ($show_sens eq '') && $eligible)) {

  # make local reference
  my $local_pngfile = $pngfile;
  if(! $ISB_VERSION) {
      if((length $SERVER_ROOT) <= (length $local_pngfile) && 
	 index((lc $local_pngfile), ($LC_SERVER_ROOT)) == 0) {
	  $local_pngfile = '/' . substr($local_pngfile, (length $SERVER_ROOT));
      }
      else {
	  die "problem: $local_pngfile is not mounted under webserver root: $SERVER_ROOT\n";
      }
  } # if iis & cygwin

    print XSL '<xsl:if test="protx:dataset_derivation/@generation_no=\'0\'">';
    print XSL '<font color="blue"> Predicted Total Number of Correct Entries: <xsl:value-of select="protx:protein_summary_header/@num_predicted_correct_prots"/></font>';
    print XSL "\n\n";
    print XSL "<TABLE><TR><TD>";

    print XSL "<IMG SRC=\"$local_pngfile\"/>";
    print XSL "</TD><TD><PRE>";

    print XSL "<font color=\"red\">sensitivity</font>\tfraction of all correct proteins" . $newline . $tab . $tab . " with probs &gt;= min_prob" . $newline;
    print XSL "<font color=\"green\">error</font>\t\tfraction of all proteins with probs" . $newline . $tab . $tab . " &gt;= min_prob that are incorrect" . $newline . '<pre>' . $newline . '</pre>';

    print XSL 'minprob' . $tab . '<font color="red">sens</font>' . $tab . '<font color="green">err</font>' . $tab . '<font color="red"># corr</font>' . $tab . '<font color ="green"># incorr</font>' . $newline;
    print XSL '<xsl:apply-templates select="protx:protein_summary_header/protx:protein_summary_data_filter">';
    print XSL '<xsl:sort select="@min_probability" order="descending" data-type="number"/>';
    print XSL '</xsl:apply-templates>';

    print XSL '</PRE></TD></TR></TABLE>';
    print XSL $newline . '<pre>' . $newline . '</pre>';
    print XSL '</xsl:if>';
}


########################## COUNT ENTRIES  #################################

my $local_xmlfile = $xmlfile;
my $windows_xmlfile = $xmlfile;
if(! $ISB_VERSION) {
    if((length $SERVER_ROOT) <= (length $local_xmlfile) && 
       index((lc $local_xmlfile), ($LC_SERVER_ROOT)) == 0) {
	$local_xmlfile = '/' . substr($local_xmlfile, (length $SERVER_ROOT));
    }
    else {
	die "problem: $local_xmlfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    if($WINDOWS_CYGWIN) {
	$windows_xmlfile = `cygpath -w '$windows_xmlfile'`;
	if($windows_xmlfile =~ /^(\S+)\s?/) {
	    $windows_xmlfile = $1;
	}
    }
} # if iis & cygwin

my $MAX_XMLFILE_LEN = 80;
my $format_choice = ($WINDOWS_CYGWIN && (length $windows_xmlfile) > $MAX_XMLFILE_LEN) || 
	(! $WINDOWS_CYGWIN && (length $local_xmlfile) > $MAX_XMLFILE_LEN) ? '<br/>' : '';


if(! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') {


    print XSL '<font color="red">';
    print XSL '<xsl:value-of select="$prot_group_count"/>';

    print XSL '<font color="black"><i> discarded</i></font>' if($discards);

    print XSL ' entries (';

    print XSL '<xsl:value-of select="$single_hits_count"/>';

    print XSL ' single hits)';

    if(! $ISB_VERSION) {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; 
}
    else {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; 
    }
} # if count
else {

    print XSL '<font color="black"><i>discarded</i></font> ' if($discards);
    if(! $ISB_VERSION) {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }
    else {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; #, '<pre>' . $newline . '</pre>';
    }

}


###################################################

print XSL $newline . '<pre>' . $newline . '</pre>';
print XSL '<FONT COLOR="990000">* indicates peptide corresponding to unique protein entry</FONT>' . $newline;


# calculate how many columns, and header line here

$num_cols += 8;
my $extra_column = '<td>' . $table_spacer . '</td>';

#my $HEADER = '<td><!-- header --></td>';


 
#$HEADER = $header{'group_number'} . $tab; # cancel it
#$HEADER .= 'group probability' . $tab if(! ($show_groups eq ''));

my $numcols = 0;

my $HEADER = $header{'protein'} . $tab;
$HEADER .= 'protein_probability';

$numcols += 1;

if ($asap_display eq $checked && ! ($asap_display eq 'hide') ) {
    $HEADER .= $tab . 'asap_ratio' ;
    $HEADER .= $tab . 'asap_stddev' ;
    $HEADER .= $tab . 'asap_numpeps' ;
    $numcols += 3;
}

if ( $xpress_display eq $checked  && ! ($xpress_display eq 'hide') ) {
    $HEADER .= $tab . 'xpress_ratio' ;
    $HEADER .= $tab . 'xpress_stddev' ;
    $HEADER .= $tab . 'xpress_numpeps' ;
    $numcols += 3;
}

$HEADER .= $newline;


print XSL $RESULT_TABLE_PRE . $RESULT_TABLE, "\n";


print XSL '<xsl:comment>' . $start_string . '</xsl:comment>' . $newline . "\n";

print XSL $HEADER;

# bypass protein groups altogether for no groups mode.....
if(! ($show_groups eq '')) {
    print XSL "\n", '<xsl:apply-templates select="protx:protein_group">', "\n";
}
else {
    print XSL '<xsl:apply-templates select="protx:protein_group/protx:protein">', "\n";
}

if(! ($sort_pvalue) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_xpress_desc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	print XSL 'sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean' . "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_xpress_asc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
    else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_asap_desc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
}
elsif(! ($sort_asap_asc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
	else {
	    print XSL '<xsl:sort select="count(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
}
elsif(! ($sort_prob eq '')) {
	print XSL '<xsl:sort select="@probability" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_prot eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@protein_name"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@protein_name"/>', "\n";

    }
}
elsif(! ($sort_cov eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@percent_coverage" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@percent_coverage" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_peps eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@total_number_peptides" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@total_number_peptides" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_spec_ids eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protx:protein[@group_sibling_id = \'a\']/@pct_spectrum_ids" order="descending" data-type="number"/>', "\n";

    }
}
else {
    if($USE_INDEX) {
	if($show_groups eq '') {
	    print XSL '<xsl:sort select="parent::node()/@group_number" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="@group_sibling_id"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="@group_number" data-type="number"/>', "\n";
	}
    }

}

print XSL '</xsl:apply-templates>', "\n";

print XSL $RESULT_TABLE_SUF, "\n";
print XSL '</form>';
print XSL '</PRE></BODY></HTML>', "\n";
print XSL '</xsl:template>', "\n";

if(! ($show_groups eq '')) {
print XSL '<xsl:template match="protx:protein_group">', "\n";

my $suffix = '';
if(@inclusions > 0) {
    $suffix = ' or @group_number=\'';
    for(my $i = 0; $i <= $#inclusions; $i++) {
	$suffix .= $inclusions[$i] . '\'';
	$suffix .= ' or @group_number=\'' if($i <= $#inclusions - 1);
    }
}

foreach(keys %parent_incls) {
    $suffix .= ' or @group_number=\'' . $_ . '\'';
}    


if($discards) {

    if(! ($show_groups eq '')) {
	# see if fails criteria
	print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	} # show adjusted
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
	# check for all exclusions
	if(@exclusions > 0) {
	    for(my $k = 0; $k <= $#exclusions; $k++) {
		print XSL ' or (@group_number = \'' . $exclusions[$k] . '\')';
	    }
 	}
	# check for excluded children of this parent
	if(@pexclusions > 0) {
	    foreach(keys %parent_excls) {
		print XSL ' or (@group_number = \'' . $_ . '\')';
	    }
	}
	print XSL '">';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '<xsl:if test="not(@group_number=\'' . $inclusions[$i] . '\')">', "\n";
	}

    } # groups
    else {  # hide groups...want to make sure no singletons pass by default
	print XSL '<xsl:if test="count(protx:protein) &gt;\'1\' or protx:protein[@group_sibling_id=\'a\']/@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPress/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\') or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	print XSL ' or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	foreach(@exclusions) {
	    print XSL ' or @group_number=\'' . $_ . '\'';
	}
	print XSL '">';
	foreach(@inclusions) {
	    print XSL '<xsl:if test="not(count(protx:protein) =\'1\' and @group_number=\'' . $_ . '\')">';
	}

    }

} # discards
else { # conventional view

    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '<xsl:if test="not(@group_number=\'' . $exclusions[$e] . '\')">', "\n";
    }
    if(! ($show_groups eq '')) {

	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));

	}
	else { # adjusted asapratios
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));
	}
	print XSL '<xsl:if test="(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' . $suffix . '">' if($max_pvalue_display < 1.0);

	print XSL '<xsl:if test="(@probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
    }
    else { # hide groups
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or @probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'))' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'))' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	else {
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	print XSL '<xsl:if test="(count(protx:protein) &gt;\'1\' or (protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'))' . $suffix . '">' if($max_pvalue_display < 1.0);

    }

} # normal mode

print XSL '<xsl:variable name="group_member" select="count(protx:protein)"/>';
print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));
print XSL '<xsl:variable name="parental_group_number" select="parent::node()/@group_number"/>';
print XSL '<xsl:variable name="sole_prot" select="protx:protein/@protein_name"/>';
print XSL '<xsl:variable name="database" select="$ref_db"/>';
print XSL '<xsl:variable name="peps1" select="protx:protein/@unique_stripped_peptides"/>';

print XSL '<xsl:apply-templates select="protx:protein">';
print XSL '<xsl:with-param name="group_member" select="$group_member"/>';
print XSL '<xsl:with-param name="group_number" select="$group_number"/>' if(! ($show_groups eq ''));
print XSL '<xsl:with-param name="parental_group_number" select="$parental_group_number"/>';
print XSL '<xsl:with-param name="sole_prot" select="$sole_prot"/>';
print XSL '<xsl:with-param name="database" select="$database"/>';
print XSL '<xsl:with-param name="peps1" select="$peps1"/>';

print XSL '</xsl:apply-templates>';

if($discards) {
    if(! ($show_groups eq '')) {
	print XSL '</xsl:if>';
	for(my $i = 0; $i <= $#inclusions; $i++) {
	    print XSL '</xsl:if>';
	}
    }
    else { # hide groups
	print XSL '</xsl:if>';
	foreach(@inclusions) {
	    print XSL '</xsl:if>';
	}
    }
}
else {
    ############################ 10/7/03
    print XSL '</xsl:if>' if(! ($asap_xpress eq ''));  # agree
    print XSL '</xsl:if>' if($minprob > 0);
    print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
    print XSL '</xsl:if>' if(! ($filter_asap eq ''));
    print XSL '</xsl:if>' if($min_xpress > 0);
    print XSL '</xsl:if>' if($max_xpress > 0);
    print XSL '</xsl:if>' if($min_asap > 0);
    print XSL '</xsl:if>' if($max_asap > 0);
    print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
    for(my $e = 0; $e <= $#exclusions; $e++) {
	print XSL '</xsl:if>', "\n";
    }
}

print XSL '</xsl:template>', "\n";

} # only if show groups

############ PROTEIN ########################
print XSL '<xsl:template match="protx:protein">';
print XSL '<xsl:param name="group_member" />';
print XSL '<xsl:param name="group_number" />' if(! ($show_groups eq ''));
print XSL '<xsl:param name="parental_group_number" />';
print XSL '<xsl:param name="sole_prot"/>';
print XSL '<xsl:param name="database"/>';
print XSL '<xsl:param name="peps1"/>';

my $num_pincl = 0;

#print XSL '<xsl:variable name="group_number" select="parent::node()/@group_number"/>' if($show_groups eq '');
#print XSL '<xsl:variable name="group_number" select="@group_number"/>' if(! ($show_groups eq ''));



# integrate inclusions....
if($discards) {
    
    print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';

    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'' . $min_xpress . '\')' if($min_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;=\'' . $max_xpress . '\')' if($max_xpress > 0);
    print XSL ' or not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));
    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio\']) or not(protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));

    }
    else {
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'xpress\'] and ((protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &lt; \'0\') or (protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));
    }
    print XSL ' or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
    if(@exclusions > 0) {
	foreach(@exclusions) {
	    print XSL ' or $group_number=\'' . $_ . '\'';
	}
    }
    if(@pexclusions > 0) {
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or ($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';

	    }
	}
    }
    print XSL '">';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '<xsl:if test="not($group_number=\'' . $inclusions[$i] . '\')">', "\n";
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')">', "\n";
	}
    }
}
else { # conventional

    # need suffix
    my $prot_suffix = '';
    if(@pinclusions > 0) {
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		$prot_suffix .= ' or($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }    
	}
    }

    if($show_groups eq '') {
	foreach(@exclusions) {
	    print XSL '<xsl:if test="not(count(parent::node()/protx:protein)=\'1\' and $group_number=\'' . $_ . '\')' . $prot_suffix . '">';
	}
    }


    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not($group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')' . $prot_suffix . '">', "\n";
	}
    }
    if($show_groups eq '') {
	print XSL '<xsl:if test="@probability &gt;= \'' . $minprob . '\'' . $prot_suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_asap eq ''));

	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'' . $prot_suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'xpress\'] and protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'' . $prot_suffix . '">' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {

	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:protx:analysis_result[@analysis=\'asapratio\'] and protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);


	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_mean + protx:protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@'. getRatioPrefix($quant_light2heavy) .'ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio_pvalue\']) or (protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));

	}
    print XSL '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'' . $prot_suffix . '">' if($max_pvalue_display < 1.0);

    }

    foreach(keys %parent_incls) {
	my @members = @{$parent_incls{$_}};
	if(@members > 0) {
	    $num_pincl++;
	    print XSL '<xsl:if test="not($group_number=\'' . $_ . '\')';
	    for(my $m = 0; $m <= $#members; $m++) {
		print XSL ' or @group_sibling_id=\'' . $members[$m] . '\'';
	    }
	    print XSL '">';
	}

    }
#####################
    print XSL '<xsl:if test="count(protx:peptide)=\'1\'">' if($SINGLE_HITS);

} # convent


# check whether part of group
print XSL '<xsl:variable name="mult_prot" select="@protein_name"/>';
print XSL '<xsl:variable name="database2" select="$ref_db"/>';
print XSL '<xsl:variable name="peps2" select="@unique_stripped_peptides"/>';
print XSL '<xsl:variable name="filextn"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn">_<xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="asap_ind" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@index"/>';
print XSL '<xsl:variable name="prot_number"><xsl:value-of select="parent::node()/@group_number"/><xsl:if test="count(parent::node()/protx:protein) &gt;\'1\'"><xsl:value-of select="@group_sibling_id"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="pvalpngfile" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']/protx:ASAP_pvalue_analysis_summary/@analysis_distribution_file"/>';

# more variables here
print XSL '<xsl:variable name="peptide_string" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@peptide_string"/>';

if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev"/>';
    print XSL '<xsl:variable name="asapratio" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean"/>';
    print XSL '<xsl:variable name="asapstd" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev"/>';
}
else { # reverse
    print XSL '<xsl:variable name="xratio" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_mean"/>';
    print XSL '<xsl:variable name="xstd" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_standard_dev"/>';
    print XSL '<xsl:variable name="asapratio" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@heavy2light_ratio_mean"/>';
    print XSL '<xsl:variable name="asapstd" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@heavy2light_ratio_standard_dev"/>';
}


print XSL '<xsl:variable name="xnum" select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>';
print XSL '<xsl:variable name="asapnum" select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>';
print XSL '<xsl:variable name="min_pep_prob" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@min_peptide_probability"/>';
# print XSL '<xsl:variable name="source" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="heavy2light"><xsl:if test="$reference_isotope=\'heavy\'">0</xsl:if><xsl:if test="$reference_isotope=\'light\'">1</xsl:if></xsl:variable>';

#if($show_groups eq '' && ! ($show_peps eq '')) {
#    print XSL $newline;
#}

#print XSL '<xsl:apply-templates select="protx:peptide">';
#print XSL '<xsl:sort select = "@nsp_adjusted_probability" order="descending" data-type="number"/>';
#print XSL '<xsl:with-param name="pvalpngfile" select="$pvalpngfile"/>';
#print XSL '<xsl:with-param name="mult_prot" select="$mult_prot"/>';
#print XSL '<xsl:with-param name="peptide_string" select="$peptide_string"/>';
#print XSL '<xsl:with-param name="xratio" select="$xratio"/>';
#print XSL '<xsl:with-param name="xstd" select="$xstd"/>';
#print XSL '<xsl:with-param name="xnum" select="$xnum"/>';
#print XSL '<xsl:with-param name="min_pep_prob" select="$min_pep_prob"/>';
#print XSL '<xsl:with-param name="source" select="$source"/>';
#print XSL '</xsl:apply-templates>';


$tab_display{'protein'} = '<xsl:value-of select="@protein_name"/><xsl:for-each select="protx:indistinguishable_protein"><xsl:text>,</xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each>';

print XSL $tab_display{'protein'} ;

# here need prot prob
print XSL $tab . '<xsl:value-of select="@probability"/>';

if ($asap_display eq $checked) {
    print XSL $tab . '<xsl:value-of select="$asapratio"/>' ;
    print XSL $tab . '<xsl:value-of select="$asapstd"/>' ;
    print XSL $tab . '<xsl:value-of select="$asapnum"/>' ;

}

if ($xpress_display eq $checked) {
    print XSL $tab . '<xsl:value-of select="$xratio"/>' ;
    print XSL $tab . '<xsl:value-of select="$xstd"/>' ;
    print XSL $tab . '<xsl:value-of select="$xnum"/>' ;
}

print XSL $newline;

###########333
print XSL '</xsl:if>' if($SINGLE_HITS);

if($discards) {
    
    print XSL '</xsl:if>';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i <= $#inclusions; $i++) {
	print XSL '</xsl:if>';
    }
    foreach(@pinclusions) {
	if(/^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }

}
else { # conve
    if($show_groups eq '') {
	print XSL '</xsl:if>' if(! ($asap_xpress eq ''));
	print XSL '</xsl:if>' if($minprob > 0);
	print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
	print XSL '</xsl:if>' if(! ($filter_asap eq ''));
	print XSL '</xsl:if>' if($min_xpress > 0);
	print XSL '</xsl:if>' if($max_xpress > 0);
	print XSL '</xsl:if>' if($min_asap > 0);
	print XSL '</xsl:if>' if($max_asap > 0);
	print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
	foreach(@exclusions) {
	    print XSL '</xsl:if>';
	}
    }
    for(my $e = 0; $e <= $#pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '</xsl:if>';
	}
    }
    if(! ($show_groups eq '')) {
	for(my $k = 0; $k < $num_pincl; $k++) {
	    print XSL '</xsl:if>';

	}
    }
} # conv


print XSL '</xsl:template>';


################### PEPTIDE  ###################################
print XSL '<xsl:template match="protx:peptide">';
print XSL '<xsl:param name="pvalpngfile"/>';
print XSL '<xsl:param name="mult_prot"/>';
print XSL '<xsl:param name="peptide_string"/>';
print XSL '<xsl:param name="xratio"/>';
print XSL '<xsl:param name="xstd"/>';
print XSL '<xsl:param name="xnum"/>';
print XSL '<xsl:param name="min_pep_prob"/>';
# print XSL '<xsl:param name="source"/>';
print XSL '<xsl:variable name="mypep"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="mycharge" select="@charge"/>';
print XSL '<xsl:variable name="PepMass"><xsl:if test="@calc_neutral_pep_mass"><xsl:value-of select="@calc_neutral_pep_mass"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="StdPep"><xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if></xsl:variable>';

print XSL '<xsl:variable name="myinputfiles" select="$source_files_alt"/>';
print XSL '<xsl:variable name="myprots"><xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each></xsl:variable>';

print XSL '<xsl:variable name="nspbin" select="@n_sibling_peptides_bin"/>';
print XSL '<xsl:variable name="nspval" select="@n_sibling_peptides"/>';

if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">0</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="$xpress_quant &gt; \'0\' or $asap_quant &gt; \'0\'">1</xsl:if><xsl:if test="not($xpress_quant &gt; \'0\') and not($asap_quant &gt; \'0\')">-1</xsl:if></xsl:variable>';
}


print XSL '<xsl:if test="@nsp_adjusted_probability &gt;=\''. $min_pepprob . '\'">' if($min_pepprob > 0);
print XSL '<xsl:if test="@n_enzymatic_termini &gt;=\''. $minntt . '\'">' if($minntt > 0);
print XSL '<xsl:if test="not(@charge=\'1\')">' if(! ($exclude_1 eq ''));
print XSL '<xsl:if test="not(@charge=\'2\')">' if(! ($exclude_2 eq ''));
print XSL '<xsl:if test="not(@charge=\'3\')">' if(! ($exclude_3 eq ''));
print XSL '<xsl:if test="@is_nondegenerate_evidence=\'Y\'">' if(! ($exclude_degens eq ''));

print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';


print XSL '<xsl:if test="position()=\'1\'">' if($show_peps eq '');


    print XSL $tab_display{'protein'} . $tab;

    # here need prot prob
    print XSL '<xsl:value-of select="parent::node()/@probability"/>' . $newline;


print XSL '</xsl:if>' if($min_pepprob > 0);
print XSL '</xsl:if>' if($minntt > 0);
print XSL '</xsl:if>' if(! ($exclude_1 eq ''));
print XSL '</xsl:if>' if(! ($exclude_2 eq ''));
print XSL '</xsl:if>' if(! ($exclude_3 eq ''));
print XSL '</xsl:if>' if(! ($exclude_degens eq ''));

#print XSL '</xsl:if>';


print XSL '</xsl:template>';



if((! ($show_sens eq '') && $eligible)) {
    print XSL '<xsl:template match="protx:protein_summary_data_filter">';
    print XSL '<xsl:value-of select="@min_probability"/>' . $tab . '<font color="red"><xsl:value-of select="@sensitivity"/></font>' . $tab . '<font color="green"><xsl:value-of select="@false_positive_error_rate"/></font>' . $tab . '<font color="red"><xsl:value-of select="@predicted_num_correct"/></font>' . $tab . '<font color="green"><xsl:value-of select="@predicted_num_incorrect"/></font>' . $newline;

    print XSL '</xsl:template>';
}

print XSL '</xsl:stylesheet>', "\n";

print XSL "\n";

close(XSL);

return $numcols;
}



sub getRatioPrefix {
    (my $is_light) = @_;
    if(! ($is_light eq 'true')) {
	return 'heavy2light_';
    }
    return '';
}


sub strip {
    (my $pep) = @_;
    my $next;
    my $stripped = '';
    for(my $k = 0; $k < (length $pep); $k++) {
	$next = substr($pep, $k, 1);
	$stripped .= $next if($next =~ /[A-Z]/);
    }
    return $stripped;
}

sub printUsage {

    print << "END_USAGE";

protxml2html.pl  ($TPPVersionInfo)
Generate HTML or EXCEL file from a prot.xml file

Command-line usage:

protxml2html.pl -file full_path_to_protXML_file [format] [options]

Format MUST be one of the following:
HTML     : Generates pathname.html
EXCEL    : Generates pathname.xls

Options:
NOGAGGLE : Do not generate Gaggle auxiliary files (runs faster; for use with HTML only)

**Make sure that you use the absolute/FULL path for the input (prot.xml) file**

END_USAGE

}
