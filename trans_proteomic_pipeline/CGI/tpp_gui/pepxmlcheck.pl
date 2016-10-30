#!/usr/bin/perl -w

#
# routine to check consistency of file refs within pepXML docs
# by Bryan Prazen, Insilicos LLC
# Copyright (C) 2007 Insilicos LLC  All Rights Reserved
#

use strict;
use CGI qw/:standard/;
use CGI::Carp qw(fatalsToBrowser warningsToBrowser);
use CGI::Pretty;
use Cwd 'realpath';
use File::Basename;
use POSIX qw(setsid);
use tpplib_perl; # our exported TPP lib function points

my $www_root = $ENV{'WEBSERVER_ROOT'};

my @basename = '';
my $stylesheet;
my @ssheet = '';
my @search = '';
my $engine = '';
my $xmlschema = '';
my $x = 0;
my $x2 = 0;
my $x3 = 0;
my @stylexml = '';
my $errors = 0;
my $style = '';
my $style2 = '';
my $shtmlcgi = '';
my $shtmlxml = '';

print header;
print start_html(-title=>"pepXMLCheck",
		 -author=>'Insilicos',
		 -encoding=>'UTF-8',
		 -dtd=>'HTML 4.0 Transitional',
		 -style=>'http://localhost/ipp-bin/tpp_gui.css',
		 -bgcolor=>"#c0c0c0",
		 );

    print <<"EOSCRIPT";
    <SCRIPT LANGUAGE=JavaScript>
    function newWindow(url1, name1) {
	var newWindow;
	newWindow = open(url1, name1, "resizable, scrollbars, height = 400, width = 450")
}
 </SCRIPT>
EOSCRIPT

print h1("pepXMLCheck");
my $xmlfile = param('File');

unless($xmlfile){ 
print "usage: http: pepxmlcheck.pl?File=/full/path <br>";
    print "b.prazen 3/07<br>";
   exit(1);
}

#Read pepXML
# handle possibly gzipped pepXML
my $tmpxmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile); # decompress .gz if needed
open(XML, $tmpxmlfile) or die "cannot open XML $xmlfile $!\n";
while(<XML>) {
    if(/(\<msms\_run\_summary\s+base_name="+)(.*?")/) {  
	$basename[$x] = $2;
        $basename[$x] =~ s/"//;
        $x++;
    }
    elsif(/\<?xml-stylesheet\s+type=+/) {
        $stylesheet = $';
    }
    elsif(/(\<search\_summary\s+base_name="+)(.*)("\ssearch_engine="+)(.*)("\sprecursor+)/) {
	$search[$x2] = $2;
        $engine = $4;
        $x2++;
    }
    elsif(/\<msms\_pipeline\_analysis\s+/) {
	$xmlschema = $';       
    }    
}
close(XML);

my $results = `DatabaseParser $tmpxmlfile`;
unlink($tmpxmlfile) if ($tmpxmlfile ne $xmlfile); # did we decompress xml.gz?

my @database = split(/\,/, $results);
print
    '<div align="right">',
	"<i><a href\='javascript:newWindow(\"pepXMLcheckinfo.html\", \"pepXMLCheckhelp\")'>pepXMLCheck help</a></i>",
    "</div>\n";

#pepXMX schema check
if($xmlschema =~ m/pepXML/) {
    print h2("PepXML File: $xmlfile");}
else {
    print "Selected file does not appear to be in the pepXML format.";
    exit(1);}
   
print 	'<TABLE width = "100%">';

# Database
unless($database[0]){
    $basename[0] = "Warning: No database references found";
    $errors++}
print '<TR><TD width = "15%">Database:<TD width = "50%">',join("<br>", @database);
print"<TD>";
my $nodatabases = @database;
for (my $i = 0; $i<$nodatabases; $i++) {
    chomp $database[$i];
    $database[$i] =~ s/\/cygdrive\/c/c:/;
    my $chek = `ls \"$database[$i]\"`;
    unless($chek){
        print "Warning: Database is not in the expected location!";
        $errors++}
print "<br>";}

# MS directories
unless($basename[0]){
    $basename[0] = "Warning: No MS directory references found";
    $errors++}
print "<TR><TD>Directories for MS data:<TD>",join("<br>", @basename);
print "<TD>";
for (my $i = 0; $i<$x; $i++) {
    $basename[$i] =~ s/\/cygdrive\/c/c:/;
    my $chek = `ipp_host \"$basename[$i]\"`;  # expand to .mzXML, mzdata etc
    unless(-e $chek){
        print "Warning: mzXML file is not in the expected location!";
        $errors++}
    print "<br>";}

#StyleSheet
@ssheet = split('"', $stylesheet);
unless($ssheet[3]){
    $ssheet[3] = "Warning: No stylesheet references found";
    $errors++}
print "<TR><TD>Stylesheet: <TD>$ssheet[3]";
my @response = `wget -q -O - \"$ssheet[3]\"`;
my $score;
foreach (@response) {
    $score++ if (m/pepXML/);
    }
if ($score < 1) {
    print "<TD>Warning: Stylesheet is not in the expected location!";
    $errors++}

#read stylesheet looking for pepXML references
else {
    $ssheet[3] =~ /([ISB|schema]\/+)(.*)/; 
    my $sheet = $2;
    $style = "$www_root/ISB/$sheet";
    $style2 = "$www_root/schema/$sheet";
    open(STY, $style) or open(STY, $style2) or die "cannot open stylesheet $style or $style2 $!\n";
    while(<STY>) {
        if(/(name="xmlfile"\s+value="+)(.*?")/) { #name="xmlfile" value="
            $stylexml[$x3] = $2;
            $stylexml[$x3] =~ s/"//;
            $x3++;
            }
        }
    close(STY); 
    }

#SHTML
my $shtml = $style;
$shtml =~ s/xsl/shtml/;
$shtmlxml = $xmlfile;
my $chek = `ls \"$shtml\"`;
    if($chek){
open(SHTM, $shtml) or die "cannot open shtml $shtml $!\n";
    while(<SHTM>) {
        if(/(cgi="+)(.*)(\?xmlfile=)(.*?&)/) { 
            $shtmlcgi = $2;
            $shtmlxml = $4;
            $shtmlxml =~ s/&//;
            }
        if(/(virtual="+)(.*)(\?xmlfile=)(.*?&)/) { 
            $shtmlcgi = $2;
            $shtmlxml = $4;
            $shtmlxml =~ s/&//;
            }
        }
close(SHTM); 
    }
    

#Search data
print "<TR><TD>Search data directory:<TD>",join("<br>", @search);
print "<TD>";
for (my $i = 0; $i<$x2; $i++) {
    my $chek = `ls \"$search[$i].tgz\"`;         
    unless($chek){
        $search[$i] =~ s/\/cygdrive\/c/c:/;
        $chek = `ls \"$search[$i]\"`;
        unless($chek){
        print "Warning: Search results are not in the expected location!";
        $errors++}
    }
    print "<br>";}
print "<TR><TD>";
print "</TABLE>";

#Scan stylesheet for bad paths
for (my $i = 0; $i<$x3; $i++) {
    $stylexml[$i] =~ s/\/cygdrive\/c/c:/;
    my $chek = `ls \"$stylexml[$i]\"`;
    if($chek){
    $stylexml[$i] = realpath($stylexml[$i]);
    $xmlfile = realpath($xmlfile);
    if($stylexml[$i] ne $xmlfile){
        print "The style sheet contains references to this pepXML in a different location.  This might cause problems with the pepXML viewer.  <br>(pepXML path = $stylexml[$i])<br>";
        $errors++}
    }
    else{
        print "The style sheet contains references to pepXML in a location that does not exist.  This might cause problems with the pepXML viewer.  <br>(pepXML path = $stylexml[$i])<br>";
        $errors++}
}
# check shtml for bad information
$shtmlxml =~ s/\/cygdrive\/c/c:/;
    $chek = `ls \"$shtmlxml\"`;
    if($chek){
    $shtmlxml = realpath($shtmlxml);
    $xmlfile = realpath($xmlfile);
    if($shtmlxml ne $xmlfile){
        print "The .shtml file contains references to this pepXML in a different location.  This might cause problems with the pepXML viewer.  <br>(pepXML path = $shtmlxml)<br>";
        $errors++}
    }
    else{
        print "The .shtml file contains references to pepXML in a location that does not exist.  This might cause problems with the pepXML viewer.  <br>(pepXML path = $shtmlxml)<br>";
        $errors++}

#Error display
if($errors == 0){
    print "No errors were found"
    }
elsif($errors > 0){
    print "<br><br>Errors were found in this pepXML file.  This might prevent processing or viewing of these results";}

print"<table width=\"100%\" border=\"0\">",
"<tr><td align=\"right\">",
"<a href=\"http://www.insilicos.com/\" title=\"Insilicos website\" target = \"_blank\"><img width=\"170\" height=\"28\" border=\"0\" src=\"http://localhost/ipp-bin/images/insilicos_logo.png\"/></a>";

print "</body>";
print "</html>";
