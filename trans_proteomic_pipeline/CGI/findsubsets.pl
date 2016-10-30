#!/usr/bin/perl

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


print "<HTML><BODY BGCOLOR=\"\#FFFFFF\" OnLoad=\"self.focus();\" ><PRE>";
print "<TITLE>Find Subsumed Entries (" . $TPPVersionInfo . ")</TITLE>";
my $xmlfile_in = $box{'Xmlfile'};
if ($^O eq 'MSWin32' ) {
	$xmlfile_in =~ s/\\/\//g;  # get those path seps pointing right!
}
# if $xmlfile_in is gzipped, returns tmpfile name, else returns $xmlfile_in
my $xmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile_in); 

#my $xslt = $box{'Xslt'};
my $protein = $box{'Protein'};

my $xslfile = $xmlfile . '.tmp.xsl';


my $NEW_XML_FORMAT = exists $box{'xml_input'} ? 1 : isXML_INPUT($xmlfile);



my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'cellpadding="0" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
my $RESULT_TABLE_SUF = '</table>';



writeXSLFile($xslfile, \%box);
printHTML($xslt, $xmlfile, $xslfile);

if(0) {
open HTML, "$xslt $xslfile $xmlfile |";
while(<HTML>) {
    print;
}
close(HTML);
}

unlink($xmlfile) if ($xmlfile ne $xmlfile_in); # did we decompress protxml.gz?


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
#$mark_aa .= 'C' if($ICAT && $mark_aa !~ /C/ && $mark_aa !~ /c/);
$box{'mark_aa'} = $mark_aa;


my @select_aas = exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '') ? split('', $box{'pep_aa'}) : ();
my $color = (exists $box{'mark_aa'} && ! ($box{'mark_aa'} eq '')) || (exists $box{'pep_aa'} && ! ($box{'pep_aa'} eq '')) || $glyc; 

my $space = ' ';
my $sort_index = 0;
my $DEBUG = 0;
open(OUT, ">temp.html") if($DEBUG);

my $counter = 0;
my $table_size = 200; #500; # write table subsets so can be displayed by browser as html is passed from xslt
my $table_specs = $RESULT_TABLE_SUF . $RESULT_TABLE_PRE . $RESULT_TABLE;

my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';

if(useXMLFormatLinks($xmlfile)) {
    if($quant_light2heavy eq 'true') {
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html.pl?xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;Ref=';
    }
    else { # add ratioType
	$html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptidexml_html.pl?xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType=1&amp;Ref=';
    }
}

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

	if(! useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?$CGI_HOME.*?peptide\_html\.pl.*?\>\-?\-?\d\_)([A-Z,\#,\@,\*]+)(.*?)$/) {
	    my $ok = 1;
	    my $first = $1;
	    my $second = $2;
	    my $third = $3;

	    if(@select_aas > 0) {
		for(my $s = 0; $s < @select_aas; $s++) {
		    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
		    $ok = 0 if($second !~ /$alt/);
		}
	    }
	    print $first, colorAAs($second, $mark_aa, $glyc), $third if($ok);
	    $reject = ! $ok;
	}
	elsif(useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?$CGI_HOME.*?peptidexml\_html\.pl.*?\>\-?\-?\d\_)([A-Z,\#,\@,\*]+)(.*?)$/) {
	    my $ok = 1;
	    my $first = $1;
	    my $second = $2;
	    my $third = $3;

	    if(@select_aas > 0) {
		for(my $s = 0; $s < @select_aas; $s++) {
		    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
		    $ok = 0 if($second !~ /$alt/);
		}
	    }
	    print $first, colorAAs($second, $mark_aa, $glyc), $third if($ok);
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


# last entry
if(! useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?CGI_HOME.*?peptide\_html\.pl.*?\>\-?\-?\d\_)([A-Z,\#,\@,\*]+)(.*?)$/) {
    my $ok = 1;
    my $first = $1;
    my $second = $2;
    my $third = $3;

    if(@select_aas > 0) {
	for(my $s = 0; $s < @select_aas; $s++) {
	    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
	    $ok = 0 if($second !~ /$alt/);
	}
    }
    print $first, colorAAs($second, $mark_aa, $glyc), $third if($ok);
    $reject = ! $ok;
}
elsif(useXMLFormatLinks($xmlfile) && $start && $printout && $color && $nextline =~ /^(.*?CGI_HOME.*?peptidexml\_html\.pl.*?\>\-?\-?\d\_)([A-Z,\#,\@,\*]+)(.*?)$/) {
    my $ok = 1;
    my $first = $1;
    my $second = $2;
    my $third = $3;

    if(@select_aas > 0) {
	for(my $s = 0; $s < @select_aas; $s++) {
	    my $alt = $select_aas[$s] =~ /[A-Z,a-z,\#]/ ? $select_aas[$s] : '\\' . $select_aas[$s];
	    $ok = 0 if($second !~ /$alt/);
	}
    }
    print $first, colorAAs($second, $mark_aa, $glyc), $third if($ok);
    $reject = ! $ok;
}
elsif($text1 && $printout) {
    print $nextline;
}
elsif(! $start || $printout) {
    print $nextline;
}


close(HTML); 


close(OUT) if($DEBUG);
}


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
    $peptide =~ s/$next_alt([\@,\*,\#]*)/\<font color\=\"red\"\>$next$1\<\/font\>/g;
}
return $peptide;
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

sub useXMLFormatLinks {
(my $file) = @_;
return $file =~ /jimmy2\/xpress\-xml\/test/ || $file =~ /guest\/holdren\/040204c/ || $file =~ /guest\/catherine\/20040408\/HumanBrainICAT\_LTQ\_032304/ || $file =~ /26\/combined/ || $file =~ /nesvi\/Mascot\/ISBsearch\/HINF/ || $file =~ /jimmy2\/comet2xml/ || $file =~ /guest\/catherine\/20040527/ || $file =~ /akeller/;
}



# # taken off the web
# sub read_query_string
# {
#     local ($buffer, @pairs, $pair, $name, $value, %FORM);
#     # Read in text
#     $ENV{'REQUEST_METHOD'} =~ tr/a-z/A-Z/;
#     if ($ENV{'REQUEST_METHOD'} eq "POST")
#     {
#         read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'});
#     } else
#     {
#         $buffer = $ENV{'QUERY_STRING'};
#     }
#     @pairs = split(/&/, $buffer);
#     foreach $pair (@pairs)
#     {
#         ($name, $value) = split(/=/, $pair);
#         $value =~ tr/+/ /;
#         $value =~ s/%(..)/pack("C", hex($1))/eg;
#         $FORM{$name} = $value;
#     }
#     %FORM;
# }



sub writeXSLFile {
(my $xfile, my $boxptr) = @_;

if(! open(XSL, ">$xfile")) {
    print " findsubsets.pl cannot open XSL $xfile: $!\n";
    exit(1);
}
print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:protx="http://regis-web.systemsbiology.net/protXML">', "\n";

my $tab = '<xsl:value-of select="$tab"/>';
my $newline = '<br/><xsl:value-of select="$newline"/>';
$newline = '<xsl:value-of select="$newline"/>' if($tab_delim);
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


my $peptide_prophet_check1 = 'count(peptide_prophet_summary) &gt; \'0\'';
my $peptide_prophet_check2 = 'count(parent::node()/peptide_prophet_summary) &gt; \'0\'';

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
my $minntt_display = $minntt > 0 ? $minntt : '';

my $asap_display = ! exists ${$boxptr}{'asap_display'} || ${$boxptr}{'asap_display'} eq 'show' ? $checked : '';
my $xpress_display = ! exists ${$boxptr}{'xpress_display'} || ${$boxptr}{'xpress_display'} eq 'show' ? $checked : '';

print XSL '<xsl:variable name="tab"><xsl:text>&#x09;</xsl:text></xsl:variable>', "\n";
print XSL '<xsl:variable name="newline"><xsl:text>', "\n";
print XSL '</xsl:text></xsl:variable>';

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
if($xmlfile =~ /\/bernd\//) {
    $tab_display{'protein'} .= $tab . 'http://regis.systemsbiology.net' . $CGI_HOME . 'comet-fastadb.cgi?Ref=<xsl:value-of select="parent::node()/@protein_name"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Db=<xsl:value-of select="/protein_summary/protein_summary_header/@reference_database"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Pep=<xsl:value-of select="parent::node()/@unique_stripped_peptides"/><xsl:for-each select="parent::node()/indistinguishable_protein">,http://regis.systemsbiology.net' . $CGI_HOME . 'comet-fastadb.cgi?Ref=<xsl:value-of select="@protein_name"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Db=<xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@reference_database"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Pep=<xsl:value-of select="parent::node()/@unique_stripped_peptides"/></xsl:for-each>';
    $header{'protein'} = 'protein' . $tab . 'protein link';
}    


$default_order{'protein'} = -1;

$display{'coverage'} = '<xsl:value-of select="@percent_coverage"/>';
$header{'coverage'} = 'percent coverage';
$tab_display{'coverage'} = '<xsl:value-of select="parent::node()/@percent_coverage"/>';
$default_order{'coverage'} = -1;

$display{'annotation'} = '<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$header{'annotation'} = 'annotation';
$tab_display{'annotation'} = '<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if>';
$default_order{'annotation'} = -1;


# add here the cgi info for peptide
my $html_peptide_lead = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';
my $html_peptide_lead2 = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'peptide_html.pl?Ref=';

if(useXMLFormatLinks($xmlfile)) {
	$html_peptide_lead = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'peptidexml_html.pl?xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype}&amp;Ref=';
	$html_peptide_lead2 = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'peptidexml_html.pl?xslt=' . $xslt . '&amp;cgi-bin=' . $CGI_HOME . '&amp;ratioType={$ratiotype2}&amp;Ref=';
}
my $html_peptide_mid = '&amp;Infile=';




$display{'peptide_sequence'} = '<td>' . $html_peptide_lead . '{$mycharge}_{$mypep}' . $html_peptide_mid . '{$myinputfiles}">' . 
'<xsl:value-of select="@charge"/>_<xsl:value-of select="@peptide_sequence"/></A></td>';

$display_ind_peptide_seq = '<td>--' . $html_peptide_lead2 . '{$mycharge2}_{$mypep2}' . $html_peptide_mid . '{$myinputfiles2}">' . '<xsl:value-of select="parent::node()/@charge"/>_<xsl:value-of select="@peptide_sequence"/></A></td>';

$header{'peptide_sequence'} = '<td>' . $header_pre . 'peptide sequence' . $header_post . '</td>';
$tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:value-of select="@peptide_sequence"/><xsl:for-each select="indistinguishable_peptide">,<xsl:value-of select="@peptide_sequence"/></xsl:for-each>';
$tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence';

# add special settings for Bernd
if($xmlfile =~ /\/bernd\//) {
    $tab_display{'peptide_sequence'} = '<xsl:value-of select="@charge"/>' . $tab . '<xsl:value-of select="@peptide_sequence"/><xsl:for-each select="indistinguishable_peptide">,<xsl:value-of select="@peptide_sequence"/></xsl:for-each>' . $tab . 'http://regis.systemsbiology.net' . $CGI_HOME . 'peptide_html.pl?Ref=<xsl:value-of select="@charge"/>_<xsl:value-of select="@peptide_sequence"/><xsl:value-of disable-output-escaping="yes" select="$amp"/>Infile=<xsl:value-of select="/protein_summary/protein_summary_header/@source_files_alt"/>';

    $tab_header{'peptide_sequence'} = 'precursor ion charge' . $tab . 'peptide sequence' . $tab . 'peptide link';
}

$default_order{'peptide_sequence'} = 2;

my $wt_header = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'prot_wt_xml.pl?xmlfile=' . $xmlfile . '&amp;cgi-home=' . $CGI_HOME . '&amp;xslt=' . $xslt . '&amp;quant_light2heavy=' . $quant_light2heavy . '&amp;peptide=';
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

$display{'num_tol_term'} = '<td><xsl:value-of select="@n_tryptic_termini"/></td>';
$header{'num_tol_term'} = '<td>' . $header_pre . 'ntt' . $header_post . '</td>';
$tab_display{'num_tol_term'} = '<xsl:value-of select="@n_tryptic_termini"/>';
$tab_header{'num_tol_term'} = 'n tol termini';
$default_order{'num_tol_term'} = 5;

my $nsp_pre = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'show_nspbin.pl?xmlfile=' . $xmlfile . '&amp;xslt=' . $xslt . '&amp;nsp_bin={$nspbin}&amp;nsp_val={$nspval}&amp;charge={$mycharge}&amp;pep={$mypep}&amp;prot={$myprots}">';
my $tempnsp = '<td><xsl:if test="@n_sibling_peptides">' . $nsp_pre . '<xsl:value-of select="@n_sibling_peptides"/></A></xsl:if>
<xsl:if test="not(@n_sibling_peptides)"><xsl:value-of select="@n_sibling_peptides_bin"/></xsl:if></td>';
$display{'n_sibling_peptides_bin'} = $tempnsp;

$header{'n_sibling_peptides_bin'} = '<td>' . $header_pre . 'nsp<xsl:if test="not(peptide/@n_sibling_peptides)"> bin</xsl:if>' . $header_post . '</td>';
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

$annot_display{'description'} = '<xsl:if test="protx:annotation/@protein_description"><xsl:if test="not(/protx:protein_summary/protx:protein_summary_header/@organism)"><font color="green"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism"><font color="#008080"><xsl:value-of select="protx:annotation/@protein_description"/></font></xsl:if>' . $table_space . ' </xsl:if>';

$annot_order{'description'} = -1;
$annot_tab_display{'description'} = '<xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@protein_description"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="aprotx:nnotation"><xsl:value-of select="protx:annotation/@protein_description"/></xsl:if></xsl:for-each>';

$header{'description'} = 'description';

my $flybase_header = '<a TARGET="Win2" href="http://flybase.bio.indiana.edu/.bin/fbidq.html?';

$annot_display{'flybase'} = '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism=\'Drosophila\'"><xsl:if test="protx:annotation/@flybase"><xsl:variable name="flybase"><xsl:value-of select="protx:annotation/@flybase"/></xsl:variable>' . $flybase_header . '{$flybase}"><font color="green">Flybase:<xsl:value-of select="$flybase"/></font></a>' . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'flybase'} = 9;
$annot_tab_display{'flybase'} = '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism=\'Drosophila\'"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@flybase"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@flybase"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';

my $ipi_header = '<a TARGET="Win2" href="http://srs.ebi.ac.uk/cgi-bin/wgetz?-id+m_RJ1KrMXG+-e+[IPI-acc:';
my $ipi_mid = ']">';
my $ipi_suf = '</a>';
$annot_display{'ipi'} = '<font color="green">&gt;</font><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism"><xsl:if test="protx:annotation/@ipi_name"><xsl:variable name="ipi"><xsl:value-of select="protx:annotation/@ipi_name"/></xsl:variable>' . $ipi_header . '{$ipi}' . $ipi_mid . '<font color="green">IPI:<xsl:value-of select="$ipi"/></font>' . $ipi_suf . $table_space . ' </xsl:if></xsl:if>';
$annot_order{'ipi'} = -1;
$annot_tab_display{'ipi'} = '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism"><xsl:if test="parent::node()/protx:annotation"><xsl:value-of select="parent::node()/protx:annotation/@ipi_name"/></xsl:if><xsl:for-each select="parent::node()/protx:indistinguishable_protein">,<xsl:if test="protx:annotation"><xsl:value-of select="protx:annotation/@ipi_name"/></xsl:if></xsl:for-each>' . $tab . '</xsl:if>';


$header{'ipi'} = '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism">ipi' . $tab . '</xsl:if>';

my $embl_header = '<a TARGET="Win2" href="http://www.ensembl.org/';
my $embl_mid = '/protview?peptide=';
my $embl_suf = '</a>';
$annot_display{'ensembl'} = '<xsl:if test="protx:annotation/@ensembl_name"><xsl:variable name="org"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@organism"/></xsl:variable><xsl:variable name="ensembl"><xsl:value-of select="protx:annotation/@ensembl_name"/></xsl:variable>' . $embl_header . '{$org}' . $embl_mid . '{$ensembl}"><font color="green">E:<xsl:value-of select="$ensembl"/></font>' . $embl_suf . $table_space . ' </xsl:if>';
$annot_order{'ensembl'} = 3;
$annot_tab_display{'ensembl'} = '<xsl:if test="parent::node()/annotation"><xsl:value-of select="parent::node()/annotation/@ensembl_name"/></xsl:if><xsl:for-each select="parent::node()/indistinguishable_protein">,<xsl:if test="annotation"><xsl:value-of select="annotation/@ensembl_name"/></xsl:if></xsl:for-each>' . $tab;
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

my $locus_header = '<a TARGET="Win2" href="http://www.ncbi.nlm.nih.gov/LocusLink/LocRpt.cgi?l=';
my $locus_suf = '</a>';

$annot_display{'locus_link'} = '<xsl:if test="protx:annotation/@locus_link_name"><xsl:variable name="loc"><xsl:value-of select="protx:annotation/@locus_link_name"/></xsl:variable>' . $locus_header . '{$loc}' . '"><font color="green">LL:<xsl:value-of select="$loc"/></font>' . $locus_suf . $table_space . ' </xsl:if>';
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

my $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'xpress-prophet.cgi?cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source}&amp;heavy2light={$heavy2light}">';

my $xpress_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';

if(useXMLFormatLinks($xmlfile)) {
    $xpress_pre = '<a target="Win2" href="' . $CGI_HOME . 'XPressCGIProteinDisplayParser.cgi?xslt=' . $xslt . '&amp;cgihome=' . $CGI_HOME . '&amp;protein={$mult_prot}&amp;peptide_string={$peptide_string}&amp;ratio={$xratio}&amp;stddev={$xstd}&amp;num={$xnum}&amp;xmlfile=' . $xmlfile . '&amp;min_pep_prob={$min_pep_prob}&amp;source_files={$source}&amp;heavy2light=' . $xpress_ratio_type . '">'; #{$heavy2light}">';
}

my $xpress_suf = '</a>';

$num_cols = 3;



if(! ($xpress_display eq '')) {

    $display{'xpress'} = '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']"><td width="350"><xsl:if test="XPressRatio">XPRESS';
    if(! ($quant_light2heavy eq 'true')) {
	$display{'xpress'} .= '(H/L)';
    }
    $display{'xpress'} .= ': ' . $xpress_pre . '<xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>(<xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>)</xsl:if>' . $xpress_suf . '</xsl:if><xsl:if test="not(protx:analysis_result[@analysis=\'xpress\'])">' . $table_spacer . '</xsl:if></td></xsl:if>';

    $tab_display{'xpress'} = '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']"><xsl:if test="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio"><xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/>' . $tab . '</xsl:if><xsl:if test="not(parent::node()/protx:analysis_result[@analysis=\'xpress\'])">' . $tab . $tab . $tab . '</xsl:if></xsl:if>';
    $header{'xpress'} = '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']">xpress';
    if(! ($quant_light2heavy eq 'true')) {
	$header{'xpress'} .= '(H/L)';
    }
    $header{'xpress'} .= ' ratio mean' . $tab . 'xpress<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope=\'light\'"> (H/L)</xsl:if></xsl:if> stdev' . $tab . 'xpress num peptides' . $tab . '</xsl:if>';
} # if display xpress

my $asap_header_old_version;
my $NEW_ASAP_CGI = 0;
$NEW_ASAP_CGI = 1; # if($DTD_FILE =~ /ProteinProphet\_v(\S+)\.dtd/ && $1 >= 1.9);

my $asap_display_cgi = 'asap-prophet-display.cgi';

if(useXMLFormatLinks($xmlfile)) {
    $asap_display_cgi = 'ASAPCGIDisplay.cgi';
}
my $asap_ratio_type = $quant_light2heavy eq 'true' ? '0' : '1';

if($NEW_ASAP_CGI) {
    $asap_header_old_version = $asap_header_pre;
    $asap_header_pre = '<A NAME="ASAPRatio_proRatio" TARGET="WIN3" HREF="' . $CGI_HOME . $asap_display_cgi . '?ratioType=' . $asap_ratio_type . '&amp;xmlFile=' . $xmlfile . '&amp;protein=';
}

my $asap_header_mid2 = '&amp;ratioType=0">';

my $asap_header_suf = '</A>';
my $pvalue_header_pre = '<a target="Win2" href="';


############ GOT UP TO THIS POINT 2.28.05



my $pvalue_header_suf = '</a>';
if(! ($asap_display eq '')) {
    if($NEW_ASAP_CGI) {


	# first display regular ratio no matter what
	$display{'asapratio'} = '<xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']"><td width="350"><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']">ASAPRatio';
	if(! ($quant_light2heavy eq 'true')) {
	    $display{'asapratio'} .= '(H/L)';
	}
	$display{'asapratio'} .= ': ' . $asap_header_pre . '{$mult_prot}">';
	$display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_standard_dev"/></xsl:if><xsl:if test="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	# now add on the adjusted only if desired and present
	if(! ($show_adjusted_asap eq '') && exists ${$boxptr}{'adj_asap'}) {
	    $display{'asapratio'} .= '<xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\']">[<xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_standard_dev"/>]</xsl:if>';
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']"><td width="200"><xsl:if test="protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if>';



	}
	else {
	    $display{'asapratio'} .= '</xsl:if></td></xsl:if>';
	}

    }
    else { # old format
	if($show_adjusted_asap eq '') {
	    $display{'asapratio'} = '<xsl:if test="count(/protein_summary/protein_group/protein/ASAPRatio) &gt; \'0\'"><xsl:if test="/protein_summary/ASAP_pvalue_analysis_summary"><td width="350"><xsl:if test="ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$file}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="ASAPRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="ASAPRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="ASAPRatio/@ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="ASAPRatio/@ratio_standard_dev"/></xsl:if><xsl:if test="ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="ASAPRatio/@ratio_number_peptides"/>)</xsl:if>' . $asap_header_suf;
	}
	else {
	    $display{'asapratio'} = '<xsl:if test="count(/protein_summary/protein_group/protein/ASAPRatio) &gt; \'0\'"><xsl:if test="/protein_summary/ASAP_pvalue_analysis_summary"><td width="400"><xsl:if test="ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$file}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="ASAPRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="ASAPRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="ASAPRatio/@ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="ASAPRatio/@ratio_standard_dev"/><xsl:if test="ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . ' [<xsl:value-of select="ASAPRatio/@adj_ratio_mean"/> ' . $plusmn . ' <xsl:value-of select="ASAPRatio/@adj_ratio_standard_dev"/>]';
	}
	$display{'asapratio'} .= '</xsl:if></td><td width="200"><xsl:if test="ASAPRatio">pvalue: ' . $pvalue_header_pre . '{$pvalpngfile}"><nobr><xsl:value-of select="ASAPRatio/@pvalue"/></nobr>' . $pvalue_header_suf . '</xsl:if></td></xsl:if><xsl:if test="not(/protein_summary/ASAP_pvalue_analysis_summary)"><td width="350"><xsl:if test="ASAPRatio">ASAPRatio: ' . $asap_header_pre . '{$file}' . $asap_header_mid . '{$asap_ind}' . $asap_header_mid2 . '<xsl:if test="ASAPRatio/@ratio_mean &lt;\'0\'">N_A</xsl:if><xsl:if test="ASAPRatio/@ratio_mean &gt;=\'0\'"><b><xsl:value-of select="ASAPRatio/@ratio_mean"/></b> ' . $plusmn . ' <xsl:value-of select="ASAPRatio/@ratio_standard_dev"/><xsl:if test="ASAPRatio/@ratio_number_peptides">(<xsl:value-of select="ASAPRatio/@ratio_number_peptides"/>)</xsl:if></xsl:if>' . $asap_header_suf . '</xsl:if></td></xsl:if></xsl:if>';
    } # if old version


    $tab_display{'asapratio'} = '<xsl:if test="count(/protein_summary/protein_group/protein/ASAPRatio) &gt; \'0\'"><xsl:if test="parent::node()/ASAPRatio"><xsl:value-of select="parent::node()/ASAPRatio/@ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/ASAPRatio/@ratio_standard_dev"/>' . $tab . '<xsl:value-of select="parent::node()/ASAPRatio/@ratio_number_peptides"/>' . $tab . '</xsl:if><xsl:if test="count(parent::node()/ASAPRatio)= \'0\'">' . 'N_A' . $tab . 'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';
    if(! ($show_adjusted_asap eq '')) {
	$tab_display{'asapratio'} .= '<xsl:if test="/protein_summary/ASAP_pvalue_analysis_summary"><xsl:if test="parent::node()/ASAPRatio"><xsl:value-of select="parent::node()/ASAPRatio/@adj_ratio_mean"/>' . $tab . '<xsl:value-of select="parent::node()/ASAPRatio/@adj_ratio_standard_dev"/>' . $tab . '</xsl:if><xsl:if test="not(parent::node()/ASAPRatio)">' .'N_A' . $tab . 'N_A' . $tab . '</xsl:if></xsl:if>';
    }
    $tab_display{'asapratio'} .= '<xsl:if test="/protein_summary/ASAP_pvalue_analysis_summary"><xsl:if test="parent::node()/ASAPRatio"><xsl:value-of select="parent::node()/ASAPRatio/@pvalue"/></xsl:if>' . $tab . '</xsl:if>';

    $header{'asapratio'} = '<xsl:if test="count(protein_group/protein/ASAPRatio) &gt; \'0\'">ratio mean' . $tab . 'ratio stdev' . $tab . 'ratio num peps' . $tab;
    if(! ($show_adjusted_asap eq '')) {
	$header{'asapratio'} .= '<xsl:if test="/protein_summary/ASAP_pvalue_analysis_summary">adjusted ratio mean' . $tab . 'adjusted ratio stdev' . $tab . '</xsl:if>';
    }
    $header{'asapratio'} .= '<xsl:if test="/protein_summary/ASAP_pvalue_analysis_summary">pvalue' . $tab . '</xsl:if></xsl:if>';
} # if display asapratio info


############ GOT DOWN FROM THIS POINT 2.28.05



my $reference = '$group_number' ; #$show_groups eq '' ? '$parental_group_number' : '$group_number';
my $gn = $show_groups eq '' ? '<xsl:value-of select="parent::node()/@group_number"/>' : '<xsl:value-of select="@group_number"/>';
if($discards) {

    $display{'group_number'} .= '<input type="hidden" name="incl{' . $reference . '}" style="height: 15px; width: 15px;" value="yes"/> ';
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

    $display{'group_number'} .= '<input type="hidden" name="excl';
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

    $display{'prot_number'} .= '<input type="hidden" name="pincl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
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

    $display{'prot_number'} .= '<input type="hidden" name="pexcl' . '{$prot_number}' . '" style="height: 15px; width: 15px;" value="yes"/> ';
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

print XSL '<HTML><BODY BGCOLOR="white" OnLoad="self.focus()"><PRE>', "\n";
print XSL '<HEAD><TITLE>ProteinProphet XML Viewer (' . $TPPVersionInfo . ')</TITLE></HEAD>';



my $sort_none = ! exists ${$boxptr}{'sort'} || ${$boxptr}{'sort'} eq 'none' ?  $checked : '';
my $sort_xcorr = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'xcorr' ? $checked : '';
my $sort_prob = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'prob' ? $checked : '';
my $sort_spec = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'spec' ? $checked : '';
my $sort_pep = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'peptide' ? $checked : '';
my $sort_prot = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'protein' ? $checked : '';
my $sort_cov = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'coverage' ? $checked : '';
my $sort_peps = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'numpeps' ? $checked : '';
my $sort_pvalue = exists ${$boxptr}{'sort'} && ${$boxptr}{'sort'} eq 'pvalue' ? $checked : '';
my $text1 = exists ${$boxptr}{'text1'} && ! (${$boxptr}{'text1'} eq '') ? ${$boxptr}{'text1'} : '';
my $glyc = exists ${$boxptr}{'glyc'} && ${$boxptr}{'glyc'} eq 'yes' ? $checked : '';
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



# make local reference
if(exists ${$boxptr}{'excel'} && ${$boxptr}{'excel'} eq 'yes') {
    my $local_excelfile = $excelfile;
    if($USING_LOCALHOST) {
	if((length $SERVER_ROOT) <= (length $local_excelfile) && 
	   index((lc $local_excelfile), ($LC_SERVER_ROOT)) == 0) {
	    $local_excelfile = '/' . substr($local_excelfile, (length $SERVER_ROOT));
	}
	else {
	    die "problem (fs1): $local_excelfile is not mounted under webserver root: $SERVER_ROOT\n";
	}
	my $windows_excelfile = $excelfile;
	$windows_excelfile =  `cygpath -w '$excelfile'` if ($WINDOWS_CYGWIN);
	if($windows_excelfile =~ /^(\S+)\s?/) {
	    $windows_excelfile = $1;
	}
	print XSL 'excel file: <a target="Win1" href="' . $local_excelfile . '">' . $windows_excelfile . '</a>'  . $newline;

    } # if iis & cygwin
    else { # unix
	print XSL 'excel file: <a target="Win1" href="' . $local_excelfile . '">' . $local_excelfile . '</a>'  . $newline;
    }
}
if((! ($show_sens eq '') && $eligible)) {

  # make local reference
  my $local_pngfile = $pngfile;
  $local_pngfile =~ s/\\/\//g;  # get those path seps pointing right!
  if($USING_LOCALHOST) {
      if((length $SERVER_ROOT) <= (length $local_pngfile) && 
	 index((lc $local_pngfile), ($LC_SERVER_ROOT)) == 0) {
	  $local_pngfile = '/' . substr($local_pngfile, (length $SERVER_ROOT));
      }
      else {
	  die "problem (fs2): $local_pngfile is not mounted under webserver root: $SERVER_ROOT\n";
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



print XSL '<b><font color="blue">' . $protein . '</font></b><p/>';
########################## COUNT ENTRIES  #################################

my $local_xmlfile = $xmlfile;
$local_xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
my $windows_xmlfile = $xmlfile;
$windows_xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
if($USING_LOCALHOST) {
    if((length $SERVER_ROOT) <= (length $local_xmlfile) && 
       index((lc $local_xmlfile), ($LC_SERVER_ROOT)) == 0) {
	$local_xmlfile = '/' . substr($local_xmlfile, (length $SERVER_ROOT));
    }
    else {
	die "problem (fs3): $local_xmlfile is not mounted under webserver root: $SERVER_ROOT\n";
    }
    $windows_xmlfile = `cygpath -w '$windows_xmlfile'` if ($WINDOWS_CYGWIN);
    if($windows_xmlfile =~ /^(\S+)\s?/) {
	$windows_xmlfile = $1;
    }
} # if iis & cygwin

my $MAX_XMLFILE_LEN = 80;
my $format_choice = ($WINDOWS_CYGWIN && (length $windows_xmlfile) > $MAX_XMLFILE_LEN) || 
	(! $WINDOWS_CYGWIN && (length $local_xmlfile) > $MAX_XMLFILE_LEN) ? '<br/>' : '';


if(! exists ${$boxptr}{'text1'} || ${$boxptr}{'text1'} eq '') {

    print XSL '<font color="red"><xsl:value-of select="';
    if($show_groups eq '') { # count prots
	if($discards) {
	    print XSL 'count(protx:protein_group/protx:protein)-';
	}
	print XSL 'count(protx:protein_group/protx:protein[(@probability &gt;= \'' . $minprob . '\'';


	print XSL ' and (@protein_name=\'' . $protein . '\' or (@subsuming_protein_entry and @subsuming_protein_entry=\'' . $protein . '\'))'; 



	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and (protx:analysis_result[@analysis=\'xpress\'] and protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or (protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}
	else {
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and (protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:analysis_result[@analysis=\'xpress\']) or not(protx:analysis_result[@analysis=\'asapratio\']) or(protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev &gt;= \'0\' and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@adj_ratio_mean + protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev - protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}

	print XSL ' and(protx:analysis_result[@analysis=\'asapratio\'] and protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	for(my $e = 0; $e < @exclusions; $e++) {
	    print XSL ' and not(parent::node()/@group_number=\'' . $exclusions[$e] . '\')';
	}
	foreach(@pexclusions) {
	    if(/^(\d+)([a-z,A-Z])$/) {
		print XSL ' and not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')';
	    }
	}

	print XSL ')';

	for(my $i = 0; $i < @inclusions; $i++) {
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
	for(my $e = 0; $e < @exclusions; $e++) {
	    print XSL ' and not(@group_number=\'' . $exclusions[$e] . '\')';
	}
	print XSL ' and (protx:protein/@protein_name=\'' . $protein . '\' or (protx:protein/@subsuming_protein_entry and protx:protein/@subsuming_protein_entry=\'' . $protein . '\'))'; 

	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']) or(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	else { # show adjusted
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' and (not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']) or not(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']) or(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev &gt;= \'0\' and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@adj_ratio_standard_dev - protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean + protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));

	}

	print XSL ' and(protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\'] and protx:protein[@group_sibling_id = \'a\']/protx:analysis_result[@analysis=\'asapratio_pvalue\']/protx:ASAPRatio_pvalue/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

	print XSL ')';
	for(my $i = 0; $i < @inclusions; $i++) {
	    print XSL ' or @group_number=\'' . $inclusions[$i] . '\'';
	}

    } # hide groups
    print XSL '])-1"/>';
    print XSL ' subsumed entries</font>';

    if($USING_LOCALHOST) {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\">" . $windows_xmlfile . '</A>'; #, '<pre>' . $newline . '</pre>';
}
    else {
	print XSL " retrieved from " . $format_choice . "<A href=\"$local_xmlfile\">" . $local_xmlfile . '</A>'; #, '<pre>' . $newline . '</pre>';
    }
} # if count
else {

    print XSL '<font color="black"><i>discarded</i></font> ' if($discards);
    if($USING_LOCALHOST) {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $windows_xmlfile . '</font></A></font>'; 
    }
    else {
	print XSL "<font color=\"red\">entries retrieved from " . $format_choice . "<A href=\"$local_xmlfile\"><font color=\"red\">" . $local_xmlfile . '</font></A></font>'; 
    }

}


###################################################
print XSL $newline . '<pre>' . $newline . '</pre>';
print XSL '<FONT COLOR="990000">* indicates peptide corresponding to unique protein entry</FONT>' . $newline;

# calculate how many columns, and header line here

$num_cols += 8;
my $extra_column = '<td>' . $table_spacer . '</td>';

my $HEADER = '<td><!-- header --></td>';

if($tab_delim) {
    $HEADER = $header{'group_number'} . $tab; # cancel it
    $HEADER .= 'group probability' . $tab if(! ($show_groups eq ''));
    $HEADER .= $header{'protein'} . $tab;
    $HEADER .= 'protein probability' . $tab;
    $HEADER .= $header{'coverage'} . $tab;
    $HEADER .= $header{'xpress'};
    $HEADER .= $header{'asapratio'}; # tab is built in 

    if(! ($show_num_unique_peps eq '')) {
	$HEADER .= 'num unique peps' . $tab;
    }
    if(! ($show_tot_num_peps eq '')) {
	$HEADER .= 'tot num peps' . $tab;
    }

    # now annotation
    if(! ($show_annot eq '')) {
	my $annot_header = $header{'ipi'};
	$annot_header .= $header{'description'} . $tab;

	$annot_header .= '<xsl:if test="/protein_summary/protein_summary_header/@organism">';

	if(scalar keys %display_annot_order > 0) {
	    foreach(sort {$display_annot_order{$a} <=> $display_annot_order{$b}} keys %display_annot_order) {
		$annot_header .= $header{$_}; # . '<xsl:text>  </xsl:text>';
	    }
	}
	else {
	    foreach(sort {$annot_order{$a} <=> $annot_order{$b}} keys %annot_order) {
		if($annot_order{$_} >= 0) {
		    $annot_header .= $header{$_};
		}
	    }
	}
	$annot_header .= '</xsl:if>';
	$HEADER .= $annot_header if(! ($show_annot eq ''));

    }

} # if tab delim
# now comes peptide info....

if(scalar keys %display_order > 0) {
    foreach(sort {$display_order{$a} <=> $display_order{$b}} keys %display_order) {
	if($tab_delim) {
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
	    if($tab_delim) {
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
}

$HEADER .= $extra_column if(! $tab_delim);


my $annotation = $annot_display{'ipi'} . $annot_display{'description'}; 


$annotation .= 	'<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism">';


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
my $prot_header = '<A TARGET="Win2" HREF="' . $CGI_HOME . 'comet-fastadb.cgi?Ref=';
my $prot_suf = '</A>';

print XSL $RESULT_TABLE_PRE . $RESULT_TABLE, "\n";


print XSL '<xsl:comment>' . $start_string . '</xsl:comment>' . $newline . "\n";;
print XSL $HEADER . $newline if($tab_delim);

# bypass protein groups altogether for no groups mode.....
if(! ($show_groups eq '')) {

    print XSL '<xsl:apply-templates select="protx:protein_group[protx:protein/@protein_name=\'' . $protein . '\']"/>';

    print XSL '</table>';
    print XSL '<hr/><p/>';
    print XSL $RESULT_TABLE_PRE . $RESULT_TABLE, "\n";


# modified 10.7.04
    print XSL '<xsl:apply-templates select="protx:protein_group[protx:protein[@subsuming_protein_entry and @subsuming_protein_entry=\'' . $protein . '\']]">';


}
else {
    print XSL '<xsl:apply-templates select="protx:protein_group/protx:protein">', "\n";
}

# not used
if(0) {
if(! ($sort_pvalue) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(ASAPRatio/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * ASAPRatio/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

	#print XSL '<xsl:sort select="-1 * ASAPRatio/@decimal_pvalue" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/ASAPRatio/@decimal_pvalue)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="-1 * protein[@group_sibling_id = \'a\']/ASAPRatio/@decimal_pvalue" order="descending" data-type="number"/>', "\n";

	#print XSL '<xsl:sort select="-1 * protein[@group_sibling_id = \'a\']/ASAPRatio/@decimal_pvalue" order="descending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_xpress_desc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	print XSL 'sort select="XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean' . "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_xpress_asc) eq '') {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
    else {
	    print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/XPressRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";
    }
}
elsif(! ($sort_asap_desc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="descending" data-type="number"/>', "\n";
	}
	else {
	    print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="descending" data-type="number"/>', "\n";
	}
    }
}
elsif(! ($sort_asap_asc) eq '') {
    if($show_groups eq '') {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
	else {
	    print XSL '<xsl:sort select="count(ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
    }
    else {
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'ratio_mean" order="ascending" data-type="number"/>', "\n";

	}
	else {
	    print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean)" order="descending" data-type="number"/>', "\n";
	    print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/ASAPRatio/@' . getRatioPrefix($quant_light2heavy) . 'adj_ratio_mean" order="ascending" data-type="number"/>', "\n";

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
	print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/@protein_name"/>', "\n";

    }
}
elsif(! ($sort_cov eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="count(@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="@percent_coverage" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="count(protein[@group_sibling_id = \'a\']/@percent_coverage)" order="descending" data-type="number"/>', "\n";
	print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/@percent_coverage" order="descending" data-type="number"/>', "\n";

    }
}
elsif(! ($sort_peps eq '')) {
    if($show_groups eq '') {
	print XSL '<xsl:sort select="@total_number_peptides" order="descending" data-type="number"/>', "\n";
    }
    else {
	print XSL '<xsl:sort select="protein[@group_sibling_id = \'a\']/@total_number_peptides" order="descending" data-type="number"/>', "\n";

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
} # if 0

print XSL '</xsl:apply-templates>', "\n";

print XSL $RESULT_TABLE_SUF, "\n";
print XSL '</PRE></BODY></HTML>', "\n";
print XSL '</xsl:template>', "\n";

if(! ($show_groups eq '')) {
print XSL '<xsl:template match="protx:protein_group">', "\n";

my $suffix = '';
if(@inclusions > 0) {
    $suffix = ' or @group_number=\'';
    for(my $i = 0; $i < @inclusions; $i++) {
	$suffix .= $inclusions[$i] . '\'';
	$suffix .= ' or @group_number=\'' if($i < @inclusions - 1);
    }
}

foreach(keys %parent_incls) {
    $suffix .= ' or @group_number=\'' . $_ . '\'';
}    


if($discards) {

    if(! ($show_groups eq '')) {
	# see if fails criteria
	print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protein[@group_sibling_id = \'a\']/XPressRatio) or not(protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protein[@group_sibling_id = \'a\']/XPressRatio) or not(protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protein[@group_sibling_id = \'a\']/XPressRatio) or not(protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/XPressRatio and (protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev &lt; \'0\') or (protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	}
	else {
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or (protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/XPressRatio and (protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev &lt; \'0\') or (protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev &lt; \'0\'))' if(! ($asap_xpress eq ''));

	}
	print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
	# check for all exclusions
	if(@exclusions > 0) {
	    for(my $k = 0; $k < @exclusions; $k++) {
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
	for(my $i = 0; $i < @inclusions; $i++) {
	    print XSL '<xsl:if test="not(@group_number=\'' . $inclusions[$i] . '\')">', "\n";
	}

    } # groups
    else {  # hide groups...want to make sure no singletons pass by default
	print XSL '<xsl:if test="count(protx:protein) &gt;\'1\' or protx:protein[@group_sibling_id=\'a\']/@probability &lt; \'' . $minprob . '\'';
	print XSL ' or not(protein[@group_sibling_id = \'a\']/XPressRatio) or not(protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
	print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));

	print XSL ' or not(protein[@group_sibling_id = \'a\']/XPress) or not(protein[@group_sibling_id = \'a\']/XPress/@ratio_mean &gt;= \'' . $min_xpress . '\')' if($min_xpress > 0);
	print XSL ' or not(protein[@group_sibling_id = \'a\']/XPress) or not(protein[@group_sibling_id = \'a\']/XPress/@ratio_mean &lt;= \'' . $max_xpress . '\')' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/XPressRatio and (protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev &gt;= \'0\') or (protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	    print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/XPressRatio and (protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev &gt;= \'0\') or (protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' if(! ($asap_xpress eq ''));
	}
	print XSL ' or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);

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

    for(my $e = 0; $e < @exclusions; $e++) {
	print XSL '<xsl:if test="not(@group_number=\'' . $exclusions[$e] . '\')">', "\n";
    }
    if(! ($show_groups eq '')) {

	print XSL '<xsl:if test="(protx:protein/@protein_name=\'' . $protein . '\' or (protx:protein/@subsuming_protein_entry and protx:protein/@subsuming_protein_entry=\'' . $protein . '\'))' . $suffix . '">';

	print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/XPressRatio and protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'0\')' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/XPressRatio and protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\')' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/XPressRatio and protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\')' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protein[@group_sibling_id = \'a\']/XPressRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or (protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev &gt;= \'0\' and protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));

	}
	else { # adjusted asapratios
	    print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' . $suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(protein[@group_sibling_id = \'a\']/XPressRatio) or not(protein[@group_sibling_id = \'a\']/ASAPRatio) or (protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev &gt;= \'0\' and protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean + protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_standard_dev - protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean + protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $suffix . '">' if(! ($asap_xpress eq ''));
	}
	print XSL '<xsl:if test="(protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' . $suffix . '">' if($max_pvalue_display < 1.0);

	print XSL '<xsl:if test="(@probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
    }
    else { # hide groups
	print XSL '<xsl:if test="(@protein_name=\'' . $protein . '\' or (@subsuming_protein_entry and @subsuming_protein_entry=\'' . $protein . '\'))' . $suffix . '">';

	print XSL '<xsl:if test="(count(protein) &gt;\'1\' or @probability &gt;= \'' . $minprob . '\')' . $suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/XPressRatio and protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'0\'))' . $suffix . '">' if(! ($filter_asap eq ''));
	print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/XPressRatio and protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'))' . $suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/XPressRatio and protein[@group_sibling_id = \'a\']/XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'))' . $suffix . '">' if($max_xpress > 0);
	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	    print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	else {
	    print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &gt;= \'' . $min_asap . '\'))' . $suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/ASAPRatio and protein[@group_sibling_id = \'a\']/ASAPRatio/@adj_ratio_mean &lt;= \'' . $max_asap . '\'))' . $suffix . '">' if($max_asap > 0);

	}
	print XSL '<xsl:if test="(count(protein) &gt;\'1\' or (protein[@group_sibling_id = \'a\']/ASAPRatio) and protein[@group_sibling_id = \'a\']/ASAPRatio/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'))' . $suffix . '">' if($max_pvalue_display < 1.0);

    }

} # normal mode



print XSL '<xsl:variable name="group_member"><xsl:value-of select="count(protx:protein)"/></xsl:variable>';
print XSL '<xsl:variable name="group_number"><xsl:value-of select="@group_number"/></xsl:variable>' if(! ($show_groups eq ''));
print XSL '<xsl:variable name="parental_group_number"><xsl:value-of select="parent::node()/@group_number"/></xsl:variable>';
print XSL '<xsl:variable name="sole_prot"><xsl:value-of select="protein/@protein_name"/></xsl:variable>';
print XSL '<xsl:variable name="database"><xsl:value-of select="parent::node()/protx:protein_summary_header/@reference_database"/></xsl:variable>';
print XSL '<xsl:variable name="peps1"><xsl:value-of select="protx:protein/@unique_stripped_peptides"/></xsl:variable>';


if($tab_delim) {
    if(! ($show_groups eq '') && ! ($show_peps eq '')) {
	print XSL $newline;
    }
}
else {

    if(! ($show_groups eq '')) {
	print XSL '<tr><td><nobr>';
	print XSL $display{'group_number'} . '</nobr></td>';
	print XSL '<td colspan="' . ($num_cols-1) . '">';

	print XSL '<xsl:if test="$group_member &gt;\'1\'">PROTEIN GROUP: <xsl:value-of select="@pseudo_name"/></xsl:if>';
	print XSL '<xsl:if test="$group_member=\'1\'">' . $prot_header . '{$sole_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$sole_prot"/>' . $prot_suf . '<xsl:for-each select="protx:protein/protx:indistinguishable_protein"><xsl:variable name="indist_prot"><xsl:value-of select="@protein_name"/></xsl:variable>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each></xsl:if>' . $table_space . $table_space;
	print XSL '<font color="red"><b><xsl:value-of select="@probability"/></b></font>';

	print XSL '</td></tr>';
    }
    else { # case hiding groups
	print XSL '<xsl:if test="$group_member = \'1\'"><tr><td><nobr>' . $display{'group_number'} . '</nobr></td><td colspan="' . ($num_cols-1) . '">';
	print XSL '<xsl:if test="$group_member=\'1\'">' . $prot_header . '{$sole_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$sole_prot"/>' . $prot_suf . '<xsl:for-each select="protx:protein/protx:indistinguishable_protein"><xsl:variable name="indist_prot"><xsl:value-of select="@protein_name"/></xsl:variable>' . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database}&amp;Pep={$peps1}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each></xsl:if>' . $table_space . $table_space;
;
	print XSL '<font color="red"><b><xsl:value-of select="@probability"/></b></font>';
	print XSL '</td></tr></xsl:if>';
    }
} # if not tab


print XSL '<xsl:apply-templates select="protx:protein">';
print XSL '</xsl:apply-templates>';


print XSL '<tr><td><pre>' . $table_spacer . '</pre></td></tr>' if(! $tab_delim);


if($discards) {
    if(! ($show_groups eq '')) {
	print XSL '</xsl:if>';
	for(my $i = 0; $i < @inclusions; $i++) {
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
    print XSL '</xsl:if>';
    print XSL '</xsl:if>' if(! ($asap_xpress eq ''));  # agree
    print XSL '</xsl:if>' if($minprob > 0);
    print XSL '</xsl:if>' if(! ($filter_xpress eq ''));
    print XSL '</xsl:if>' if(! ($filter_asap eq ''));
    print XSL '</xsl:if>' if($min_xpress > 0);
    print XSL '</xsl:if>' if($max_xpress > 0);
    print XSL '</xsl:if>' if($min_asap > 0);
    print XSL '</xsl:if>' if($max_asap > 0);
    print XSL '</xsl:if>' if($max_pvalue_display < 1.0);
    for(my $e = 0; $e < @exclusions; $e++) {
	print XSL '</xsl:if>', "\n";
    }
}

print XSL '</xsl:template>', "\n";

} # only if show groups

############ PROTEIN ########################
print XSL '<xsl:template match="protx:protein">';
my $num_pincl = 0;


print XSL '<xsl:variable name="group_number"><xsl:value-of select="parent::node()/@group_number"/></xsl:variable>' if($show_groups eq '');
print XSL '<xsl:variable name="group_number"><xsl:value-of select="@group_number"/></xsl:variable>' if(! ($show_groups eq ''));



# integrate inclusions....
if($discards) {
    
    print XSL '<xsl:if test="@probability &lt; \'' . $minprob . '\'';

    print XSL ' or not(XPressRatio) or not(XPressRatio/@ratio_mean &gt;=\'' . $min_xpress . '\')' if($min_xpress > 0);
    print XSL ' or not(XPressRatio) or not(XPressRatio/@ratio_mean &lt;=\'' . $max_xpress . '\')' if($max_xpress > 0);
    print XSL ' or not(XPressRatio) or not(XPressRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_xpress eq ''));
    print XSL ' or not(ASAPRatio) or not(ASAPRatio/@ratio_mean &gt;= \'0\')' if(! ($filter_asap eq ''));
    if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {
	print XSL ' or not(ASAPRatio) or not(ASAPRatio/@ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(ASAPRatio) or not(ASAPRatio/@ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (ASAPRatio and XPressRatio and ((XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev - ASAPRatio/@ratio_mean + ASAPRatio/@ratio_standard_dev &lt; \'0\') or (ASAPRatio/@ratio_mean + ASAPRatio/@ratio_standard_dev - XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));

    }
    else {
	print XSL ' or not(ASAPRatio) or not(ASAPRatio/@adj_ratio_mean &gt;= \'' . $min_asap . '\')' if($min_asap > 0);
	print XSL ' or not(ASAPRatio) or not(ASAPRatio/@adj_ratio_mean &lt;= \'' . $max_asap . '\')' if($max_asap > 0);
	print XSL ' or (ASAPRatio and XPressRatio and ((XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev - ASAPRatio/@adj_ratio_mean + ASAPRatio/@ratio_standard_dev &lt; \'0\') or (ASAPRatio/@adj_ratio_mean + ASAPRatio/@ratio_standard_dev - XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev &lt; \'0\')))' if(! ($asap_xpress eq ''));
    }
    print XSL ' or not(ASAPRatio) or not(ASAPRatio/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\')' if($max_pvalue_display < 1.0);
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
    for(my $i = 0; $i < @inclusions; $i++) {
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
	    print XSL '<xsl:if test="not(count(parent::node()/protein)=\'1\' and parent::node()/@group_number=\'' . $_ . '\')' . $prot_suffix . '">';
	}
    }


    for(my $e = 0; $e < @pexclusions; $e++) {
	if($pexclusions[$e] =~ /^(\d+)([a-z,A-Z])$/) {
	    print XSL '<xsl:if test="not(parent::node()/@group_number=\'' . $1 . '\' and @group_sibling_id=\'' . $2 . '\')' . $prot_suffix . '">', "\n";
	}
    }
    if($show_groups eq '') {
	print XSL '<xsl:if test="@probability &gt;= \'' . $minprob . '\'' . $prot_suffix . '">' if($minprob > 0);
	print XSL '<xsl:if test="XPressRatio and XPressRatio/@ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_xpress eq ''));
	print XSL '<xsl:if test="ASAPRatio and ASAPRatio/@ratio_mean &gt;= \'0\'' . $prot_suffix . '">' if(! ($filter_asap eq ''));

	print XSL '<xsl:if test="XPressRatio and XPressRatio/@ratio_mean &gt;= \'' . $min_xpress . '\'' . $prot_suffix . '">' if($min_xpress > 0);
	print XSL '<xsl:if test="XPressRatio and XPressRatio/@ratio_mean &lt;= \'' . $max_xpress . '\'' . $prot_suffix . '">' if($max_xpress > 0);

	if($show_adjusted_asap eq '' || ! exists ${$boxptr}{'adj_asap'}) {

	    print XSL '<xsl:if test="ASAPRatio and ASAPRatio/@ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="ASAPRatio and ASAPRatio/@ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);


	    print XSL '<xsl:if test="(not(XPressRatio) or not(ASAPRatio) or (XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev - ASAPRatio/@ratio_mean + ASAPRatio/@ratio_standard_dev &gt;= \'0\' and ASAPRatio/@ratio_mean + ASAPRatio/@ratio_standard_dev - XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));


	}
	else {
	    print XSL '<xsl:if test="ASAPRatio and ASAPRatio/@adj_ratio_mean &gt;= \'' . $min_asap . '\'' . $prot_suffix . '">' if($min_asap > 0);
	    print XSL '<xsl:if test="ASAPRatio and ASAPRatio/@adj_ratio_mean &lt;= \'' . $max_asap . '\'' . $prot_suffix . '">' if($max_asap > 0);
	    print XSL '<xsl:if test="(not(XPressRatio) or not(ASAPRatio) or (XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev - ASAPRatio/@adj_ratio_mean + ASAPRatio/@ratio_standard_dev &gt;= \'0\' and ASAPRatio/@adj_ratio_mean + ASAPRatio/@ratio_standard_dev - XPressRatio/@ratio_mean + XPressRatio/@ratio_standard_dev &gt;= \'0\'))' . $prot_suffix . '">' if(! ($asap_xpress eq ''));

	}
    print XSL '<xsl:if test="ASAPRatio and ASAPRatio/@decimal_pvalue &lt;= \'' . $max_pvalue_display . '\'' . $prot_suffix . '">' if($max_pvalue_display < 1.0);

    }

    foreach(keys %parent_incls) {
	my @members = @{$parent_incls{$_}};
	if(@members > 0) {
	    $num_pincl++;
	    print XSL '<xsl:if test="not(parent::node()/@group_number=\'' . $_ . '\')';
	    for(my $m = 0; $m < @members; $m++) {
		print XSL ' or @group_sibling_id=\'' . $members[$m] . '\'';
	    }
	    print XSL '">';
	}

    }
#####################
    print XSL '<xsl:if test="count(protx:peptide)=\'1\'">' if($SINGLE_HITS);
    print XSL '<xsl:if test="@protein_name=\'' . $protein . '\' or (@subsuming_protein_entry and @subsuming_protein_entry=\'' . $protein . '\')">' if(! ($show_groups eq ''));

} # convent


# check whether part of group
print XSL '<xsl:variable name="mult_prot"><xsl:value-of select="@protein_name"/></xsl:variable>';
print XSL '<xsl:variable name="database2"><xsl:value-of select="parent::node()/parent::node()/protx:protein_summary_header/@reference_database"/></xsl:variable>';
print XSL '<xsl:variable name="peps2"><xsl:value-of select="@unique_stripped_peptides"/></xsl:variable>';
print XSL '<xsl:variable name="file"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_files"/></xsl:variable>';
print XSL '<xsl:variable name="filextn"><xsl:if test="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn">_<xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_file_xtn"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="asap_ind"><xsl:value-of select="protx:analysis_result[@analysis=\'asapratio\']/protx:ASAPRatio/@index"/></xsl:variable>';
print XSL '<xsl:variable name="prot_number"><xsl:value-of select="parent::node()/@group_number"/><xsl:if test="count(parent::node()/protx:protein) &gt;\'1\'"><xsl:value-of select="@group_sibling_id"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="pvalpngfile"><xsl:value-of select="/protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio_pvalue\']/protx:ASAP_pvalue_analysis_summary/@analysis_distribution_file"/></xsl:variable>';

# more variables here
print XSL '<xsl:variable name="peptide_string"><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@peptide_string"/></xsl:variable>';
if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="xratio"><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_mean"/></xsl:variable>';
    print XSL '<xsl:variable name="xstd"><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_standard_dev"/></xsl:variable>';
}
else { # reverse
    print XSL '<xsl:variable name="xratio"><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_mean"/></xsl:variable>';
    print XSL '<xsl:variable name="xstd"><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@heavy2light_ratio_standard_dev"/></xsl:variable>';
}
print XSL '<xsl:variable name="xnum"><xsl:value-of select="protx:analysis_result[@analysis=\'xpress\']/protx:XPressRatio/@ratio_number_peptides"/></xsl:variable>';
print XSL '<xsl:variable name="min_pep_prob"><xsl:value-of select="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@min_peptide_probability"/></xsl:variable>';
print XSL '<xsl:variable name="source"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_files"/></xsl:variable>';
print XSL '<xsl:variable name="heavy2light"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope=\'heavy\'">0</xsl:if><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\']/protx:XPress_analysis_summary/@reference_isotope=\'light\'">1</xsl:if></xsl:variable>';
if($tab_delim) {
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
    print XSL '</nobr></td><td colspan="' . ($num_cols-1) . '">' . $prot_header . '{$mult_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$mult_prot"/>' . $prot_suf . '<xsl:for-each select="indistinguishable_protein"><xsl:variable name="indist_prot"><xsl:value-of select="@protein_name"/></xsl:variable>' . $table_space . $table_space . ' ' . $prot_header . '{$indist_prot}&amp;Db={$database2}&amp;Pep={$peps2}"><xsl:value-of select="$indist_prot"/>' . $prot_suf . '</xsl:for-each>' . $table_space . $table_space . ' '; # . ' <font color="';
    if($show_groups eq '') {
	print XSL ' <font color="red"><b><xsl:value-of select="@probability"/></b></font>';
    }
    else {
	print XSL ' <font color="#B40000"><xsl:value-of select="@probability"/></font>';
    }
    print XSL '</td></tr>';
    print XSL '</xsl:if>' if(! ($show_groups eq ''));

    print XSL '<tr><td colspan="12">';
    print XSL '<table ' . $RESULT_TABLE . '<tr>';

    print XSL '<td width="150"><xsl:if test="@percent_coverage &gt;\'0\'"><xsl:if test="@n_indistinguishable_proteins &gt; \'1\'">max<xsl:text> </xsl:text></xsl:if>coverage: <xsl:value-of select="@percent_coverage"/>%</xsl:if></td>';

    print XSL $display{'xpress'};
    print XSL $display{'asapratio'};

    if(! ($show_num_unique_peps eq '')) {
	print XSL '<td width="225">num unique peps: <xsl:value-of select="count(protx:peptide[@is_contributing_evidence=\'Y\'])"/></td>';
    }
    if(! ($show_tot_num_peps eq '')) {
	print XSL '<td width="225">tot num peps: <xsl:value-of select="@total_number_peptides"/></td>';
    }
    if(! ($show_pct_spectrum_ids eq '')) {
	print XSL '<td width="225"><xsl:if test="@pct_spectrum_ids">share of spectrum id\'s: <xsl:value-of select="@pct_spectrum_ids"/>%</xsl:if></td>';
    }


    print XSL '<xsl:variable name="myprotein3"><xsl:value-of select="@protein_name"/></xsl:variable>';
    print XSL '<xsl:variable name="mychildren"><xsl:value-of select="count(/protx:protein_summary/protx:protein_group/protx:protein[@subsuming_protein_entry and @subsuming_protein_entry=$myprotein3])"/></xsl:variable>';
    print XSL '<td><xsl:if test="not(@protein_name=\''. $protein . '\') and $mychildren &gt; \'0\'"><A Target="Win1" HREF="' . $CGI_HOME . 'findsubsets.pl?Protein={$myprotein3}&amp;Xmlfile=' . $xmlfile . '&amp;Xslt=' . $xslt . '">subsumed entries: <xsl:value-of select="$mychildren"/></A></xsl:if></td>';

    print XSL '</tr></table></td>';

    print XSL '</tr>';

    if(! ($show_annot eq '')) {
	print XSL '<xsl:if test="protx:annotation">';
	print XSL '<tr><td>' . $table_spacer . '</td><td colspan="' . ($num_cols-1) . '">';
	print XSL $annotation;
	print XSL '</td></tr></xsl:if>';

	print XSL '<xsl:for-each select="protx:indistinguishable_protein">';
	print XSL '<xsl:if test="protx:annotation">';
	print XSL '<tr><td>' . $table_spacer . '</td><td colspan="' . ($num_cols-1) . '">';
	print XSL $annotation;
	print XSL '</td></tr></xsl:if>';
	print XSL '</xsl:for-each>'; # if 
    }

    print XSL '<tr>' . $HEADER . '</tr>' if(! ($show_peps eq ''));

} # not tab display

print XSL '<xsl:apply-templates select="protx:peptide">';
print XSL '<xsl:sort select = "@nsp_adjusted_probability" order="descending" data-type="number"/>';
print XSL '</xsl:apply-templates>';
print XSL '<tr><td>' . $table_spacer . '</td></tr>' if($show_groups eq '' && ! $tab_delim);

###########
print XSL '</xsl:if>' if($SINGLE_HITS);
print XSL '</xsl:if>' if(! ($show_groups eq ''));

if($discards) {
    
    print XSL '</xsl:if>';

    # now add on inclusions which must be avoided
    for(my $i = 0; $i < @inclusions; $i++) {
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
    for(my $e = 0; $e < @pexclusions; $e++) {
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


print XSL '<xsl:variable name="mypep"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
print XSL '<xsl:variable name="mycharge"><xsl:value-of select="@charge"/></xsl:variable>';
print XSL '<xsl:variable name="myinputfiles"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/></xsl:variable>';
print XSL '<xsl:variable name="myprots"><xsl:value-of select="parent::node()/@protein_name"/><xsl:for-each select="parent::node()/protx:indistinguishable_protein"><xsl:text> </xsl:text><xsl:value-of select="@protein_name"/></xsl:for-each></xsl:variable>';

print XSL '<xsl:variable name="nspbin"><xsl:value-of select="@n_sibling_peptides_bin"/></xsl:variable>';
print XSL '<xsl:variable name="nspval"><xsl:value-of select="@n_sibling_peptides"/></xsl:variable>';

if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'] or /protx:protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">0</xsl:if><xsl:if test="not(/protein_summary/protx:analysis_summary[@analysis=\'xpress\']) and not(/protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'] or /protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">1</xsl:if><xsl:if test="not(/protein_summary/protx:analysis_summary[@analysis=\'xpress\']) and not(/protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">-1</xsl:if></xsl:variable>';
}


print XSL '<xsl:if test="@nsp_adjusted_probability &gt;=\''. $min_pepprob . '\'">' if($min_pepprob > 0);
print XSL '<xsl:if test="@n_tryptic_termini &gt;=\''. $minntt . '\'">' if($minntt > 0);
print XSL '<xsl:if test="not(@charge=\'1\')">' if(! ($exclude_1 eq ''));
print XSL '<xsl:if test="not(@charge=\'2\')">' if(! ($exclude_2 eq ''));
print XSL '<xsl:if test="not(@charge=\'3\')">' if(! ($exclude_3 eq ''));
print XSL '<xsl:if test="@is_nondegenerate_evidence=\'Y\'">' if(! ($exclude_degens eq ''));

print XSL '<xsl:variable name="amp"><xsl:text><![CDATA[&]]></xsl:text></xsl:variable>';
if($tab_delim) {

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
    print XSL $tab_display{'xpress'};
    print XSL $tab_display{'asapratio'}; # tab built in

    if(! ($show_num_unique_peps eq '')) {
	print XSL '<xsl:value-of select="count(parent::node()/protx:peptide[@is_contributing_evidence=\'Y\'])"/>' . $tab;
    }
    if(! ($show_tot_num_peps eq '')) {
	print XSL '<xsl:value-of select="parent::node()/@total_number_peptides"/>' . $tab;
    }


    # now the annotation
    if(! ($show_annot eq '')) {
	print XSL $annot_tab_display{'ipi'};
	print XSL $annot_tab_display{'description'} . $tab;

	print XSL '<xsl:if test="/protx:protein_summary/protx:protein_summary_header/@organism">';


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
	if($tab_delim) {
	    print XSL $tab_display{$_} . $tab if(! ($show_peps eq ''));
	}
	else {
	    print XSL $display{$_} if(! ($show_peps eq ''));
	}
    }
}
else { # use default
    foreach(sort {$default_order{$a} <=> $default_order{$b}} keys %default_order) {
	if($tab_delim) {
	    print XSL $tab_display{$_} . $tab  if(! ($show_peps eq '') && $default_order{$_} >= 0);
	}
	else {
	    print XSL $display{$_} if($default_order{$_} >= 0 && ! ($show_peps eq ''));
	}
    }

}

print XSL '</tr>' if(! $tab_delim && ! ($show_peps eq ''));
print XSL $newline if($tab_delim);
print XSL '<xsl:apply-templates select="protx:indistinguishable_peptide"/>' if(! $tab_delim && ! ($show_peps eq '')); # make extra entry(s)


print XSL '</xsl:if>' if($min_pepprob > 0);
print XSL '</xsl:if>' if($minntt > 0);
print XSL '</xsl:if>' if(! ($exclude_1 eq ''));
print XSL '</xsl:if>' if(! ($exclude_2 eq ''));
print XSL '</xsl:if>' if(! ($exclude_3 eq ''));
print XSL '</xsl:if>' if(! ($exclude_degens eq ''));
if($tab_delim && $show_peps eq '') {
    print XSL '</xsl:if>';
}

print XSL '</xsl:template>';

# indistinguishable_peptide
if(! $tab_delim && ! ($show_peps eq '')) {
    print XSL '<xsl:template match="protx:indistinguishable_peptide">';
    print XSL '<xsl:variable name="mycharge2"><xsl:value-of select="parent::node()/@charge"/></xsl:variable>';
    print XSL '<xsl:variable name="mypep2"><xsl:if test="@pound_subst_peptide_sequence"><xsl:value-of select="@pound_subst_peptide_sequence"/></xsl:if><xsl:if test="not(@pound_subst_peptide_sequence)"><xsl:value-of select="@peptide_sequence"/></xsl:if></xsl:variable>';
    print XSL '<xsl:variable name="myinputfiles2"><xsl:value-of select="/protx:protein_summary/protx:protein_summary_header/@source_files_alt"/></xsl:variable>';
if($quant_light2heavy eq 'true') {
    print XSL '<xsl:variable name="ratiotype2"><xsl:if test="/protx:protein_summary/protx:analysis_summary[@analysis=\'xpress\'] or /protein_summary/protx:analysis_summary[@analysis=\'asapratio\']">0</xsl:if><xsl:if test="not(/protein_summary/protx:analysis_summary[@analysis=\'xpress\']) and not(/protein_summary/protx:analysis_summary[@analysis=\'asapratio\'])">-1</xsl:if></xsl:variable>';
}
else {
    print XSL '<xsl:variable name="ratiotype2"><xsl:if test="/protein_summary/protx:analysis_summary[@analysis=\'xpress\'] or /protein_summary/protx:analysis_summary[@analysis=\'xpress\']">1</xsl:if><xsl:if test="not(/protein_summary/protx:analysis_summary[@analysis=\'xpress\']) and not(/protein_summary/protx:analysis_summary[@analysis=\'xpress\'])">-1</xsl:if></xsl:variable>';
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
		print XSL $display_ind_peptide_seq if($default_order{$_} >= 0);
	    }
	    else {
		#print XSL '<td>' . $table_space . '</td>' if($default_order{$_} >= 0);
		print XSL '<td></td>' if($default_order{$_} >= 0);
	    }
	}
    }

    print XSL '</tr>';
    print XSL '</xsl:template>';
} # if not tab delim


if((! ($show_sens eq '') && $eligible)) {
    print XSL '<xsl:template match="protx:protein_summary_data_filter">';
    print XSL '<xsl:value-of select="@min_probability"/>' . $tab . '<font color="red"><xsl:value-of select="@sensitivity"/></font>' . $tab . '<font color="green"><xsl:value-of select="@false_positive_error_rate"/></font>' . $tab . '<font color="red"><xsl:value-of select="@predicted_num_correct"/></font>' . $tab . '<font color="green"><xsl:value-of select="@predicted_num_incorrect"/></font>' . $newline;

    print XSL '</xsl:template>';
}

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


