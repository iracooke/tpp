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

my $MALDI; # = 0;
#
# gather TPP version info
#
my $CGI_HOME;
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

print "Content-type: text/html\n\n";

%box = &tpplib_perl::read_query_string;      # Read keys and values


print "<HTML><BODY BGCOLOR=\"\#FFFFFF\" OnLoad=\"self.focus();\" ><PRE>";
my $checked = 'CHECKED="yes"';

my $peptide = $box{'Ref'};
$peptide =~ s/p/\#/g;  # replace pound signs that were encrypted in html tag
# replace ~ for #
$peptide =~ s/\~/\#/g;
my $charge;
if($peptide =~ /^(\d)?\_(\S+)/) {
    $charge = $1;
    $peptide = $2;
}
$peptide =~ s/\_//g;

my $cgi_bin = exists $box{'cgi-bin'} ? $box{'cgi-bin'} : '';
$cgi_bin .= '/' if($cgi_bin !~ /\/$/);

my $pep_mass = exists $box{'PepMass'} ? $box{'PepMass'} : 0.0;
my $error = exists $box{'MassError'} ? $box{'MassError'} : 0.0;
my $std_pep = exists $box{'StdPep'} ? $box{'StdPep'} : '';

my $libra = exists $box{'libra'} && $box{'libra'} == 1 ? $box{'libra'} : '';
my $libra_abs = ! exists $box{'libra_display'} || $box{'libra_display'} eq 'absolute' ? $checked : '';
my $libra_norm = ! $libra_abs ? $checked : '';



my $mark_aa = exists $box{'mark_aa'} ? $box{'mark_aa'} : '';
my $glyc = exists $box{'glyc'} && $box{'glyc'} eq 'Y';

$std_pep =~ s/\[/\<font size\=\"\-2\"\>/g;
$std_pep =~ s/\]/\<\/font\>/g;

my $DISPLAY_MODS = 1;

if(exists $box{'prots_display'}) {
    $ref_prots = $box{'prots_display'} eq 'show_all' ? '' : 'checked';
    $show_prots = $box{'prots_display'} eq 'show_all' ? 'checked' : '';
}
else {
    $ref_prots = 'checked';
    $show_prots = '';
}
my $ratio_type = exists $box{'ratioType'} ? $box{'ratioType'} : -1;

if(exists $box{'quant_light2heavy'}) {
    if($box{'quant_light2heavy'} eq 'true') {
	$ratio_type = 0;
    }
    else {
	$ratio_type = 1;
    }
}

my $table_space = '&#160;';
my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';

#my $xslt = $box{'xslt'};


if($xslt =~ /xsltproc/) {
    $table_space = '<xsl:text> </xsl:text>';
    $table_spacer = '<xsl:text>     </xsl:text>';
}
my @files = split(' ', $box{'Infile'});
my $temp_xslfile = $files[0] . '.tmpxsl';

my $counter = 1;

print "\n";
print '<TABLE cellpadding="0" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
print '<tr><td>';
print '<FORM ACTION="' . $cgi_bin . 'peptidexml_html2.pl" METHOD="POST" TARGET="Win1">';
if(! ($std_pep eq '')) {
    if ($charge ne "") {
	printf "<font color=\"red\"><b>%d_%s</b></font>", $charge, $std_pep;
    }
    else {
	printf "<font color=\"red\"><b>%s</b></font>", $std_pep;
    }
}
else {
    if ($charge ne "") {
	printf "<font color=\"red\"><b>%d_%s</b></font>", $charge, $peptide;
    }
    else {
	printf "<font color=\"red\"><b>%s</b></font>", $peptide;
    }
}

print '<p/>';
print 'Proteins: ';
print '<input type="radio" name="prots_display" value="ref_all" ' . $ref_prots . '/>Ref all';
print '<input type="radio" name="prots_display" value="show_all" ' . $show_prots . '/>Show all     ';

# send hidden information
foreach(keys %box) {
    print '<input type="hidden" name="' . $_ . '" value="' . $box{$_} . '"/>' if(! ($_ eq 'prots_display' && ! ($_ eq 'quant_light2heavy')));
}
# show proteins


if($ratio_type >= 0) {
    print '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
    print 'Quantitation Ratio: <input type="radio" name="quant_light2heavy" value="true" ';
    print 'checked' if($ratio_type == 0);
    print '/>light/heavy<input type="radio" name="quant_light2heavy" value="false" ';
    print 'checked' if($ratio_type == 1);
    print '/>heavy/light';
}
print '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';

if($libra) {
print 'Libra Quantitation: <input type="radio" name="libra_display" value="absolute" ' . $libra_abs . '/>absolute ';
print '<input type="radio" name="libra_display" value="normalized" ' . $libra_norm . '/>normalized  ';
print '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';

}

print '<INPUT TYPE="SUBMIT" name="submit" VALUE="Reload Browser"/>' . $table_spacer;
print '</FORM>';


print '</td>';

print '</tr>';

print '</TABLE>';
printf '<br/>';
print '<HR>';
foreach(@files) {

    # if $_ is gzipped, returns tmpfile name, else returns $_
    my $fname = tpplib_perl::uncompress_to_tmpfile($_); 


    # write xslfile
    writeXSLFile($temp_xslfile, $fname, $peptide, $charge);


    print "<A TARGET=\"Win3\" HREF=\"";
        
    if ($USING_LOCALHOST) {
	die "Could not find WEBSERVER_ROOT environment variable.\n" unless 
	    exists $ENV{'WEBSERVER_ROOT'};
	
	my $tmp = $_;
	my $wsrt = $ENV{'WEBSERVER_ROOT'};
	if ($WINDOWS_CYGWIN) {
		$wsrt = `cygpath -u '$ENV{'WEBSERVER_ROOT'}'`;
	}
	if ($^O eq 'MSWin32' ) {
		$wsrt =~ s/\\/\//g;  # get those path seps pointing right!
	}
	$tmp =~ s/$wsrt//gix;
	
	if (substr($tmp, 0, 1) ne "/") {
	    $tmp = "/" . $tmp;
	}
	
	my @tmparr = split /\./, $tmp;
	if(/(^\S+\.)xml(\.gz)?$/) {
	    print $tmparr[0]."shtml";
	}
	else {
	    print $tmp;
	}
    }
    else {
	if(/(^\S+\.)xml(\.gz)?$/) {
	    print $1."shtml";
	}
	else {
	    print $_;
	}
    }

    print "\">";
    print "<font color=\"red\">";
    if ($WINDOWS_CYGWIN) {
	print `cygpath -w '$_'`; 
    }
    else {
	print $_;
    }
    print "</font></A>";

    print '<TABLE cellspacing="5" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';

    # then apply xslfile to file

    if($xslt =~ /xsltproc/) {
	open XSLT, "$xslt $temp_xslfile $fname |";
    }
    else {
	open XSLT, "$xslt $fname $temp_xslfile |";
    }
    # pipe input, output along with counter++;
    my @results = <XSLT>;
    for(my $k = 1; $k < @results; $k++) {
	while($results[$k] =~ /COUNTER/) {
	    $results[$k] =~ s/COUNTER/$counter/;
	    $counter++;
	}
	while($results[$k] =~ /^(.*?ncbi\.nlm\.nih\.gov\/blast\/Blast\.cgi.*?\>)([A-Z,\#,\@,\*,0-9,\$,\!,\~,\^,\?,\',\.,\[,\],n,c]+)(.*?)$/) {
	    my $first = $1;
	    my $second = $2;
	    $results[$k] = $3;
	    print $first, colorAAs($second, $mark_aa, $glyc);
	}




	print $results[$k];

    } # next k
    close(XSLT);
    unlink($temp_xslfile) if (-e $temp_xslfile);

    print "</TABLE>";
    print "\n";

    # here put Pep3D link....
    print '<FORM ACTION="' . $cgi_bin . 'Pep3D_xml.cgi" METHOD="POST" TARGET="Win2">';
    print '<table BGCOLOR="white"><tr><td>';

    print '<input type="hidden" name="mzRange" value="Full"/>';
    print '<input type="hidden" name="mzGrid" value="3"/>';
    print '<input type="hidden" name="mzImgGrid" value="1"/>';
    print '<input type="hidden" name="scanRange" value="Full"/>';
    print '<input type="hidden" name="scanGrid" value="0.5"/>';
    print '<input type="hidden" name="scanImgGrid" value="2"/>';
    print '<input type="hidden" name="peakRange" value="unit of background"/>';
    print '<input type="hidden" name="peakLower" value="1"/>';
    print '<input type="hidden" name="peakUpper" value="20"/>';
    print '<input type="hidden" name="pepDisplay" value="All"/>';
    print '<input type="hidden" name="pepImgGrid" value="2"/>';
    print '<input type="hidden" name="probLower" value="0.5"/>';
    print '<input type="hidden" name="probUpper" value="1.0"/>';
    print '<input type="hidden" name="function" value="Linear"/>';
    print '<input type="hidden" name="image" value="Full"/>';
    print '<INPUT TYPE="SUBMIT" name="submit" VALUE="Generate Pep3D image"/>' . $table_spacer . '<INPUT TYPE="SUBMIT" name="submit" VALUE="Save as"/> <input type="text" name="saveFile" size="10" value="Pep3D.htm"/>';
    print $table_spacer;
    print '<input type="hidden" name="display_all" value="yes"/>';
    print '<input type="hidden" name="htmFile" value="' . $_ . '"/>';
    print '</td></tr></table></FORM>';


    print "<HR>";
    unlink($fname) if ($fname ne $_); # did we decompress xml.gz?


} # next file

print "</PRE></HTML>";




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

if($DISPLAY_MODS) {
    $peptide =~ s/\[/\<font size\=\"\-2\"\>/g;
    $peptide =~ s/\]/\<\/font\>/g;
}

return $peptide;
}


# taken off the web
sub read_query_string
{
    local ($buffer, @pairs, $pair, $name, $value, %FORM);
    # Read in text
    $ENV{'REQUEST_METHOD'} =~ tr/a-z/A-Z/;
    if ($ENV{'REQUEST_METHOD'} eq "POST")
    {
        read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'});
    } else
    {
        $buffer = $ENV{'QUERY_STRING'};
    }
    @pairs = split(/&/, $buffer);
    foreach $pair (@pairs)
    {
        ($name, $value) = split(/=/, $pair);
        $value =~ tr/+/ /;
        $value =~ s/%(..)/pack("C", hex($1))/eg;
        $FORM{$name} = $value;
    }
    %FORM;
}



sub writeXSLFile {
(my $temp_xslfile, my $xmlfile, my $peptide, my $charge) = @_;


open(XSL, ">$temp_xslfile") or die "cannot open $temp_xslfile $!\n";



my $table_spacer = '&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;';
if($xslt =~ /xsltproc/) {
    $table_spacer = '<xsl:text>     </xsl:text>';
    $table_spacer = ''; #<xsl:text>     </xsl:text>';
}


$display{'index'} = '<td align="center"><xsl:value-of select="@index"/>' . $table_spacer . '</td>';

my $spec_ref_seq = '<A TARGET="Win2" HREF="' . $cgi_bin . 'sequest-tgz-out.cgi?OutFile={$basename}/{$xpress_spec}.out">';
my $spec_ref_mas = '<A TARGET="Win2" HREF="' . $cgi_bin . 'mascotout.pl?OutFile={$basename}/{$xpress_spec}.out">';
my $spec_ref_com = '<A TARGET="Win2" HREF="' . $cgi_bin . 'cometresult.cgi?TarFile={$basename}.cmt.tar.gz&amp;File={$xpress_spec}.cmt">'; 
my $spec_ref_spc = '<A TARGET="Win2" HREF="' . $cgi_bin . 
    'plotspectrast.cgi?LibFile={$lib_path}' . '&amp;LibFileOffset={$lib_offset}' . '&amp;QueryFile={$basename}{$qry_type}' . '&amp;QueryScanNum={$qry_scan}">'; 


$display{'spectrum'} = '<td>' . '<xsl:if test="contains($search_engine,\'X! Tandem\')"><xsl:value-of select="@spectrum"/></xsl:if>' . '<xsl:if test="contains($search_engine,\'SpectraST\')">' . $spec_ref_spc . '<xsl:value-of select="@spectrum"/></A></xsl:if>' . '<xsl:if test="$search_engine=\'SEQUEST\'">' . $spec_ref_seq . '<xsl:value-of select="@spectrum"/></A></xsl:if><xsl:if test="$search_engine=\'MASCOT\'">' . $spec_ref_mas . '<xsl:value-of select="@spectrum"/></A></xsl:if><xsl:if test="$search_engine=\'COMET\'">' . $spec_ref_com . '<xsl:value-of select="@spectrum"/></A></xsl:if>' . $table_spacer . '</td>';


my $ions_ref = '<xsl:choose><xsl:when test="$search_engine=\'COMET\'"><A TARGET="Win3" HREF="'.$cgi_bin.'plot-msms.cgi?MassType={$masstype}&amp;NumAxis=1&amp;{$PeptideMods2}Pep={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide}&amp;Dta={$basename}/{@spectrum}.dta&amp;COMET=1"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@tot_num_ions"/></nobr></A></xsl:when><xsl:otherwise><A TARGET="Win3" HREF="'.$cgi_bin.'plot-msms.cgi?MassType={$masstype}&amp;NumAxis=1&amp;{$PeptideMods2}Pep={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide}&amp;Dta={$basename}/{@spectrum}.dta"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@tot_num_ions"/></nobr></A></xsl:otherwise></xsl:choose>';

$display{'ions'} = '<xsl:choose><xsl:when test="$search_engine=\'SpectraST\'"></xsl:when><xsl:otherwise><td align="RIGHT">' . $ions_ref . $table_spacer . '</td></xsl:otherwise></xsl:choose>';


$display{'fval'} = '<td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter[@name=\'fval\']/@value"/></nobr></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary)">N_A</xsl:if>' . $table_spacer . '</td>';


my $xpress_ref = '<A TARGET="Win2" HREF="' . $cgi_bin . 'XPressPeptideUpdateParser.cgi?LightFirstScan={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@light_firstscan}&amp;LightLastScan={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@light_lastscan}&amp;HeavyFirstScan={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy_firstscan}&amp;HeavyLastScan={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy_lastscan}&amp;XMLFile={$basename}.mzXML&amp;ChargeState={@assumed_charge}&amp;LightMass={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@light_mass}&amp;HeavyMass={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy_mass}&amp;MassTol={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@mass_tol}&amp;index={@index}&amp;xmlfile=' . $xmlfile . '&amp;bXpressLight1=0&amp;OutFile={@spectrum}&amp;bXpressLight1={parent::node()/pepx:analysis_timestamp[@analysis=\'xpress\']/pepx:xpressratio_timestamp/@display_ref}">';


my $plusmn = '+-'; #TODO: figure out what's wrong with displaying plusmn symbol '&#177;';

my $asap_ref = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'ASAPRatioPeptideCGIDisplayParser.cgi?Xmlfile=' . $xmlfile . '&amp;Basename={$basename}&amp;Indx={@index}&amp;Timestamp={parent::node()/pepx:analysis_timestamp[@analysis=\'asapratio\']/@time}&amp;Spectrum={@spectrum}&amp;ratioType=' . $ratio_type . '&amp;quantHighBG={$asap_quantHighBG}&amp;zeroBG={$asap_zeroBG}&amp;mzBound={$asap_mzBound}">';


if($ratio_type == 1) {
    $display{'xpress'} = '<td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']">' . $xpress_ref . '<nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy2light_ratio"/></nobr></A></xsl:if></td>';
    $display{'asap'} = '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'asapratio\']"><td>' . $asap_ref . '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&lt;\'0\'">N_A</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&gt;\'-1\'"><xsl:choose><xsl:when test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean=\'0\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@heavy2light_mean=\'999\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@heavy2light_error &gt; 0.5 * pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@heavy2light_mean"><font color="red"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@heavy2light_mean"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@heavy2light_error"/></nobr></font></xsl:when><xsl:otherwise><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@heavy2light_mean"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@heavy2light_error"/></nobr></xsl:otherwise></xsl:choose></xsl:if></A></td></xsl:if>';
}
else { # light2heavy
    $display{'xpress'} = '<td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']">' . $xpress_ref . '<nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@ratio"/></nobr></A></xsl:if></td>';

    $display{'asap'} = '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'asapratio\']"><td>' . $asap_ref . '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&lt;\'0\'">N_A</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean&gt;\'-1\'"><xsl:choose><xsl:when test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean=\'0\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean=\'999\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error &gt; 0.5 * pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean"><font color="red"><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error"/></nobr></font></xsl:when><xsl:otherwise><nobr><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@mean"/><xsl:text> </xsl:text>' . $plusmn . '<xsl:text> </xsl:text><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'asapratio\']/pepx:asapratio_result/@error"/></nobr></xsl:otherwise></xsl:choose></xsl:if></A></td></xsl:if>';
}


my $pep_ref = '<A TARGET="Win2" HREF="http://www.ncbi.nlm.nih.gov/blast/Blast.cgi?CMD=Web&amp;LAYOUT=TwoWindows&amp;AUTO_FORMAT=Semiauto&amp;ALIGNMENTS=50&amp;ALIGNMENT_VIEW=Pairwise&amp;CDD_SEARCH=on&amp;CLIENT=web&amp;COMPOSITION_BASED_STATISTICS=on&amp;DATABASE=nr&amp;DESCRIPTIONS=100&amp;ENTREZ_QUERY=(none)&amp;EXPECT=1000&amp;FILTER=L&amp;FORMAT_OBJECT=Alignment&amp;FORMAT_TYPE=HTML&amp;I_THRESH=0.005&amp;MATRIX_NAME=BLOSUM62&amp;NCBI_GI=on&amp;PAGE=Proteins&amp;PROGRAM=blastp&amp;SERVICE=plain&amp;SET_DEFAULTS.x=41&amp;SET_DEFAULTS.y=5&amp;SHOW_OVERVIEW=on&amp;END_OF_HTTPGET=Yes&amp;SHOW_LINKOUT=yes&amp;QUERY={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide}">';


if($DISPLAY_MODS) {
    $display{'peptide_sequence'} = '<td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>.</xsl:if>' . $pep_ref . '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide"/></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@modified_peptide)"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass">n[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass"/>]</xsl:if><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/pepx:mod_aminoacid_mass"><xsl:value-of select="@position"/>[<xsl:value-of select="@mass"/>]</xsl:for-each><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass">c[<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass"/>]</xsl:if></xsl:if></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info)"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/></xsl:if></A><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa">.<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/></xsl:if>' . $table_spacer . '</td>';
}
else {
$display{'peptide_sequence'} = '<td><xsl:if test="search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"><xsl:value-of select="search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>.</xsl:if>' . $pep_ref . '<xsl:value-of select="search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/></A><xsl:if test="search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa">.<xsl:value-of select="search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide_next_aa"/></xsl:if>' . $table_spacer . '</td>';
}

my $peptide_ref = '<A TARGET="Win2" HREF="' .  $cgi_bin . 'ModelParser.cgi?Xmlfile={/pepx:msms_pipeline_analysis/@summary_xml}&amp;Timestamp={$pepproph_timestamp}&amp;Spectrum={@spectrum}&amp;Scores={$scores}&amp;Prob={$prob}">';



$display{'probability'} = '<td><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']">' . $peptide_ref . '<xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis=\'adjusted\'"><font color="#FF00FF"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability"/></font></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis) or not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis=\'adjusted\')"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability"/></xsl:if></A></xsl:if><xsl:if test="not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\'])">N_A</xsl:if>' . $table_spacer . '</td>';


my $prot_ref = '<A TARGET="Win2" HREF="' . $cgi_bin . 'comet-fastadb.cgi?Ref={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein}&amp;Db={$Database}&amp;Pep={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide}&amp;MassType={$masstype}">';
my $alt_ref = '<A TARGET="Win2" HREF="' . $cgi_bin . 'comet-fastadb.cgi?Ref={$AltProtein}&amp;Db={$Database}&amp;Pep={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide}&amp;MassType={$masstype}">';

my $prot_ref2 = '<A TARGET="Win2" HREF="' . $cgi_bin . 'comet-fastadb.cgi?Db={$Database}&amp;Pep={pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide}&amp;MassType={$masstype}&amp;sample_enzyme={parent::node()/pepx:sample_enzyme/@name}&amp;min_ntt={$minntt}">';




if($show_prots eq 'checked') {
    $display{'protein'} = '<td>' . $prot_ref2 . '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein"/></A><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:alternative_protein"><xsl:variable name="AltProtein" select="@protein"/><xsl:text> </xsl:text>' . $alt_ref . '<xsl:value-of select="@protein"/></A></xsl:for-each>' . $spacer . '</td>';
}
else {
# alternative listing only first entry, with link to all
    $display{'protein'} = '<td>' . $prot_ref2 . '<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/@protein"/></A><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tot_proteins &gt; 1"><xsl:if test="parent::node()/pepx:analysis_timestamp[@analysis=\'database_refresh\']"><xsl:text> </xsl:text>' . $prot_ref2 . '+<xsl:value-of select="number(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@num_tot_proteins)-1"/></A></xsl:if></xsl:if>' . $spacer . '</td>';
}

$header{'generic_scores'} = '<td><font color="#FF8C00"><b>search scores</b></font></td>';




$SEQ_display{'xcorr'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'xcorr\']/@value"/>' . $table_spacer . '</td>';
$SEQ_display{'deltacn'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltacn\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltacnstar\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$SEQ_display{'sprank'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'sprank\']/@value"/>' . $table_spacer . '</td>';
$SEQ_header{'xcorr'} = '<td align="center"><font color="#FF8C00"><b>xcorr</b></font></td>';
$SEQ_header{'deltacn'} = '<td align="center"><font color="#FF8C00"><b>dCn</b></font></td>';
$SEQ_header{'sprank'} = '<td align="center"><font color="#FF8C00"><b>sprank</b></font></td>';


$MAS_display{'ionscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'ionscore\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'star\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$MAS_display{'idscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'identityscore\']/@value"/>' . $table_spacer . '</td>';
$MAS_display{'homologyscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'homologyscore\']/@value"/>' . $table_spacer . '</td>';
$MAS_header{'ionscore'} = '<td><font color="#FF8C00"><b>ionscore</b></font></td>';
$MAS_header{'idscore'} = '<td><font color="#FF8C00"><b>id score</b></font></td>';
$MAS_header{'homologyscore'} = '<td><font color="#FF8C00"><b>homology score</b></font></td>';

$COM_display{'dotproduct'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'dotproduct\']/@value"/>' . $table_spacer . '</td>';
$COM_header{'dotproduct'} = '<td><font color="#FF8C00"><b>dot product</b></font></td>';
$COM_display{'delta'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'delta\']/@value"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'deltastar\']/@value=\'1\'">*</xsl:if>' . $table_spacer . '</td>';
$COM_header{'delta'} = '<td><font color="#FF8C00"><b>delta</b></font></td>';
$COM_display{'zscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'zscore\']/@value"/>' . $table_spacer . '</td>';
$COM_header{'zscore'} = '<td><font color="#FF8C00"><b>zscore</b></font></td>';

$TAN_display{'hyperscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'hyperscore\']/@value"/>' . $table_spacer . '</td>';
$TAN_header{'hyperscore'} = '<td><font color="#FF8C00"><b>hyperscore</b></font></td>';

$TAN_display{'nextscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'nextscore\']/@value"/>' . $table_spacer . '</td>';
$TAN_header{'nextscore'} = '<td><font color="#FF8C00"><b>nextscore</b></font></td>';

$TAN_display{'bscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'bscore\']/@value"/>' . $table_spacer . '</td>';
$TAN_header{'bscore'} = '<td><font color="#FF8C00"><b>bscore</b></font></td>';
$TAN_display{'yscore'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'yscore\']/@value"/>' . $table_spacer . '</td>';
$TAN_header{'yscore'} = '<td><font color="#FF8C00"><b>yscore</b></font></td>';

$TAN_display{'expect'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'expect\']/@value"/>' . $table_spacer . '</td>';
$TAN_header{'expect'} = '<td><font color="#FF8C00"><b>expect</b></font></td>';


$SPC_display{'dot'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'dot\']/@value"/>' . $table_spacer . '</td>';
$SPC_header{'dot'} = '<td><font color="#FF8C00"><b>dot</b></font></td>';

$SPC_display{'delta'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'delta\']/@value"/>' . $table_spacer . '</td>';
$SPC_header{'delta'} = '<td><font color="#FF8C00"><b>delta</b></font></td>';

$SPC_display{'dot_bias'} = '<td align="center"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank = \'1\']/pepx:search_score[@name=\'dot_bias\']/@value"/>' . $table_spacer . '</td>';
$SPC_header{'dot_bias'} = '<td><font color="#FF8C00"><b>dot_bias</b></font></td>';


my $libra_tag = $libra_norm eq '' ? '@absolute' : '@normalized';
my $libra_pre = $libra_norm eq '' ? '' : '(norm)';
$header{'libra'} = '<xsl:if test="$libra_count=\'1\'"><xsl:for-each select="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']/pepx:libra_summary/pepx:fragment_masses"><td><font color="#FF8C00"><b>libra' . $libra_pre . ' <xsl:value-of select="@mz"/></b></font>' . $table_spacer . '</td></xsl:for-each></xsl:if>';


$display{'libra'} = '<xsl:if test="$libra_count=\'1\'"><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'libra\']/pepx:libra_result/pepx:intensity"><td><xsl:value-of select="' . $libra_tag . '"/>' . $table_spacer . '</td></xsl:for-each></xsl:if>';



$display{'scores'} .= '<xsl:if test="$search_engine=\'SEQUEST\'">';
$display{'scores'} .= $SEQ_display{'xcorr'} . $SEQ_display{'deltacn'} . $SEQ_display{'sprank'} . '</xsl:if>';
$display{'scores'} .= '<xsl:if test="$search_engine=\'MASCOT\'">';
$display{'scores'} .= $MAS_display{'ionscore'} . $MAS_display{'idscore'} . $MAS_display{'homologyscore'} . '</xsl:if>';
$display{'scores'} .= '<xsl:if test="$search_engine=\'COMET\'">';
$display{'scores'} .= $COM_display{'dotproduct'} . $COM_display{'delta'} . $COM_display{'zscore'} . '</xsl:if>';
$display{'scores'} .= '<xsl:if test="contains($search_engine,\'X! Tandem\')">';
$display{'scores'} .= $TAN_display{'hyperscore'} . $TAN_display{'nextscore'} . $TAN_display{'expect'}. '</xsl:if>' ;
$display{'scores'} .= '<xsl:if test="contains($search_engine,\'SpectraST\')">';
$display{'scores'} .= $SPC_display{'dot'} . $SPC_display{'delta'} . $SPC_display{'dot_bias'}. '</xsl:if>' ;

$header{'sequest_scores'} = $SEQ_header{'xcorr'} . $SEQ_header{'deltacn'} . $SEQ_header{'sprank'};
$header{'mascot_scores'} = $MAS_header{'ionscore'} . $MAS_header{'idscore'} . $MAS_header{'homologyscore'};
$header{'comet_scores'} = $COM_header{'dotproduct'} . $COM_header{'delta'} . $COM_header{'zscore'};
$header{'tandem_scores'} = $TAN_header{'hyperscore'} . $TAN_header{'nextscore'} . $TAN_header{'expect'};
$header{'spectrast_scores'} = $SPC_header{'dot'} . $SPC_header{'delta'} . $SPC_header{'dot_bias'};


$anotherheader{'scores'} = '<xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])=\'1\'"><xsl:choose><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SEQUEST\'">' . $header{'sequest_scores'} . '</xsl:when><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'MASCOT\'">' . $header{'mascot_scores'} . '</xsl:when><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'">' . $header{'comet_scores'} . '</xsl:when><xsl:when test="contains(/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine,\'X! Tandem\')">' . $header{'tandem_scores'} . '</xsl:when><xsl:when test="contains(/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine,\'SpectraST\')">' . $header{'spectrast_scores'} . '</xsl:when>' .  '<xsl:otherwise><font color="#FF8C00"><b>search scores</b></font></xsl:otherwise></xsl:choose></xsl:if><xsl:if test="count(pepx:msms_run_summary/pepx:search_summary/@search_engine[generate-id()=generate-id(key(\'search_engine\',.)[1])])&gt;\'1\'">' . $header{'generic_scores'} . '</xsl:if>';

print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">', "\n";
print XSL '<xsl:key name="search_engine" match="/pepx:msms_pipeline_analysis//pepx:msms_run_summary/pepx:search_summary/@search_engine" use="."/>';
print XSL '<xsl:key name="libra_channels" match="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']/pepx:libra_summary/@channel_code" use="."/>';
print XSL '<xsl:variable name="libra_count" select="count(/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'libra\']/pepx:libra_summary/@channel_code[generate-id()=generate-id(key(\'libra_channels\',.)[1])])"/>';


print XSL '<xsl:template match="pepx:msms_pipeline_analysis">';

print XSL '<xsl:variable name="masstype"><xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/@precursor_mass_type=\'average\'">0</xsl:if><xsl:if test="not(/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/@precursor_mass_type=\'average\')">1</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="fragmasstype"><xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/@fragment_mass_type=\'average\'">0</xsl:if><xsl:if test="not(/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/@fragment_mass_type=\'average\')">1</xsl:if></xsl:variable>';
print XSL '<xsl:variable name="asap_quantHighBG">';
print XSL '			<xsl:choose>';
print XSL '				<xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'quantHighBG\']">';
print XSL '			                      <xsl:choose>';
print XSL '				                      <xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@value=\'True\']">';
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
print XSL '				<xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'zeroBG\']">';
print XSL '			                      <xsl:choose>';
print XSL '				                      <xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@value=\'True\']">';
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
print XSL '				<xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'mzBound\']">';
print XSL '				       <xsl:value-of select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_summary[@analysis=\'asapratio\']/pepx:parameter[@name=\'mzBound\']/@value"/>';
print XSL '				</xsl:when>';
print XSL '				<xsl:otherwise>';
print XSL '					<xsl:value-of select="0.5"/>';
print XSL '				</xsl:otherwise>';
print XSL '			</xsl:choose>';
print XSL '		</xsl:variable>';

print XSL '<xsl:variable name="aa_mods"><xsl:for-each select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/pepx:aminoacid_modification"><xsl:value-of select="@aminoacid"/><xsl:if test="@symbol"><xsl:value-of select="@symbol"/></xsl:if>-<xsl:value-of select="@mass"/>:</xsl:for-each></xsl:variable>';
print XSL '<xsl:variable name="term_mods"><xsl:for-each select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/pepx:terminal_modification"><xsl:value-of select="@terminus"/><xsl:if test="@symbol"><xsl:value-of select="@symbol"/></xsl:if>-<xsl:value-of select="@mass"/>:</xsl:for-each></xsl:variable>';

print XSL '<xsl:variable name="asap_time" select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:analysis_timestamp[@analysis=\'asapratio\']/@time"/>';

print XSL '<xsl:variable name="pepproph_timestamp" select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:analysis_timestamp[@analysis=\'peptideprophet\']/@time"/>';

print XSL '<xsl:variable name="comet_md5_check_sum"><xsl:if test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@search_engine=\'COMET\'"><xsl:value-of select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/pepx:parameter[@name=\'md5_check_sum\']/@value"/></xsl:if></xsl:variable>';

print XSL '<xsl:variable name="minntt"><xsl:choose><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/pepx:enzymatic_search_constraint and /pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:sample_enzyme/@sample_enzyme"><xsl:value-of select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary/pepx:enzymatic_search_constraint/@min_number_termini"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose></xsl:variable>';

print XSL '<xsl:variable name="lib_path"><xsl:choose><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary[@search_engine=\'SpectraST\']"><xsl:value-of select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/pepx:search_summary[@search_engine=\'SpectraST\']/pepx:parameter[@name=\'spectral_library\']/@value"/></xsl:when><xsl:otherwise>NOT_FOUND</xsl:otherwise></xsl:choose></xsl:variable>';

print XSL '<xsl:variable name="lib_offset"><xsl:choose><xsl:when test="pepx:search_result/pepx:search_hit/pepx:search_score[@name=\'lib_file_offset\']"><xsl:value-of select="pepx:search_result/pepx:search_hit/pepx:search_score[@name=\'lib_file_offset\']/@value"/></xsl:when><xsl:otherwise>NOT_FOUND</xsl:otherwise></xsl:choose></xsl:variable>';

print XSL '<xsl:variable name="enzyme" select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme//pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:sample_enzyme/@name"/>';
#print XSL '<xsl:variable name="basename" select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@base_name"/>';
print XSL '<xsl:variable name="qry_type"><xsl:value-of select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/@raw_data"/></xsl:variable>';
print XSL '<xsl:variable name="Database"><xsl:choose><xsl:when test="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_timestamp[@analysis=\'database_refresh\']"><xsl:value-of select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:analysis_timestamp[@analysis=\'database_refresh\']/pepx:database_refresh_timestamp/@database"/></xsl:when><xsl:otherwise><xsl:value-of select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary/pepx:search_summary/pepx:search_database/@local_path"/></xsl:otherwise></xsl:choose></xsl:variable>';
print XSL '<xsl:variable name="xpress_display" select="/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:search_summary//pepx:analysis_timestamp[@analysis=\'xpress\']/pepx:xpressratio_timestamp/@xpress_light"/>';
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

print XSL '<tr><td><font color="#FF8C00"><b>entry</b></font></td>';

print XSL '<td><font color="#FF8C00"><b>index</b></font>' . $table_spacer . '</td><td align="center"><font color="#FF8C00"><b>prob</b></font>' . $table_spacer . '</td>';
print XSL '<td><font color="#FF8C00"><b>spectrum</b></font>' . $table_spacer . '</td>';

print XSL $anotherheader{'scores'};
print XSL $header{'libra'};

print XSL '<xsl:choose><xsl:when test="pepx:msms_run_summary/pepx:search_summary/@search_engine=\'SpectraST\'"></xsl:when><xsl:otherwise><td><font color="#FF8C00"><b>ions</b></font>' . $table_spacer . '</td></xsl:otherwise></xsl:choose>';

print XSL '<td><font color="#FF8C00"><b>peptide</b></font>' . $table_spacer . '</td>';
print XSL '<td><font color="#FF8C00"><b>protein</b></font>' . $table_spacer . '</td>';
if($ratio_type == 1) {
    print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'xpress\']"><td><font color="#FF8C00"><b>(H/L) xpress</b></font>' . $table_spacer . '</td></xsl:if>';
    print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'asapratio\']"><td><font color="#FF8C00"><b>(H/L) asapratio</b></font>' . $table_spacer . '</td></xsl:if>';


}
else {
    print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'xpress\']"><td><font color="#FF8C00"><b>xpress</b></font>' . $table_spacer . '</td></xsl:if>';
    print XSL '<xsl:if test="/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis=\'asapratio\']"><td><font color="#FF8C00"><b>asapratio</b></font>' . $table_spacer . '</td></xsl:if>';
}


#print XSL '<td><font color="#FF8C00"><b>fval</b></font>' . $table_spacer . '</td>';

print XSL '</tr>';
print XSL "\n";




if($pep_mass && $error > 0) {
    if ($charge ne "") {
	print XSL '<xsl:apply-templates select="pepx:msms_run_summary/pepx:spectrum_query[pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide=\'' . $peptide . '\' and @assumed_charge=\'' . $charge . '\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass -' . $pep_mass . '&lt;=\'' . $error . '\' and ' . $pep_mass . ' - pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass &lt;=\'' . $error . '\']">', "\n";
    }
    else {
	print XSL '<xsl:apply-templates select="pepx:msms_run_summary/pepx:spectrum_query[pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide=\'' . $peptide . '\' and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass -' . $pep_mass . '&lt;=\'' . $error . '\' and ' . $pep_mass . ' - pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass &lt;=\'' . $error . '\']">', "\n";
    }
}
else {
    if ($charge ne "") {
	print XSL '<xsl:apply-templates select="pepx:msms_run_summary/pepx:spectrum_query[pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide=\'' . $peptide . '\' and @assumed_charge=\'' . $charge . '\']">', "\n";
    }
    else {
	print XSL '<xsl:apply-templates select="pepx:msms_run_summary/pepx:spectrum_query[pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide=\'' . $peptide . '\']">', "\n";

    }

}
print XSL  '<xsl:with-param name="summaryxml" select="$summaryxml"/>
            <xsl:with-param name="pepproph_flag" select="$pepproph_flag"/>
            <xsl:with-param name="asapratio_flag" select="$asapratio_flag"/>
	    <xsl:with-param name="xpress_display" select="$xpress_display"/>
	    <xsl:with-param name="qry_type" select="$qry_type"/>
	    <xsl:with-param name="Database" select="$Database"/>
	    <xsl:with-param name="enzyme" select="$enzyme"/>
	    <xsl:with-param name="lib_path" select="$lib_path"/>
	    <xsl:with-param name="aa_mods" select="$aa_mods"/>
	    <xsl:with-param name="term_mods" select="$term_mods"/>
	    <xsl:with-param name="asap_time" select="$asap_time"/>
	    <xsl:with-param name="asap_mzBound" select="$asap_mzBound"/>
	    <xsl:with-param name="asap_zeroBG" select="$asap_zeroBG"/>
	    <xsl:with-param name="asap_quantHighBG" select="$asap_quantHighBG"/>
	    <xsl:with-param name="masstype" select="$masstype"/>
	    <xsl:with-param name="fragmasstype" select="$fragmasstype"/>
	    <xsl:with-param name="lib_offset" select="$lib_offset"/>
	    <xsl:with-param name="minntt" select="$minntt"/>
            <xsl:with-param name="comet_md5_check_sum" select="$comet_md5_check_sum"/>
	    <xsl:with-param name="xpress_flag" select="$xpress_flag"/>	', "\n";
print XSL '</xsl:apply-templates>'; 
print XSL '</xsl:template>';

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
		<xsl:param name="qry_type" /> 

		<xsl:param name="lib_path" /> 
		<xsl:param name="lib_offset" /> 
                
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

print XSL '<xsl:variable name="summaryxml" select="/pepx:msms_pipeline_analysis/@summary_xml"/>';
print XSL '<xsl:variable name="LightMass" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@light_mass"/>';

print XSL '<xsl:variable name="HeavyMass" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@heavy_mass"/>';

print XSL '<xsl:variable name="MassTol" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'xpress\']/pepx:xpressratio_result/@mass_tol"/>';

print XSL '<xsl:variable name="xpress_index" select="@index"/>';

print XSL '<xsl:variable name="xpress_spec" select="@spectrum"/>';

print XSL '<xsl:variable name="Peptide" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/>';
print XSL '<xsl:variable name="StrippedPeptide" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@peptide"/>';
print XSL '<xsl:variable name="Protein" select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@protein"/>';


print XSL '<xsl:variable name="qry_scan"><xsl:value-of select="@start_scan"/></xsl:variable>';



print XSL '<xsl:variable name="pep_mass"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/@calc_neutral_pep_mass"/></xsl:variable>';

print XSL '<xsl:variable name="PeptideMods2"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass">ModN=<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_nterm_mass"/>&amp;</xsl:if><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass">ModC=<xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/@mod_cterm_mass"/>&amp;</xsl:if><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:modification_info/pepx:mod_aminoacid_mass">Mod<xsl:value-of select="@position"/>=<xsl:value-of select="@mass"/>&amp;</xsl:for-each></xsl:if></xsl:variable>';


print XSL '<xsl:variable name="prob"><xsl:value-of select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@probability"/><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/@analysis=\'adjusted\'">a</xsl:if></xsl:variable>';

print XSL '<xsl:variable name="scores"><xsl:if test="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary"><xsl:for-each select="pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis=\'peptideprophet\']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter"><xsl:value-of select="@name"/>:<xsl:value-of select="@value"/><xsl:text> </xsl:text></xsl:for-each></xsl:if></xsl:variable>';


print XSL '<tr>';
printf XSL "<td align=\"center\">%s</td>", "COUNTER";
print XSL $display{'index'};
print XSL $display{'probability'};
print XSL $display{'spectrum'};
print XSL $display{'scores'};
print XSL $display{'libra'};
print XSL $display{'ions'};
print XSL $display{'peptide_sequence'};
print XSL $display{'protein'};
print XSL $display{'xpress'};
print XSL $display{'asap'};
#print XSL $display{'fval'};
print XSL '</tr>';
print XSL "\n";
print XSL '</xsl:template>', "\n";


print XSL '</xsl:stylesheet>', "\n";

close(XSL);

}
