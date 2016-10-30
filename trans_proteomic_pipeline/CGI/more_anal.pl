#!/usr/bin/perl
#############################################################################
# Program       : more_anal.pl                                              #
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


#my $xslt = '/tools/bin/Xalan';

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

#if(exists $box{'xslt'}) {
#    $xslt = $box{'xslt'};
#}

my $shtml = exists $box{'shtml'} && $box{'shtml'} eq 'yes';
my $help_dir = exists $box{'helpdir'} ? $box{'helpdir'} : '';


my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'frame="border" rules="all" cellpadding="2" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
my $RESULT_TABLE_SUF = '</table>';
my $table_space = '&#160;';
if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
}
if(! open(XSL, ">$xmlfile.tmp.xsl")) {
    print "cannot open $xmlfile.tmp.xsl $!\n";
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

print XSL '<HTML><BODY><PRE>';
print XSL '<HEAD><TITLE>XML Viewer: More Analysis Info (' . $TPPVersionInfo . ')</TITLE></HEAD>';

print XSL '<b>ProteinProphet<sup><font size="3">&#xAE;</font></sup> analysis results</b>' . $newline;
print XSL 'Version: <xsl:value-of select="protx:protein_summary_header/protx:program_details/@version"/>'. $newline;
print XSL 'Analysis Date: <xsl:value-of select="protx:protein_summary_header/protx:program_details/@time"/>' . $newline;
print XSL $newline;
print XSL '<xsl:apply-templates select="protx:protein_summary_header"/>';

print XSL $newline;


print XSL '</PRE></BODY></HTML>';

print XSL '</xsl:template>';


print XSL '<xsl:template match="protx:protein_summary_header">';
print XSL '<table><tr><td>';
print XSL '<IMG SRC="' . $help_dir . 'prot-proph.jpg"/>';
print XSL '</td><td><pre>';

if(-f '/bin/cygpath' ) {
    print XSL 'Source Files: <xsl:if test="@win-cyg_source_files"><xsl:value-of select="@win-cyg_source_files"/></xsl:if><xsl:if test="not(@win-cyg_source_files)"><xsl:value-of select="@source_files"/></xsl:if>' . $newline;
}
else {
    print XSL 'Source Files: <xsl:value-of select="@source_files"/>' . $newline;
}
print XSL $newline;
print XSL 'Number of input spectra with minimum probability <xsl:value-of select="@initial_min_peptide_prob"/>: <xsl:value-of select="@num_input_1_spectra"/> 1+, <xsl:value-of select="@num_input_2_spectra"/> 2+, and <xsl:value-of select="@num_input_3_spectra"/> 3+' . $newline;
print XSL $newline;

if(-f '/bin/cygpath' ) {
    print XSL 'Reference Database: <xsl:if test="@win-cyg_reference_database"><xsl:value-of select="@win-cyg_reference_database"/></xsl:if><xsl:if test="not(@win-cyg_reference_database)"><xsl:value-of select="@reference_database"/></xsl:if>' . $newline;
}
else {
    print XSL 'Reference Database: <xsl:value-of select="@reference_database"/>' . $newline;
}
print XSL 'Residue substitutions: <xsl:value-of select="@residue_substitution_list"/>' . $newline;
print XSL '<xsl:if test="@organism">Organism: <xsl:value-of select="@organism"/>' . $newline . '</xsl:if>';
print XSL '<xsl:if test="@enzyme">Enzyme used: <xsl:value-of select="@enzyme"/>' . $newline . '</xsl:if>';
print XSL 'Run Options: <xsl:if test="@run_options"><xsl:value-of select="@run_options"/></xsl:if>' . $newline;

print XSL $tab . 'Occam\'s Razor used: <xsl:value-of select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/@occam_flag"/>' . $newline;
print XSL $tab . 'Protein Groups: <xsl:value-of select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/@groups_flag"/>' . $newline;
print XSL $tab . 'Peptide degeneracies: <xsl:value-of select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/@degen_flag"/>' . $newline;
print XSL $tab . 'Number of Sibling Peptides used: <xsl:value-of select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/@nsp_flag"/>' . $newline;
print XSL $tab . 'Min peptide probability: <xsl:value-of select="@min_peptide_probability"/>' . $newline;
print XSL $tab . 'Min peptide weight: <xsl:value-of select="@min_peptide_weight"/>' . $newline;
print XSL $newline;
print XSL 'Original results written to file: ';



# make local reference
my $local_datafile = $xmlfile;
my $SERVER_ROOT = '';
if(-f '/bin/cygpath' ) {
    # use windows name
    $local_datafile = `cygpath -w '$local_datafile'`;
    if($local_datafile =~ /^(\S+)\s?/) {
	$local_datafile = $1;
    }
    
    if(0) {
	if(exists $ENV{'WEBSERVER_ROOT'}) {
	    my ($serverRoot) = ($ENV{'WEBSERVER_ROOT'} =~ /(\S+)/);
	    if ( $serverRoot =~ /\:/ ) {
		$serverRoot = `cygpath '$serverRoot'`;
		($serverRoot) = ($serverRoot =~ /(\S+)/);
	    }
	    # make sure ends with '/'
	    $serverRoot .= '/' if($serverRoot !~ /\/$/);
	    $SERVER_ROOT = $serverRoot;
	    if($SERVER_ROOT eq '') {
		die "could not find WEBSERVER_ROOT environment variable\n";
	    }
	    if((length $serverRoot) <= (length $local_datafile) && 
	       index((lc $local_datafile), (lc $serverRoot)) == 0) {
		$local_datafile = '/' . substr($local_datafile, (length $serverRoot));
	    }
	    else {
		die "problem (ma1): $local_datafile is not mounted under webserver root: $serverRoot\n";
	    }
	}
	else {
	    die "cannot find WEBSERVER_ROOT environment variable\n";
	}
    } # if 0
} # if iis & cygwin
my $ref_datafile = $local_datafile;
if($shtml && $local_datafile =~ /^(\S+\.)xml(\.gz)?$/) {
    $ref_datafile = $1 . 'shtml';
}

print XSL '<xsl:if test="not(/protx:protein_summary/@summary_xml=\'' . $xmlfile . '\')"><a target="Win1" href="' . $ref_datafile . '"><xsl:value-of select="/protx:protein_summary/@summary_xml"/></a></xsl:if><xsl:if test="/protx:protein_summary/@summary_xml=\'' . $xmlfile . '\'">' . $local_datafile . '</xsl:if>' . $newline;
print XSL '</pre></td></tr></table>';

print XSL $newline;
print XSL '<p/>';
print XSL '<b>Analysis Iterations</b>' . $newline;
print XSL $RESULT_TABLE_PRE . $RESULT_TABLE;
print XSL '<tr><td><b><font color="brown">EM step</font></b></td><td><b><font color="brown">number of iterations</font></b></td></tr>';
print XSL '<tr><td>Initial Peptide Weights</td><td><xsl:value-of select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/@initial_peptide_wt_iters"/></td></tr>';
print XSL '<tr><td>NSP Distributions</td><td><xsl:value-of select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/@nsp_distribution_iters"/></td></tr>';
print XSL '<tr><td>Final Peptide Weights</td><td><xsl:value-of select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/@final_peptide_wt_iters"/></td></tr>';
print XSL $RESULT_TABLE_SUF;

print XSL $newline . $newline;;

print XSL '<xsl:apply-templates select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/protx:nsp_information"/>';
print XSL '<xsl:apply-templates select="protx:program_details[@analysis=\'proteinprophet\']/protx:proteinprophet_details/protx:ni_information"/>';



print XSL '<xsl:if test="@total_no_spectrum_ids"><p/><b>Total Number of Contributing Spectrum IDs (100% share): </b><xsl:value-of select="@total_no_spectrum_ids"/><p/></xsl:if>';


if($xmlfile =~ /^(\S+)\.xml(\.gz)?$/) {
    my $shtml = $1 . '.shtml';

    my $local_xmlfile = $xmlfile;
    my $local_shtml = $shtml;

    if(-f '/bin/cygpath' ) {
	if(0) {

	    if((length $SERVER_ROOT) <= (length $local_shtml) && 
	       index((lc $local_shtml), (lc $SERVER_ROOT)) == 0) {
		$local_shtml = '/' . substr($local_shtml, (length $SERVER_ROOT));
	    }
	    else {
		die "problem (ma2): $local_shtml is not mounted under webserver root: $SERVER_ROOT\n";
	    }
		my $TPPhostname = tpplib_perl::get_tpp_hostname();
	    $local_shtml = 'http://'. $TPPhostname . $local_shtml; # put on browser origin
	} # if 0
	$local_xmlfile = `cygpath -w '$local_xmlfile'`;
	if($local_xmlfile =~ /^(\S+)\s?/) {
	    $local_xmlfile = $1;
	}
	$local_shtml = `cygpath -w '$local_shtml'`;
	if($local_shtml =~ /^(\S+)\s?/) {
	    $local_shtml = $1;
	}
    } # if windows cygwin


# BSP until shtml fixed #   print XSL '<table bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;" width="100%"><tr><td>' . $table_space . '</td></tr><tr><td><font color="blue">WARNING: This file (' . $local_shtml . ') references an XML document, ' . $local_xmlfile . ', which must be present for viewing these results with a browser.  Copying ' . $local_shtml . ' to a new file location will not replicate the data, but instead give another access to view and filter the original XML file.  If you want to duplicate the dataset, use the \'Write Displayed Data Subset to File\' feature on the ProteinProphet XML Viewer, entering the file name of the desired duplicate copy.</font></td></tr></table>' . $newline;
} 


print XSL '</xsl:template>';


print XSL '<xsl:template match="protx:nsp_information">';
print XSL $newline;
print XSL '<b>Learned Number of Sibling Peptide Distributions</b>' . $newline;
print XSL 'Neighboring bin smoothing: <xsl:value-of select="@neighboring_bin_smoothing"/>' . $newline;
print XSL $RESULT_TABLE_PRE . $RESULT_TABLE;
print XSL '<tr><td><b><font color="brown">bin number</font></b></td><td><b><font color="brown">nsp range</font></b></td><td><b><font color="brown">positive freq</font></b></td><td><b><font color="brown">negative freq</font></b></td><td><b><font color="brown">positive/negative ratio</font></b></td></tr>';
print XSL '<xsl:apply-templates select="protx:nsp_distribution">';
print XSL '<xsl:sort select="@bin_no" order="ascending" data-type="number"/>';
print XSL '</xsl:apply-templates>';
print XSL $RESULT_TABLE_SUF;
print XSL $newline;


print XSL '</xsl:template>';

#print XSL '<xsl:template match="protx:ni_information">';
#print XSL $newline;
#print XSL '<b>Expected Number of Ion Instances Distributions</b>' . $newline;
#print XSL 'Neighboring bin smoothing: <xsl:value-of select="@neighboring_bin_smoothing"/>' . $newline;
#print XSL $RESULT_TABLE_PRE . $RESULT_TABLE;
#print XSL '<tr><td><b><font color="brown">bin number</font></b></td><td><b><font color="brown">ni range</font></b></td><td><b><font color="brown">positive freq</font></b></td><td><b><font color="brown">negative freq</font></b></td><td><b><font color="brown">positive/negative ratio</font></b></td></tr>';
#print XSL '<xsl:apply-templates select="protx:ni_distribution">';
#print XSL '<xsl:sort select="@bin_no" order="ascending" data-type="number"/>';
#print XSL '</xsl:apply-templates>';
#print XSL $RESULT_TABLE_SUF;
#print XSL $newline;
#print XSL '</xsl:template>';

print XSL '<xsl:template match="protx:nsp_distribution">';
print XSL '<tr><td><xsl:if test="@bin_no=\'' . $nsp_bin . '\'">' . $delim . 
                       '<xsl:value-of select="@bin_no"/>' . $endelim . 
                   '</xsl:if>
                   <xsl:if test="@bin_no!=\'' . $nsp_bin . '\'">
                       <xsl:value-of select="@bin_no"/>
                   </xsl:if>
           </td><td>
                   <xsl:if test="@bin_no=\'' . $nsp_bin . '\'">' . $delim . 
                       '<xsl:if test="@nsp_lower_bound_incl">
                               <xsl:value-of select="@nsp_lower_bound_incl"/>  &lt;= nsp
                        </xsl:if>
                        <xsl:if test="@nsp_lower_bound_excl">
                               <xsl:value-of select="@nsp_lower_bound_excl"/>  &lt; nsp
                        </xsl:if>
                        <xsl:if test="@nsp_upper_bound_excl">
                               &lt; <xsl:value-of select="@nsp_upper_bound_excl"/>
                        </xsl:if> 
                        <xsl:if test="@nsp_upper_bound_incl">
                               &lt;= <xsl:value-of select="@nsp_upper_bound_incl"/>
                        </xsl:if>' . $endelim . 
                  '</xsl:if>
                   <xsl:if test="@bin_no!=\'' . $nsp_bin . '\'">
                        <xsl:if test="@nsp_lower_bound_incl">
                               <xsl:value-of select="@nsp_lower_bound_incl"/>  &lt;= nsp
                        </xsl:if>
                        <xsl:if test="@nsp_lower_bound_excl">
                               <xsl:value-of select="@nsp_lower_bound_excl"/>  &lt; nsp
                        </xsl:if>
                        <xsl:if test="@nsp_upper_bound_excl">
                               &lt; <xsl:value-of select="@nsp_upper_bound_excl"/>
                        </xsl:if> 
                        <xsl:if test="@nsp_upper_bound_incl">
                               &lt;= <xsl:value-of select="@nsp_upper_bound_incl"/>
                        </xsl:if>
                   </xsl:if>
           </td><td>
                   <xsl:if test="@bin_no=\'' . $nsp_bin . '\'">' . $delim . 
                        '<xsl:value-of select="@pos_freq"/>' . $endelim . 
                  '</xsl:if>
                   <xsl:if test="@bin_no!=\'' . $nsp_bin . '\'">
                         <xsl:value-of select="@pos_freq"/>
                  </xsl:if>
          </td><td>
                  <xsl:if test="@bin_no=\'' . $nsp_bin . '\'">' . $delim . 
                        '<xsl:value-of select="@neg_freq"/>' . $endelim . 
                 '</xsl:if>
                  <xsl:if test="@bin_no!=\'' . $nsp_bin . '\'">
                         <xsl:value-of select="@neg_freq"/>
                  </xsl:if>
         </td><td>
                  <xsl:if test="@bin_no=\'' . $nsp_bin . '\'">' . $delim . 
                         '<xsl:value-of select="@pos_to_neg_ratio"/>
                          <xsl:if test="@alt_pos_to_neg_ratio"> 
                                 (<xsl:value-of select="@alt_pos_to_neg_ratio"/>)
                          </xsl:if>' . $endelim . 
                 '</xsl:if>
                  <xsl:if test="@bin_no!=\'' . $nsp_bin . '\'">
                          <xsl:value-of select="@pos_to_neg_ratio"/>
                          <xsl:if test="@alt_pos_to_neg_ratio"> 
                                 (<xsl:value-of select="@alt_pos_to_neg_ratio"/>)
                          </xsl:if>
                  </xsl:if>
         </td></tr>';


print XSL '</xsl:template>';

print XSL '<xsl:template match="protx:ni_distribution">';
print XSL '<tr><td><xsl:if test="@bin_no=\'' . $ni_bin . '\'">' . $delim . 
                       '<xsl:value-of select="@bin_no"/>' . $endelim . 
                   '</xsl:if>
                   <xsl:if test="@bin_no!=\'' . $ni_bin . '\'">
                       <xsl:value-of select="@bin_no"/>
                   </xsl:if>
           </td><td>
                   <xsl:if test="@bin_no=\'' . $ni_bin . '\'">' . $delim . 
                       '<xsl:if test="@ni_lower_bound_incl">
                               <xsl:value-of select="@ni_lower_bound_incl"/>  &lt;= ni
                        </xsl:if>
                        <xsl:if test="@ni_lower_bound_excl">
                               <xsl:value-of select="@ni_lower_bound_excl"/>  &lt; ni
                        </xsl:if>
                        <xsl:if test="@ni_upper_bound_excl">
                               &lt; <xsl:value-of select="@ni_upper_bound_excl"/>
                        </xsl:if> 
                        <xsl:if test="@ni_upper_bound_incl">
                               &lt;= <xsl:value-of select="@ni_upper_bound_incl"/>
                        </xsl:if>' . $endelim . 
                  '</xsl:if>
                   <xsl:if test="@bin_no!=\'' . $ni_bin . '\'">
                        <xsl:if test="@ni_lower_bound_incl">
                               <xsl:value-of select="@ni_lower_bound_incl"/>  &lt;= ni
                        </xsl:if>
                        <xsl:if test="@ni_lower_bound_excl">
                               <xsl:value-of select="@ni_lower_bound_excl"/>  &lt; ni
                        </xsl:if>
                        <xsl:if test="@ni_upper_bound_excl">
                               &lt; <xsl:value-of select="@ni_upper_bound_excl"/>
                        </xsl:if> 
                        <xsl:if test="@ni_upper_bound_incl">
                               &lt;= <xsl:value-of select="@ni_upper_bound_incl"/>
                        </xsl:if>
                   </xsl:if>
           </td><td>
                   <xsl:if test="@bin_no=\'' . $ni_bin . '\'">' . $delim . 
                        '<xsl:value-of select="@pos_freq"/>' . $endelim . 
                  '</xsl:if>
                   <xsl:if test="@bin_no!=\'' . $ni_bin . '\'">
                         <xsl:value-of select="@pos_freq"/>
                  </xsl:if>
          </td><td>
                  <xsl:if test="@bin_no=\'' . $ni_bin . '\'">' . $delim . 
                        '<xsl:value-of select="@neg_freq"/>' . $endelim . 
                 '</xsl:if>
                  <xsl:if test="@bin_no!=\'' . $ni_bin . '\'">
                         <xsl:value-of select="@neg_freq"/>
                  </xsl:if>
         </td><td>
                  <xsl:if test="@bin_no=\'' . $ni_bin . '\'">' . $delim . 
                         '<xsl:value-of select="@pos_to_neg_ratio"/>
                          <xsl:if test="@alt_pos_to_neg_ratio"> 
                                 (<xsl:value-of select="@alt_pos_to_neg_ratio"/>)
                          </xsl:if>' . $endelim . 
                 '</xsl:if>
                  <xsl:if test="@bin_no!=\'' . $ni_bin . '\'">
                          <xsl:value-of select="@pos_to_neg_ratio"/>
                          <xsl:if test="@alt_pos_to_neg_ratio"> 
                                 (<xsl:value-of select="@alt_pos_to_neg_ratio"/>)
                          </xsl:if>
                  </xsl:if>
         </td></tr>';


print XSL '</xsl:template>';


print XSL '</xsl:stylesheet>';

close(XSL);

# if $xml is gzipped, returns tmpfile name, else returns $xml
my $tmpxmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile); 

# now apply the template
if($xslt =~ /xsltproc/) {
    open DATA, "$xslt $xmlfile.tmp.xsl $tmpxmlfile |"; 
}
else {
    open DATA, "$xslt $tmpxmlfile $xmlfile.tmp.xsl |"; 
}
while(<DATA>) {
    print if(! /xml.*version/);
}
close(DATA);

unlink("$xmlfile.tmp.xsl") if(-e "$xmlfile.tmp.xsl");
unlink($tmpxmlfile) if ($tmpxmlfile ne $xmlfile); # did we decompress pepxml.gz?


