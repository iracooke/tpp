#!/usr/bin/perl

#############################################################################
# Program       : peptide_html.pl                                           #
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


my $MALDI; # = 0;




print "Content-type: text/html\n\n";
%box = &tpplib_perl::read_query_string;      # Read keys and values

print "<HTML><BODY BGCOLOR=\"\#FFFFFF\" OnLoad=\"self.focus();\" ><PRE>";
print "<TITLE>More Analysis Info (" . $TPPVersionInfo . ")</TITLE>";
my $xmlfile_in = $box{'xmlfile'};
if ($^O eq 'MSWin32' ) {
	$xmlfile_in =~ s/\\/\//g;  # get those path seps pointing right!
}
# if $xmlfile_in is gzipped, returns tmpfile name, else returns $xmlfile_in
my $xmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile_in); 


#my $xslt = $box{'xslt'};
my $cgi_bin = $box{'cgi_bin'};
my $temp_xslfile = $xmlfile . '.tmpxsl';


writeXSLFile($temp_xslfile, $xmlfile);


print "<b>$xmlfile</b><p/>\n";
print '<TABLE frame="border" rules="all" cellpadding="2" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';

    # then apply xslfile to file

my $counter = 1;

if($xslt =~ /xsltproc/) {
    open XSLT, "$xslt --novalid $temp_xslfile $xmlfile |";
}
else {
    open XSLT, "$xslt $xmlfile $temp_xslfile |";
}
# pipe input, output along with counter++;
my @results = <XSLT>;
for(my $k = 1; $k < @results; $k++) {
    while($results[$k] =~ /COUNTER/) {
	$results[$k] =~ s/COUNTER/$counter/;
	$counter++;
    }



    print $results[$k];

}
close(XSLT);
unlink($temp_xslfile) if (-e $temp_xslfile);
unlink($xmlfile) if ($xmlfile ne $xmlfile_in); # did we decompress xml.gz?

print '</TABLE>';


print "</PRE></HTML>";






sub writeXSLFile {
(my $temp_xslfile, my $xmlfile) = @_;

my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'cellpadding="0" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
my $RESULT_TABLE_SUF = '</table>';

open(XSL, ">$temp_xslfile") or die "cannot open $temp_xslfile $!\n";



my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_spacer = '<xsl:text>     </xsl:text>';
    $table_spacer = ''; #<xsl:text>     </xsl:text>';
}


print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">', "\n";
print XSL '<xsl:key name="search_engine" match="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine" use="."/>';


print XSL '<xsl:template match="pepx:msms_pipeline_analysis">';


print XSL '<tr><td><font color="brown"><b>#</b></font></td>';

print XSL '<td><font color="brown"><b>search run</b></font>' . $table_spacer . '</td>';

print XSL '<td><font color="brown"><b>no. results</b></font>' . $table_spacer . '</td>';
# MS INSTRUMENT STUFF HERE>>>
print XSL '<xsl:if test="count(/pepx:msms_pipeline_analysis/pepx:msms_run_summary/@msModel) &gt; \'0\'">';
print XSL '<td><font color="brown"><b>mass spectrometer</b></font>' . $table_spacer . '</td>';
print XSL '</xsl:if>';

print XSL '<td><font color="brown"><b>sample enzyme</b></font>' . $table_spacer . '</td>';
print XSL '<td><font color="brown"><b>database search</b></font>' . $table_spacer . '</td>';


print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'database_refresh\']"><td><font color="brown"><b>refresh database</b></font>' . $table_spacer . '</td></xsl:if>';
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'peptideprophet\']"><td><font color="brown"><b>PeptideProphet<sup><small>TM</small></sup> analysis</b></font>' . $table_spacer . '</td></xsl:if>';
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'xpress\']"><td><font color="brown"><b>XPRESS<sup><small>TM</small></sup> analysis</b></font>' . $table_spacer . '</td></xsl:if>';
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'asapratio\']"><td><font color="brown"><b>ASAPRatio<sup><small>TM</small></sup> analysis</b></font>' . $table_spacer . '</td></xsl:if>';
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']"><td><font color="brown"><b>Libra Quantitation analysis</b></font>' . $table_spacer . '</td></xsl:if>';
print XSL '</tr>';
print XSL "\n";
print XSL '<xsl:apply-templates select="pepx:msms_run_summary"/>', "\n";


print XSL '<tr><td colspan="10">';
print XSL '<FORM ACTION="' . $cgi_bin . 'Pep3D_xml.cgi" METHOD="POST" TARGET="Win2">';
print XSL '<table border="3" BGCOLOR="white" frame="void" rules="none"><tr><td><pre>' . "\n\n";


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
print XSL '<input type="hidden" name="display_all" value="yes"/>';


print XSL '<INPUT TYPE="SUBMIT" name="submit" VALUE="Generate Pep3D image"/>      </pre></td><td><pre>     <INPUT TYPE="SUBMIT" name="submit" VALUE="Save as"/> <input type="text" name="saveFile" size="10" value="Pep3D.htm"/></pre></td>';
print XSL '<input type="hidden" name="htmFile" value="' . $xmlfile . '"/>';

print XSL '</tr></table>';
print XSL '</FORM></td></tr>';

print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:msms_run_summary">', "\n";

print XSL '<xsl:variable name="basename"><xsl:value-of select="@base_name"/></xsl:variable>';
print XSL '<xsl:variable name="summaryxml"><xsl:value-of select="/pepx:msms_pipeline_analysis/@summary_xml"/></xsl:variable>';
print XSL '<xsl:variable name="engine"><xsl:value-of select="pepx:search_summary/@search_engine"/></xsl:variable>';
print XSL '<xsl:variable name="pepproph_timestamp"><xsl:value-of select="pepx:analysis_timestamp[@analysis=\'peptideprophet\']/@time"/></xsl:variable>';
print XSL '<xsl:variable name="xpress_time"><xsl:value-of select="pepx:analysis_timestamp[@analysis=\'xpress\']/@time"/></xsl:variable>';
print XSL '<xsl:variable name="asap_time"><xsl:value-of select="pepx:analysis_timestamp[@analysis=\'asapratio\']/@time"/></xsl:variable>';
print XSL '<xsl:variable name="libra_time"><xsl:value-of select="pepx:analysis_timestamp[@analysis=\'libra\']/@time"/></xsl:variable>';
print XSL '<xsl:variable name="refresh_time"><xsl:value-of select="pepx:analysis_timestamp[@analysis=\'database_refresh\']/@time"/></xsl:variable>';


my $engine_pre = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'show_search_params.pl?xmlfile={$summaryxml}&amp;basename={$basename}&amp;engine={$engine}&amp;xslt=' . $xslt . '">';
my $peptide_ref = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'ModelParser.cgi?Xmlfile={$summaryxml}&amp;Timestamp={$pepproph_timestamp}">';

my $xpress_pre = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'AnalysisSummaryParser.cgi?xmlfile={$summaryxml}&amp;analysis=xpressratio&amp;timestamp={$xpress_time}">';

my $asap_pre = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'AnalysisSummaryParser.cgi?xmlfile={$summaryxml}&amp;analysis=asapratio&amp;timestamp={$asap_time}">';

my $libra_pre = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'AnalysisSummaryParser.cgi?xmlfile={$summaryxml}&amp;analysis=libra&amp;timestamp={$libra_time}">';


# add additional info here....matched ions....light_labels and heavy_labels.
# other_info=light_labels-{$lightl},heavy_labels-{$heavyl}

my $refresh_pre = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'AnalysisSummaryParser.cgi?xmlfile={$summaryxml}&amp;analysis=database_refresh&amp;timestamp={$refresh_time}&amp;timestamp_only=1">';

print XSL '<tr>';
printf XSL "<td>%s</td>", "COUNTER";
print XSL '<td><xsl:value-of select="@base_name"/></td>';

print XSL '<td align="right"><xsl:value-of select="count(pepx:spectrum_query)"/></td>';
# MS INSTRUMENT STUFF HERE>>>
print XSL '<xsl:if test="count(/pepx:msms_pipeline_analysis/pepx:msms_run_summary/@msModel) &gt; \'0\'">';
print XSL '<td><xsl:for-each select="@msManufacturer"><xsl:value-of select="."/><xsl:text> </xsl:text></xsl:for-each><xsl:for-each select="@msModel"><xsl:value-of select="."/><xsl:text> </xsl:text></xsl:for-each><xsl:for-each select="@msIonisation"><xsl:value-of select="."/><xsl:text> </xsl:text></xsl:for-each><xsl:for-each select="@msMassAnalyzer"><xsl:value-of select="."/><xsl:text> </xsl:text></xsl:for-each><xsl:for-each select="@msDetector"><xsl:value-of select="."/></xsl:for-each></td>';
print XSL '</xsl:if>';

print XSL '<td><xsl:value-of select="pepx:sample_enzyme/@name"/>'; #</td>';
print XSL '</td><td><xsl:value-of select="pepx:search_database/@local_path"/><xsl:text> </xsl:text>' . $engine_pre . '<xsl:value-of select="pepx:search_summary/@search_engine"/></A></td>';

print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'database_refresh\']"><td><xsl:if test="pepx:analysis_timestamp[@analysis=\'database_refresh\']"><xsl:value-of select="pepx:analysis_timestamp[@analysis=\'database_refresh\']/pepx:database_refresh_timestamp/@database"/><xsl:text> </xsl:text>' . $refresh_pre . '<xsl:value-of select="pepx:analysis_timestamp[@analysis=\'database_refresh\']/@time"/></A></xsl:if></td></xsl:if>';
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'peptideprophet\']"><td><xsl:if test="pepx:analysis_timestamp[@analysis=\'peptideprophet\']">' . $peptide_ref . '<xsl:value-of select="pepx:analysis_timestamp[@analysis=\'peptideprophet\']/@time"/></A></xsl:if></td></xsl:if>';

print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'xpress\']"><td><xsl:if test="pepx:analysis_timestamp[@analysis=\'xpress\']">' . $xpress_pre . '<xsl:value-of select="pepx:analysis_timestamp[@analysis=\'xpress\']/@time"/></A></xsl:if></td></xsl:if>';
print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'asapratio\']"><td><xsl:if test="pepx:analysis_timestamp[@analysis=\'asapratio\']">' . $asap_pre  . '<xsl:value-of select="pepx:analysis_timestamp[@analysis=\'asapratio\']/@time"/></A></xsl:if></td></xsl:if>';

print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']"><td><xsl:if test="pepx:analysis_timestamp[@analysis=\'libra\']">' . $libra_pre  . '<xsl:value-of select="pepx:analysis_timestamp[@analysis=\'libra\']/@time"/></A></xsl:if></td></xsl:if>';

print XSL '</tr>';



print XSL '</xsl:template>', "\n";


print XSL '</xsl:stylesheet>', "\n";

close(XSL);

}
