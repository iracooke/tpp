#!/usr/bin/perl
#############################################################################
# Program       : show_dataset_derivation.pl                                #
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

print "Content-type: text/html\n\n";

%box = &tpplib_perl::read_query_string;      # Read keys and values
$| = 1; # autoflush


my $xmlfile;
my $htmlfile;
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



my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'rules="all" cellpadding="2" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
my $RESULT_TABLE_SUF = '</table>';


if(! open(XSL, ">$xmlfile.tmp.xsl")) {
    print "cannot open $xmlfile.tmp.xsl $!\n";
    exit(1);
}

print XSL '<?xml version="1.0"?>';
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">';
print XSL '<xsl:template match="pepx:msms_pipeline_analysis">';


print XSL '<HTML><BODY BGCOLOR="white" OnLoad="self.focus()"><pre>', "\n";
print XSL '<HEAD><TITLE>XML Viewer: Dataset Derivation (' . $TPPVersionInfo . ')</TITLE></HEAD>';

print XSL $RESULT_TABLE_PRE . $RESULT_TABLE;
print XSL '<tr><td><b><font color="brown">Filter Number</font></b></td><td><b><font color="brown">Data File</font></b></td><td><b><font color="brown">Filter</font></b></td></tr>';

print XSL '<xsl:apply-templates select="pepx:dataset_derivation/data_filter">';

print XSL '<xsl:sort select="@number" order="ascending" data-type="number"/>';
print XSL '</xsl:apply-templates>';

print XSL $RESULT_TABLE_SUF;
print XSL '</pre></BODY></HTML>', "\n";
print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:data_filter">';
print XSL '<xsl:variable name="parent_file"><xsl:value-of select="@parent_file"/></xsl:variable>';
print XSL '<tr><td><xsl:value-of select="@number"/></td><td><a target="Win1" href="{$parent_file}"><xsl:if test="@windows_parent"><xsl:value-of select="@windows_parent"/></xsl:if><xsl:if test="not(@windows_parent)"><xsl:value-of select="@parent_file"/></xsl:if></a></td><td><xsl:value-of select="@description"/></td></tr>';
print XSL '</xsl:template>';

print XSL '</xsl:stylesheet>';

close(XSL);

# now apply to xml
my $tmpxmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile); # decompress .gz if needed

if($xslt =~ /xsltproc/) {
    open HTML, "$xslt $xmlfile.tmp.xsl $tmpxmlfile |"; 
}
else {
    open HTML, "$xslt $tmpxmlfile $xmlfile.tmp.xsl |"; 
}
while(<HTML>) {
    print;
}
close(HTML);

#unlink("$xmlfile.tmp.xsl") if(-e "$xmlfile.tmp.xsl");
unlink($tmpxmlfile) if ($tmpxmlfile ne $xmlfile); # did we decompress xml.gz?




