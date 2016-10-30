#!/usr/bin/perl
#############################################################################
# Program       : prot_wt_xml.pl                                            #
# Author        : Andrew Keller <akeller@systemsbiology.org>                #
# Date          : 3.28.03                                                   #
# SVN Info      : $Id: prot_wt_xml.pl 5629 2011-11-11 20:29:42Z dshteyn $
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

# WRITE SUBSET: no more max length nec

print "Content-type: text/html\n\n";

%box = &tpplib_perl::read_query_string;      # Read keys and values



# write out new xsl stylesheet
#my $xslt = '/usr/bin/xsltproc';
my $xmlfile = $box{'xmlfile'};
if ($^O eq 'MSWin32' ) {
	$xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
}

my $CGI_HOME = '/cgi-bin/';  # ???
if(! -e $xmlfile) {
    print "cannot find xmlfile $xmlfile! \n\n";
    exit(1);
}
#if(exists $box{'xslt'}) {
#    $xslt = $box{'xslt'};
#}
if(exists $box{'cgi-home'}) {
    $CGI_HOME = $box{'cgi-home'};
}
my $quant_light2heavy = ! exists $box{'quant_light2heavy'} || $box{'quant_light2heavy'} eq 'true' ? 'true' : 'false';
my $glyc = exists $box{'glyc'} ? 1 : 0;
my $mark_aa = exists $box{'mark_aa'} ? $box{'mark_aa'} : '';

my $NEW_XML_FORMAT = exists $box{'xml_input'} ? 1 : isXML_INPUT($xmlfile);
my $MOD_MASS_ERROR = 0.5;

my $xslfile = $xmlfile . '.tmp.xsl';

my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'cellpadding="0" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;" width="100%">';
my $RESULT_TABLE_SUF = '</table>';

my $table_space = '&#160;';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
    $table_spacer = '<xsl:text>     </xsl:text>';
}

$| = 1; # autoflush

my $colored_peptide = '#DA143C'; # in between

# if $xmlfile is gzipped, returns tmpfile name, else returns $xmlfile
my $tmpxmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile); 

writeXSLFile($xslfile, \%box, 0);
printHTML($xslt, $tmpxmlfile, $xslfile);
unlink($xslfile) if(-e $xslfile);	
unlink($tmpxmlfile) if ($tmpxmlfile ne $xmlfile); # did we decompress xml.gz?


sub colorAAs {
    (my $peptide, my $aas, my $glyc) = @_;
# old version (pre-TPP)
    $peptide =~ s/(N[\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.]?[A-O,Q-Z][\#,\@,\*,0-9,\$,\!,\.]?[S,T])/\<font color\=\"\#FF00FF\"\>$1<\/font\>/g if($glyc);

# new version (TPP)
    $peptide =~ s/(N\[?\d?\d?\d?\d?\]?[A-O,Q-Z]\[?\d?\d?\d?\d?\]?[S,T])/\<font color\=\"\#FF00FF\"\>$1<\/font\>/g if($glyc);

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
    if(1) {
	$peptide =~ s/\[/\<font size\=\"\-2\"\>/g;
	$peptide =~ s/\]/\<\/font\>/g;
    }
    return $peptide;
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

    if($xslt =~ /xsltproc/) {
	open HTML, "$xslt $xslfile $xmlfile |";
    }
    else {
	open HTML, "$xslt $xmlfile $xslfile |";
    }
    my $counter = 0;
    my $start = '<font size="-2">';
    my $end = '</font>';

    while(<HTML>) { 

	if(! /Peptide/ && /^(.*\>\d\_)(\S+)(\s+.*)/) {
	    print $1 . colorAAs($2, $mark_aa, $glyc) . $3;
	}
	else {
	    print;
	}

    } # while

    close(HTML);
}


sub writeXSLFile {
(my $xfile, my $boxptr, my $tab_delim) = @_;
if(! open(XSL, ">$xfile")) {
    print " prot_wt_xml.pl cannot open $xfile: $!\n";
    exit(1);
}
print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">', "\n";
my $tab = '<xsl:value-of select="$tab"/>';
my $newline = '<br/><xsl:value-of select="$newline"/>';

my $newlinespace = '<p/>';
my $doubleline = $newline . $newline;
my $space = '&#160';
my $checked = 'CHECKED="yes"';

my $charge = ${$boxptr}{'charge'};



########################################################################################
# in the future, pass peptide (stripped) or MODIFIED PEPTIDE (if available), for display
# MODIFIED is available, compare with modified entry (MUST BE AVAILABLE)
# otherwise, compare peptide with peptide
# Use the modified peptide name as check for correct peptide if present, otherwise other....


my $peptide = ${$boxptr}{'peptide'};
$peptide =~ s/\~/\#/g; # switch back


my $modpep = exists ${$boxptr}{'modpep'} ? ${$boxptr}{'modpep'} : '';
my $pepmass = exists ${$boxptr}{'pepmass'} ? ${$boxptr}{'pepmass'} : '';

my $prot_header = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'comet-fastadb.cgi?Ref=';
my $prot_suf = '</A>';
my $plusmn = ' +-';

my $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'xpress-prophet.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source}&amp;heavy2light={$heavy2light}">';

if(useXMLFormatLinks($xmlfile)) {
    $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?xslt=' . $xslt . '&amp;cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source}&amp;heavy2light=' . $xpress_ratio_type . '">'; #{$heavy2light}">';
}


my $xpress_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';
my $xpress_suf = '</a>';



$display{'xpress'} = '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']"><td width="175"><xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio">XPRESS';
if(! ($quant_light2heavy eq 'true')) {
    $display{'xpress'} .= '(H/L)';
}
$display{'xpress'} .= ': ' . $xpress_pre . '<xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>(<xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>)</xsl:if>' . $xpress_suf . '</xsl:if><xsl:if test="not(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio)">' . $table_spacer . '</xsl:if></td></xsl:if>';

my $asap_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';

my $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . 'xli/ASAPRatio_lstProRatio.cgi?orgFile=';
my $asap_header_mid = '.orig&amp;proBofFile=' . $asap_file_pre . '{$filextn}' . $asap_proph_suf . '&amp;pepBofFile=' . $asap_file_pre . '{$filextn}' . $asap_pep_suf . '&amp;proIndx=';



my $asap_display_cgi = 'asap-prophet-display.cgi';
if(useXMLFormatLinks($xmlfile)) {
    $asap_display_cgi = 'ASAPCGIDisplay.cgi';
}
my $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '&amp;xmlFile=' . $xmlfile . '&amp;protein=';
my $asap_header_suf = '</A>';
my $pvalue_header_pre = '<a target="Win2" href="';
my $pvalue_header_suf = '</a>';
my $asap_header_old_version = $asap_header_pre;
my $asap_header_mid2 = '&amp;ratioType=0">';



# first display regular ratio no matter what
$display{'asapratio'} = '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']">ASAPRatio';
if(! ($quant_light2heavy eq 'true')) {
    $display{'asapratio'} .= '(H/L)';
}

#DDS:
#$display{'asapratio'} .= ': ' . $asap_header_pre . '{$file}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/></xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
$display{'asapratio'} .= ': ' . $asap_header_pre . '{$mult_prot}' . '">' . '<xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/></xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;

# now add on the adjusted only if desired and present
if(1 || ! ($show_adjusted_asap eq '') && exists ${$boxptr}{'adj_asap'}) {
    $display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\']">[<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_standard_dev"/>]</xsl:if>';
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><td width="200"><xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if>';



}
else {
    $display{'asapratio'} .= '</xsl:if></td></xsl:if>';
}


print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
print XSL '</xsl:text></xsl:variable>';

print XSL '<xsl:template match="protx:protein_summary">', "\n";
print XSL '<HTML><HEAD><TITLE>protXML Peptide Weight Viewer</TITLE></HEAD><BODY BGCOLOR="white" OnLoad="self.focus()">', "\n<PRE>";

#print XSL '<xsl:comment>' . $start_string . '</xsl:comment>'; # . $newline . "\n";

if(! ($modpep eq '')) {
    my $stdpep = $modpep;
    $stdpep =~ s/\[/\<font size\=\"\-2\"\>/g;
    $stdpep =~ s/\]/\<\/font\>/g;
    print XSL '<b>Peptide: <font color="' . $colored_peptide . '">' . $charge . '_' . $stdpep . '</font></b>'; # . $newline;
}
else {
    print XSL '<b>Peptide: <font color="' . $colored_peptide . '">' . $charge . '_' . $peptide . '</font></b>'; # . $newline;
}


print XSL $RESULT_TABLE_PRE . $RESULT_TABLE;
print XSL "\n", '<xsl:apply-templates select="protx:protein_group">', "\n";
print XSL '</xsl:apply-templates>', "\n";
print XSL $RESULT_TABLE_SUF, "\n<BR/><BR/>\n";

print XSL '<HR SIZE="1" NOSHADE="noshade"/>', "\n";
print XSL "<FONT COLOR='#999999'>protXML Peptide Weight Viewer\n$TPPVersionInfo</FONT>\n";

print XSL '</PRE></BODY></HTML>', "\n";
print XSL '</xsl:template>', "\n";


print XSL '<xsl:template match="protx:protein_group">', "\n";
print XSL '<xsl:apply-templates select="protx:protein"/>';
print XSL '</xsl:template>', "\n";


print XSL '<xsl:template match="protx:protein">';
if ($charge ne "") {
    print XSL '<xsl:if test="protx:peptide[@peptide_sequence=\'' . $peptide . '\' and @charge=\'' . $charge . '\'';
}
else {
    print XSL '<xsl:if test="protx:peptide[@peptide_sequence=\'' . $peptide . '\'';   
}
if($NEW_XML_FORMAT) {
    if(! ($modpep eq '')) {
	print XSL ' and protx:modification_info and protx:modification_info/@modified_peptide and protx:modification_info/@modified_peptide = \'' . $modpep . '\'';
    }
    else {
	if ($pepmass ne "") {
	    print XSL ' and (not(protx:peptide/protx:modification_info) or not(protx:peptide/protx:modification_info/@modified_peptide)) and @calc_neutral_pep_mass = \'' . $pepmass . '\'';
	}
	else {
	    print XSL ' and (not(protx:peptide/protx:modification_info) or not(protx:peptide/protx:modification_info/@modified_peptide))';
	}
    }
}

print XSL ']">';


# check whether part of group


print XSL '<xsl:variable name="mult_prot" select="@protein_name"/>';
print XSL '<xsl:variable name="database2" select="parent::node()/parent::node()/protx:protein_summary_header/@reference_database"/>'; # . $newline;
print XSL '<xsl:variable name="peps2" select="@unique_stripped_peptides"/>';
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
print XSL '<xsl:variable name="source" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="heavy2light"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope=\'heavy\'">0</xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope=\'light\'">1</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="pvalpngfile" select="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']/protx:ASAP_pvalue_analysis_summary/@analysis_distribution_file"/>';
print XSL '<xsl:variable name="file" select="/protx:protein_summary/protx:protein_summary_header/@source_files"/>';
print XSL '<xsl:variable name="filextn"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn">_<xsl:value-of select="/protein_summary/protein_summary_header/@source_file_xtn"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="asap_ind" select="ASAPRatio/@index"/>';
print XSL '<tr><td>' . $newline . '</td></tr>';

if ($charge ne "") {
    print XSL '<xsl:if test="count(parent::node()/protx:protein) &gt; \'1\'"><tr><td><u><xsl:value-of select="parent::node()/@group_number"/><xsl:value-of select="@group_sibling_id"/>' . $table_space . '</u></td><td colspan="15">PROTEIN GROUP: <xsl:value-of select="parent::node()/@pseudo_name"/></td></tr><tr><td>' . $table_space . '</td><td><nobr>wt-<xsl:value-of select="protx:peptide[@peptide_sequence=\'' . $peptide . '\' and @charge=\'' . $charge . '\']/@weight"/>'.$table_spacer.'</nobr></td><td>' . $prot_header . '{$mult_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$mult_prot"/>' . $prot_suf . '<xsl:for-each select="protx:indistinguishable_protein"><xsl:variable name="indist_prot" select="@protein_name"/>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each>' . $table_space . $table_space . $table_space . '<font color="#FF0000"><b><xsl:value-of select="@probability"/></b></font></td></tr></xsl:if>';
    
    print XSL '<xsl:if test="count(parent::node()/protx:protein) = \'1\'"><tr><td><u><xsl:value-of select="parent::node()/@group_number"/>' . $table_space . '</u></td><td><nobr>wt-<xsl:value-of select="protx:peptide[@peptide_sequence=\'' . $peptide . '\' and @charge=\'' . $charge . '\']/@weight"/>' . $table_spacer . '</nobr></td><td colspan="15">' . $prot_header . '{$mult_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$mult_prot"/>' . $prot_suf . '<xsl:for-each select="protx:indistinguishable_protein"><xsl:variable name="indist_prot" select="@protein_name"/>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each>' . $table_space . $table_space . $table_space . '<font color="#FF0000"><b><xsl:value-of select="@probability"/></b></font></td></tr></xsl:if>';
}
else {
    print XSL '<xsl:if test="count(parent::node()/protx:protein) &gt; \'1\'"><tr><td><u><xsl:value-of select="parent::node()/@group_number"/><xsl:value-of select="@group_sibling_id"/>' . $table_space . '</u></td><td colspan="15">PROTEIN GROUP: <xsl:value-of select="parent::node()/@pseudo_name"/></td></tr><tr><td>' . $table_space . '</td><td><nobr>wt-<xsl:value-of select="protx:peptide[@peptide_sequence=\'' . $peptide . '\']/@weight"/>'.$table_spacer.'</nobr></td><td>' . $prot_header . '{$mult_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$mult_prot"/>' . $prot_suf . '<xsl:for-each select="protx:indistinguishable_protein"><xsl:variable name="indist_prot" select="@protein_name"/>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each>' . $table_space . $table_space . $table_space . '<font color="#FF0000"><b><xsl:value-of select="@probability"/></b></font></td></tr></xsl:if>';
    
    print XSL '<xsl:if test="count(parent::node()/protx:protein) = \'1\'"><tr><td><u><xsl:value-of select="parent::node()/@group_number"/>' . $table_space . '</u></td><td><nobr>wt-<xsl:value-of select="protx:peptide[@peptide_sequence=\'' . $peptide . '\']/@weight"/>' . $table_spacer . '</nobr></td><td colspan="15">' . $prot_header . '{$mult_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$mult_prot"/>' . $prot_suf . '<xsl:for-each select="protx:indistinguishable_protein"><xsl:variable name="indist_prot" select="@protein_name"/>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each>' . $table_space . $table_space . $table_space . '<font color="#FF0000"><b><xsl:value-of select="@probability"/></b></font></td></tr></xsl:if>';
}

print XSL '<tr><td>' . $table_spacer . '</td><td>' . $table_spacer . '</td><td colspan="8">' . $RESULT_TABLE_PRE . $RESULT_TABLE . '<tr><td width="90">';

print XSL '<xsl:if test="@percent_coverage"><xsl:if test="@n_indistinguishable_proteins &gt; \'1\'">max<xsl:text> </xsl:text></xsl:if>coverage: <xsl:value-of select="@percent_coverage"/><xsl:text> </xsl:text>%</xsl:if></td>';

print XSL $display{'xpress'};
print XSL $display{'asapratio'};


print XSL '<td width="105">num unique peps: <xsl:value-of select="count(protx:peptide[@is_contributing_evidence=\'Y\'])"/></td>';
print XSL '<td width="95">tot num peps: <xsl:value-of select="@total_number_peptides"/></td>';
print XSL '<td width="125"><xsl:if test="@pct_spectrum_ids">share of spectrum id\'s: <xsl:value-of select="@pct_spectrum_ids"/>%</xsl:if></td>';
print XSL '</tr>' . $RESULT_TABLE_SUF . '</td>';

print XSL '</tr>';
print XSL '<tr><td>' . $table_spacer . '</td><td align="right" valign="top"><font color="green">&gt; </font></td><td colspan="15">';
print XSL '<xsl:if test="protx:annotation"><font color="green"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if>';
print XSL '</td></tr>';

print XSL '<xsl:apply-templates select="protx:peptide">';
print XSL '<xsl:sort select = "@nsp_adjusted_probability" order="descending" data-type="number"/>';
print XSL '</xsl:apply-templates>';

print XSL '</xsl:if>';

print XSL '</xsl:template>';


print XSL '<xsl:template match="protx:peptide">';

my $html_peptide_lead = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_mid = '&amp;Infile=';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_spacer = '<xsl:text>  </xsl:text>';
}
if(useXMLFormatLinks($xmlfile)) {
    if($quant_light2heavy eq 'true') {
	$html_peptide_lead = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'peptidexml_html.pl?xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;Ref=';
    }
    else { # add ratioType
	$html_peptide_lead = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'peptidexml_html.pl?xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType=1&amp;Ref=';
    }
}

print XSL '<xsl:variable name="mypep"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="mycharge" select="@charge"/>';

print XSL '<xsl:variable name="myinputfiles" select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/>';

print XSL '<xsl:variable name="StdPep"><xsl:if test="protx:modification_info and protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="PepMass"><xsl:if test="@calc_neutral_pep_mass"><xsl:value-of select="@calc_neutral_pep_mass"/></xsl:if></xsl:variable>';
if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'] or /protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">0</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']) and not(/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'] or /protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">1</xsl:if><xsl:if test="not(/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']) and not(/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">-1</xsl:if></xsl:variable>';
}


print XSL '<tr><td>' . $table_spacer . '</td>';
print XSL '<td>' . $table_spacer . '</td>';
my $wt_header = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'prot_wt_xml.pl?xmlfile=' . $xmlfile . '&amp;cgi-home=' . $CGI_HOME . '&amp;xslt=' . $xslt . '&amp;modpep={$StdPep}&amp;pepmass={$PepMass}&amp;';
$wt_header .= 'xml_input=1&amp;' if($NEW_XML_FORMAT);
$wt_header .= 'glyc=1&amp;' if($glyc);
$wt_header .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
$wt_header .= 'peptide=';

my $wt_suf = '</A>';

print XSL '<td><xsl:if test="@is_nondegenerate_evidence = \'Y\'"><font color="#990000">*</font></xsl:if></td><td>' . $wt_header . '{$mypep}&amp;charge={$mycharge}&amp;">' . '<nobr>wt-<xsl:value-of select="@weight"/><xsl:text> </xsl:text></nobr>' . $wt_suf . '</td>';

my $html_peptide_lead3 = $html_peptide_lead;
if(useXMLFormatLinks($xmlfile)) {
    $html_peptide_lead3 = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'peptidexml_html2.pl?PepMass={$PepMass}&amp;StdPep={$StdPep}&amp;MassError=' . $MOD_MASS_ERROR . '&amp;xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;';
    $html_peptide_lead3 .= 'mark_aa=' . $mark_aa . '&amp;' if(! ($mark_aa eq ''));
    $html_peptide_lead3 .= 'glyc=Y&amp;' if($glyc);
    $html_peptide_lead3 .= 'Ref=';
}


############################################################################
# MUST REPLACE peptide_lead with peptide_lead3!!!!!

if ($charge ne "") {
    print XSL '<td><xsl:if test="@peptide_sequence=\'' . $peptide . '\' and @charge=\'' . $charge . '\'';
}
else {
    print XSL '<td><xsl:if test="@peptide_sequence=\'' . $peptide . '\'';
}

# if modpep is not empty
if(! ($modpep eq '')) {
    print XSL ' and protx:modification_info and protx:modification_info/@modified_peptide and protx:modification_info/@modified_peptide = \'' . $modpep . '\'';
}
else {
    if ($pepmass ne "") {
	print XSL ' and (not(protx:peptide/protx:modification_info) or not(protx:peptide/protx:modification_info/@modified_peptide)) and @calc_neutral_pep_mass = \'' . $pepmass . '\'';
    }
    else {
	print XSL ' and (not(protx:peptide/protx:modification_info) or not(protx:peptide/protx:modification_info/@modified_peptide))';
    }
}
print XSL '">' . $html_peptide_lead3 . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><font color="' . $colored_peptide . '"><xsl:value-of select="@charge"/>_<xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></font></A></xsl:if>';

if ($charge ne "") {
    print XSL '<xsl:if test="not(@peptide_sequence=\'' . $peptide . '\') or not(@charge=\'' . $charge . '\')';
}
else {
    print XSL '<xsl:if test="not(@peptide_sequence=\'' . $peptide . '\')';
}

if(! ($modpep eq '')) {
    print XSL ' or not(protx:modification_info) or not(protx:modification_info/@modified_peptide) or not(protx:modification_info/@modified_peptide = \'' . $modpep . '\')';
}
else {
    if ($pepmass ne "") {
	print XSL ' or(protx:peptide/protx:modification_info and protx:peptide/protx:modification_info/@modified_peptide) or not(@calc_neutral_pep_mass = \'' . $pepmass . '\')';
    }
    else {
	print XSL ' or(protx:peptide/protx:modification_info and protx:peptide/protx:modification_info/@modified_peptide) ';
    }
}

print XSL '">' . $html_peptide_lead3 . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}"><xsl:value-of select="@charge"/>_<xsl:if test="protx:modification_info"><xsl:if test="protx:modification_info/@modified_peptide"><xsl:value-of select="protx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(protx:modification_info/@modified_peptide)"><xsl:value-of select="@peptide_sequence"/><xsl:if test="protx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="protx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="protx:modification_info/protx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="protx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="protx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(protx:modification_info)"><xsl:value-of select="@peptide_sequence"/></xsl:if></A></xsl:if>';

print XSL '</td>';
# have to add mods here!!!!!!

print XSL '<td><xsl:if test="@is_contributing_evidence = \'Y\'"><font COLOR="#FF9933"><xsl:value-of select="@nsp_adjusted_probability"/></font></xsl:if><xsl:if test="@is_contributing_evidence = \'N\'"><xsl:value-of select="@nsp_adjusted_probability"/></xsl:if></td>';
print XSL '</tr>';
print XSL '<xsl:apply-templates select="protx:indistinguishable_peptide"/>'; # make extra entry(s)
print XSL '</xsl:template>';

print XSL '<xsl:template match="protx:indistinguishable_peptide">';
print XSL '<xsl:variable name="mycharge2" select="parent::node()/@charge"/>';

print XSL '<xsl:variable name="mypep2"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';

#print XSL '<xsl:variable name="myinputfiles2"><xsl:value-of select="/protein_summary/protein_summary_header/@source_files_alt"/></xsl:variable>';
print XSL '<xsl:variable name="myinputfiles2" select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/>';

print XSL '<tr><td>' . $table_spacer . '</td>';
print XSL '<td>' . $table_spacer . '</td>';
print XSL '<td>' . $table_spacer . '</td>';
print XSL '<td>' . $table_spacer . '</td>';

print XSL '<td>--' . $html_peptide_lead . '{$mycharge2}_{$mypep2}' . $html_peptide_mid . '{$myinputfiles2}">' . '<xsl:value-of select="parent::node()/@charge"/>_<xsl:value-of select="@peptide_sequence"/></A></td>';
print XSL '</tr>';
print XSL '</xsl:template>';

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


sub isXML_INPUT {  # deprecate this?
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

sub useXMLFormatLinks {  # deprecate this!
    (my $file) = @_;
    return $NEW_XML_FORMAT;
}
