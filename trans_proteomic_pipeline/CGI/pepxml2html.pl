#!/usr/bin/perl
#############################################################################
# Program       : pepxml2html.pl                                           #
# Author        : Andrew Keller <akeller@systemsbiology.org>                #
#                 David Shteynberg <dshteynb@systemsbiology.org>            #
# Date          : 3.28.03                                                   #
#                                                                           #
# ProteinProphet                                                            #
#                                                                           #
# Program       : TransProteomicPipeline                                       #   
# Author        : Andrew Keller <akeller@systemsbiology.org>                # 
#                 David Shteynberg <dshteynb@systemsbiology.org>            # 
# Date          : 11.27.02                                                  #
#                                                                           #
#                                                                           #
# Copyright (C) 2003 Andrew Keller  2005 David Shteynberg                   #
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


use POSIX;

use File::Spec; # use perl libs instead of depending on /^\/ as fullpath indicator

# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath);
use lib realpath(dirname($0));
use tpplib_perl; # exported TPP lib function points

print "Content-type: text/html\n\n" if(@ARGV == 0 || ! ($ARGV[0] eq '-file'));


%box = &tpplib_perl::read_query_string;      # Read keys and values




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
my $ISB_VERSION = 1;
########################################

my $WINDOWS_CYGWIN = -f '/bin/cygpath'; #0;
my $USING_LOCALHOST = 0;

# forward declare variables (for scoping reasons)
my $CGI_HOME;		# Full path web server home directory
my $HELP_DIR;		# where all help png's are kept
my $xslt;		# filesystem path to stylesheet processor
#my $DTD_FILE;
my $WEBSRVR;		# enum { IIS, APACHE, WEBSITEPRO }
my $SERVER_ROOT;	# filesystem path to web files
my $TOP_PATH;
my $CGI_BIN;

#
# Linux installation
#
if ( $^O eq 'linux' ) {
    $TOP_PATH = '/tools/bin/TPP/tpp/';  #DDS: The last '/' is important!
    $CGI_BIN = 'cgi-bin/';
    $CGI_HOME = '/tpp/cgi-bin/';  
    $HELP_DIR = '/tpp/html/'; 
    $xslt = '/bin/nice -19 /usr/bin/xsltproc';  # disconnect dtd check (since only has web server reference name)
#  $DTD_FILE = '/usr/local/tpp/schema/ProteinProphet_v1.9.dtd';;
    $WEBSRVR = "APACHE";

#
# Cygwin installation
#
} elsif ( ($^O eq 'cygwin' )||($^O eq 'MSWin32' )) {

	$USING_LOCALHOST = 1;
    $CGI_HOME = '/tpp-bin/';
    $HELP_DIR = '/tpp-bin/';

    if(exists $ENV{'WEBSERVER_ROOT'}) {
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
} # end configuration

#
# gather TPP version info
#
$TPPVersionInfo = tpplib_perl::getTPPVersionInfo();
my $LC_SERVER_ROOT = lc $SERVER_ROOT; # lower case

my $DISPLAY_MODS = 1; # whether or not to display mods as superscripted masses

my $HTML = 0;
my $HTML_ORIENTATION = 1; # whether or not
my $SHTML = 1; # whether or not to use SSI to launch cgi instead of traditional written html file
my $ICAT = 0;
my $GLYC = 0;

my $SINGLE_HITS = 0;


# write out new xsl stylesheet
my $xmlfile;
my $xslfile;
my $pngfile = '';
my $htmlfile;
my $excelfile;
my $sort_index = -1;
my $alt_prots_index = -1;
my $start_string = 'start';
my $start_string_comment = '<!--' . $start_string . '-->';
my $USE_INDEX = 1; # whether to use explicit @index rather than num siblings
$USE_INDEX = 1 if(@ARGV >= 2 && $ARGV[1] eq 'index');

my $PAINT_SCORES = 1; # whether to color rank, delta, ntt pink when good

my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'cellpadding="2" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
my $RESULT_TABLE_SUF = '</table>';
my $inital_xsl = 0;
my $MAX_NUM_ENTRIES = 2000; # if more than that, will filter at min prob
my $MIN_PROT_PROB = 0.1;
# in some environments, form info is transmitted as $ARGV[0]


my $entry_delimiter = 8; # empty cell height for delimiting successive entries

my $initiate = 0;

my $ext = tpplib_perl::hasValidPepXMLFilenameExt($ARGV[1]) if (@ARGV > 1);

if(@ARGV > 1 && $ARGV[0] eq '-file' && defined($ext)) { # take file name from arg

	$xmlfile = $ARGV[1];
	if ($^O eq 'MSWin32' ) {
		$xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
	}
	$ext = substr $ext,4  if ($ext =~ m/\.pep\./); # preserve .pep from .pep.xml if any
	$xmlfile =~ /^(\S+)($ext)$/; # reevaluate $1
	$xslfile = $1 . '.xsl';
	$excelfile = $1 . '.xls';
	$pngfile = $1 . '.png';
	if($SHTML) {
	    $htmlfile = $1 . '.shtml';
	}
	else {
	    $htmlfile = $1 . '.htm';
	}

	$inital_xsl = @ARGV > 2 && $ARGV[2] > $MAX_NUM_ENTRIES;
	$initiate = 1;

	# check for icat
	for(my $k = 3; $k < @ARGV; $k++) {
	    $ICAT = 1 if($ARGV[$k] eq 'ICAT');
	    $GLYC = 1 if($ARGV[$k] eq 'GLYC');
	}
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
    if($SHTML) {
	$htmlfile = $1 . 'shtml';
    }
    else {
	$htmlfile = $1 . 'htm';
    }
} # if




$| = 1; # autoflush

my $restore_view = exists $box{'restore_view'} && $box{'restore_view'} eq 'yes';

$ICAT = 1 if(exists $box{'icat_mode'} && $box{'icat_mode'} eq 'yes');
$GLYC = 1 if(exists $box{'glyc_mode'} && $box{'glyc_mode'} eq 'yes');
my $pre_existing_xsl = 0;


if(0 && $restore_view && ! -e $xslfile) {
    print "Error: Cannot find stylesheet for most recent view of dataset.  Please recreate.\n\n";
    exit(1);
}


#foreach(keys %box) {
#    print "$_: $box{$_}\n";
#}

if(exists $box{'custom_settings'}) {

    #print "custom settings: ", $box{'custom_settings'}, "\n";

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

	#foreach(keys %box) {
	#    print "$_: $box{$_}\n";
	#}


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
		($vol,$dirs,$file) = File::Spec->splitpath($xmlfile);
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

	if($USING_LOCALHOST) {
	    $local_datafile = $box{'outfile'} . '.xml'; # BSP until shtml fixed

	    # this should be just like after analysis: use local datafile link, but windows name for shtml file

	    if((length $SERVER_ROOT) <= (length $local_datafile) && 
	       index((lc $local_datafile), ($LC_SERVER_ROOT)) == 0) {
		$local_datafile = '/' . substr($local_datafile, (length $SERVER_ROOT));
	    }
	    else {
		die "problem (ph1): $local_datafile is not mounted under webserver root: $SERVER_ROOT\n";
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
   		my $TPPhostname = tpplib_perl::get_tpp_hostname();
	    print ' direct your browser to <a target="Win1" href="' .  $local_datafile . '">' . "http://$TPPhostname$local_datafile" . '</a>' . "\n\n"; 
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
    my $debugger = 0;
    if($initiate) {
	initialize($xslt, $xmlfile, $xslfile, \%box, $htmlfile, 0);
	print "\n results written to file ";

	if($USING_LOCALHOST) {
	    my $local_ref = $HTML_ORIENTATION ? $htmlfile : $xmlfile;

	    $windows_ref = $xmlfile;
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
		#print $local_ref;
		my $TPPhostname = tpplib_perl::get_tpp_hostname();
		print "\n direct your browser to http://$TPPhostname$local_ref" if($HTML_ORIENTATION);
	    }
	    else {
		die "problem (ph2): $local_ref is not mounted under webserver root: $SERVER_ROOT\n";
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
    else {
	if($restore_view) {

	    if(-e $xslfile) {
		$pre_existing_xsl = 1;
		%box = %{readXSLFile($xslfile)};
#		foreach(keys %box) {
#		    print "$_: $box{$_}\n";
#		}

	    }
	    else {
		%box = %{initialize($xslt, $xmlfile, $xslfile, \%box, $htmlfile, 1)}; # write the xsl file
		#foreach(keys %box) {
		#    print "$_: $box{$_}\n";
		#}
	    }
	}
	else {
	    %box = %{getCustomizedSettings($xmlfile)} if(exists $box{'restore'} && $box{'restore'} eq 'yes'); # no longer need to keep track of 'restore'
	}

	if(exists $box{'excel'} && $box{'excel'} eq 'yes') {
	    writeTabDelimData($excelfile, $xslt, $xmlfile);
	    print "\n"; # write something to prevent cgi timeout
	}
	writeXSLFile($xslfile, \%box, 0) if(! $restore_view);
	printHTML($xslt, $xmlfile, $xslfile);
    }
}

sub writeXMLFile {
(my $file, my $boxptr, my $xslt, my $xml) = @_;


#foreach(keys %{$boxptr}) { print "$_ = ${$boxptr}{$_}\n"; }

my $tempxslfile = $file . '.tmp.xsl';


open(OUT, ">$tempxslfile");
print OUT '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">', "\n";

print OUT '<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>', "\n";


print OUT '<xsl:template match="node() | @*">', "\n";
print OUT '<xsl:copy>', "\n";
print OUT '<xsl:apply-templates select="*[not(self::pepx:spectrum_query)] | @*"/>';
print OUT '<xsl:text>' . "\n" . '</xsl:text>';

my $minprob = exists ${$boxptr}{'min_prob'} && ! (${$boxptr}{'min_prob'} eq '') ? ${$boxptr}{'min_prob'} : 0;


my $minntt = exists ${$boxptr}{'min_ntt'} && ! (${$boxptr}{'min_ntt'} eq '') ? ${$boxptr}{'min_ntt'} : 0;

my $maxnmc = exists ${$boxptr}{'max_nmc'} && ! (${$boxptr}{'max_nmc'} eq '') ? ${$boxptr}{'max_nmc'} : -1;
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';
my $exclude_1 = exists ${$boxptr}{'ex1'} && ${$boxptr}{'ex1'} eq 'yes' ? ${$boxptr}{'ex1'} : '';
my $exclude_2 = exists ${$boxptr}{'ex2'} && ${$boxptr}{'ex2'} eq 'yes' ? ${$boxptr}{'ex2'} : '';
my $exclude_3 = exists ${$boxptr}{'ex3'} && ${$boxptr}{'ex3'} eq 'yes' ? ${$boxptr}{'ex3'} : '';
my $exclude_4 = exists ${$boxptr}{'ex4'} && ${$boxptr}{'ex4'} eq 'yes' ? ${$boxptr}{'ex4'} : '';

my @inclusions = exists ${$boxptr}{'inclusions'} ? split(' ', ${$boxptr}{'inclusions'}) : ();
my @exclusions = exists ${$boxptr}{'exclusions'} ? split(' ', ${$boxptr}{'exclusions'}) : ();
my @pinclusions = exists ${$boxptr}{'pinclusions'} ? split(' ', ${$boxptr}{'pinclusions'}) : ();
my @pexclusions = exists ${$boxptr}{'pexclusions'} ? split(' ', ${$boxptr}{'pexclusions'}) : ();

#print "inclusions: ", join(',', @inclusions), "\n";

# other variables needed?
my $min_asap = exists ${$boxptr}{'min_asap'} && ! ${$boxptr}{'min_asap'} eq '' ? ${$boxptr}{'min_asap'} : 0;
my $max_asap = exists ${$boxptr}{'max_asap'} && ! ${$boxptr}{'max_asap'} eq '' ? ${$boxptr}{'max_asap'} : 0;
my $min_xpress = exists ${$boxptr}{'min_xpress'} && ! ${$boxptr}{'min_xpress'} eq '' ? ${$boxptr}{'min_xpress'} : 0;
my $max_xpress = exists ${$boxptr}{'max_xpress'} && ! ${$boxptr}{'max_xpress'} eq '' ? ${$boxptr}{'max_xpress'} : 0;
my $show_groups = 0; #exists ${$boxptr}{'show_groups'} && ! ${$boxptr}{'show_groups'} eq '' ? ${$boxptr}{'show_groups'} : '';
my $min_pepprob = exists ${$boxptr}{'min_pepprob'} && ! ${$boxptr}{'min_pepprob'} eq '' ? ${$boxptr}{'min_pepprob'} : 0;
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ! ${$boxptr}{'filter_asap'} eq '' ? ${$boxptr}{'filter_asap'} : '';
my $filter_xpress = exists ${$boxptr}{'filter_xpress'} ? ${$boxptr}{'filter_xpress'} : '';
my $show_adjusted_asap = (! exists ${$boxptr}{'show_adjusted_asap'} && ! exists ${$boxptr}{'adj_asap'}) || (${$boxptr}{'show_adjusted_asap'} eq 'yes') ?  ${$boxptr}{'show_adjusted_asap'} : '';
my $max_pvalue_display = exists ${$boxptr}{'max_pvalue'} && ! (${$boxptr}{'max_pvalue'} eq '') ? ${$boxptr}{'max_pvalue'} : 1.0;
my $asap_xpress = exists ${$boxptr}{'asap_xpress'} ? ${$boxptr}{'asap_xpress'} : '';
my $quant_light2heavy = ! exists ${$boxptr}{'quant_light2heavy'} || ${$boxptr}{'quant_light2heavy'} eq 'true' ? 'true' : 'false';

$exclude_1 = exists ${$boxptr}{'ex1'} ? ${$boxptr}{'ex1'} : '';
$exclude_2 = exists ${$boxptr}{'ex2'} ? ${$boxptr}{'ex2'} : '';
$exclude_3 = exists ${$boxptr}{'ex3'} ? ${$boxptr}{'ex3'} : '';
$exclude_4 = exists ${$boxptr}{'ex4'} ? ${$boxptr}{'ex4'} : '';

my $min_SEQ_xcorr = exists ${$boxptr}{'min_SEQ_xcorr'} ? ${$boxptr}{'min_SEQ_xcorr'} : '';
my $min_SEQ_delta = exists ${$boxptr}{'min_SEQ_delta'} ? ${$boxptr}{'min_SEQ_delta'} : '';
my $max_SEQ_sprank = exists ${$boxptr}{'max_SEQ_sprank'} ? ${$boxptr}{'max_SEQ_sprank'} : '';
my $exclude_SEQ = exists ${$boxptr}{'exclSEQ'} ? ${$boxptr}{'exclSEQ'} : '';
my $min_MAS_ionscore = exists ${$boxptr}{'min_MAS_ionscore'} ? ${$boxptr}{'min_MAS_ionscore'} : '';
my $min_MAS_idscore = exists ${$boxptr}{'min_MAS_idscore'} ? ${$boxptr}{'min_MAS_idscore'} : '';
my $exclude_MAS = exists ${$boxptr}{'exclMAS'} ? ${$boxptr}{'exclMAS'} : '';

my $min_TAN_hyperscore = exists ${$boxptr}{'min_TAN_hyperscore'} ? ${$boxptr}{'min_TAN_hyperscore'} : '';
my $min_TAN_nextscore = exists ${$boxptr}{'min_TAN_nextscore'} ? ${$boxptr}{'min_TAN_nextscore'} : '';
my $min_TAN_expectscore = exists ${$boxptr}{'min_TAN_expectscore'} ? ${$boxptr}{'min_TAN_expectscore'} : '';
my $exclude_TAN = exists ${$boxptr}{'exclTAN'} ? ${$boxptr}{'exclTAN'} : '';

my $min_PHEN_zscore = exists ${$boxptr}{'min_PHEN_zscore'} ? ${$boxptr}{'min_PHEN_zscore'} : '';
my $min_PHEN_origScore = exists ${$boxptr}{'min_PHEN_origScore'} ? ${$boxptr}{'min_PHEN_origScore'} : '';
my $exclude_PHEN = exists ${$boxptr}{'exclPHEN'} ? ${$boxptr}{'exclPHEN'} : '';

my $min_COM_dotproduct = exists ${$boxptr}{'min_COM_dotproduct'} ? ${$boxptr}{'min_COM_dotproduct'} : '';
my $min_COM_delta = exists ${$boxptr}{'min_COM_delta'} ? ${$boxptr}{'min_COM_delta'} : '';
my $min_COM_zscore = exists ${$boxptr}{'min_COM_zscore'} ? ${$boxptr}{'min_COM_zscore'} : '';
my $exclude_COM = exists ${$boxptr}{'exclCOM'} ? ${$boxptr}{'exclCOM'} : '';



# add filter_xpress and asap_xpress capabilities here....
#print "here and ready!\n";

# apply-templates select="protein_group"
# SHOW GROUPS

    print OUT '<xsl:apply-templates select="pepx:spectrum_query[pepx:search_result/pepx:search_hit/@hit_rank=\'1\''; #"/>';
    print OUT ' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability  &gt;=\'' . $minprob . '\'' if($minprob > 0);

    print OUT ' and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= \'0\')' if(! ($filter_xpress eq ''));
    print OUT ' and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'] and search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean &gt;= \'0\')' if(! ($filter_asap eq ''));

    if($quant_light2heavy eq 'true') {
	print OUT ' and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print OUT ' and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/xpepx:analysis_result[@analysis=\'xpress\'] and search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
    }
    else { # reverse
	print OUT ' and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;= \'' . 1.0 / $min_xpress . '\')' if($min_xpress > 0);
	print OUT ' and (search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= \'' . 1.0 / $max_xpress . '\')' if($max_xpress > 0);
    }


    print OUT ' and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
    print OUT ' and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
    print OUT ' and (not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']) or not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']) or (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/xpressratio_result/@decimal_ratio &lt;= pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean + pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean - pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error))' if(! ($asap_xpress eq ''));


    print OUT ' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tol_term &gt;=\'' . $minntt . '\'' if(! ($minntt eq '') && $minntt > 0);

    print OUT ' and not(@assumed_charge=\'1\')' if(! ($exclude_1 eq ''));
    print OUT ' and not(@assumed_charge=\'2\')' if(! ($exclude_2 eq ''));
    print OUT ' and not(@assumed_charge=\'3\')' if(! ($exclude_3 eq ''));
    print OUT ' and not(@assumed_charge &gt;\'3\')' if(! ($exclude_4 eq ''));

    foreach(@exclusions) {
	print OUT ' and not(@index=\'' . $_ . '\')';
    }
    print OUT ']"/>';

    if(@inclusions + (scalar keys %parent_incls) > 0) {
	my $first = 1;
	foreach(@inclusions) {
	    if($first) {
		print OUT '<xsl:apply-templates select="pepx:spectrum_query[@index=\'' . $_ . '\'';
		$first = 0;
	    }
	    else {
		print OUT ' or @index=\'' . $_ . '\'';
	    }
	}
	print OUT ']"/>';
    } # if have some inclusions



# apply-templates select="protein"
# apply-templates select="peptide"


print OUT '</xsl:copy>';
print OUT '</xsl:template>', "\n";
print OUT '</xsl:stylesheet>', "\n";
close(OUT);

my $outfile = $file . '.xml'; #'tempfile.xml';



# now compute filter
my $filter = '';
$filter .= 'min_prob=\'' . $minprob . '\' ' if($minprob > 0);
$filter .= 'exclude_illegal_XPRESSRatios=\'Y\' ' if($filter_xpress);
$filter .= 'exclude_illegal_ASAPRatios=\'Y\' ' if($filter_asap);
my $asap_prefix = $show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'} ? '' : 'adj_';
$filter .= 'asap_xpress_consistency=\'Y\' ' if(! ($asap_xpress eq ''));
$filter .= 'min_' . getRatioPrefix($quant_light2heavy) . 'xpress=\'' . $min_xpress . '\' ' if($min_xpress > 0);
$filter .= 'max_' . getRatioPrefix($quant_light2heavy) . 'xpress=\'' . $max_xpress . '\' ' if($max_xpress > 0);
$filter .= $asap_prefix . 'min_' . getRatioPrefix($quant_light2heavy) . 'asap=\'' . $min_asap . '\' ' if($min_asap > 0);
$filter .= $asap_prefix . 'max_' . getRatioPrefix($quant_light2heavy) . 'asap=\'' . $max_asap . '\' ' if($max_asap > 0);
$filter .= 'minntt=\'' . $minntt . '\' ' if($minntt > 0);
$filter .= 'exclude_1+_peptides=\'Y\' ' if($exclude_1);
$filter .= 'exclude_2+_peptides=\'Y\' ' if($exclude_2);
$filter .= 'exclude_3+_peptides=\'Y\' ' if($exclude_3);
$filter .= 'exclude_4++_peptides=\'Y\' ' if($exclude_4);
$filter .= 'group_entry_inclusions=\'' . join(',', @inclusions) . '\' ' if(@inclusions > 0);
$filter .= 'group_entry_exclusions=\'' . join(',', @exclusions) . '\' ' if(@exclusions > 0);
$filter .= 'protein_entry_inclusions=\'' . join(',', @pinclusions) . '\' ' if(@pinclusions > 0);
$filter .= 'protein_entry_exclusions=\'' . join(',', @pexclusions) . '\' ' if(@pexclusions > 0);
$filter .= 'min_SQ_xorr=\'' . $min_SEQ_xcorr . '\' ' if($min_SEQ_xcorr > 0);
$filter .= 'min_SQ_delta=\'' . $min_SEQ_delta . '\' ' if($min_SEQ_delta > 0);
$filter .= 'max_SQ_sprank=\'' . $max_SEQ_sprank . '\' ' if($max_SEQ_sprank > 0);
$filter .= 'filter_SEQUEST=\'Y\' ' if($exclude_SEQ);
$filter .= 'min_MAS_ionscore=\'' . $min_MAS_ionscore . '\' ' if($min_MAS_ionscore > 0);
$filter .= 'min_MAS_idscore=\'' . $min_MAS_idscore . '\' ' if($min_MAS_idscore > 0);
$filter .= 'filter_MASCOT=\'Y\' ' if($exclude_MAS);
$filter .= 'min_TAN_hyperscore=\'' . $min_TAN_hyperscore . '\' ' if($min_TAN_hyperscore > 0);
$filter .= 'min_TAN_nextscore=\'' . $min_TAN_nextscore . '\' ' if($min_TAN_nextscore > 0);
$filter .= 'min_TAN_expectscore=\'' . $min_TAN_expectscore . '\' ' if($min_TAN_expectscore > 0);
$filter .= 'filter_TANDEM=\'Y\' ' if($exclude_TAN);
$filter .= 'min_PHEN_zscore=\'' . $min_PHEN_zscore . '\' ' if($min_PHEN_zscore > 0);
$filter .= 'min_PHEN_origScore=\'' . $min_PHEN_origScore . '\' ' if($min_PHEN_origScore > 0);
$filter .= 'filter_PHENYX=\'Y\' ' if($exclude_PHEN);
$filter .= 'min_COM_dotproduct=\'' . $min_COM_dotproduct . '\' ' if($min_COM_dotproduct > 0);
$filter .= 'min_COM_delta=\'' . $min_COM_delta . '\' ' if($min_COM_delta > 0);
$filter .= 'min_COM_zscore=\'' . $min_COM_zscore . '\' ' if($min_COM_zscore > 0);
$filter .= 'filter_COMET=\'Y\' ' if($exclude_COM);

# if $xml is gzipped, returns tmpfile name, else returns $xml
my $tmpxml = tpplib_perl::uncompress_to_tmpfile($xml); 

if($xslt =~ /xsltproc/) {
    open XALAN, "$xslt $tempxslfile $tmpxml |" or print "pepxml2html.pl: cannot open xslt $xslt\n";;
}
else {
    open XALAN, "$xslt $tmpxml $tempxslfile |" or print "pepxml2html.pl: cannot open xslt $xslt\n";;
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
if($USING_LOCALHOST) {
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
	die "problem (ph3): $local_parent is not mounted under webserver root: $SERVER_ROOT\n";
    }
} # if iis & cygwin


# check for stylesheet reference
my $xsltproc = $xslt =~ /xsltproc/;
my $doctype = 0;
my $generation = -1;
my $write_der = 0;
my $next;
my $last = 'start';

# this is a hack to try to condense xsltproc output from having extraneous independent end tags, for the reason
# of brevity as well as perhaps some Parsers depending on tags being both start and end simultaneously

while(<XALAN>) {
    chomp();
    if(/\S/) {
	my @tags = ();
	if(! ($last eq 'start')) {
	    push(@tags, $last);;
	}
	my @parsed = split('>');
	for(my $k = 0; $k < @parsed - 1; $k++) {
	    push(@tags, $parsed[$k]);
	}
	$last = $parsed[$#parsed];
	    
	for(my $t = 0; $t < @tags; $t++) {
	    if($t < @tags - 1 && isTagPair($tags[$t], $tags[$t+1])) {
		$next = $tags[$t++] . '/>';
	    }
	    else {
		$next = $tags[$t] . '>';
	    }
	    if($start && $next =~ /^(\<\?xml\-stylesheet type\=\"text\/xsl\" href\=\")\S+(\"\?\>.*)$/) {
		my $local_xslfile = $file . '.xsl';

		if($USING_LOCALHOST) {
		    if((length $SERVER_ROOT) <= (length $local_xslfile) && 
		       index((lc $local_xslfile), ($LC_SERVER_ROOT)) == 0) {
			$local_xslfile = '/' . substr($local_xslfile, (length $SERVER_ROOT));
		    }
		    else {
			die "problem (ph4): $local_xslfile is not mounted under webserver root: $SERVER_ROOT\n";
		    }
		} # if iis & cygwin

		print XML $1 . $local_xslfile . $2 . "\n";
		$start = 0;
	    }
	    elsif($start && ! $doctype && $next =~ /^\<msms\_pipeline\_analysis/) {
		my $local_xslfile = $file . '.xsl';

		if($USING_LOCALHOST) {
		    if((length $SERVER_ROOT) <= (length $local_xslfile) && 
		       index((lc $local_xslfile), ($LC_SERVER_ROOT)) == 0) {
			$local_xslfile = '/' . substr($local_xslfile, (length $SERVER_ROOT));
		    }
		    else {
			die "problem (ph5): $local_xslfile is not mounted under webserver root: $SERVER_ROOT\n";
		    }
		} # if iis & cygwin
		print XML '<?xml-stylesheet type="text/xsl" href="' . $local_xslfile . '"?>' . "\n";
		if($next =~ /^(.*summary\_xml\=\")\S+(\".*)$/) {
		    $next = $1 . $outfile . $2;
		}
		print XML "$next\n";
		$start = 0;
	    }
	    # make replacements to num predicted correct prots and parent data file, as well as sens / err data
	    elsif(! ($filter eq '') && $next =~ /^(.*dataset\_derivation generation\_no\=\")(\d+)\".*$/) {
		$generation = $2;

		print XML $1 . ($generation+1) . '">', "\n";
		if($generation == 0) { # first filter
		    print XML '<data_filter number="' . ($generation+1) . '" parent_file="' . $local_parent;
		    print XML '" windows_parent="' . $windows_parent if($WINDOWS_CYGWIN);
		    print XML '" description="' . $filter . '"/>', "\n";
		    print XML '</dataset_derivation>' . "\n";
		}
	    }
	    elsif(! ($filter eq '') && $next =~ /^\<data\_filter number\=\"(\d+)\".*?\/\>/ && $1 == $generation) {

		print XML "$next\n";
		print XML '<data_filter number="' . ($generation+1) . '" parent_file="' . $local_parent;
		print XML '" windows_parent="' . $windows_parent if($WINDOWS_CYGWIN);
		print XML '" description="' . $filter . '"/>', "\n";
		print XML '</dataset_derivation>' . "\n" if($generation == 0);

	    }
	    elsif(! ($filter eq '') && $next =~ /^\<data\_filter number\=\"(\d+)\"/ && $1 == $generation) {
		$write_der = 1;
		print XML "$next\n";
	    }
	    elsif(! ($filter eq '') && $write_der && $next =~ /^\<\/data\_filter/ ) {
		print XML "$next\n";
		print XML '<data_filter number="' . ($generation+1) . '" parent_file="' . $local_parent;
		print XML '" windows_parent="' . $windows_parent if($WINDOWS_CYGWIN);
		print XML '" description="' . $filter . '"/>', "\n";
		print XML '</dataset_derivation>' . "\n" if($generation == 0);
		$write_der = 0;
	    }
	    elsif($USE_INDEX && $next =~ /^(.*\<spectrum_query.*?index\=\")\d+(\".*)$/) {
		print XML $1 . $counter++ . $2;
	    }
	    elsif($next =~ /^\<msms\_pipeline\_analysis/) {
		if($next =~ /^(.*summary\_xml\=\")\S+(\".*)$/) {
		    $next = $1 . $outfile . $2;
		}
		print XML "$next\n";
	    }
	    else {
		if($start && $next =~ /DOCTYPE/) {
		    $doctype = 1;
		}
		print XML "$next\n";
	    }
	} # next tag
    } # if nonzero
}

print XML "$last>\n";
close(XML);
close(XALAN);
unlink($tmpxml) if ($tmpxml ne $xml); # did we decompress pepxml.gz?

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


sub isTagPair {
(my $first, my $second) = @_;
if($first =~ /^\<(\S+)\s/) {
    return $second =~ /^\<\/$1/;
}
return 0;
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

if(1) {
    unlink($tempxslfile) if(-e $tempxslfile);
    unlink($tempfile) if(-e $tempfile);
    writeXSLFile($tempxslfile, \%box, 1);

    if($xslt =~ /xsltproc/) {
	system("$engine $tempxslfile $xml > $tempfile");
    }
    else {
	system("$engine $xml $tempxslfile > $tempfile");
    }
    open(OUT, ">$outfile") or die "pepxml2html.pl: cannot open outfile $outfile $!\n";
    open(IN, "$tempfile") or die "pepxml2html.pl: cannot open tempfile $tempfile $!\n";
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
			for(my $p = 0; $p < @parsed; $p++) {
			    if($prot_ind == -1 && $parsed[$p] eq 'spectrum') {
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
			for(my $p = 0; $p < @parsed; $p++) {
			    for(my $s = 0; $s < @select_aas; $s++) {
#				print "$select_aas[$s] vs $parsed[$pep_ind]...\n";
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
#    unlink($tempfile) if(-e $tempfile);
 #   unlink($tempxslfile) if(-e $tempxslfile);
}
else {
    open(OUT, ">$outfile") or die "pepxml2html.pl: cannot open outfile $outfile $!\n";
    print OUT "1\t2\t3\n";
    close(OUT);
}
chmod 0666, $outfile;

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
    if(/checkbox\"\s+name\=\"glyc\"\s+value\=\"yes\" CHECKED=\"yes\"/) {
	$output{'glyc'} = 'yes';
    }
    elsif(/CHECKBOX\"\s+NAME\=\"glyc\"\s+VALUE\=\"YES\" CHECKED=\"yes\"/) {
	$output{'glyc'} = 'yes';
    }
    if(/\<input type\=\"radio\" name=\"sort\" value=\"peptide\" CHECKED=\"yes\"\/\>/) {
	$output{'sort'} = 'peptide';
    }
    elsif(/\<input type\=\"radio\" name=\"sort\" value=\"protein\" CHECKED=\"yes\"\/\>/) {
	$output{'sort'} = 'protein';
    }
}
close(XSL);

#foreach(keys %output) {
#    print "$_: $output{$_}\n";
#}


return \%output;
}


sub clearSpecialSymbols {
(my $string) = @_;
$string =~ s/\&gt\;/\>/g;
$string =~ s/\&lt\;/\</g;
$string =~ s/\&quot\;/\"/g;
$string =~ s/\&apos\;/\'/g;
return $string;
}


sub colorAAs {
(my $peptide, my $aas, my $glyc) = @_;
$peptide =~ s/(N[\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\]]*[A-O,Q-Z][\#,\@,\*,\[,0-9,\],\.]*[S,T])/\<font color\=\"\#FF00FF\"\>$1<\/font\>/g if($glyc);
my $next;
my $next_alt;
for(my $k = 0; $k < length $aas; $k++) {
    $next = substr($aas, $k, 1);
    $next_alt = $next =~ /[A-Z,a-z,\#]/ ? $next : '\\' . $next;
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

sub colorAAs2 {
(my $peptide, my $aasptr, my $glyc) = @_;
$peptide =~ s/(N[\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\]]*[A-O,Q-Z][\#,\@,\*,\[,0-9,\],\.]*[S,T])/\<font color\=\"\#FF00FF\"\>$1<\/font\>/g if($glyc);
my $next;
my $next_alt;
for(my $k = 0; $k < @{$aasptr}; $k++) {
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
	for(my $s = 0; $s < @settings; $s++) {
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

# this is pretty much broken at the moment, disable it: BSP
	    return '';  


(my $xmlfile) = @_;
my $default_settings = 'pepsettings.txt';
return $SERVER_ROOT . $default_settings if($USING_LOCALHOST);

# have default for non-ISB users
#if($DTD_FILE !~ /akeller/ && $DTD_FILE =~ /^(\S+)ProteinProphet\_v\d\.\d\.dtd/) {
#    return $1 . $default_settings;
#}

my $prefix = '/data3/search/akeller/PEPXML/'; # settings file prefix
my $suffix = '_' . $default_settings; # settings file suffix

# extract user name from xml file name
if($xmlfile =~ /^\/data[2,3]?\/search\/([^\/]+)\//) {
    if($1 eq 'guest') {
	if($xmlfile =~ /^\/data[2,3]?\/search\/guest\/([^\/]+)\//) {
	    return $prefix . 'g_' . $1 . $suffix;
	}
	else {
	    return '';
	}
    }
    elsif($1 eq 'samd') {
	if($xmlfile =~ /^\/data[2,3]?\/search\/samd\/([^\/]+)\//) {
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
	    print "error: pepxml2html.pl cannot open settings file $settingsfile $!\n";
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
    writeXSLFile($xslfile, $boxptr, 0);
    return if($restore);
    if(1 || $xmlfile =~ /^(\S+\.)xml(\.gz)?$/) {
	open(OUT, ">$htmlfile") or die "pepxml2html.pl: cannot open htmlfile $htmlfile $!\n";
	print OUT "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 3.0//EN\">\n";
        if ( $WEBSRVR eq "IIS" ) {
	    print OUT "<!--#exec cgi=\"" . $CGI_HOME . "PepXMLViewer.cgi?xmlFileName=$xmlfile";
        }else {
	    print OUT "<!--#include virtual=\"" . $CGI_HOME . "PepXMLViewer.cgi?xmlFileName=$xmlfile";
        }
	print OUT "\" -->\n";
	close(OUT);
	# permissions
    }
}
}

sub initialize_htm {
(my $xslt, my $xmlfile, my $xslfile, my $boxptr, my $htmlfile) = @_;
if($HTML_ORIENTATION) {
    $HTML = 1;
    writeXSLFile($xslfile, $boxptr, 0);

    if($xslt =~ /xsltproc/) {
	open HTML, "$xslt $xslfile $xmlfile |";
    }
    else {
	open HTML, "$xslt $xmlfile $xslfile |";
    }
    open(OUT, ">$htmlfile") or die "pepxml2html.pl: cannot open htmlfile $htmlfile $!\n";
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
    writeXSLFile($xslfile, $boxptr, 0);
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
(my $xslt, my $xmlfile, my $xslfile) = @_;
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

my $mark_aa = exists $box{'mark_aa'} ? $box{'mark_aa'} : '';
$mark_aa .= 'C' if($ICAT && ! exists $box{'icat_mode'});
$box{'mark_aa'} = $mark_aa;
# allow for modified aminoacids
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

my $color = $DISPLAY_MODS || (exists $box{'mark_aa'} && ! ($box{'mark_aa'} eq '')) || (exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) || $glyc; 

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
my $sort_pep = exists $box{'sort'} && $box{'sort'} eq 'peptide' ? 'checked' : '';
my $sort_prot = exists $box{'sort'} && $box{'sort'} eq 'protein' ? 'checked' : '';

my $space = ' ';
my $sort_index = 0;
my $DEBUG = 0;
open(OUT, ">temp.html") if($DEBUG);

my $counter = 0;
my $table_size = 200; #500; # write table subsets so can be displayed by browser as html is passed from xslt
my $table_specs = $RESULT_TABLE_SUF . $RESULT_TABLE_PRE . $RESULT_TABLE;

my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';

my $new_entry;
if($discards) {
    $new_entry_gr = '"checkbox" name="incl';
    $new_entry = '"checkbox" name="pincl';
}
else {
    $new_entry_gr = '"checkbox" name="excl';
    $new_entry = '"checkbox" name="pexcl';
}
my $reject = 0;
my $reset;
my $reset_group;
my $xsltproc = $xslt =~ /xsltproc/; # special way to parse data
my $line_separator = '</tr>';
my $nextline = '';
my $complete_line = 0;
my %unique_peps = ();
my %unique_prots = ();
my %unique_stripped_peps = ();
# MUST USE <TR> for score tables!!!!!
my $tot_glyc_peps = 0;

my $next_prot = '';
my $last_prot = '';
my $next_pep = '';
my $last_pep = '';
while(<HTML>) { 
    chomp(); 
    

    # hack to recover +/- sign after xsltproc transorm (except for IPI link)
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

	if($start && $printout && $color && $nextline =~ /^(.*ncbi\.nlm\.nih\.gov\/blast\/Blast\.cgi.*?\>)([A-Z,\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\],n,c]+)(.*?)$/) {

	    my $ok = 1;
	    my $first = $1;
	    my $second = $2;
	    my $third = $3;

	    $tot_glyc_peps++ if($glyc && $second =~ /N[\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\]]*[A-O,Q-Z][\#,\@,\*,\[,0-9,\],\.]*[S,T]/);

	    if(! ($sort_prot eq '') && $third =~ /comet\-fastadb\.cgi\?.*?\>([^\<]+)\</) {
		$next_prot = $1;
		$unique_prots{$next_prot}++;
	    }

	    if(! ($sort_pep eq '')) {
		$unique_peps{$second}++;
		$unique_stripped_peps{strip($second)}++;
		$next_pep = $second;
	    }
	    if(@select_aas > 0) {
		for(my $s = 0; $s < @select_aas; $s++) {
		    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
		    $ok = 0 if(index($second, $alt) < 0);
		}
	    }
	    if(! ($sort_prot eq '')) {
		print "<tr><td>&nbsp;</td></tr>" if(! ($last_prot eq '') && ! ($last_prot eq $next_prot));
		$last_prot = $next_prot;
	    }
	    if(! ($sort_pep eq '')) {
		print "<tr><td>&nbsp;</td></tr>" if(! ($last_pep eq '') && ! ($last_pep eq $next_pep));
		$last_pep = $next_pep;
	    }

	    print $first, colorAAs2($second, \@color_aas, $glyc), $third if($ok);



	    $reject = ! $ok;
	}
	elsif($text1 && $printout) {
	    print $nextline;
	}
	elsif(! $start || $printout) {
	    print $nextline;
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


# last entry (only used if not xsltproc)
if($start && $printout && $color && $nextline =~ /^(.*ncbi\.nlm\.nih\.gov\/blast\/Blast\.cgi.*?\>)([A-Z,\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\],n,c]+)(.*?)$/) {


    my $ok = 1;
    my $first = $1;
    my $second = $2;
    my $third = $3;

    $tot_glyc_peps++ if($glyc && $second =~ /N[\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\]]*[A-O,Q-Z][\#,\@,\*,\[,0-9,\],\.]*[S,T]/);

    if(! ($sort_prot eq '') && $third =~ /comet\-fastadb\.cgi\?.*?\>([^\<]+)\</) {
	$next_prot = $1;
	$unique_prots{$next_prot}++;
    }
    if(! ($sort_pep eq '')) {
	$unique_peps{$second}++;
	$unique_stripped_peps{strip($second)}++;
	$next_pep = $second;
    }

    if(@select_aas > 0) {
	for(my $s = 0; $s < @select_aas; $s++) {
	    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
	    $ok = 0 if(index($second, $alt) < 0);
	}
    }
    if(! ($sort_prot eq '')) {
	print "<tr><td>&nbsp;</td></tr>" if(! ($last_prot eq '') && ! ($last_prot eq $next_prot));
	$last_prot = $next_prot;
    }
    if(! ($sort_pep eq '')) {
	print "<tr><td>&nbsp;</td></tr>" if(! ($last_pep eq '') && ! ($last_pep eq $next_pep));
	$last_pep = $next_pep;
    }

    print $first, colorAAs2($second, \@color_aas, $glyc), $third if($ok);
    $reject = ! $ok;
}
elsif($text1 && $printout) {
    print $nextline;
}
elsif(! $start || $printout) {
    print $nextline;
}



close(HTML); 

if(! ($sort_pep eq '')) {
    if($glyc) {
	printf "<HR/><center><font color=\"red\"><b>%d peptides with NXS/T motif, %d unique peptides, %d unique stripped peptides</b></font></center>", $tot_glyc_peps, scalar keys %unique_peps, scalar keys %unique_stripped_peps;
    }
    else {
	printf "<HR/><center><font color=\"red\"><b>%d unique peptides, %d unique stripped peptides</b></font></center>", scalar keys %unique_peps, scalar keys %unique_stripped_peps;
    }
}
elsif(! ($sort_prot eq '')) {
    my $num_singles = 0;
    foreach(keys %unique_prots) {
	$num_singles++ if($unique_prots{$_} == 1);
    }
    printf "<HR/><center><font color=\"red\"><b>%d unique proteins (%d single hits)</b></font></center>", scalar keys %unique_prots, $num_singles;

}




close(OUT) if($DEBUG);
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

sub writeXSLFile {
(my $xfile, my $boxptr, my $tab_delim) = @_;

if(! open(XSL, ">$xfile")) {
    print "pepxml2html.pl: cannot open xsl file $xfile: $!\n";
    exit(1);
}
print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">', "\n";
my $tab = '<xsl:value-of select="$tab"/>';
my $newline = '<br/><xsl:value-of select="$newline"/>';
$newline = '<xsl:value-of select="$newline"/>' if($tab_delim >= 1);
my $nonbreakline = '<xsl:value-of select="$newline"/>';
my $newlinespace = '<p/>';
my $doubleline = $newline . $newline;
my $space = '&#160';
my $checked = 'CHECKED="yes"';

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

my $libra_abs = ! exists ${$boxptr}{'libra_display'} || ${$boxptr}{'libra_display'} eq 'absolute' ? $checked : '';
my $libra_norm = ! $libra_abs ? $checked : '';

my $hide_prots = ! exists ${$boxptr}{'show_prots'} || ${$boxptr}{'show_prots'} eq 'hide' ? $checked : '';
my $show_prots = $hide_prots eq '' ? $checked : '';
my $sort_prot = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'protein' ? $checked : '';
if(! ($sort_prot eq '')) { # can only sort by prot if hide extras
    $hide_prots = $checked;
    $show_prots = '';
}

my $show_scores = ! exists ${$boxptr}{'show_search_scores'} || ${$boxptr}{'show_search_scores'} eq 'show' ? $checked : '';
my $hide_scores = $show_scores eq '' ? $checked : '';

my $hide_pep3d = ! exists ${$boxptr}{'show_pep3d'} || ${$boxptr}{'show_pep3d'} eq 'hide' ? $checked : '';
my $show_pep3d = $hide_pep3d eq '' ? $checked : '';

my $sort = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'yes';
${$boxptr}{'pep_aa'} = uc ${$boxptr}{'pep_aa'} if(exists ${$boxptr}{'pep_aa'});
my $pep_aa = exists ${$boxptr}{'pep_aa'} && ! (${$boxptr}{'pep_aa'} eq '') ? ${$boxptr}{'pep_aa'} : '';

# make adjustments
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
my $exclude_4 = exists ${$boxptr}{'ex4'} && ${$boxptr}{'ex4'} eq 'yes' ? $checked : '';



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

my $show_prot_descr = exists ${$boxptr}{'show_prot_descr'} && ${$boxptr}{'show_prot_descr'} eq 'true' ? 'true' : 'false';
$show_prot_descr = 'true' if(exists ${$boxptr}{'annot'} && ${$boxptr}{'annot'});

if(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'classic') {
    ${$boxptr}{'group_number'} = 1;
    ${$boxptr}{'spectrum'} = 3;
    ${$boxptr}{'probability'} = 2;
    ${$boxptr}{'scores'} = 4;
    ${$boxptr}{'ions'} = 5;
    ${$boxptr}{'peptide_sequence'} = 6;
    ${$boxptr}{'protein'} = 7;
    ${$boxptr}{'xpress'} = 8;
    ${$boxptr}{'fval'} = 9;
    ${$boxptr}{'ntt'} = -1;
    ${$boxptr}{'enzyme'} = -1;
    ${$boxptr}{'xcorr'} = 1;
    ${$boxptr}{'deltacn'} = 2;
    ${$boxptr}{'sprank'} = 3;
    ${$boxptr}{'spscore'} = -1;

}
elsif(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'default') {
    ${$boxptr}{'group_number'} = -1;
    ${$boxptr}{'spectrum'} = -1;
    ${$boxptr}{'probability'} = -1;
    ${$boxptr}{'scores'} = -1;
    ${$boxptr}{'ions'} = -1;
    ${$boxptr}{'peptide_sequence'} = -1;
    ${$boxptr}{'protein'} = -1;
    ${$boxptr}{'xpress'} = -1;
    ${$boxptr}{'fval'} = -1;
    ${$boxptr}{'ntt'} = -1;
    ${$boxptr}{'enzyme'} = -1;
    ${$boxptr}{'xcorr'} = -1;
    ${$boxptr}{'deltacn'} = -1;
    ${$boxptr}{'sprank'} = -1;
    ${$boxptr}{'spscore'} = -1;

    # add on additional search engine stuff here....



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
	for(my $i = 0; $i < @inclusions; $i++) {
	    if($inclusions[$i] == $1) {
		@inclusions = @inclusions[0..$i-1, $i+1..$#inclusions]; # delete it from inclusions
		$done = 1;
		$i = @inclusions;
		# cancel all previous pexclusions with same parent
		my $next_ex = $1;
		for(my $p = 0; $p < @pinclusions; $p++) {
		    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
			@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_ex = $1;
	push(@exclusions, $next_ex) if(! $done); # add to exclusions
	# cancel all previous pinclusions with same parent
	for(my $p = 0; $p < @pinclusions; $p++) {
	    if($pinclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_ex) {
		@pinclusions = @pinclusions[0..$p-1, $p+1..$#pinclusions]; # delete it from inclusions
	    }
	}

    }
    elsif(/^incl(\d+)$/ && ${$boxptr}{$_} eq 'yes') {
	# first make sure not on exclusion list
	my $done = 0;
	for(my $e = 0; $e < @exclusions; $e++) {
	    if($exclusions[$e] == $1) {
		@exclusions = @exclusions[0..$e-1, $e+1..$#exclusions]; # delete it from inclusions
		$done = 1;
		$e = @exclusions;
		# cancel all previous pexclusions with same parent
		my $next_in = $1;
		for(my $p = 0; $p < @pexclusions; $p++) {
		    if($pexclusions[$p] =~ /^(\d+)[a-z,A-Z]$/ && $1 == $next_in) {
			@pexclusions = @pexclusions[0..$p-1, $p+1..$#pexclusions]; # delete it from inclusions
		    }
		}
	    }
	}
	my $next_in = $1;
	push(@inclusions, $next_in) if(! $done); # add to inclusions
	# cancel all previous pexclusions with same parent
	for(my $p = 0; $p < @pexclusions; $p++) {
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
	for(my $i = 0; $i < @pinclusions; $i++) {
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
	for(my $e = 0; $e < @pexclusions; $e++) {
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
$minntt_display = $minntt > 0 ? $minntt : '';



my $asap_display = ! exists ${$boxptr}{'asap_display'} || ${$boxptr}{'asap_display'} eq 'show' ? $checked : '';
my $xpress_display = ! exists ${$boxptr}{'xpress_display'} || ${$boxptr}{'xpress_display'} eq 'show' ? $checked : '';

my $min_SEQ_xcorr_display = exists ${$boxptr}{'min_SEQ_xcorr'} ? ${$boxptr}{'min_SEQ_xcorr'} : '';
my $min_SEQ_delta_display = exists ${$boxptr}{'min_SEQ_delta'} ? ${$boxptr}{'min_SEQ_delta'} : '';
my $max_SEQ_sprank_display = exists ${$boxptr}{'max_SEQ_sprank'} ? ${$boxptr}{'max_SEQ_sprank'} : '';
my $exclude_SEQ = exists ${$boxptr}{'exclSEQ'} ? $checked : '';
my $min_SEQ_xcorr = $min_SEQ_xcorr_display eq '' ? 0 : $min_SEQ_xcorr_display;
if(! ($exclude_SEQ eq '')) {
    $min_SEQ_xcorr = 100; # set to highest...
}
my $min_MAS_ionscore_display = exists ${$boxptr}{'min_MAS_ionscore'} ? ${$boxptr}{'min_MAS_ionscore'} : '';
my $min_MAS_idscore_display = exists ${$boxptr}{'min_MAS_idscore'} ? $checked : ''; 

my $exclude_MAS = exists ${$boxptr}{'exclMAS'} ? $checked : '';
if(! ($exclude_MAS eq '')) {
    $min_MAS_ionscore = 900; # set to highest...
}
my $min_TAN_hyperscore_display = exists ${$boxptr}{'min_TAN_hyperscore'} ? ${$boxptr}{'min_TAN_hyperscore'} : '';
my $min_TAN_nextscore_display = exists ${$boxptr}{'min_TAN_nextscore'} ? ${$boxptr}{'min_TAN_nextscore'} : '';
my $min_TAN_expectscore_display = exists ${$boxptr}{'min_TAN_expectscore'} ? ${$boxptr}{'min_TAN_expectscore'} : '';

my $exclude_TAN = exists ${$boxptr}{'exclTAN'} ? $checked : '';
if(! ($exclude_TAN eq '')) {
    $min_TAN_hyperscore = 900; # set to highest...
}

my $min_PHEN_zscore_display = exists ${$boxptr}{'min_PHEN_zscore'} ? ${$boxptr}{'min_PHEN_zscore'} : '';
my $min_PHEN_origScore_display = exists ${$boxptr}{'min_PHEN_origScore'} ? ${$boxptr}{'min_PHEN_origScore'} : '';

my $exclude_PHEN = exists ${$boxptr}{'exclPHEN'} ? $checked : '';
if(! ($exclude_PHEN eq '')) {
    $min_PHEN_zscore = 900; # set to highest...
}

my $min_COM_dotproduct_display = exists ${$boxptr}{'min_COM_dotproduct'} ? ${$boxptr}{'min_COM_dotproduct'} : '';
my $min_COM_delta_display = exists ${$boxptr}{'min_COM_delta'} ? ${$boxptr}{'min_COM_delta'} : '';
my $min_COM_zscore_display = exists ${$boxptr}{'min_COM_zscore'} ? ${$boxptr}{'min_COM_zscore'} : '';
my $exclude_COM = exists ${$boxptr}{'exclCOM'} ? $checked : '';
if(! ($exclude_COM eq '')) {
    $min_COM_dotproduct = 1001; # set to highest...
}



$score_display = exists ${$boxptr}{'score_display'} ? ${$boxptr}{'score_display'} : 'generic';

print XSL '<xsl:variable name="tab"><xsl:text>&#x09;</xsl:text></xsl:variable>', "\n";
print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
print XSL '</xsl:text></xsl:variable>';
print XSL '<xsl:key name="search_engine" match="/pepx:msms_pipeline_analysis//pepx:msms_run_summary/pepx:search_summary/@search_engine" use="."/>';
print XSL '<xsl:key name="libra_channels" match="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']/pepx:libra_summary/@channel_code" use="."/>';

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

$PROBID_display{'bays'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'bays_score\']/@value"/>' . $table_spacer . '</td>';
$PROBID_default_order{'bays'} = 1;
$PROBID_header{'bays'} = '<td><font color="brown"><b>bays score</b></font></td>';
$PROBID_tab_header{'bays'} = 'PROBID bays';
$PROBID_tab_display{'bays'} = '<xsl:if test="$search_engine=\'PROBID\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'bays_score\']/@value"/></xsl:if>';

$PROBID_display{'pid_zscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'z_score\']/@value"/>' . $table_spacer . '</td>';
$PROBID_default_order{'pid_zscore'} = 2;
$PROBID_header{'pid_zscore'} = '<td><font color="brown"><b>zscore</b></font></td>';
$PROBID_tab_header{'pid_zscore'} = 'PROBID zscore';
$PROBID_tab_display{'pid_zscore'} = '<xsl:if test="$search_engine=\'PROBID\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'z_score\']/@value"/></xsl:if>';


$SEQ_display{'xcorr'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'xcorr\']/@value"/>' . $table_spacer . '</td>';
$SEQ_default_order{'xcorr'} = 1;
$SEQ_header{'xcorr'} = '<td><font color="brown"><b>xcorr</b></font></td>';
$SEQ_tab_header{'xcorr'} = 'SEQUEST xcorr';
$SEQ_tab_display{'xcorr'} = '<xsl:if test="$search_engine=\'SEQUEST\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'xcorr\']/@value"/></xsl:if>';


$SEQ_display{'deltacn'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltacn\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltacnstar\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$SEQ_default_order{'deltacn'} = 2;
$SEQ_header{'deltacn'} = '<td><font color="brown"><b>deltacn</b></font></td>';
$SEQ_tab_header{'deltacn'} = 'SEQUEST deltacn';
$SEQ_tab_display{'deltacn'} = '<xsl:if test="$search_engine=\'SEQUEST\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltacn\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltacnstar\']/@value=\'1\'">*</xsl:if></xsl:if>';

$SEQ_display{'sprank'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'sprank\']/@value"/>' . $table_spacer . '</td>';
$SEQ_default_order{'sprank'} = 3;
$SEQ_header{'sprank'} = '<td><font color="brown"><b>sprank</b></font></td>';
$SEQ_tab_header{'sprank'} = 'SEQUEST sprank';
$SEQ_tab_display{'sprank'} = '<xsl:if test="$search_engine=\'SEQUEST\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'sprank\']/@value"/></xsl:if>';


$MAS_display{'ionscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'ionscore\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'star\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$MAS_default_order{'ionscore'} = 1;
$MAS_header{'ionscore'} = '<td width="50"><font color="brown"><b>ionscore</b></font></td>';
$MAS_tab_header{'ionscore'} = 'MASCOT ionscore';
$MAS_tab_display{'ionscore'} = '<xsl:if test="$search_engine=\'MASCOT\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'ionscore\']/@value"/></xsl:if>';

$MAS_display{'idscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'identityscore\']/@value"/>' . $table_spacer . '</td>';
$MAS_default_order{'idscore'} = 2;
$MAS_header{'idscore'} = '<td width="50"><font color="brown"><b>id score</b></font></td>';
$MAS_tab_header{'idscore'} = 'MASCOT id score';
$MAS_tab_display{'idscore'} = '<xsl:if test="$search_engine=\'MASCOT\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'identityscore\']/@value"/></xsl:if>';

$MAS_display{'homologyscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'homologyscore\']/@value"/>' . $table_spacer . '</td>';
$MAS_default_order{'homologyscore'} = 3;
$MAS_header{'homologyscore'} = '<td width="50"><font color="brown"><b>homology score</b></font></td>';
$MAS_tab_header{'homologyscore'} = 'MASCOT homology score';
$MAS_tab_display{'homologyscore'} = '<xsl:if test="$search_engine=\'MASCOT\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'homologyscore\']/@value"/></xsl:if>';


$TAN_display{'hyperscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'hyperscore\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'star\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$TAN_default_order{'hyperscore'} = 1;
$TAN_header{'hyperscore'} = '<td width="50"><font color="brown"><b>hyper score</b></font></td>';
$TAN_tab_header{'hyperscore'} = 'X! Tandem hyperscore';
$TAN_tab_display{'hyperscore'} = '<xsl:if test="$search_engine=\'X! Tandem\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'hyperscore\']/@value"/></xsl:if>';

$TAN_display{'nextscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'nextscore\']/@value"/>' . $table_spacer . '</td>';
$TAN_default_order{'nextscore'} = 2;
$TAN_header{'nextscore'} = '<td width="50"><font color="brown"><b>next score</b></font></td>';
$TAN_tab_header{'nextscore'} = 'X! Tandem nextscore';
$TAN_tab_display{'nextscore'} = '<xsl:if test="$search_engine=\'X! Tandem\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'nextscore\']/@value"/></xsl:if>';

$TAN_display{'expect'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'expect\']/@value"/>' . $table_spacer . '</td>';
$TAN_default_order{'expect'} = 3;
$TAN_header{'expect'} = '<td width="50"><font color="brown"><b>expect score</b></font></td>';
$TAN_tab_header{'expect'} = 'X! Tandem expect score';
$TAN_tab_display{'expect'} = '<xsl:if test="$search_engine=\'X! Tandem\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'expect\']/@value"/></xsl:if>';


$PHEN_display{'zscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'zscore\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'star\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$PHEN_default_order{'zscore'} = 1;
$PHEN_header{'zscore'} = '<td width="50"><font color="brown"><b>zscore</b></font></td>';
$PHEN_tab_header{'zscore'} = 'PHENYX zscore';
$PHEN_tab_display{'zscore'} = '<xsl:if test="$search_engine=\'PHENYX\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'zscore\']/@value"/></xsl:if>';

$PHEN_display{'origScore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'origScore\']/@value"/>' . $table_spacer . '</td>';
$PHEN_default_order{'origScore'} = 2;
$PHEN_header{'origScore'} = '<td width="50"><font color="brown"><b>origScore</b></font></td>';
$PHEN_tab_header{'origScore'} = 'PHENYX origScore';
$PHEN_tab_display{'origScore'} = '<xsl:if test="$search_engine=\'PHENYX\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'origScore\']/@value"/></xsl:if>';


$COM_display{'dotproduct'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'dotproduct\']/@value"/>' . $table_spacer . '</td>';
$COM_default_order{'dotproduct'} = 1;
$COM_header{'dotproduct'} = '<td width="50"><font color="brown"><b>dot product</b></font></td>';
$COM_tab_header{'dotproduct'} = 'COMET dotproduct';
$COM_tab_display{'dotproduct'} = '<xsl:if test="$search_engine=\'COMET\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'dotproduct\']/@value"/></xsl:if>';

$COM_display{'delta'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'delta\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltastar\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$COM_default_order{'delta'} = 2;
$COM_header{'delta'} = '<td width="50"><font color="brown"><b>delta</b></font></td>';
$COM_tab_header{'delta'} = 'COMET delta';
$COM_tab_display{'delta'} = '<xsl:if test="$search_engine=\'COMET\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'delta\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltastar\']/@value=\'1\'">*</xsl:if></xsl:if>';

$COM_display{'zscore'} = '<td width="50" align="right"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'zscore\']/@value"/>' . $table_spacer . '</td>';
$COM_default_order{'zscore'} = 3;
$COM_header{'zscore'} = '<td width="50"><font color="brown"><b>zscore</b></font></td>';
$COM_tab_header{'zscore'} = 'COMET zscore';
$COM_tab_display{'zscore'} = '<xsl:if test="$search_engine=\'COMET\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'zscore\']/@value"/></xsl:if>';




foreach(keys %SEQ_display) {
    $SEQ_display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $SEQ_register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
}


foreach(keys %MAS_display) {
    $MAS_display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $MAS_register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
    #print "$_: $MAS_display_order{$_}\n";
}

foreach(keys %PHEN_display) {
    $PHEN_display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $PHEN_register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
    #print "$_: $PHEN_display_order{$_}\n";
}

foreach(keys %TAN_display) {
    $TAN_display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $TAN_register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
    #print "$_: $TAN_display_order{$_}\n";
}

foreach(keys %COM_display) {
    $COM_display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $COM_register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
    #print "$_: $COM_display_order{$_}\n";
}

foreach(keys %PROBID_display) {
    $PROBID_display_order{$_} = ${$boxptr}{$_} if(exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0);
    $PROBID_register_order{$_} = exists ${$boxptr}{$_} && ! (${$boxptr}{$_} eq '') && ${$boxptr}{$_} > 0 ? ${$boxptr}{$_} : '';
    #print "$_: $COM_display_order{$_}\n";
}


$display{'scores'} = '<td>' . $RESULT_TABLE_PRE . $RESULT_TABLE . '<TR>';
$display{'scores'} .= '<xsl:if test="$search_engine=\'SEQUEST\'">';

$header{'sequest_scores'} = '';
$tab_display{'scores'} = '<xsl:if test="$search_engine=\'SEQUEST\'">';
$tab_header{'sequest_scores'} = '<xsl:if test="$search_engine=\'SEQUEST\'">';

if(scalar keys %SEQ_display_order > 0) {

    foreach(sort {$SEQ_display_order{$a} <=> $SEQ_display_order{$b}} keys %SEQ_display_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $SEQ_tab_display{$_} . $tab;
	    $tab_header{'sequest_scores'} .= $SEQ_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $SEQ_display{$_};
	    $header{'sequest_scores'} .= $SEQ_header{$_};
	}
    }
}
else {
    foreach(sort {$SEQ_default_order{$a} <=> $SEQ_default_order{$b}} keys %SEQ_default_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $SEQ_tab_display{$_} . $tab;
	    $tab_header{'sequest_scores'} .= $SEQ_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $SEQ_display{$_};
	    $header{'sequest_scores'} .= $SEQ_header{$_};
	}
    }
}
$display{'scores'} .= '</xsl:if>';
$tab_display{'scores'} .= '</xsl:if>';
$tab_header{'sequest_scores'} .= '</xsl:if>';


# then the other search engines here....
# MASCOT
$display{'scores'} .= '<xsl:if test="$search_engine=\'MASCOT\'">';

$header{'mascot_scores'} = '';
$tab_display{'scores'} .= '<xsl:if test="$search_engine=\'MASCOT\'">';
$tab_header{'mascot_scores'} .= '<xsl:if test="$search_engine=\'MASCOT\'">';

if(scalar keys %MAS_display_order > 0) {

    foreach(sort {$MAS_display_order{$a} <=> $MAS_display_order{$b}} keys %MAS_display_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $MAS_tab_display{$_} . $tab;
	    $tab_header{'mascot_scores'} .= $MAS_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $MAS_display{$_};
	    $header{'mascot_scores'} .= $MAS_header{$_};
	}
    }
}
else {
    foreach(sort {$MAS_default_order{$a} <=> $MAS_default_order{$b}} keys %MAS_default_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $MAS_tab_display{$_} . $tab;
	    $tab_header{'mascot_scores'} .= $MAS_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $MAS_display{$_};
	    $header{'mascot_scores'} .= $MAS_header{$_};
	}
    }
}
$display{'scores'} .= '</xsl:if>';
$tab_display{'scores'} .= '</xsl:if>';
$tab_header{'mascot_scores'} .= '</xsl:if>';

# Tandem
$display{'scores'} .= '<xsl:if test="$search_engine=\'X! Tandem\'">';

$header{'tandem_scores'} = '';
$tab_display{'scores'} .= '<xsl:if test="$search_engine=\'X! Tandem\'">';
$tab_header{'tandem_scores'} .= '<xsl:if test="$search_engine=\'X! Tandem\'">';

if(scalar keys %TAN_display_order > 0) {

    foreach(sort {$TAN_display_order{$a} <=> $TAN_display_order{$b}} keys %TAN_display_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $TAN_tab_display{$_} . $tab;
	    $tab_header{'tandem_scores'} .= $TAN_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $TAN_display{$_};
	    $header{'tandem_scores'} .= $TAN_header{$_};
	}
    }
}
else {
    foreach(sort {$TAN_default_order{$a} <=> $TAN_default_order{$b}} keys %TAN_default_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $TAN_tab_display{$_} . $tab;
	    $tab_header{'tandem_scores'} .= $TAN_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $TAN_display{$_};
	    $header{'tandem_scores'} .= $TAN_header{$_};
	}
    }
}
$display{'scores'} .= '</xsl:if>';
$tab_display{'scores'} .= '</xsl:if>';
$tab_header{'tandem_scores'} .= '</xsl:if>';


# Phenyx
$display{'scores'} .= '<xsl:if test="$search_engine=\'PHENYX\'">';

$header{'PHENYX_scores'} = '';
$tab_display{'scores'} .= '<xsl:if test="$search_engine=\'PHENYX\'">';
$tab_header{'PHENYX_scores'} .= '<xsl:if test="$search_engine=\'PHENYX\'">';

if(scalar keys %PHEN_display_order > 0) {

    foreach(sort {$PHEN_display_order{$a} <=> $PHEN_display_order{$b}} keys %PHEN_display_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $PHEN_tab_display{$_} . $tab;
	    $tab_header{'PHENYX_scores'} .= $PHEN_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $PHEN_display{$_};
	    $header{'PHENYX_scores'} .= $PHEN_header{$_};
	}
    }
}
else {
    foreach(sort {$PHEN_default_order{$a} <=> $PHEN_default_order{$b}} keys %PHEN_default_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $PHEN_tab_display{$_} . $tab;
	    $tab_header{'PHENYX_scores'} .= $PHEN_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $PHEN_display{$_};
	    $header{'PHENYX_scores'} .= $PHEN_header{$_};
	}
    }
}
$display{'scores'} .= '</xsl:if>';
$tab_display{'scores'} .= '</xsl:if>';
$tab_header{'PHENYX_scores'} .= '</xsl:if>';


# COMET
$display{'scores'} .= '<xsl:if test="$search_engine=\'COMET\'">';

$header{'comet_scores'} = '';
$tab_display{'scores'} .= '<xsl:if test="$search_engine=\'COMET\'">';
$tab_header{'comet_scores'} .= '<xsl:if test="$search_engine=\'COMET\'">';

if(scalar keys %COM_display_order > 0) {

    foreach(sort {$COM_display_order{$a} <=> $COM_display_order{$b}} keys %COM_display_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $COM_tab_display{$_} . $tab;
	    $tab_header{'comet_scores'} .= $COM_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $COM_display{$_};
	    $header{'comet_scores'} .= $COM_header{$_};
	}
    }
}
else {
    foreach(sort {$COM_default_order{$a} <=> $COM_default_order{$b}} keys %COM_default_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $COM_tab_display{$_} . $tab;
	    $tab_header{'comet_scores'} .= $COM_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $COM_display{$_};
	    $header{'comet_scores'} .= $COM_header{$_};
	}
    }
}
$display{'scores'} .= '</xsl:if>';
$tab_display{'scores'} .= '</xsl:if>';
$tab_header{'comet_scores'} .= '</xsl:if>';




# PROBID
$display{'scores'} .= '<xsl:if test="$search_engine=\'PROBID\'">';

$header{'probid_scores'} = '';
$tab_display{'scores'} .= '<xsl:if test="$search_engine=\'PROBID\'">';
$tab_header{'probid_scores'} .= '<xsl:if test="$search_engine=\'PROBID\'">';

if(scalar keys %PROBID_display_order > 0) {

    foreach(sort {$PROBID_display_order{$a} <=> $PROBID_display_order{$b}} keys %PROBID_display_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $PROBID_tab_display{$_} . $tab;
	    $tab_header{'probid_scores'} .= $PROBID_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $PROBID_display{$_};
	    $header{'probid_scores'} .= $PROBID_header{$_};
	}
    }
}
else {
    foreach(sort {$PROBID_default_order{$a} <=> $PROBID_default_order{$b}} keys %PROBID_default_order) {
	if($tab_delim >= 1) {
	    $tab_display{'scores'} .= $PROBID_tab_display{$_} . $tab;
	    $tab_header{'probid_scores'} .= $PROBID_tab_header{$_} . $tab;
	}
	else {
	    $display{'scores'} .= $PROBID_display{$_};
	    $header{'probid_scores'} .= $PROBID_header{$_};
	}
    }
}
$display{'scores'} .= '</xsl:if>';
$tab_display{'scores'} .= '</xsl:if>';
$tab_header{'probid_scores'} .= '</xsl:if>';




$display{'scores'} .= '</TR>' . $RESULT_TABLE_SUF . '</td>';

$default_order{'scores'} = 4;


# change this to group number at some time
$display{'index'} = '<td><xsl:value-of select="@index"/>' . $table_spacer . '</td>';
$default_order{'group_number'} = 1;
$header{'group_number'} = '<td><font color="brown"><b>index</b></font>' . $table_spacer . '</td>';
$tab_header{'group_number'} = 'index' . $tab;
$tab_display{'group_number'} = '<xsl:value-of select="@index"/>' . $tab;

my $spec_ref = '<xsl:choose><xsl:when test="$search_engine=\'SEQUEST\' or $search_engine=\'PROBID\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'sequest-tgz-out.cgi?OutFile={$basename}/{$xpress_spec}.out"><xsl:value-of select="@spectrum"/></A></xsl:when><xsl:when test="$search_engine=\'MASCOT\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'mascotout.pl?OutFile={$basename}/{$xpress_spec}.out"><xsl:value-of select="@spectrum"/></A></xsl:when><xsl:when test="$search_engine=\'X! Tandem\'"><xsl:variable name="tandxml" select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:parameter[@name=\'output, path\']/@value"/><xsl:variable name="tandid" select="@start_scan"/><A TARGET="Win1" HREF="{$tandxml}#{$tandid}"><xsl:value-of select="@spectrum"/></A></xsl:when><xsl:when test="$search_engine=\'COMET\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'cometresult.cgi?TarFile={$basename}.cmt.tar.gz&amp;File={$xpress_spec}.cmt"><xsl:value-of select="@spectrum"/></A></xsl:when><xsl:otherwise><xsl:value-of select="@spectrum"/></xsl:otherwise></xsl:choose>';

$display{'spectrum'} = '<td>' . $spec_ref . $table_spacer . '</td>';
$default_order{'spectrum'} = 3;
$header{'spectrum'} = '<td><font color="brown"><b>spectrum</b></font>' . $table_spacer . '</td>';
$tab_header{'spectrum'} = 'spectrum' . $tab;
$tab_display{'spectrum'} = '<xsl:value-of select="@spectrum"/>' . $tab;

# deprecated
my $ions_ref_orig = '<xsl:choose><xsl:when test="$search_engine=\'COMET\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'cometplot.cgi?TarFile={$basename}.cmt.tar.gz&amp;File={$xpress_spec}.dta&amp;Xmin=0&amp;Xmax=0&amp;Ymin=2&amp;Ymax=3&amp;LabelType=0&amp;NumAxis=1&amp;Pep={$StrippedPeptide}&amp;ConfigFile=comet.def&amp;MD5={$comet_md5_check_sum}&amp;PepMass={$pep_mass}&amp;ShowB=1&amp;ShowY=1&amp;Mods={$PeptideMods}"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@tot_num_ions"/></nobr></A></xsl:when><xsl:otherwise><A TARGET="Win1" HREF="' . $CGI_HOME . 'sequest-tgz-plot.cgi?MassType={$masstype}&amp;NumAxis=1&amp;Pep={$StrippedPeptide}&amp;Dta={$basename}/{$xpress_spec}.dta&amp;PeptideMods={$PeptideMods}"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@tot_num_ions"/></nobr></A></xsl:otherwise></xsl:choose>';

my $ions_ref = '<xsl:choose><xsl:when test="$search_engine=\'COMET\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'plot-msms.cgi?MassType={$masstype}&amp;NumAxis=1&amp;{$PeptideMods2}Pep={$StrippedPeptide}&amp;Dta={$basename}/{$xpress_spec}.dta&amp;COMET=1"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@tot_num_ions"/></nobr></A></xsl:when><xsl:otherwise><A TARGET="Win1" HREF="' . $CGI_HOME . 'plot-msms.cgi?MassType={$masstype}&amp;NumAxis=1&amp;{$PeptideMods2}Pep={$StrippedPeptide}&amp;Dta={$basename}/{$xpress_spec}.dta"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@tot_num_ions"/></nobr></A></xsl:otherwise></xsl:choose>';


$display{'ions'} = '<td align="RIGHT">' . $ions_ref . $table_spacer . '</td>';
$default_order{'ions'} = 5;
$header{'ions'} = '<td><font color="brown"><b>m ions</b></font>' . $table_spacer . '</td>';
$tab_header{'ions'} = 'ions' . $tab;
$tab_display{'ions'} = '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@tot_num_ions"/>' . $tab;

$display{'fval'} = '<xsl:if test="$pepproph_flag=\'true\'"><td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'fval\']/@value"/></nobr></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary)">N_A</xsl:if>' . $table_spacer . '</td></xsl:if>';
$default_order{'fval'} = 9;
$header{'fval'} = '<xsl:if test="$pepproph_flag=\'true\'"><td><font color="brown"><b>fval</b></font>' . $table_spacer . '</td></xsl:if>';
$tab_header{'fval'} = '<xsl:if test="$pepproph_flag=\'true\'">fval' . $tab . '</xsl:if>';
$tab_display{'fval'} = '<xsl:if test="$pepproph_flag=\'true\'"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'fval\']/@value"/></xsl:if>' . $tab . '</xsl:if>';

#$display{'pI'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"><td align="right"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"/></xsl:if></td></xsl:if>';

$display{'pI'} = '<xsl:if test="$pI_flag=\'true\'"><td><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'pI\']/@value"/></nobr>' . $table_spacer . '</td></xsl:if>';

$default_order{'pI'} = 10;

$display{'pI_zscore'} = '<xsl:if test="$pI_flag=\'true\'"><td><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'pI_zscore\']/@value"/></nobr>' . $table_spacer . '</td></xsl:if>';

$default_order{'pI_zscore'} = 11;

#$header{'pI'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"><td><font color="brown"><b>calc pI</b></font>' . $table_spacer . '</td></xsl:if>';

$header{'pI'} = '<xsl:if test="$pI_flag=\'true\'"><td><font color="brown"><b>pI</b></font>' . $table_spacer . '</td></xsl:if>';

$header{'pI_zscore'} = '<xsl:if test="$pI_flag=\'true\'"><td><font color="brown"><b>pI z-score</b></font>' . $table_spacer . '</td></xsl:if>';

#$tab_header{'pI'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI">calc pI' . $tab . '</xsl:if>';

$tab_header{'pI'} = '<xsl:if test="$pI_flag=\'true\'">pI' . $tab . '</xsl:if>';

$tab_header{'pI_zscore'} = '<xsl:if test="$pI_flag=\'true\'">pI z-score' . $tab . '</xsl:if>';


#$tab_display{'pI'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"><xsl:if test="search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"><xsl:value-of select="search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"/></xsl:if>' . $tab . '</xsl:if>';

$tab_display{'pI'} = '<xsl:if test="$pI_flag=\'true\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'pI\']/@value"/>' . $tab . '</xsl:if>';

$tab_display{'pI_zscore'} = '<xsl:if test="$pI_flag=\'true\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'pI_zscore\']/@value"/>' . $tab . '</xsl:if>';

my $ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $quant_header = $quant_light2heavy eq 'true' ? '' : '(H/L) ';


my $xpress_ref = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'XPressPeptideUpdateParser.cgi?LightFirstScan={$light_first_scan}&amp;LightLastScan={$light_last_scan}&amp;HeavyFirstScan={$heavy_first_scan}&amp;HeavyLastScan={$heavy_last_scan}&amp;XMLFile={$basename}.mzXML&amp;ChargeState={$xpress_charge}&amp;LightMass={$LightMass}&amp;HeavyMass={$HeavyMass}&amp;MassTol={$MassTol}&amp;PpmTol={$PpmTol}&amp;index={$xpress_index}&amp;xmlfile=' . $xmlfile . '&amp;OutFile={$xpress_spec}&amp;bXpressLight1={$xpress_display}">';

$display{'xpress'} = '<xsl:if test="$xpress_flag=\'true\'"><td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']">' . $xpress_ref . '<nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@' . getRatioPrefix($quant_light2heavy) . 'ratio"/></nobr></A></xsl:if></td></xsl:if>';
$default_order{'xpress'} = 8;
$header{'xpress'} = '<xsl:if test="$xpress_flag=\'true\'"><td><font color="brown"><b>' . $quant_header . 'xpress</b></font>' . $table_spacer . '</td></xsl:if>';
$tab_header{'xpress'} = '<xsl:if test="$xpress_flag=\'true\'">' . $quant_header . 'xpress' . $tab . '</xsl:if>';
$tab_display{'xpress'} = '<xsl:if test="$xpress_flag=\'true\'"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@' . getRatioPrefix($quant_light2heavy) . 'ratio"/></xsl:if>' . $tab . '</xsl:if>';

my $asap_ref = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'ASAPRatioPeptideCGIDisplayParser.cgi?Xmlfile=' . $xmlfile . '&amp;Basename={$basename}&amp;Indx={$xpress_index}&amp;Timestamp={$asap_time}&amp;Spectrum={$xpress_spec}&amp;ratioType=' . $ratio_type . '">';
my $plusmn = '&#177;';
$plusmn = ' +-' if($xslt =~ /xsltproc/);

$display{'asap'} = '<xsl:if test="$asapratio_flag=\'true\'"><td>' . $asap_ref . 
'<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&lt;\'0\'">N_A</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&gt;\'-1\'"><xsl:choose><xsl:when test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean=\'0\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean=\'999\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'error &gt; 0.5 * pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean"><font color="red"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'error"/></nobr></font></xsl:when><xsl:otherwise><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'error"/></nobr></xsl:otherwise></xsl:choose></xsl:if></A></td></xsl:if>';

$default_order{'asap'} = 8.5;
$header{'asap'} = '<xsl:if test="$asapratio_flag=\'true\'"><td><font color="brown"><b>' . $quant_header . 'asapratio</b></font>' . $table_spacer . '</td></xsl:if>';
$tab_header{'asap'} = '<xsl:if test="$asapratio_flag=\'true\'">' . $quant_header . 'asap mean' . $tab . $quant_header . 'asap error' . $tab . '</xsl:if>';
$tab_display{'asap'} = '<xsl:if test="$asapratio_flag=\'true\'"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean=\'-1\'">N_A</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&gt;\'-1\'"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean"/></xsl:if>' . $tab . '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'error"/></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result)">' . $tab . '</xsl:if>' . $tab . '</xsl:if>';

my $prot_ref = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'comet-fastadb.cgi?Ref={$Protein}&amp;Db={$Database}&amp;Pep={$StrippedPeptide}&amp;MassType={$masstype}">';
my $alt_ref = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'comet-fastadb.cgi?Ref={$AltProtein}&amp;Db={$Database}&amp;Pep={$StrippedPeptide}&amp;MassType={$masstype}">';
my $prot_ref2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'comet-fastadb.cgi?Db={$Database}&amp;Pep={$StrippedPeptide}&amp;MassType={$masstype}&amp;sample_enzyme={$enzyme}&amp;min_ntt={$minntt}">';
# this one to use in future...
my $prot_ref3 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'comet-fastadb.cgi?Db={$Database}&amp;Pep={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide}&amp;MassType={$masstype}&amp;sample_enzyme={$enzyme}&amp;min_ntt={$minntt}">';


if(! ($show_prots eq '')) {
    $display{'protein'} = '<td>' . $prot_ref3 . '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein"/></A><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:alternative_protein"><xsl:variable name="AltProtein" select="@protein"/><xsl:text> </xsl:text>' . $alt_ref . '<xsl:value-of select="@protein"/></A></xsl:for-each>' . $spacer . '</td>';
}
else {
# alternative listing only first entry, with link to all
$display{'protein'} = '<td>' . $prot_ref3 . '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein"/></A><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tot_proteins &gt; 1"><xsl:if test="$dbrefresh_flag=\'true\'"><xsl:text> </xsl:text>' . $prot_ref3 . '+<xsl:value-of select="number(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tot_proteins)-1"/></A></xsl:if></xsl:if>' . $spacer . '</td>';
}
$tab_header{'protein'} = 'protein' . $tab . '<xsl:if test="pepx:search_result/pepx:search_hit/@protein_mw">protein mw' . $tab . '</xsl:if>';
$tab_display{'protein'} = '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein"/><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:alternative_protein">,<xsl:value-of select="@protein"/></xsl:for-each>' . $tab . '<xsl:if test="pepx:search_result/pepx:search_hit/@protein_mw"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein_mw"/><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:alternative_protein">,<xsl:value-of select="@protein_mw"/></xsl:for-each>' . $tab . '</xsl:if>';

$header{'protein'} = '<td><font color="brown"><b>protein</b></font>' . $table_spacer . '</td>';
$default_order{'protein'} = 7;

if($show_prot_descr eq 'true') {
    $display{'annot'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein_descr"><td width="300"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein_descr"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein_descr"/></xsl:if></td></xsl:if>';
    $default_order{'annot'} = 7.5;
    $header{'annot'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein_descr"><td width="300"><font color="brown"><b>protein descr</b></font>' . $table_spacer . '</td></xsl:if>';
$tab_display{'annot'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein_descr"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein_descr"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein_descr"/></xsl:if>' . $tab . '</xsl:if>';
}



$header{'ntt'} = '<td><font color="brown"><b>ntt</b></font>' . $table_spacer . '</td>';;
$display{'ntt'} = '<td><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tol_term"/>' . $table_spacer . '</td>';
$tab_header{'ntt'} = 'num tol term' . $tab;
$tab_display{'ntt'} = '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tol_term"/>' . $tab;


$header{'enzyme'} = '<td><font color="brown"><b>enzyme</b></font>' . $table_spacer . '</td>';;
$display{'enzyme'} = '<td><xsl:value-of select="$enzyme"/>' . $table_spacer . '</td>';
$tab_header{'enzyme'} = 'sample enzyme' . $tab;
$tab_display{'enzyme'} = '<xsl:value-of select="$enzyme"/>' . $tab;

$header{'peptide_mass'} = '<td><font color="brown"><b>peptide mass</b></font>' . $table_spacer . '</td>';;
$display{'peptide_mass'} = '<td><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass"/>' . $table_spacer . '</td>';
$tab_header{'peptide_mass'} = 'peptide mass' . $tab;
$tab_display{'peptide_mass'} = '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass"/>' . $tab;

$header{'massdiff'} = '<td><font color="brown"><b>mass diff</b></font>' . $table_spacer . '</td>';;
$display{'massdiff'} = '<td><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@massdiff"/>' . $table_spacer . '</td>';
$tab_header{'massdiff'} = 'mass diff' . $tab;
$tab_display{'massdiff'} = '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@massdiff"/>' . $tab;


my $libra_tag = $libra_norm eq '' ? '@absolute' : '@normalized';
my $libra_pre = $libra_norm eq '' ? '' : '(norm)';
$header{'libra'} = '<xsl:if test="$libra_count=\'1\'"><xsl:for-each select="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']/pepx:libra_summary/pepx:fragment_masses"><td><font color="brown"><b>libra' . $libra_pre . ' <xsl:value-of select="@mz"/></b></font>' . $table_spacer . '</td></xsl:for-each></xsl:if>';


$display{'libra'} = '<xsl:if test="$libra_count=\'1\'"><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'libra\']/pepx:libra_result/pepx:intensity"><td><xsl:value-of select="' . $libra_tag . '"/>' . $table_spacer . '</td></xsl:for-each></xsl:if>';

$tab_header{'libra'} = '<xsl:if test="$libra_count=\'1\'"><xsl:for-each select="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']/pepx:libra_summary/pepx:fragment_masses">libra' . $libra_pre . ' <xsl:value-of select="@mz"/>' . $tab . '</xsl:for-each></xsl:if>';
$tab_display{'libra'} = '<xsl:if test="$libra_count=\'1\'"><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'libra\']/pepx:libra_result/pepx:intensity"><xsl:value-of select="' . $libra_tag . '"/>' . $tab . '</xsl:for-each></xsl:if>';
$default_order{'libra'} = 5.5;


# add here the cgi info for peptide
my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_mid = '&amp;Infile=';

my $pep_ref = '<A TARGET="Win1" HREF="http://www.ncbi.nlm.nih.gov/blast/Blast.cgi?CMD=Web&amp;LAYOUT=TwoWindows&amp;AUTO_FORMAT=Semiauto&amp;ALIGNMENTS=50&amp;ALIGNMENT_VIEW=Pairwise&amp;CDD_SEARCH=on&amp;CLIENT=web&amp;COMPOSITION_BASED_STATISTICS=on&amp;DATABASE=nr&amp;DESCRIPTIONS=100&amp;ENTREZ_QUERY=(none)&amp;EXPECT=1000&amp;FILTER=L&amp;FORMAT_OBJECT=Alignment&amp;FORMAT_TYPE=HTML&amp;I_THRESH=0.005&amp;MATRIX_NAME=BLOSUM62&amp;NCBI_GI=on&amp;PAGE=Proteins&amp;PROGRAM=blastp&amp;SERVICE=plain&amp;SET_DEFAULTS.x=41&amp;SET_DEFAULTS.y=5&amp;SHOW_OVERVIEW=on&amp;END_OF_HTTPGET=Yes&amp;SHOW_LINKOUT=yes&amp;QUERY={$StrippedPeptide}">';

# special case
if($xmlfile =~ /Monica\_Orellana/) {
    $pep_ref = '<A TARGET="Win1" HREF="http://genome.jgi-psf.org/cgi-bin/browseAlignment?program=tblastn&amp;dataLib=thaps1&amp;sequence={$StrippedPeptide}">';

}

my $peptide_atlas_ref = '<A TARGET="Win1" HREF="https://db.systemsbiology.net/sbeams/cgi/PeptideAtlas/Search?organism_name=Human;search_key=%25{$StrippedPeptide}%25;action=GO"><IMG BORDER="0" SRC="'. $HELP_DIR. 'pa_tiny.png"/></A>';

if($DISPLAY_MODS) {
    $display{'peptide_sequence'} = '<td nowrap="TRUE"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>.</xsl:if>' . $pep_ref . '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide)"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/pepx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info)"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/></xsl:if></A><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa">.<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/></xsl:if>' . $table_spacer . $peptide_atlas_ref . $table_spacer . '</td>';
}
else {
    $display{'peptide_sequence'} = '<td nowrap="TRUE"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>.</xsl:if>' . $pep_ref . '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/></A><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa">.<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/></xsl:if>' . $table_spacer . $peptide_atlas_ref . $table_spacer . '</td>';
}


my $peptide_ref = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'ModelParser.cgi?Xmlfile={$summaryxml}&amp;Timestamp={$pepproph_timestamp}&amp;Spectrum={$xpress_spec}&amp;Scores={$scores}&amp;Prob={$prob}">';


$display{'probability'} = '<xsl:if test="$pepproph_flag=\'true\'"><td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']">' . $peptide_ref . '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis=\'adjusted\'"><font color="#FF00FF"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability"/></font></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis) or not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis=\'adjusted\')"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability"/></xsl:if></A></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'])">N_A</xsl:if>' . $table_spacer . '</td></xsl:if>';
$default_order{'probability'} = 2;
$header{'probability'} = '<xsl:if test="$pepproph_flag=\'true\'"><td><font color="brown"><b>prob</b></font>' . $table_spacer . '</td></xsl:if>';
$tab_header{'probability'} = '<xsl:if test="$pepproph_flag=\'true\'">probability' . $tab . '</xsl:if>';
$tab_display{'probability'} = '<xsl:if test="$pepproph_flag=\'true\'"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability"/></xsl:if>' . $tab . '</xsl:if>';



$header{'peptide_sequence'} = '<td><font color="brown"><b>peptide</b></font>' . $table_spacer . '</td>';
$tab_header{'peptide_sequence'} = 'peptide sequence' . $tab;
if($DISPLAY_MODS) {
    $tab_display{'peptide_sequence'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>.</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide)"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/pepx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info)"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/></xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa">.<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/></xsl:if>' . $tab;
}
else {


$tab_display{'peptide_sequence'} = '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>.</xsl:if><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa">.<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/></xsl:if>' . $tab;
}
# add special settings for Bernd
if($xmlfile =~ /\/bernd\//) {
    $tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:value-of select="@peptide_sequence"/><xsl:for-each select="pepx:indistinguishable_peptide">,<xsl:value-of select="@peptide_sequence"/></xsl:for-each>' . $tab . $html_peptide_lead . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . '<xsl:value-of select="@charge"/>_<xsl:value-of select="@peptide_sequence"/></A>';
    $tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence' . $tab . 'peptide link';
}

$default_order{'peptide_sequence'} = 6;


$tab_header{'search_info'} = 'search information' . $tab;
$tab_display{'search_info'} = 'SampleEnzyme=<xsl:value-of select="$enzyme"/> Engine=<xsl:value-of select="$search_engine"/> Database=<xsl:value-of select="$Database"/> 
<xsl:if test="$dbrefresh_flag=\'true\' and not(parent::node()/pepx:analysis_timestamp[@analysis=\'database_refresh\']/pepx:database_refresh_timestamp/@database=$Database)"> RefreshDb=<xsl:value-of select="parent::node()/pepx:analysis_timestamp[@analysis=\'database_refresh\']/pepx:database_refresh_timestamp/@database"/></xsl:if><xsl:for-each select="parent::node()/pepx:search_summary/pepx:sequence_search_constraint"> SequenceConstraint=<xsl:value-of select="@sequence"/></xsl:for-each><xsl:for-each select ="parent::node()/pepx:search_summary/pepx:enzymatic_search_constraint"> EnzymeConstraint=(<xsl:value-of select="@enzyme"/> MinTerm=<xsl:value-of select="@min_number_termini"/> MaxMissed=<xsl:value-of select="@max_num_internal_cleavages"/>)</xsl:for-each><xsl:for-each select="parent::node()/pepx:search_summary/pepx:terminal_modification"> TerminalMod=(<xsl:value-of select="@terminus"/> Mass=<xsl:value-of select="@mass"/> Variable=<xsl:value-of select="@variable"/>)</xsl:for-each><xsl:for-each select="parent::node()/pepx:search_summary/pepx:aminoacid_modification"> AminoAcidMod=(<xsl:value-of select="@aminoacid"/> Mass=<xsl:value-of select="@mass"/> Variable=<xsl:value-of select="@variable"/>)</xsl:for-each>';
$default_order{'search_info'} = 100; # last one


$plusmn = '&#177;';
$plusmn = ' +-' if($xslt =~ /xsltproc/);


$num_cols = 3;



my $reference = '$index' ; #$show_groups eq '' ? '$parental_group_number' : '$group_number';
my $gn = '<xsl:value-of select="@index"/>'; #$show_groups eq '' ? '<xsl:value-of select="parent::node()/@group_number"/>' : '<xsl:value-of select="@group_number"/>';
if($discards) {

    $display{'group_number'} .= '<input type="checkbox" name="incl{' . $reference . '}" style="height: 15px; width: 15px;" value="yes"/> ';
    if(@exclusions > 0) {
	$display{'group_number'} .= '<xsl:if test="' . $reference . '=\'';
	for(my $e = 0; $e < @exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e < @exclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e < @exclusions; $e++) {
	    $display{'group_number'} .= $exclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e < @exclusions - 1);
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
	for(my $e = 0; $e < @inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\'';
	    $display{'group_number'} .= ' or ' . $reference . '=\'' if($e < @inclusions - 1);
	}
	$display{'group_number'} .= '"><font color="#FF00FF">' . $gn . '</font></xsl:if><xsl:if test="not(' . $reference . '=\'';
	for(my $e = 0; $e < @inclusions; $e++) {
	    $display{'group_number'} .= $inclusions[$e] . '\')';
	    $display{'group_number'} .= ' and not(' . $reference . '=\'' if($e < @inclusions - 1);
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
	for(my $e = 0; $e < @pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e < @pexclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e < @pexclusions; $e++) {
	    $display{'prot_number'} .= $pexclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e < @pexclusions - 1);
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
	for(my $e = 0; $e < @pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\'';
	    $display{'prot_number'} .= ' or $prot_number=\'' if($e < @pinclusions - 1);
	}
	$display{'prot_number'} .= '"><font color="#FF00FF"><xsl:value-of select="$prot_number"/></font></xsl:if><xsl:if test="not($prot_number=\'';
	for(my $e = 0; $e < @pinclusions; $e++) {
	    $display{'prot_number'} .= $pinclusions[$e] . '\')';
	    $display{'prot_number'} .= ' and not($prot_number=\'' if($e < @pinclusions - 1);
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

print XSL '<xsl:variable name="libra_count" select="count(/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']/pepx:libra_summary/@channel_code[generate-id()=generate-id(key(\'libra_channels\',.)[1])])"/>';
print XSL '<xsl:template match="pepx:msms_pipeline_analysis">', "\n";
print XSL '     <xsl:variable name="search_engine" select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine"/>
                <xsl:variable name="summaryxml" select="@summary_xml"/> 
                <xsl:variable name="run_count" select="count(pepx:msms_run_summary)"/>
                <xsl:variable name="pepproph_flag">
			<xsl:choose>
				<xsl:when test="pepx:analysis_summary[@analysis=\'peptideprophet\']">
					<xsl:value-of select="true()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="false()"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
	
		<xsl:variable name="asapratio_flag">
			<xsl:choose>
				<xsl:when test="pepx:analysis_summary[@analysis=\'asapratio\']">
					<xsl:value-of select="true()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="false()"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		
		<xsl:variable name="xpress_flag">
			<xsl:choose>
				<xsl:when test="pepx:analysis_summary[@analysis=\'xpress\']">
					<xsl:value-of select="true()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="false()"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

                <xsl:variable name="pepproph_opts" select="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'peptideprophet\']/pepx:peptideprophet_summary/@options"/> 
                <xsl:variable name="watch_for1"> PI</xsl:variable>
                <xsl:variable name="watch_for2">PI</xsl:variable>
                <xsl:variable name="pI_flag">       
                        <xsl:choose>
				<xsl:when test="contains($pepproph_opts, $watch_for1) or 
                                                starts-with($pepproph_opts, $watch_for2)">
					<xsl:value-of select="true()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="false()"/>
				</xsl:otherwise>
			</xsl:choose>

                </xsl:variable>', "\n";
		
print XSL '<HTML><BODY BGCOLOR="white" OnLoad="self.focus()"><PRE>', "\n";
print XSL '<HEAD><TITLE>' . $xmlfile . ', (' . $TPPVersionInfo .')</TITLE></HEAD>';
print XSL '<table width="100%" border="3" BGCOLOR="#8FBC6F" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;"><tr><td align="center">', "\n";
print XSL '<form method="GET" action="' . $CGI_HOME . 'pepxml2html.pl">', "\n";
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
    print XSL '<pre>pepXML Viewer<br/>'. $TPPVersionInfo . '<br/></pre>A.Keller  2.23.05</td>';

my $sort_none = ! exists ${$boxptr}{'sort'} || ${$boxptr}{'sort'} eq 'none' ?  $checked : '';
my $sort_xcorr = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xcorr' ? $checked : '';
my $sort_prob = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'prob' ? $checked : '';
my $sort_spec = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spec' ? $checked : '';
my $sort_pep = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'peptide' ? $checked : '';
my $sort_cov = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'coverage' ? $checked : '';
my $sort_pvalue = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'pvalue' ? $checked : '';
my $text1 = exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq '') ? ${$boxptr}{'text1'} : '';
my $glyc = exists ${$boxptr}{'glyc'} && ${$boxptr}{'glyc'} eq 'yes' ? $checked : '';
my $sort_asap_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_desc' ? $checked : '';
my $sort_asap_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_asc' ? $checked : '';
my $filter_asap = exists ${$boxptr}{'filter_asap'} && ${$boxptr}{'filter_asap'} eq 'yes' ? $checked : '';
my $filter_xpress = exists ${$boxptr}{'filter_xpress'} && ${$boxptr}{'filter_xpress'} eq 'yes' ? $checked : '';

$sort_xpress_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_desc' ? $checked : '';
$sort_xpress_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xpress_asc' ? $checked : '';
$sort_asap_desc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_desc' ? $checked : '';
$sort_asap_asc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'asap_asc' ? $checked : '';

my $exclude_degens = exists ${$boxptr}{'no_degens'} && ${$boxptr}{'no_degens'} eq 'yes' ? $checked : '';
my $sort_SEQ_xcorr = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'SEQ_xcorr' ? $checked : '';
my $sort_MAS_ionsc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'MAS_ionsc' ? $checked : '';
my $sort_TAN_hypersc = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'TAN_hypersc' ? $checked : '';
my $sort_TAN_expect = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'TAN_expect' ? $checked : '';
my $sort_PHEN_zscore = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'PHEN_zscore' ? $checked : '';
my $sort_COM_dotprod = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'COM_dotprod' ? $checked : '';

$sort_spec = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spectrum' ? $checked : '';
$sort_pep = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'peptide' ? $checked : '';

# show sens error info (still relevant for filtered dataset)
my $show_sens = exists ${$boxptr}{'senserr'} && ${$boxptr}{'senserr'} eq 'show' ? $checked : '';
my $eligible = ($filter_asap eq '' && $min_asap == 0 && $max_asap == 0 && (! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') && @exclusions == 0 && @inclusions == 0 && @pexclusions == 0 && @pexclusions == 0 && $filter_xpress eq '' && $min_xpress == 0 && $max_xpress == 0 && $asap_xpress eq '');
my $show_tot_num_peps = ! exists ${$boxptr}{'tot_num_peps'} || ${$boxptr}{'tot_num_peps'} eq 'show' ? $checked : '';
my $show_num_unique_peps = ! exists ${$boxptr}{'num_unique_peps'} || ${$boxptr}{'num_unique_peps'} eq 'show' ? $checked : '';

my $suffix = $HTML_ORIENTATION ? '.htm' : '.xml';
$suffix = '.shtml' if($SHTML);

# write output xml
print XSL '<td align="center"><form method="GET" target="Win1" action="' . $CGI_HOME . 'pepxml2html.pl">', "\n";

print XSL '<pre>';
print XSL '<input type="submit" value="Write Displayed Data Subset to File" /><pre>' . $nonbreakline . '</pre>';
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";

print XSL '<input type="hidden" name="ex1" value="yes"/>', "\n" if(! ($exclude_1 eq ''));
print XSL '<input type="hidden" name="ex2" value="yes"/>', "\n" if(! ($exclude_2 eq ''));
print XSL '<input type="hidden" name="ex3" value="yes"/>', "\n" if(! ($exclude_3 eq ''));
print XSL '<input type="hidden" name="ex4" value="yes"/>', "\n" if(! ($exclude_4 eq ''));
print XSL '<input type="hidden" name="text1" value="' . ${$boxptr}{'text1'} . '"/>' if(exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq ''));
print XSL '<input type="hidden" name="min_prob" value="' . $minprob . '"/>' if($minprob > 0);
print XSL '<input type="hidden" name="min_ntt" value="' . $minntt . '"/>' if($minntt > 0);
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
print XSL '<input type="hidden" name="asap_xpress" value="yes"/>' if(! ($asap_xpress eq ''));
# SEQUEST
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SEQUEST\'">';
print XSL '<INPUT TYPE="hidden" NAME="min_SEQ_xcorr" VALUE="', $min_SEQ_xcorr_display, '"/>' if($min_SEQ_xcorr_display);
print XSL '<INPUT TYPE="hidden" NAME="min_SEQ_delta" VALUE="', $min_SEQ_delta_display, '"/>' if($min_SEQ_delta_display);
print XSL '<INPUT TYPE="hidden" NAME="max_SEQ_sprank" VALUE="', $max_SEQ_sprank_display, '"/>' if($max_SEQ_sprank_display);
print XSL '<input type="hidden" name="exclSEQ" value="yes"/>' if($exclude_SEQ);
print XSL '</xsl:if>';
# MASCOT
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'MASCOT\'">';
print XSL '<INPUT TYPE="hidden" NAME="min_MAS_ionscore" VALUE="', $min_MAS_ionscore_display, '"/>' if($min_MAS_ionscore_display);
print XSL '<INPUT TYPE="hidden" NAME="min_MAS_idscore" VALUE="yes"/>' if(! ($min_MAS_idscore_display eq ''));
print XSL '<input type="hidden" name="exclMAS" value="yes"/>' if($exclude_MAS);
print XSL '</xsl:if>';
# Tandem
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'X! Tandem\'">';
print XSL '<INPUT TYPE="hidden" NAME="min_TAN_hyperscore" VALUE="', $min_TAN_hyperscore_display, '"/>' if($min_TAN_hyperscore_display);
print XSL '<INPUT TYPE="hidden" NAME="min_TAN_nextscore" VALUE="', $min_TAN_nextscore_display, '"/>' if($min_TAN_nextscore_display);
print XSL '<INPUT TYPE="hidden" NAME="min_TAN_expectscore" VALUE="', $min_TAN_expectscore_display, '"/>' if($min_TAN_expectscore_display);
print XSL '<input type="hidden" name="exclTAN" value="yes"/>' if($exclude_TAN);
print XSL '</xsl:if>';
# Phenyx
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'PHENYX\'">';
print XSL '<INPUT TYPE="hidden" NAME="min_PHEN_zscore" VALUE="', $min_PHEN_zscore_display, '"/>' if($min_PHEN_zscore_display);
print XSL '<INPUT TYPE="hidden" NAME="min_PHEN_origScore" VALUE="', $min_PHEN_origScore_display, '"/>' if($min_PHEN_origScore_display);
print XSL '<input type="hidden" name="exclPHEN" value="yes"/>' if($exclude_PHEN);
print XSL '</xsl:if>';
# COMET
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'">';
print XSL '<INPUT TYPE="hidden" NAME="min_COM_dotproduct" VALUE="', $min_COM_dotproduct_display, '"/>' if($min_COM_dotproduct_display);
print XSL '<INPUT TYPE="hidden" NAME="min_COM_delta" VALUE="', $min_COM_delta_display, '"/>' if($min_COM_delta_display);
print XSL '<INPUT TYPE="hidden" NAME="min_COM_zscore" VALUE="', $min_COM_zscore_display, '"/>' if($min_COM_zscore_display);
print XSL '<input type="hidden" name="exclCOM" value="yes"/>' if($exclude_COM);
print XSL '</xsl:if>';



print XSL 'file name: <input type="text" name="outfile" value="" size="20" maxlength="100"/>' . $suffix . '</pre>', "\n";
print XSL '</form></td></tr></table>';

if($hide_pep3d eq '') {
    print XSL '<FORM ACTION="' . $CGI_HOME . 'Pep3D_xml.cgi" METHOD="POST" TARGET="Win1">';
    print XSL '<table width="100%" border="3" BGCOLOR="#8FBC6F"><tr><td><pre>';

    print XSL '<input type="hidden" name="mzRange" value="Full"/>';
    print XSL '<input type="hidden" name="mzGrid" value="3"/>';
    print XSL '<input type="hidden" name="mzImgGrid" value="1"/>';
    print XSL '<input type="hidden" name="scanRange" value="Full"/>';
    print XSL '<input type="hidden" name="scanGrid" value="0.25"/>';
    print XSL '<input type="hidden" name="scanImgGrid" value="2"/>';
    print XSL '<input type="hidden" name="peakRange" value="unit of background"/>';
    print XSL '<input type="hidden" name="peakLower" value="1"/>';
    print XSL '<input type="hidden" name="peakUpper" value="20"/>';
    print XSL '<input type="hidden" name="pepDisplay" value="All"/>';
    print XSL '<input type="hidden" name="pepImgGrid" value="2"/>';
    print XSL '<input type="hidden" name="probLower" value="0.5"/>';
    print XSL '<input type="hidden" name="probUpper" value="1.0"/>';
    print XSL '<input type="hidden" name="function" value="Linear"/>';
    print XSL '<input type="hidden" name="image" value="Full"/>';
print XSL '<INPUT TYPE="SUBMIT" name="submit" VALUE="Generate Pep3D image"/>' . $table_spacer . '<INPUT TYPE="SUBMIT" name="submit" VALUE="Save as"/> <input type="text" name="saveFile" size="10" value="Pep3D.htm"/>';
    print XSL $table_spacer;
    print XSL '<input type="hidden" name="display_all" value="yes"/>';
    print XSL '<input type="hidden" name="htmFile" value="' . $xmlfile . '"/>';
    
print XSL '</pre></td></tr></table></FORM>';
} # if not hide




print XSL '<form method="GET" action="' . $CGI_HOME . 'pepxml2html.pl"><table width="100%" border="3" BGCOLOR="#8FBC6F"><tr><td><pre>';

if($discards) {
    print XSL '<input type="submit" value="Filter / Sort / Restore checked entries" />';
}
else {
    print XSL '<input type="submit" value="Filter / Sort / Discard checked entries" />';
}

print XSL $table_spacer . '<xsl:if test="pepx:dataset_derivation/@generation_no &gt;\'0\'"><a target="Win1" href="' . $CGI_HOME . 'show_pepdataset_derivation.pl?xmlfile=' . $xmlfile . '&amp;xslt=' . $xslt . '">Dataset Derivation Info</a></xsl:if>';

print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'more_pepanal.pl?xmlfile=' . $xmlfile . '&amp;xslt=' . $xslt . '&amp;cgi_bin=' . $CGI_HOME;
print XSL '&amp;shtml=yes' if($SHTML);
print XSL '">More Analysis Info</a>';


print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'show_pipeline_help.pl?help_dir=' . $HELP_DIR . '">Help</a>';

### JMT ###
#print XSL $table_spacer . '<a target="Win1" href="' . $CGI_HOME . 'PepXMLViewer.cgi?xmlFileName=' . $xmlfile . '">Open in Beta Viewer</a>';

### JMT ###



print XSL $newline;



print XSL '<xsl:text> </xsl:text>sort by: <input type="radio" name="sort" value="none" ' . $sort_none . '/>index';
print XSL ' <input type="radio" name="sort" value="spectrum" ' . $sort_spec, '/>spectrum';
print XSL ' <input type="radio" name="sort" value="peptide" ' . $sort_pep, '/>peptide';
print XSL ' <input type="radio" name="sort" value="protein" ' . $sort_prot, '/>protein';
print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SEQUEST\'"> <input type="radio" name="sort" value="SEQ_xcorr" ' . $sort_SEQ_xcorr, '/>SEQUEST xcorr</xsl:if>';


print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'MASCOT\'"> <input type="radio" name="sort" value="MAS_ionsc" ' . $sort_MAS_ionsc, '/>MASCOT ionscore</xsl:if>';

print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'X! Tandem\'"> <input type="radio" name="sort" value="TAN_hypersc" ' . $sort_TAN_hypersc, '/>X!Tandem hyperscore</xsl:if>';
print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'X! Tandem\'"> <input type="radio" name="sort" value="TAN_expect" ' . $sort_TAN_expect, '/>X!Tandem expectscore</xsl:if>';


print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'PHENYX\'"> <input type="radio" name="sort" value="PHEN_zscore" ' . $sort_PHEN_zscore, '/>Phenyx zscore</xsl:if>';

print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'"> <input type="radio" name="sort" value="COM_dotprod" ' . $sort_COM_dotprod, '/>COMET dotproduct</xsl:if>';

print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'peptideprophet\'] or pepx:analysis_summary[@analysis=\'xpress\'] or pepx:analysis_summary[@analysis=\'asapratio\']">';
print XSL $newline . '<xsl:text>          </xsl:text>';
print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'peptideprophet\']"> <input type="radio" name="sort" value="prob" ' . $sort_prob, '/>PeptideProphet<sup><small>TM</small></sup> probability</xsl:if>';

print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'xpress\']">';
print XSL ' <input type="radio" name="sort" value="xpress_desc" ' . $sort_xpress_desc, '/>xpress desc';
print XSL ' <input type="radio" name="sort" value="xpress_asc" ' . $sort_xpress_asc, '/>xpress asc';
print XSL '</xsl:if>';

print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'asapratio\']">';
print XSL ' <input type="radio" name="sort" value="asap_desc" ' . $sort_asap_desc, '/>asap desc';
print XSL ' <input type="radio" name="sort" value="asap_asc" ' . $sort_asap_asc, '/>asap asc';
print XSL '</xsl:if>';
print XSL '</xsl:if>';


print XSL $newline;

print XSL '<xsl:text> </xsl:text><xsl:if test="pepx:analysis_summary[@analysis=\'peptideprophet\']">min probability: <INPUT TYPE="text" NAME="min_prob" VALUE="' . $minprob_display . '" SIZE="3" MAXLENGTH="15"/><xsl:text>   </xsl:text></xsl:if>';  
print XSL 'min num tol term: <INPUT TYPE="text" NAME="min_ntt" VALUE="', $minntt_display, '" SIZE="1" MAXLENGTH="1"/><xsl:text>   </xsl:text>';
print XSL '  exclude charge: <input type="checkbox" name="ex1" value="yes" ' . $exclude_1 . '/>1+';
print XSL '<input type="checkbox" name="ex2" value="yes" ' . $exclude_2 . '/>2+';
print XSL '<input type="checkbox" name="ex3" value="yes" ' . $exclude_3 . '/>3+';
print XSL '<input type="checkbox" name="ex4" value="yes" ' . $exclude_4 . '/>others' . '<xsl:text>   </xsl:text>';

print XSL $newline;

# pick one of the following
print XSL $newline;

# now scores and quantitation
# SEQUEST
print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SEQUEST\'"> SEQUEST results: ';
print XSL '  min xcorr: <INPUT TYPE="text" NAME="min_SEQ_xcorr" VALUE="', $min_SEQ_xcorr_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  min deltacn: <INPUT TYPE="text" NAME="min_SEQ_delta" VALUE="', $min_SEQ_delta_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max sprank: <INPUT TYPE="text" NAME="max_SEQ_sprank" VALUE="', $max_SEQ_sprank_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">  exclude: <input type="checkbox" name="exclSEQ" value="yes" ' . $exclude_SEQ . '/></xsl:if>';
print XSL $newline . '</xsl:if>';
# MASCOT
print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'MASCOT\'"> MASCOT results: ';
print XSL '  min ionscore: <INPUT TYPE="text" NAME="min_MAS_ionscore" VALUE="', $min_MAS_ionscore_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  ionscore &gt; identityscore: <input type="checkbox" name="min_MAS_idscore" value="yes" ' . $min_MAS_idscore_display . '/>';
print XSL '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">  exclude: <input type="checkbox" name="exclMAS" value="yes" ' . $exclude_MAS . '/></xsl:if>';
print XSL $newline . '</xsl:if>';
# X! Tandem
print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'X! Tandem\'"> Tandem results: ';
print XSL '  min hyperscore: <INPUT TYPE="text" NAME="min_TAN_hyperscore" VALUE="', $min_TAN_hyperscore_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  min nextscore: <INPUT TYPE="text" NAME="min_TAN_nextscore" VALUE="', $min_TAN_nextscore_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max expectscore: <INPUT TYPE="text" NAME="min_TAN_expectscore" VALUE="', $min_TAN_expectscore_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">  exclude: <input type="checkbox" name="exclTAN" value="yes" ' . $exclude_TAN . '/></xsl:if>';
print XSL $newline . '</xsl:if>';
# Phenyx
print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'PHENYX\'"> PHENYX results: ';
print XSL '  min zscore: <INPUT TYPE="text" NAME="min_PHEN_zscore" VALUE="', $min_PHEN_zscore_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  min origScore: <INPUT TYPE="text" NAME="min_PHEN_origScore" VALUE="', $min_PHEN_origScore_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">  exclude: <input type="checkbox" name="exclPHEN" value="yes" ' . $exclude_PHEN . '/></xsl:if>';
print XSL $newline . '</xsl:if>';
# COMET
print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'"> COMET results: ';
print XSL '  min dotproduct: <INPUT TYPE="text" NAME="min_COM_dotproduct" VALUE="', $min_COM_dotproduct_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  min delta: <INPUT TYPE="text" NAME="min_COM_delta" VALUE="', $min_COM_delta_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  min zscore: <INPUT TYPE="text" NAME="min_COM_zscore" VALUE="', $min_COM_zscore_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">  exclude: <input type="checkbox" name="exclCOM" value="yes" ' . $exclude_COM . '/></xsl:if>';
print XSL $newline . '</xsl:if>';

print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'xpress\']">';
print XSL ' exclude w/o XPRESS Ratio: <INPUT TYPE="checkbox" NAME="filter_xpress" VALUE="yes" ', $filter_xpress, '/>';
print XSL ' min XPRESS Ratio: <INPUT TYPE="text" NAME="min_xpress" VALUE="', $min_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max XPRESS Ratio: <INPUT TYPE="text" NAME="max_xpress" VALUE="', $max_xpress_display, '" SIZE="3" MAXLENGTH="8"/>';

print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'asapratio\']">  ASAPRatio consistent: <input type="checkbox" name="asap_xpress" value="yes" ' . $asap_xpress . '/></xsl:if>';;
print XSL $newline . '</xsl:if>';

print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'asapratio\']">';
print XSL ' exclude w/o ASAP Ratio: <INPUT TYPE="checkbox" NAME="filter_asap" VALUE="yes" ', $filter_asap, '/>';
print XSL ' min ASAP Ratio: <INPUT TYPE="text" NAME="min_asap" VALUE="', $min_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL '  max ASAP Ratio: <INPUT TYPE="text" NAME="max_asap" VALUE="', $max_asap_display, '" SIZE="3" MAXLENGTH="8"/>';
print XSL $newline . '</xsl:if>';

# give show all option only if refresh has been done
print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'database_refresh\']">';
print XSL '<xsl:text> </xsl:text>proteins: <input type="radio" name="show_prots" value="hide" ' . $hide_prots . '/>ref all ';
print XSL '<input type="radio" name="show_prots" value="show" ' . $show_prots . '/>show all  ';
print XSL '</xsl:if>  ';

print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'libra\'] and $libra_count=\'1\'">';
print XSL '<xsl:text> </xsl:text>Libra Quantitation: <input type="radio" name="libra_display" value="absolute" ' . $libra_abs . '/>absolute ';
print XSL '<input type="radio" name="libra_display" value="normalized" ' . $libra_norm . '/>normalized  ';
print XSL $newline . '</xsl:if>';

print XSL '<xsl:text> </xsl:text>search scores: <input type="radio" name="show_search_scores" value="show" ' . $show_scores . '/>show  ';
print XSL '<input type="radio" name="show_search_scores" value="hide" ' . $hide_scores . '/>hide  ';

print XSL '<xsl:text> </xsl:text>Pep3D menu: <input type="radio" name="show_pep3d" value="show" ' . $show_pep3d . '/>show  ';
print XSL '<input type="radio" name="show_pep3d" value="hide" ' . $hide_pep3d . '/>hide  ';

if($score_display eq '' || $score_display eq 'generic') {
    print XSL '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">' . $newline . ' label search scores: generic:<INPUT TYPE="RADIO" NAME="score_display" VALUE="generic" CHECKED="yes"/><xsl:for-each select="pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])]"><xsl:text>  </xsl:text><xsl:variable name="self" select="."/><xsl:value-of select="."/>:<INPUT TYPE="RADIO" NAME="score_display" VALUE="{$self}"/></xsl:for-each></xsl:if>';
print XSL $newline;
}
else { # must check off the designated one
    print XSL '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">' . $newline . ' label search scores: generic:<INPUT TYPE="RADIO" NAME="score_display" VALUE="generic"/><xsl:for-each select="pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])]"><xsl:text>  </xsl:text><xsl:variable name="self" select="."/><xsl:value-of select="."/>:<xsl:if test="$self=\'' . $score_display . '\'"><INPUT TYPE="RADIO" NAME="score_display" VALUE="{$self}" CHECKED="yes"/></xsl:if><xsl:if test="not($self=\'' . $score_display . '\')"><INPUT TYPE="RADIO" NAME="score_display" VALUE="{$self}"/></xsl:if></xsl:for-each></xsl:if>';
print XSL $newline;
}


print XSL '<xsl:text> </xsl:text>include aa: <INPUT TYPE="text" NAME="pep_aa" VALUE="', $pep_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   mark aa: <INPUT TYPE="text" NAME="mark_aa" VALUE="', $mark_aa, '" SIZE="5" MAXLENGTH="15"/>';
print XSL '   NxS/T: <input type="checkbox" name="glyc" value="yes" ' . $glyc . '/><xsl:text>   </xsl:text>';
print XSL 'spectrum text: <input type="text" name="text1" value="', $text1, '" size="12" maxlength="24"/><xsl:text>   </xsl:text>';
print XSL 'export to excel: <input type="checkbox" name="excel" value="yes" />', "\n";
print XSL '<input type="hidden" name="restore" value="no"/>', "\n";
print XSL '<input type="hidden" name="xmlfile" value="' . $xmlfile . '"/>', "\n";
print XSL '<input type="hidden" name="exclusions" value="' . $exclusions . '"/>', "\n";
print XSL '<input type="hidden" name="inclusions" value="' . $inclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pexclusions" value="' . $pexclusions . '"/>', "\n";
print XSL '<input type="hidden" name="pinclusions" value="' . $pinclusions . '"/>', "\n";
print XSL '<input type="hidden" name="icat_mode" value="yes"/>' if($ICAT);
print XSL '<input type="hidden" name="glyc_mode" value="yes"/>' if($GLYC);

if($full_menu) {
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;

    # protein description
    print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein_descr"><xsl:text> </xsl:text>Protein Description: <input type="radio" name="show_prot_descr" value="true" ';
    print XSL $checked if(! ($show_prot_descr eq 'false'));
    print XSL '/>show<input type="radio" name="show_prot_descr" value="false" ';
    print XSL $checked if($show_prot_descr eq 'false');
    print XSL '/>hide';
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    # quantitation info
    print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'xpress\'] or pepx:analysis_summary[@analysis=\'asapratio\']"><xsl:text> </xsl:text>Quantitation Ratio: <input type="radio" name="quant_light2heavy" value="true" ';
    print XSL $checked if(! ($quant_light2heavy eq 'false'));
    print XSL '/>light/heavy<input type="radio" name="quant_light2heavy" value="false" ';
    print XSL $checked if($quant_light2heavy eq 'false');
    print XSL '/>heavy/light';
    print XSL '<pre>' . $newline . '</pre></xsl:if>';


    print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'xpress\']"><xsl:text> </xsl:text>XPRESS information: <input type="radio" name="xpress_display" value="show" ';
    print XSL $checked if(! ($xpress_display eq ''));
    print XSL '/>show<input type="radio" name="xpress_display" value="hide" ';
    print XSL $checked if($xpress_display eq '');
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'asapratio\']"><xsl:text> </xsl:text>ASAPRatio information: <input type="radio" name="asap_display" value="show" ';
    print XSL $checked if(! ($asap_display eq ''));
    print XSL '/>show<input type="radio" name="asap_display" value="hide" ';
    print XSL $checked if($asap_display eq '');
    print XSL '/>hide' . $newline;
    print XSL '<pre>' . $newline . '</pre></xsl:if>';

    print XSL '<xsl:text> </xsl:text>msms search results display   ' . $newline;

    print XSL '<input type="radio" name="order" value="default" /> default', $newline;
    print XSL '<input type="radio" name="order" value="user" checked = "true" /> order desired columns left to right below (i.e. 1,2,3...)', $newline;

    print XSL '<xsl:text> </xsl:text>index <input type="text" name="group_number" value="' . $register_order{'group_number'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'peptideprophet\']"><xsl:text> </xsl:text>PeptideProphet probability <input type="text" name="probability" value="' . $register_order{'probability'} . '" size="2" maxlength="3"/>' . $newline . '</xsl:if>';
    print XSL '<xsl:text> </xsl:text>spectrum <input type="text" name="spectrum" value="' . $register_order{'spectrum'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>search scores <input type="text" name="scores" value="' . $register_order{'scores'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>matched ions <input type="text" name="ions" value="' . $register_order{'ions'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>peptide <input type="text" name="peptide_sequence" value="' . $register_order{'peptide_sequence'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>protein <input type="text" name="protein" value="' . $register_order{'protein'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:if test="$xpress_flag=\'true\'"><xsl:text> </xsl:text>XPRESS ratio <input type="text" name="xpress" value="' . $register_order{'xpress'} . '" size="2" maxlength="3"/>' . $newline . '</xsl:if>';
    print XSL '<xsl:if test="$asapratio_flag=\'true\'"><xsl:text> </xsl:text>ASAPRatio ratio <input type="text" name="asap" value="' . $register_order{'asap'} . '" size="2" maxlength="3"/>' . $newline . '</xsl:if>';
    
    print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'peptideprophet\']"><xsl:text> </xsl:text>PeptideProphet fval <input type="text" name="fval" value="' . $register_order{'fval'} . '" size="2" maxlength="3"/>' . $newline . '</xsl:if>';
    print XSL '<xsl:text> </xsl:text>protein annotation <input type="text" name="annot" value="' . $register_order{'annot'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>ntt <input type="text" name="ntt" value="' . $register_order{'ntt'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>enzyme <input type="text" name="enzyme" value="' . $register_order{'enzyme'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>peptide mass <input type="text" name="peptide_mass" value="' . $register_order{'peptide_mass'} . '" size="2" maxlength="3"/>', $newline;
    print XSL '<xsl:text> </xsl:text>mass diff <input type="text" name="massdiff" value="' . $register_order{'massdiff'} . '" size="2" maxlength="3"/>', $newline;

# SEQUEST
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SEQUEST\'">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL ' SEQUEST score display' . $newline;
    print XSL '<xsl:text> </xsl:text>xcorr <input type="text" name="xcorr" value="' . $SEQ_register_order{'xcorr'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>deltacn <input type="text" name="deltacn" value="' . $SEQ_register_order{'delta'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>sprank <input type="text" name="sprank" value="' . $SEQ_register_order{'sprank'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>spscore <input type="text" name="spscore" value="' . $SEQ_register_order{'spscore'} . '" size="2" maxlength="3"/>';

    print XSL '</xsl:if>';
# MASCOT
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'MASCOT\'">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL ' MASCOT score display' . $newline;
    print XSL '<xsl:text> </xsl:text>ionscore <input type="text" name="ionscore" value="' . $MAS_register_order{'ionscore'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>id score <input type="text" name="idscore" value="' . $MAS_register_order{'idscore'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>homology score <input type="text" name="homologyscore" value="' . $MAS_register_order{'homologyscore'} . '" size="2" maxlength="3"/>' . $newline;

    print XSL '</xsl:if>';
    
    # Tandem
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'X! Tandem\'">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL ' X! Tandem score display (Indicate display order) ' . $newline;
    print XSL '<xsl:text> </xsl:text>hyperscore <input type="text" name="hyperscore" value="' . $TAN_register_order{'hyperscore'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>nextscore <input type="text" name="nextscore" value="' . $TAN_register_order{'nextscore'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>expect <input type="text" name="expect" value="' . $TAN_register_order{'expect'} . '" size="2" maxlength="3"/>' . $newline;

    print XSL '</xsl:if>';
    
# Phenyx
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'PHENYX\'">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL ' PHENYX score display (Indicate display order) ' . $newline;
    print XSL '<xsl:text> </xsl:text>zscore <input type="text" name="zscore" value="' . $PHEN_register_order{'zscore'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>origScore <input type="text" name="origScore" value="' . $PHEN_register_order{'origScore'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '</xsl:if>';
    
# COMET
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL ' COMET score display' . $newline;
    print XSL '<xsl:text> </xsl:text>dotproduct <input type="text" name="dotproduct" value="' . $COM_register_order{'dotproduct'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>delta <input type="text" name="delta" value="' . $COM_register_order{'delta'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>zscore <input type="text" name="zscore" value="' . $COM_register_order{'zscore'} . '" size="2" maxlength="3"/>' . $newline;

    print XSL '</xsl:if>';

# PROBID
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'PROBID\'">';
    print XSL $newline . "---------------------------------------------------------------------------------------------------------" . $newline;
    print XSL ' PROBID score display' . $newline;
    print XSL '<xsl:text> </xsl:text>bays score <input type="text" name="bays" value="' . $PROBID_register_order{'bays'} . '" size="2" maxlength="3"/>' . $newline;
    print XSL '<xsl:text> </xsl:text>zscore <input type="text" name="pid__score" value="' . $COM_register_order{'pid_zscore'} . '" size="2" maxlength="3"/>' . $newline;
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
    print XSL '<pre>' . $nonbreakline . '</pre>' . $newline;
    print XSL ' full menu <input type="checkbox" name="full_menu" value="yes"/>  '; 
    print XSL '      show discarded entries <input type="checkbox" name="discards" value="yes" ' . $discards . '/>      clear manual discards/restores <input type="checkbox" name="clear" value="yes"/>' .$nonbreakline ;

    # hidden information
    print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'xpress\'] or pepx:analysis_summary[@analysis=\'asapratio\']"><input type="hidden" name="quant_light2heavy" value="';
#true" ';
    if($quant_light2heavy eq 'false') {
	print XSL 'false';
    }
    else {
	print XSL 'true';
    }
    print XSL '"/></xsl:if>';


    foreach(keys %register_order) {
	print XSL '<input type="hidden" name="' . $_ . '" value="' . $register_order{$_} . '"/>';
    }
# SEQUEST
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SEQUEST\'">';
    print XSL '<input type="hidden" name="xcorr" value="' . $SEQ_register_order{'xcorr'} . '"/>';
    print XSL '<input type="hidden" name="deltacn" value="' . $SEQ_register_order{'delta'} . '"/>';
    print XSL '<input type="hidden" name="sprank" value="' . $SEQ_register_order{'sprank'} . '"/>';
    print XSL '<input type="hidden" name="spscore" value="' . $SEQ_register_order{'spscore'} . '"/>';

    print XSL '</xsl:if>';
# MASCOT
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'MASCOT\'">';
    print XSL '<input type="hidden" name="ionscore" value="' . $MAS_register_order{'ionscore'} . '"/>';
    print XSL '<input type="hidden" name="idscore" value="' . $MAS_register_order{'idscore'} . '"/>';
    print XSL '<input type="hidden" name="homologyscore" value="' . $MAS_register_order{'homologyscore'} . '"/>';
    print XSL '</xsl:if>';
# Tandem
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'X! Tandem\'">';
    print XSL '<input type="hidden" name="hyperscore" value="' . $TAN_register_order{'hyperscore'} . '"/>';
    print XSL '<input type="hidden" name="nextscore" value="' . $TAN_register_order{'nextscore'} . '"/>';
    print XSL '<input type="hidden" name="expect" value="' . $TAN_register_order{'expect'} . '"/>';
    print XSL '</xsl:if>';
# PHENYX
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'PHENYX\'">';
    print XSL '<input type="hidden" name="zscore" value="' . $PHEN_register_order{'zscore'} . '"/>';
    print XSL '<input type="hidden" name="origScore" value="' . $PHEN_register_order{'origScore'} . '"/>';
    print XSL '</xsl:if>';
# COMET
    print XSL '<xsl:if test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'">';
    print XSL '<input type="hidden" name="dotproduct" value="' . $COM_register_order{'dotproduct'} . '"/>';
    print XSL '<input type="hidden" name="delta" value="' . $COM_register_order{'delta'} . '"/>';
    print XSL '<input type="hidden" name="zscore" value="' . $COM_register_order{'zscore'} . '"/>';
    print XSL '</xsl:if>';


# more here

    print XSL '<input type="hidden" name="order" value="default" />' if(exists ${$boxptr}{'order'} && ${$boxptr}{'order'} eq 'default');
    print XSL '<input type="hidden" name="senserr" value="show"/>' if(! ($show_sens eq ''));
    print XSL '<input type="hidden" name="num_unique_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="tot_num_peps" value="show"/>' if(! ($show_num_unique_peps eq ''));
    print XSL '<input type="hidden" name="xpress_display" value="';
    if($xpress_display eq '') {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";
    print XSL '<input type="hidden" name="asap_display" value="';
    if($asap_display eq '') {
	print XSL 'hide';
    }
    else {
	print XSL 'show';
    }
    print XSL '"/>', "\n";

}

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
	$local_excelfile =~ s/\\/\//g;  # get those path seps pointing right!
    if((length $SERVER_ROOT) <= (length $local_excelfile) && 
       index((lc $local_excelfile), ($LC_SERVER_ROOT)) == 0) {
       $local_excelfile = substr($local_excelfile, (length $SERVER_ROOT));
       if (substr($local_excelfile, 0, 1) ne '/') {
	   $local_excelfile = '/' . $local_excelfile;
       }
    }
    else {
	die "problem (ph6): $local_excelfile is not mounted under webserver root: $SERVER_ROOT\n";
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
  if($USING_LOCALHOST) {
      if((length $SERVER_ROOT) <= (length $local_pngfile) && 
	 index((lc $local_pngfile), ($LC_SERVER_ROOT)) == 0) {
	  $local_pngfile = '/' . substr($local_pngfile, (length $SERVER_ROOT));
      }
      else {
	  die "problem (ph7): $local_pngfile is not mounted under webserver root: $SERVER_ROOT\n";
      }
  } # if iis & cygwin

    print XSL '<xsl:if test="pepx:dataset_derivation/@generation_no=\'0\'">';
    print XSL '<xsl:if test="pepx:analysis_summary[@analysis=\'peptideprophet\']"><font color="blue"> Predicted Total Number of Correct Entries: <xsl:value-of select="pepx:analysis_summary[@analysis=\'peptideprophet\']/pepx:peptideprophet_summary/@est_tot_num_correct"/></font></xsl:if>';
    print XSL "\n\n";
    print XSL "<TABLE><TR><TD>";

    print XSL "<IMG SRC=\"$local_pngfile\"/>";
    print XSL "</TD><TD><PRE>";

    print XSL "<font color=\"red\">sensitivity</font>\tfraction of all correct proteins" . $newline . $tab . $tab . " with probs &gt;= min_prob" . $newline;
    print XSL "<font color=\"green\">error</font>\t\tfraction of all proteins with probs" . $newline . $tab . $tab . " &gt;= min_prob that are incorrect" . $newline . '<pre>' . $newline . '</pre>';

    print XSL 'minprob' . $tab . '<font color="red">sens</font>' . $tab . '<font color="green">err</font>' . $tab . '<font color="red"># corr</font>' . $tab . '<font color ="green"># incorr</font>' . $newline;
    print XSL '<xsl:apply-templates select="protein_summary_header/protein_summary_data_filter">';
    print XSL '<xsl:sort select="@min_probability" order="descending" data-type="number"/>';
    print XSL '</xsl:apply-templates>';

    print XSL '</PRE></TD></TR></TABLE>';
    print XSL $newline . '<pre>' . $newline . '</pre>';
    print XSL '</xsl:if>';
}


########################## COUNT ENTRIES  #################################

my $local_xmlfile = $xmlfile;
my $windows_xmlfile = $xmlfile;
$local_xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
$windows_xmlfile =~ s/\\/\//g;  # get those path seps pointing right!

if($USING_LOCALHOST) {
    if((length $SERVER_ROOT) <= (length $local_xmlfile) && 
       index((lc $local_xmlfile), ($LC_SERVER_ROOT)) == 0) {
	$local_xmlfile = '/' . substr($local_xmlfile, (length $SERVER_ROOT));
    }
    else {
	die "problem (ph8): $local_xmlfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    if($WINDOWS_CYGWIN) {
	$windows_xmlfile = `cygpath -w '$windows_xmlfile'`;
	if($windows_xmlfile =~ /^(\S+)\s?/) {
	    $windows_xmlfile = $1;
	}
    }
} # if iis & cygwin

# show_groups always equal to ''
$show_groups = '';
if((! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') &&
   (! exists ${$boxptr}{'pep_aa'} || ${$boxptr}{'pep_aa'} eq '')){

    print XSL '<font color="red"><xsl:value-of select="';
    if($show_groups eq '') { # count prots
	if($discards) {
	    print XSL 'count(pepx:msms_run_summary/pepx:spectrum_query/pepx:search_result/pepx:search_hit[@hit_rank=\'1\'])-';
	}
#	print XSL 'count(msms_run_summary/spectrum_query/search_result/pepx:search_hit[@hit_rank=\'1\'])+';
	print XSL 'count(pepx:msms_run_summary/pepx:spectrum_query/pepx:search_result/pepx:search_hit[(@hit_rank=\'1\'';
	print XSL ' and @num_tol_term &gt;=\'' . $minntt . '\'' if(! ($minntt eq '') && $minntt > 0);

       print XSL ' and not(parent::node()/pepx:search_summary/@search_engine=\'SEQUEST\')' if(! ($exclude_SEQ eq ''));
	print XSL ' and not(parent::node()/pepx:search_summary/@search_engine=\'MASCOT\')' if(! ($exclude_MAS eq ''));
        print XSL ' and not(parent::node()/pepx:search_summary/@search_engine=\'X! Tandem\')' if(! ($exclude_TAN eq ''));
        print XSL ' and not(parent::node()/pepx:search_summary/@search_engine=\'PHENYX\')' if(! ($exclude_PHEN eq ''));
	print XSL ' and not(parent::node()/pepx:search_summary/@search_engine=\'COMET\')' if(! ($exclude_COM eq ''));

	print XSL ' and not(parent::node()/@assumed_charge=\'1\')' if(! ($exclude_1 eq ''));
	print XSL ' and not(parent::node()/@assumed_charge=\'2\')' if(! ($exclude_2 eq ''));
	print XSL ' and not(parent::node()/@assumed_charge=\'3\')' if(! ($exclude_3 eq ''));
	print XSL ' and not(parent::node()/@assumed_charge &gt;\'3\')' if(! ($exclude_4 eq ''));
	
	print XSL ' and pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability &gt;= \'' . $minprob . '\'' if($minprob > 0);

	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'SEQUEST\') or pepx:search_score[@name=\'xcorr\']/@value &gt;= \'' . $min_SEQ_xcorr_display . '\')' if($min_SEQ_xcorr_display > 0);
	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'SEQUEST\') or pepx:search_score[@name=\'deltacn\']/@value &gt;= \'' . $min_SEQ_delta_display . '\')' if(! ($min_SEQ_delta_display eq ''));
	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'SEQUEST\') or pepx:search_score[@name=\'sprank\']/@value &lt;= \'' . $max_SEQ_sprank_display . '\')' if(! ($max_SEQ_sprank_display eq ''));

	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'MASCOT\') or pepx:search_score[@name=\'ionscore\']/@value &gt;= \'' . $min_MAS_ionscore_display . '\')' if($min_MAS_ionscore_display > 0);
	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'MASCOT\') or pepx:search_score[@name=\'ionscore\']/@value &gt; pepx:search_score[@name=\'identityscore\']/@value)' if(! ($min_MAS_idscore_display eq ''));

	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'X! Tandem\') or pepx:search_score[@name=\'hyperscore\']/@value &gt;= \'' . $min_TAN_hyperscore_display . '\')' if($min_TAN_hyperscore_display > 0);
        print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'X! Tandem\') or pepx:search_score[@name=\'nextscore\']/@value &gt;= \'' . $min_TAN_nextscore_display . '\')' if(! ($min_TAN_nextscore_display eq ''));
        print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'X! Tandem\') or pepx:search_score[@name=\'expect\']/@value &lt;= \'' . $min_TAN_expectscore_display . '\')' if(! ($min_TAN_expectscore_display eq ''));

        print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'PHENYX\') or pepx:search_score[@name=\'zscore\']/@value &gt;= \'' . $min_PHEN_zscore_display . '\')' if($min_PHEN_zscore_display > 0);
        print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'PHENYX\') or pepx:search_score[@name=\'origScore\']/@value &gt;= \'' . $min_PHEN_origScore_display . '\')' if(! ($min_PHEN_origScore_display eq ''));

	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'COMET\') or pepx:search_score[@name=\'dotproduct\']/@value &gt;= \'' . $min_COM_dotproduct_display . '\')' if($min_COM_dotproduct_display > 0);
	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'COMET\') or pepx:search_score[@name=\'delta\']/@value &gt;= \'' . $min_COM_delta_display . '\')' if($min_COM_delta_display > 0);
	print XSL ' and (not(parent::node()/parent::node()/parent::node()/pepx:search_summary/@search_engine=\'COMET\') or pepx:search_score[@name=\'zscore\']/@value &gt;= \'' . $min_COM_zscore_display . '\')' if($min_COM_zscore_display > 0);



	print XSL ' and (pepx:analysis_result[@analysis=\'xpress\'] and pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and (pepx:analysis_result[@analysis=\'asapratio\'] and pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean &gt;= \'0\')' if(! ($filter_asap eq ''));


	if($quant_light2heavy eq 'true') {
	    print XSL ' and (pepx:analysis_result[@analysis=\'xpress\'] and pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	    print XSL ' and (pepx:analysis_result[@analysis=\'xpress\'] and pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	}
	else { #reverse
	    print XSL ' and (pepx:analysis_result[@analysis=\'xpress\'] and pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;= \'' . 1.0 / $min_xpress . '\')' if($min_xpress > 0);
	    print XSL ' and (pepx:analysis_result[@analysis=\'xpress\'] and pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= \'' . 1.0 / $max_xpress . '\')' if($max_xpress > 0);
	}

	print XSL ' and (pepx:analysis_result[@analysis=\'asapratio\'] and pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' and (pepx:analysis_result[@analysis=\'asapratio\'] and pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' and (not(pepx:analysis_result[@analysis=\'xpress\']) or not(pepx:analysis_result[@analysis=\'asapratio\']) or (pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;= pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean + pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error and pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean - asapratio_result/@error))' if(! ($asap_xpress eq ''));


	for(my $e = 0; $e < @exclusions; $e++) {
	    print XSL ' and not(parent::node()/@index=\'' . $exclusions[$e] . '\')';
	}
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' and not(parent::node()/@index=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

	print XSL ')';

	for(my $i = 0; $i < @inclusions; $i++) {
	    print XSL ' or (parent::node()/@index=\'' . $inclusions[$i] . '\'';
	    foreach(@pexclusions) {
		if(/^(\d+)([a-z,A-Z])$/) {
		    print XSL ' and not(parent::node()/@index=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
		}
	    }
	    print XSL ')';
	}
	foreach(@pinclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' or (parent::node()/@index=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

    } # hide groups eq ''
    print XSL ']';
    print XSL ')"/>';
    print XSL '<font color="black"><i> discarded</i></font>' if($discards);
    if($WINDOWS_CYGWIN) {
	print XSL " entries retrieved from <A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; 
}
    else {
	print XSL " entries retrieved from <A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; 
    }
} # if count
else {

    print XSL '<font color="black"><i>discarded</i></font> ' if($discards);
    if($WINDOWS_CYGWIN) {
	print XSL "<font color=\"red\">entries retrieved from <A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; 
    }
    else {
	print XSL "<font color=\"red\">entries retrieved from <A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; 
    }

}

###################################################



print XSL $newline . '<pre>' . $newline . '</pre>';

# calculate how many columns, and header line here

$num_cols += 8;
my $extra_column = '<td>' . $table_spacer . '</td>';

my $HEADER = '<td><!-- header --></td>';

# now comes peptide info....

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
		$HEADER .= $header{$_}; # . '<xsl:text>  </xsl:text>';
	    }
	}
    }


$HEADER .= $extra_column if(! $tab_delim);



print XSL $RESULT_TABLE_PRE . $RESULT_TABLE, "\n";


print XSL '<xsl:comment>' . $start_string . '</xsl:comment>' . $newline . "\n";;


$header{'generic_scores'} = '<td><font color="brown"><b>search scores</b></font></td>';

$anotherheader{'scores'} = '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])=\'1\'"><xsl:choose><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SEQUEST\'">' . $header{'sequest_scores'} . '</xsl:when><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'MASCOT\'">' . $header{'mascot_scores'} . '</xsl:when><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'X! Tandem\'">' . $header{'tandem_scores'} . '</xsl:when><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'">' . $header{'comet_scores'} . '</xsl:when><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'PROBID\'">' . $header{'probid_scores'} . '</xsl:when><xsl:otherwise><font color="brown"><b>search scores</b></font></xsl:otherwise></xsl:choose></xsl:if><xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">' . $header{'generic_scores'} . '</xsl:if>';


if(! $tab_delim) {
    print XSL '<tr>';
}


$tab_header{'scores'} = $tab_header{'sequest_scores'};
$tab_header{'scores'} .= $tab_header{'mascot_scores'};
$tab_header{'scores'} .= $tab_header{'tandem_scores'};
$tab_header{'scores'} .= $tab_header{'comet_scores'};
$tab_header{'scores'} .= $tab_header{'probid_scores'};
# append mascot now....



if($score_display eq 'generic' || $score_display eq '') {
    $header{'scores'} = '<td>' . $RESULT_TABLE_PRE . $RESULT_TABLE . '<TR>' . $anotherheader{'scores'} . '</TR>' . $RESULT_TABLE_SUF . '</td>';
}
elsif($score_display eq 'SEQUEST') {
    $header{'scores'} = '<td>' . $RESULT_TABLE_PRE . $RESULT_TABLE . '<TR>' . $header{'sequest_scores'} . '</TR>' . $RESULT_TABLE_SUF . '</td>';
}
elsif($score_display eq 'MASCOT') {
    $header{'scores'} = '<td>' . $RESULT_TABLE_PRE . $RESULT_TABLE . '<TR>' . $header{'mascot_scores'} . '</TR>' . $RESULT_TABLE_SUF . '</td>';
}
elsif($score_display eq 'X! Tandem') {
    $header{'scores'} = '<td>' . $RESULT_TABLE_PRE . $RESULT_TABLE . '<TR>' . $header{'tandem_scores'} . '</TR>' . $RESULT_TABLE_SUF . '</td>';
}
elsif($score_display eq 'COMET') {
    $header{'scores'} = '<td>' . $RESULT_TABLE_PRE . $RESULT_TABLE . '<TR>' . $header{'comet_scores'} . '</TR>' . $RESULT_TABLE_SUF . '</td>';
}


if(scalar keys %display_order > 0) {
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	if($tab_delim >= 1) {
	    print XSL $tab_header{$_} if(! ($_ eq 'scores') || ! ($show_scores eq ''));
	}
	else {
	    print XSL $header{$_} if(! ($_ eq 'scores') || ! ($show_scores eq ''));
	}
    }
}
else { # use default
    foreach(sort {$default_order{$a} <=> $default_order{$b}} keys %default_order) {
	if($tab_delim >= 1) {
	    print XSL $tab_header{$_} if(! ($_ eq 'scores') || ! ($show_scores eq ''));
	}
	else {
	    print XSL $header{$_} if(! ($_ eq 'scores') || ! ($show_scores eq ''));
	}
    }
}



if($tab_delim >= 1) {
    print XSL $newline;
}
else {
    print XSL '</tr>';
}
if(! ($sort_SEQ_xcorr eq '') || ! ($sort_MAS_ionsc eq '') || ! ($sort_TAN_hypersc eq '') || ! ($sort_TAN_expect eq '') ||! ($sort_PHEN_zscore eq '')||! ($sort_COM_dotprod eq '') || 
   ! ($sort_xpress_desc eq '') || ! ($sort_xpress_asc eq '') || ! ($sort_asap_desc eq '') || 
   ! ($sort_asap_asc eq '')|| ! ($sort_prob eq '') || ! ($sort_prot eq '') || ! ($sort_pep eq '') || ! ($sort_spec eq '')) {

 print XSL "\n", '

    <xsl:if test="$run_count!=\'1\'">
         <xsl:apply-templates select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:spectrum_query">
		     <xsl:with-param name="summaryxml" select="$summaryxml"/>
                     <xsl:with-param name="pepproph_flag" select="$pepproph_flag"/>
		     <xsl:with-param name="asapratio_flag" select="$asapratio_flag"/>
		     <xsl:with-param name="xpress_flag" select="$xpress_flag"/>	', "\n";
 if(! ($sort_SEQ_xcorr) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'xcorr\'])" order="descending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'xcorr\']/@value" order="descending" data-type="number"/>', "\n";
     
 }
 elsif(! ($sort_MAS_ionsc) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\'])" order="descending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\']/@value" order="descending" data-type="number"/>', "\n";
 }
 elsif(! ($sort_TAN_hypersc) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'hyperscore\'])" order="descending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'hyperscore\']/@value" order="descending" data-type="number"/>', "\n";
 }
 elsif(! ($sort_TAN_expect) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'expect\'])" order="ascending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'expect\']/@value" order="ascending" data-type="number"/>', "\n";
 }
 elsif(! ($sort_PHEN_zscore) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\'])" order="descending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\']/@value" order="descending" data-type="number"/>', "\n";
 }
 elsif(! ($sort_COM_dotprod) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'dotproduct\'])" order="descending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'dotproduct\']/@value" order="descending" data-type="number"/>', "\n";
 }
 elsif(! ($sort_xpress_desc) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'])" order="descending" data-type="number"/>', "\n";
     
     if($quant_light2heavy eq 'true') {
	 
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="descending" data-type="number"/>', "\n";
     }
     else { # reverse
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="ascending" data-type="number"/>', "\n";
     }
 }
 elsif(! ($sort_xpress_asc) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'])" order="descending" data-type="number"/>', "\n";
     if($quant_light2heavy eq 'true') {
	 
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="ascending" data-type="number"/>', "\n";
     }
     else { #reverse
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="descending" data-type="number"/>', "\n";
     }
 }
 elsif(! ($sort_asap_desc) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'])" order="descending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean" order="descending" data-type="number"/>', "\n";
     
 }
 elsif(! ($sort_asap_asc) eq '') {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'])" order="descending" data-type="number"/>', "\n";
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean" order="ascending" data-type="number"/>', "\n";
     
 }
 elsif(! ($sort_prob eq '')) {
     print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'])" order="descending" data-type="number"/>';
     print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability" order="descending" data-type="number"/>', "\n";
 }
 elsif(! ($sort_prot eq '')) {
     if($show_groups eq '') {
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein"/>', "\n";
     }
 }
 elsif(! ($sort_pep eq '')) {
     if($show_groups eq '') {
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>', "\n";
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/>', "\n";
	 print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/>', "\n";
     }
 }
 elsif(! ($sort_spec eq '')) {
     if($show_groups eq '') {
	 print XSL '<xsl:sort select="@spectrum"/>', "\n";
     }
 }
 
 print XSL '</xsl:apply-templates>', "\n";

 
 print XSL '</xsl:if>', "\n";
 print XSL ' <xsl:if test="$run_count=\'1\'">', "\n";
 print XSL "\n", '<xsl:apply-templates select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary">
		     <xsl:with-param name="summaryxml" select="$summaryxml"/>
                     <xsl:with-param name="pepproph_flag" select="$pepproph_flag"/>
		     <xsl:with-param name="asapratio_flag" select="$asapratio_flag"/>
		     <xsl:with-param name="xpress_flag" select="$xpress_flag"/>	', "\n";
 print XSL '</xsl:apply-templates>', "\n";							
 print XSL '</xsl:if>', "\n";


}
else {
    print XSL "\n", '<xsl:apply-templates select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary">
		     <xsl:with-param name="summaryxml" select="$summaryxml"/>
                     <xsl:with-param name="pepproph_flag" select="$pepproph_flag"/>
		     <xsl:with-param name="asapratio_flag" select="$asapratio_flag"/>
		     <xsl:with-param name="xpress_flag" select="$xpress_flag"/>	', "\n";
    print XSL '</xsl:apply-templates>', "\n";							
}


print XSL $RESULT_TABLE_SUF, "\n";
print XSL '</form>';
print XSL '</PRE></BODY></HTML>', "\n";
print XSL '</xsl:template>', "\n";

print XSL  '<xsl:template match="pepx:msms_run_summary">	
		<xsl:param name="pepproph_flag"/>
		<xsl:param name="asapratio_flag"/>
		<xsl:param name="xpress_flag"/>
		<xsl:param name="summaryxml"/>
				
		<xsl:variable name="asap_time" select="pepx:analysis_timestamp[@analysis=\'asapratio\']/@time" />
		<xsl:variable name="basename" select="@base_name"/>
		<xsl:variable name="enzyme" select="pepx:sample_enzyme/@name" />	
		<xsl:variable name="xpress_display" select="pepx:analysis_timestamp[@analysis=\'xpress\']/pepx:xpressratio_timestamp/@xpress_light" />
		<xsl:variable name="pepproph_timestamp" select="pepx:analysis_timestamp[@analysis=\'peptideprophet\']/@time" />
		<xsl:variable name="search_engine" select="pepx:search_summary/@search_engine"/>
        
        <xsl:variable name="dbrefresh_flag">
			<xsl:choose>
				<xsl:when test="pepx:analysis_timestamp[@analysis=\'database_refresh\']">
					<xsl:value-of select="true()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="false()"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<xsl:variable name="Database">
			<xsl:choose>
				<xsl:when test="pepx:analysis_timestamp[@analysis=\'database_refresh\']">
					<xsl:value-of select="pepx:analysis_timestamp[@analysis=\'database_refresh\']/pepx:database_refresh_timestamp/@database" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="pepx:search_summary/pepx:search_database/@local_path" />
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		
		<xsl:variable name="minntt">
			<xsl:choose>
				<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint and pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=pepx:sample_enzyme/@name">
					<xsl:value-of select="pepx:search_summary/pepx:enzymatic_search_constraint/@min_number_termini" />
				</xsl:when>
				<xsl:otherwise>0</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		
		<xsl:variable name="comet_md5_check_sum">
			<xsl:if test="pepx:search_summary/@search_engine=\'COMET\'">
				<xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'md5_check_sum\']/@value" />
			</xsl:if>
		</xsl:variable>
		
		<xsl:variable name="aa_mods">
			<xsl:for-each select="pepx:search_summary/pepx:aminoacid_modification"><xsl:value-of select="@aminoacid" />
				<xsl:if test="@symbol">
					<xsl:value-of select="@symbol" />
				</xsl:if>-
				<xsl:value-of select="@mass" />:
			</xsl:for-each>
		</xsl:variable>
		
		<xsl:variable name="term_mods">
			<xsl:for-each select="pepx:search_summary/pepx:terminal_modification"><xsl:value-of select="@terminus" />
				<xsl:if test="@symbol">
					<xsl:value-of select="@symbol" />
				</xsl:if>-
				<xsl:value-of select="@mass" />:
			</xsl:for-each>
		</xsl:variable>
		
		<xsl:variable name="masstype">
			<xsl:choose>
				<xsl:when test="pepx:search_summary/@precursor_mass_type=\'average\'">0</xsl:when>
			    <xsl:otherwise>1</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>		
		
		<xsl:apply-templates select="pepx:spectrum_query">
		        <xsl:with-param name="summaryxml" select="$summaryxml"/>
			<xsl:with-param name="basename" select="$basename"/>
			<xsl:with-param name="enzyme" select="$enzyme"/>
			<xsl:with-param name="aa_mods" select="$aa_mods"/>
			<xsl:with-param name="term_mods" select="$term_mods"/>
			<xsl:with-param name="xpress_display" select="$xpress_display"/>
			<xsl:with-param name="asap_time" select="$asap_time"/>
			<xsl:with-param name="Database" select="$Database"/>
			<xsl:with-param name="minntt" select="$minntt"/>
			<xsl:with-param name="masstype" select="$masstype"/>
			<xsl:with-param name="search_engine" select="$search_engine"/>
			<xsl:with-param name="pepproph_timestamp" select="$pepproph_timestamp"/>
			<xsl:with-param name="comet_md5_check_sum" select="$comet_md5_check_sum"/>
			<xsl:with-param name="pepproph_flag" select="$pepproph_flag"/>
			<xsl:with-param name="asapratio_flag" select="$asapratio_flag"/>
			<xsl:with-param name="xpress_flag" select="$xpress_flag"/>	
			<xsl:with-param name="dbrefresh_flag" select="$dbrefresh_flag"/>', "\n"	;


if(! ($sort_SEQ_xcorr) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'xcorr\'])" order="descending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'xcorr\']/@value" order="descending" data-type="number"/>', "\n";
    
}
elsif(! ($sort_MAS_ionsc) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\'])" order="descending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\']/@value" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_TAN_hypersc) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'hyperscore\'])" order="descending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'hyperscore\']/@value" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_TAN_expect) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'expect\'])" order="ascending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'expect\']/@value" order="ascending" data-type="number"/>', "\n";
}
elsif(! ($sort_PHEN_zscore) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\'])" order="descending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\']/@value" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_COM_dotprod) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'dotproduct\'])" order="descending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'dotproduct\']/@value" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_xpress_desc) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'])" order="descending" data-type="number"/>', "\n";
    
    if($quant_light2heavy eq 'true') {
	
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="descending" data-type="number"/>', "\n";
    }
    else { # reverse
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="ascending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_xpress_asc) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'])" order="descending" data-type="number"/>', "\n";
    if($quant_light2heavy eq 'true') {
	
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="ascending" data-type="number"/>', "\n";
    }
    else { #reverse
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio" order="descending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_asap_desc) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'])" order="descending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean" order="descending" data-type="number"/>', "\n";
    
}
elsif(! ($sort_asap_asc) eq '') {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'])" order="descending" data-type="number"/>', "\n";
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean" order="ascending" data-type="number"/>', "\n";
    
}
elsif(! ($sort_prob eq '')) {
    print XSL '<xsl:sort select="count(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'])" order="descending" data-type="number"/>';
    print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability" order="descending" data-type="number"/>', "\n";
}
elsif(! ($sort_prot eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein"/>', "\n";
    }
}
elsif(! ($sort_pep eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>', "\n";
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/>', "\n";
	print XSL '<xsl:sort select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/>', "\n";
    }
}
elsif(! ($sort_spec eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@spectrum"/>', "\n";
    }
}

print XSL '</xsl:apply-templates>', "\n";
print XSL '</xsl:template>', "\n";


print XSL '<xsl:template match="pepx:spectrum_query">', "\n";

print XSL '<xsl:param name="enzyme">
			<xsl:value-of select="parent::node()/pepx:sample_enzyme/@name" />
		</xsl:param>
		<xsl:param name="basename">
			<xsl:value-of select="parent::node()/@base_name" />
		</xsl:param>
		<xsl:param name="pepproph_timestamp">
			<xsl:value-of select="parent::node()/pepx:analysis_timestamp[@analysis=\'peptideprophet\']/@time" />
		</xsl:param>
		<xsl:param name="xpress_display">
			<xsl:value-of select="parent::node()/pepx:analysis_timestamp[@analysis=\'xpress\']/pepx:xpressratio_timestamp/@xpress_light" />
		</xsl:param>
		<xsl:param name="search_engine">
			<xsl:value-of select="parent::node()/pepx:search_summary/@search_engine" />
		</xsl:param>
		<xsl:param name="dbrefresh_flag">
			<xsl:choose>
				<xsl:when test="parent::node()/pepx:analysis_timestamp[@analysis=\'database_refresh\']">
					<xsl:value-of select="true()" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="false()" />
				</xsl:otherwise>
			</xsl:choose>
		</xsl:param>
		<xsl:param name="Database">		
			<xsl:choose>
				<xsl:when test="parent::node()/pepx:analysis_timestamp[@analysis=\'database_refresh\']">
					<xsl:value-of select="parent::node()/pepx:analysis_timestamp[@analysis=\'database_refresh\']/pepx:database_refresh_timestamp/@database" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="parent::node()/pepx:search_summary/pepx:search_database/@local_path" />
				</xsl:otherwise>
			</xsl:choose>
		</xsl:param>
		<xsl:param name="minntt">
			<xsl:choose>
				<xsl:when test="parent::node()/pepx:search_summary/pepx:enzymatic_search_constraint and parent::node()/pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=parent::node()/pepx:sample_enzyme/@name">
					<xsl:value-of select="parent::node()/pepx:search_summary/pepx:enzymatic_search_constraint/@min_number_termini" />
				</xsl:when>
				<xsl:otherwise>0</xsl:otherwise>
			</xsl:choose>
		</xsl:param>
		<xsl:param name="comet_md5_check_sum">
			<xsl:if test="parent::node()/pepx:search_summary/@search_engine=\'COMET\'">
				<xsl:value-of select="parent::node()/pepx:search_summary/pepx:parameter[@name=\'md5_check_sum\']/@value" />
			</xsl:if>
		</xsl:param>
		<xsl:param name="aa_mods">
			<xsl:for-each select="parent::node()/pepx:search_summary/pepx:aminoacid_modification"><xsl:value-of select="@aminoacid" />
				<xsl:if test="@symbol">
					<xsl:value-of select="@symbol" />
				</xsl:if>-
				<xsl:value-of select="@mass" />:
			</xsl:for-each>
		</xsl:param>
		<xsl:param name="term_mods">			
			<xsl:for-each select="parent::node()/pepx:search_summary/pepx:terminal_modification"><xsl:value-of select="@terminus" />
				<xsl:if test="@symbol">
					<xsl:value-of select="@symbol" />
				</xsl:if>-
				<xsl:value-of select="@mass" />:
			</xsl:for-each>
		</xsl:param>
		<xsl:param name="asap_time">
			<xsl:value-of select="parent::node()/pepx:analysis_timestamp[@analysis=\'asapratio\']/@time" />
		</xsl:param>
		<xsl:param name="masstype">
			<xsl:choose>
				<xsl:when test="parent::node()/pepx:search_summary/@precursor_mass_type=\'average\'">0</xsl:when>
				<xsl:otherwise>1</xsl:otherwise>
			</xsl:choose>
		</xsl:param>		
		
		
		<xsl:param name="pepproph_flag" />
		<xsl:param name="asapratio_flag" />
		<xsl:param name="xpress_flag" />
		<xsl:param name="summaryxml" />                 
                <xsl:variable name="pI_flag">
			<xsl:choose>
				<xsl:when test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'pI\']">
					<xsl:value-of select="true()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="false()"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>', "\n";

print XSL '<xsl:variable name="light_first_scan" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@light_firstscan"/>';
print XSL '<xsl:variable name="light_last_scan" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@light_lastscan"/>';
print XSL '<xsl:variable name="heavy_first_scan" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy_firstscan"/>';
print XSL '<xsl:variable name="heavy_last_scan" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy_lastscan"/>';
print XSL '<xsl:variable name="xpress_charge" select="@assumed_charge"/>';

print XSL '<xsl:variable name="LightMass" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@light_mass"/>';
print XSL '<xsl:variable name="HeavyMass" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy_mass"/>';
print XSL '<xsl:variable name="MassTol" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@mass_tol"/>';
print XSL '<xsl:variable name="PpmTol" select="parent::node()/parent::node()/pepx:analysis_summary[@analysis=\'xpress\']/pepx:xpressratio_summary/@ppmtol"/>';
print XSL '<xsl:variable name="xpress_index" select="@index"/>';
print XSL '<xsl:variable name="xpress_spec" select="@spectrum"/>';

print XSL '<xsl:variable name="index" select="@index"/>';

print XSL '<xsl:variable name="Peptide" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/>';
if($DISPLAY_MODS) {
    print XSL '<xsl:variable name="StrippedPeptide" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/>';
}
else {
    print XSL '<xsl:variable name="StrippedPeptide" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@stripped_peptide"/>';
}
print XSL '<xsl:variable name="Protein" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein"/>';

print XSL '<xsl:variable name="calc_pI" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_pI"/>';
print XSL '<xsl:variable name="pep_mass" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass"/>';
print XSL '<xsl:variable name="PeptideMods"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass"/>]</xsl:if><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/pepx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each></xsl:if></xsl:variable>';

print XSL '<xsl:variable name="PeptideMods2"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass">ModN=<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass"/>&amp;</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass">ModC=<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass"/>&amp;</xsl:if><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/pepx:mod_aminoacid_mass">Mod<xsl:value-of select="@position"/>=<xsl:value-of select="@mass"/>&amp;</xsl:for-each></xsl:if></xsl:variable>';

print XSL '<xsl:variable name="prob"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis=\'adjusted\'">a</xsl:if></xsl:variable>';

# alternative for parameters...
print XSL '<xsl:variable name="scores"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary"><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter"><xsl:value-of select="@name"/>:<xsl:value-of select="@value"/><xsl:text> </xsl:text></xsl:for-each></xsl:if></xsl:variable>';
  

print XSL '<xsl:variable name="asap_quantHighBG">';
print XSL '			<xsl:choose>';
print XSL '				<xsl:when test="parent::node()/parent::node()/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'quantHighBG\']">';
print XSL '			                      <xsl:choose>';
print XSL '				                      <xsl:when test="parent::node()/parent::node()/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@value=\'True\']">';
print XSL '					                       <xsl:value-of select="1"/>';
print XSL '				                      </xsl:when>';
print XSL '				                      <xsl:otherwise>';
print XSL '					                       <xsl:value-of select="0"/>';
print XSL '				                      </xsl:otherwise>';
print XSL '			                       </xsl:choose>'; 
print XSL '				</xsl:when>';
print XSL '				<xsl:otherwise>';
print XSL '					<xsl:value-of select="0"/>';
print XSL '				</xsl:otherwise>';
print XSL '			</xsl:choose>';
print XSL '		</xsl:variable>';

print XSL '<xsl:variable name="asap_zeroBG">';
print XSL '			<xsl:choose>';
print XSL '				<xsl:when test="parent::node()/parent::node()/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'zeroBG\']">';
print XSL '			                      <xsl:choose>';
print XSL '				                      <xsl:when test="parent::node()/parent::node()/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@value=\'True\']">';
print XSL '					                       <xsl:value-of select="1"/>';
print XSL '				                      </xsl:when>';
print XSL '				                      <xsl:otherwise>';
print XSL '					                       <xsl:value-of select="0"/>';
print XSL '				                      </xsl:otherwise>';
print XSL '			                       </xsl:choose>'; 
print XSL '				</xsl:when>';
print XSL '				<xsl:otherwise>';
print XSL '					<xsl:value-of select="0"/>';
print XSL '				</xsl:otherwise>';
print XSL '			</xsl:choose>';
print XSL '		</xsl:variable>';
 
print XSL '<xsl:variable name="asap_mzBound">';
print XSL '			<xsl:choose>';
print XSL '				<xsl:when test="parent::node()/parent::node()/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'mzBound\']">';
print XSL '				       <xsl:value-of select="parent::node()/parent::node()/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'mzBound\']/@value"/>';
print XSL '				</xsl:when>';
print XSL '				<xsl:otherwise>';
print XSL '					<xsl:value-of select="0.5"/>';
print XSL '				</xsl:otherwise>';
print XSL '			</xsl:choose>';
print XSL '		</xsl:variable>';



# place all the if's here......

# must add the inclusions/exclusions here too
$suffix = '';
if(@inclusions > 0) {
    $suffix = ' or @index=\'';
    for(my $i = 0; $i < @inclusions; $i++) {
	$suffix .= $inclusions[$i] . '\'';
	$suffix .= ' or @index=\'' if($i < @inclusions - 1);
    }
}

foreach(keys %parent_incls) {
    $suffix .= ' or @index=\'' . $_ . '\'';
}    

if($discards) {
    print XSL '<xsl:if test="(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability &lt;\'' . $minprob . '\')';
    print XSL ' or not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'])' if($minprob > 0);

    print XSL ' or $search_engine=\'SEQUEST\'' if(! ($exclude_SEQ eq ''));
    print XSL ' or $search_engine=\'MASCOT\'' if(! ($exclude_MAS eq ''));
    print XSL ' or $search_engine=\'X! Tandem\'' if(! ($exclude_TAN eq ''));
       print XSL ' or $search_engine=\'PHENYX\'' if(! ($exclude_PHEN eq ''));
    print XSL ' or $search_engine=\'COMET\'' if(! ($exclude_COM eq ''));

    print XSL ' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tol_term &lt;\'' . $minntt . '\'' if(! ($minntt eq '') && $minntt > 0);
    print XSL ' or @assumed_charge=\'1\'' if(! ($exclude_1 eq ''));
    print XSL ' or @assumed_charge=\'2\'' if(! ($exclude_2 eq ''));
    print XSL ' or @assumed_charge=\'3\'' if(! ($exclude_3 eq ''));
    print XSL ' or @assumed_charge &gt; \'3\'' if(! ($exclude_4 eq ''));
    print XSL ' or($search_engine=\'SEQUEST\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'xcorr\']/@value &lt; \'' . $min_SEQ_xcorr_display . '\')' if($min_SEQ_xcorr_display > 0);
    print XSL ' or($search_engine=\'SEQUEST\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'deltacn\']/@value &lt; \'' . $min_SEQ_delta_display . '\')' if(! ($min_SEQ_delta_display eq ''));
    print XSL ' or($search_engine=\'SEQUEST\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'sprank\']/@value &gt; \'' . $max_SEQ_sprank_display . '\')' if(! ($max_SEQ_sprank_display eq ''));

    print XSL ' or($search_engine=\'MASCOT\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\']/@value &lt; \'' . $min_MAS_ionscore_display . '\')' if($min_MAS_ionscore_display > 0);
    print XSL ' or($search_engine=\'MASCOT\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\']/@value &gt; pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'identityscore\']/@value)' if(! ($min_MAS_idscore_display eq ''));

    print XSL ' or($search_engine=\'X! Tandem\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'hyperscore\']/@value &lt; \'' . $min_TAN_hyperscore_display . '\')' if($min_TAN_hyperscore_display > 0);
    print XSL ' or($search_engine=\'X! Tandem\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'nextscore\']/@value &lt;= \'' . $min_TAN_nextscore_display . '\')' if(! ($min_TAN_nextscore_display eq ''));
    print XSL ' or($search_engine=\'X! Tandem\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'expect\']/@value &gt;= \'' . $min_TAN_expectscore_display . '\')' if(! ($min_TAN_expectscore_display eq ''));

    print XSL ' or($search_engine=\'PHENYX\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\']/@value &lt; \'' . $min_PHEN_zscore_display . '\')' if($min_PHEN_zscore_display > 0);
    print XSL ' or($search_engine=\'PHENYX\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'origScore\']/@value &lt;= \'' . $min_PHEN_origScore_display . '\')' if(! ($min_PHEN_origScore_display eq ''));    

    print XSL ' or($search_engine=\'COMET\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'dotproduct\']/@value &lt; \'' . $min_COM_dotproduct_display . '\')' if($min_COM_dotproduct_display > 0);
    print XSL ' or($search_engine=\'COMET\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'delta\']/@value &lt; \'' . $min_COM_delta_display . '\')' if($min_COM_delta_display > 0);
    print XSL ' or($search_engine=\'COMET\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\']/@value &lt; \'' . $min_COM_zscore_display . '\')' if($min_COM_zscore_display > 0);


    print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'])' if($filter_xpress);
    print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']) or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&lt;\'0\'' if($filter_asap);


    if($quant_light2heavy eq 'true') {
	print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']) or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[anlaysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;\'' . $min_xpress . '\'' if($min_xpress > 0);
	print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']) or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[anlaysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;\'' . $max_xpress . '\'' if($max_xpress > 0);
    }
    else { # reverse
	print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']) or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[anlaysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;\'' . 1.0 / $min_xpress . '\'' if($min_xpress > 0);
	print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']) or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[anlaysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;\'' . 1.0 / $max_xpress . '\'' if($max_xpress > 0);
    }
    print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']) or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[anlaysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &lt;\'' . $min_asap . '\'' if($min_asap > 0);
    print XSL ' or(not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']) or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[anlaysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &gt;\'' . $max_asap . '\'' if($max_asap > 0);
    print XSL ' or(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'] and (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt; pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean + pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']asapratio_result/@error or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt; pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean - pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error))' if(! ($asap_xpress eq ''));


    if(@exclusions > 0) {
	for(my $k = 0; $k < @exclusions; $k++) {
	    print XSL ' or (@index = \'' . $exclusions[$k] . '\')';
	}
    }
    # check for excluded children of this parent
    if(@pexclusions > 0) {
	foreach(keys %parent_excls) {
	    print XSL ' or (@index = \'' . $_ . '\')';
	}
    }
    print XSL '">';
    for(my $i = 0; $i < @inclusions; $i++) {
	print XSL '<xsl:if test="not(@index=\'' . $inclusions[$i] . '\')">', "\n";
    }



} # if discards
else {

    for(my $e = 0; $e < @exclusions; $e++) {
	print XSL '<xsl:if test="not(@index=\'' . $exclusions[$e] . '\')">', "\n";
    }


    print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability &gt;=\'' . $minprob . '\'' . $suffix . '">' if(! ($minprob eq '') && $minprob > 0);
    print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tol_term &gt;=\'' . $minntt . '\'' . $suffix . '">' if(! ($minntt eq '') && $minntt > 0);
    print XSL '<xsl:if test="not(@assumed_charge=\'1\')' . $suffix . '">' if(! ($exclude_1 eq ''));
    print XSL '<xsl:if test="not(@assumed_charge=\'2\')' . $suffix . '">' if(! ($exclude_2 eq ''));
    print XSL '<xsl:if test="not(@assumed_charge=\'3\')' . $suffix . '">' if(! ($exclude_3 eq ''));
    print XSL '<xsl:if test="not(@assumed_charge &gt; \'3\')' . $suffix . '">' if(! ($exclude_4 eq ''));

    print XSL '<xsl:if test="not($search_engine=\'SEQUEST\')' . $suffix . '">' if(! ($exclude_SEQ eq ''));
    print XSL '<xsl:if test="not($search_engine=\'MASCOT\')' . $suffix . '">' if(! ($exclude_MAS eq ''));
    print XSL '<xsl:if test="not($search_engine=\'X! Tandem\')' . $suffix . '">' if(! ($exclude_TAN eq ''));
    print XSL '<xsl:if test="not($search_engine=\'PHENYX\')' . $suffix . '">' if(! ($exclude_PHEN eq ''));
    print XSL '<xsl:if test="not($search_engine=\'COMET\')' . $suffix . '">' if(! ($exclude_COM eq ''));


    print XSL '<xsl:if test="not($search_engine=\'SEQUEST\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'xcorr\']/@value &gt;= \'' . $min_SEQ_xcorr_display . '\'' . $suffix . '">' if($min_SEQ_xcorr_display > 0);
    print XSL '<xsl:if test="not($search_engine=\'SEQUEST\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'deltacn\']/@value &gt;= \'' . $min_SEQ_delta_display . '\'' . $suffix . '">' if(! ($min_SEQ_delta_display eq ''));
    print XSL '<xsl:if test="not($search_engine=\'SEQUEST\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'sprank\']/@value &lt;= \'' . $max_SEQ_sprank_display . '\'' . $suffix  . '">' if(! ($max_SEQ_sprank_display eq ''));

    print XSL '<xsl:if test="not($search_engine=\'MASCOT\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\']/@value &gt;= \'' . $min_MAS_ionscore_display . '\'' . $suffix . '">' if($min_MAS_ionscore_display > 0);
    print XSL '<xsl:if test="not($search_engine=\'MASCOT\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'ionscore\']/@value &gt; pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'identityscore\']/@value' . $suffix . '">' if($min_MAS_idscore_display);

    print XSL '<xsl:if test="not($search_engine=\'X! Tandem\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'hyperscore\']/@value &gt;= \'' . $min_TAN_hyperscore_display . '\'' . $suffix . '">' if($min_TAN_hyperscore_display > 0);
    print XSL '<xsl:if test="not($search_engine=\'X! Tandem\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'nextscore\']/@value &gt;= \'' . $min_TAN_nextscore_display . '\'' . $suffix . '">' if(! ($min_TAN_nextscore_display eq ''));
    print XSL '<xsl:if test="not($search_engine=\'X! Tandem\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'expect\']/@value &lt;= \'' . $min_TAN_expectscore_display . '\'' . $suffix . '">' if(! ($min_TAN_expectscore_display eq ''));

    print XSL '<xsl:if test="not($search_engine=\'PHENYX\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\']/@value &gt;= \'' . $min_PHEN_zscore_display . '\'' . $suffix . '">' if($min_PHEN_zscore_display > 0);
    print XSL '<xsl:if test="not($search_engine=\'PHENYX\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'origScore\']/@value &gt;= \'' . $min_PHEN_origScore_display . '\'' . $suffix . '">' if(! ($min_PHEN_origScore_display eq ''));

    print XSL '<xsl:if test="not($search_engine=\'COMET\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'dotproduct\']/@value &gt;= \'' . $min_COM_dotproduct_display . '\'' . $suffix . '">' if($min_COM_dotproduct_display > 0);
    print XSL '<xsl:if test="not($search_engine=\'COMET\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'delta\']/@value &gt;= \'' . $min_COM_delta_display . '\'' . $suffix . '">' if($min_COM_delta_display > 0);
    print XSL '<xsl:if test="not($search_engine=\'COMET\') or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:search_score[@name=\'zscore\']/@value &gt;= \'' . $min_COM_zscore_display . '\'' . $suffix . '">' if($min_COM_zscore_display > 0);


    print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']">' if($filter_xpress);;
    print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean &gt;=\'0\'">' if($filter_asap);;
    #print XSL ' or(not(search_result/pepx:search_hit[@hit_rank=\'1\']/xpressratio_result) or search_result/pepx:search_hit[@hit_rank=\'1\']/xpressratio_result/@decimal_ratio &lt;\'' . $min_xpress . '\'' if($min_xpress > 0);
    if($quant_light2heavy eq 'true') {
	print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;=\'' . $min_xpress . '\'">' if($min_xpress > 0);
	print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;=\'' . $max_xpress . '\'">' if($max_xpress > 0);
    }
    else { # reverse
	print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;=\'' . 1.0 / $min_xpress . '\'">' if($min_xpress > 0);
	print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;=\'' . 1.0 / $max_xpress . '\'">' if($max_xpress > 0);
    }
    print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &gt;=\'' . $min_asap . '\'">' if($min_asap > 0);
    print XSL '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\'] and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@' . getRatioPrefix($quant_light2heavy) . 'mean &lt;=\'' . $max_asap . '\'">' if($max_asap > 0);
    print XSL '<xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']) or not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']) or (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &lt;= (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean + search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error) and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@decimal_ratio &gt;= (pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean - pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error))">' if(! ($asap_xpress eq ''));


} # not discards

$display{'group_number'} = '<td><nobr>' . $display{'group_number'} . '</nobr></td>';

if(! $tab_delim) {
    print XSL '<tr>';
}
if(scalar keys %display_order > 0) {
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	if($tab_delim >= 1) {
	    print XSL $tab_display{$_}; # . $tab;
	}
	else {
	    print XSL $display{$_};
	}
    }
}
else { # use default
    foreach(sort {$default_order{$a} <=> $default_order{$b}} keys %default_order) {
	if($tab_delim >= 1) {
	    print XSL $tab_display{$_};
	}
	else {
	    print XSL $display{$_};
	}
    }
}
if($tab_delim >= 1) {
    print XSL $newline;
}
else {
    print XSL '</tr>';
}



if($discards) {
    print XSL '</xsl:if>';
    for(my $i = 0; $i < @inclusions; $i++) {
	print XSL '</xsl:if>';
    }
}
else {
    for(my $e = 0; $e < @exclusions; $e++) {
	print XSL '</xsl:if>';
    }
    print XSL '</xsl:if>' if(! ($minprob eq '') && $minprob > 0);
    print XSL '</xsl:if>' if(! ($minntt eq '') && $minntt > 0);
    print XSL '</xsl:if>' if(! ($exclude_1 eq ''));
    print XSL '</xsl:if>' if(! ($exclude_2 eq ''));
    print XSL '</xsl:if>' if(! ($exclude_3 eq ''));
    print XSL '</xsl:if>' if(! ($exclude_4 eq ''));
    print XSL '</xsl:if>' if($min_SEQ_xcorr_display > 0);
    print XSL '</xsl:if>' if(! ($min_SEQ_delta_display eq ''));
    print XSL '</xsl:if>' if(! ($max_SEQ_sprank_display eq ''));
    print XSL '</xsl:if>' if(! ($exclude_SEQ eq ''));
    print XSL '</xsl:if>' if(! ($exclude_MAS eq ''));
    print XSL '</xsl:if>' if(! ($exclude_TAN eq ''));
    print XSL '</xsl:if>' if(! ($exclude_PHEN eq ''));    
    print XSL '</xsl:if>' if(! ($exclude_COM eq ''));
    print XSL '</xsl:if>' if($min_MAS_ionscore_display > 0);
    print XSL '</xsl:if>' if($min_MAS_idscore_display);
    print XSL '</xsl:if>' if($min_TAN_hyperscore_display > 0);
    print XSL '</xsl:if>' if($min_TAN_nextscore_display);
    print XSL '</xsl:if>' if($min_TAN_expectscore_display);
    print XSL '</xsl:if>' if($min_PHEN_zscore_display > 0);
    print XSL '</xsl:if>' if($min_PHEN_origScore_display);    
    print XSL '</xsl:if>' if($min_COM_dotproduct_display > 0);
    print XSL '</xsl:if>' if($min_COM_delta_display > 0);
    print XSL '</xsl:if>' if($min_COM_zscore_display > 0);

    print XSL '</xsl:if>' if($filter_xpress);
    print XSL '</xsl:if>' if($filter_asap);
    print XSL '</xsl:if>' if($min_xpress > 0);
    print XSL '</xsl:if>' if($max_xpress > 0);
    print XSL '</xsl:if>' if($min_asap > 0);
    print XSL '</xsl:if>' if($max_asap > 0);
    print XSL '</xsl:if>' if(! ($asap_xpress eq ''));
} # NOT discards


print XSL '</xsl:template>', "\n";



print XSL '</xsl:stylesheet>', "\n";

print XSL "\n";

close(XSL);


}

sub getRatioPrefix {
(my $is_light) = @_;
if(! ($is_light eq 'true')) {
    return 'heavy2light_';
}
return '';
}

