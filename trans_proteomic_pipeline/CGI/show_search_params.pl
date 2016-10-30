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



print "Content-type: text/html\n\n";
%box = &tpplib_perl::read_query_string;      # Read keys and values

print "<HTML><BODY BGCOLOR=\"\#FFFFFF\" OnLoad=\"self.focus();\" ><PRE>";
print "<TITLE>Search Parameters (" . $TPPVersionInfo . ")</TITLE>";

my $basename = exists $box{'basename'} ? $box{'basename'} : '';
my $xmlfile = $box{'xmlfile'};
if ($^O eq 'MSWin32' ) {
	$xmlfile =~ s/\\/\//g;  # get those path seps pointing right!
}

my $engine = exists $box{'engine'} ? $box{'engine'} : '';

my $temp_xslfile = $xmlfile . '.tmpxsl';

my @variable_mods = ('*', '#', '@');  # TO BE REPLACED BY BELOW SYMBOLS....
my @comet_variable_mods = ('1', '2', '3');


if($engine eq 'SEQUEST') {
    writeSequestXSLFile($temp_xslfile, $xmlfile);
}
elsif($engine eq 'COMET') {
    writeCometXSLFile($temp_xslfile, $xmlfile);
}
else {
    writeXSLFile($temp_xslfile, $xmlfile);
}

print '# ' if($engine eq 'SEQUEST' || $engine eq 'COMET');
print "<b>$xmlfile</b><p/>\n";
print '# ' if($engine eq 'SEQUEST' || $engine eq 'COMET');
print "<b><font color=\"red\">$basename $engine parameters</font></b><p/>\n";


my $tmpxmlfile = tpplib_perl::uncompress_to_tmpfile($xmlfile); # decompress .gz if needed

if($xslt =~ /xsltproc/) {
    open XSLT, "$xslt --novalid $temp_xslfile $tmpxmlfile |";
}
else {
    open XSLT, "$xslt $tmpxmlfile $temp_xslfile |";
}
my @results = <XSLT>;
my $counter = 1;
for(my $k = 1; $k < @results; $k++) {
    if(! ($engine eq 'SEQUEST')) {
	while($results[$k] =~ /COUNTER/) {
	    $results[$k] =~ s/COUNTER/$counter/;
	    $counter++;
	}
    }

    print $results[$k];

}
close(XSLT);
unlink($temp_xslfile) if (-e $temp_xslfile);
unlink($tmpxmlfile) if ($tmpxmlfile ne $xmlfile); # did we decompress xml.gz?



print "</PRE></HTML>";

sub writeSequestXSLFile {
(my $temp_xslfile, my $xmlfile) = @_;
open(XSL, ">$temp_xslfile") or die "cannot open $temp_xslfile $!\n";

print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">', "\n";

print XSL '<xsl:template match="pepx:msms_pipeline_analysis">';
print XSL '<xsl:apply-templates select="pepx:msms_run_summary[pepx:search_summary[@search_engine=\'' . $engine . '\' and @base_name=\'' . $basename . '\']]"/>';
print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:msms_run_summary">';

print XSL '# comment lines begin with a \'#\' in the first position';
print XSL '<p/>';

print XSL '[SEQUEST]';
print XSL '<p/>';
print XSL 'database_name = <xsl:value-of select="pepx:search_database/@local_path"/>';
print XSL '<p/>';

print XSL 'peptide_mass_tolerance = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'peptide_mass_tol\']/@value"/>';
print XSL '<p/>';
print XSL 'create_output_files = 1                ; 0=no, 1=yes';
print XSL '<p/>';

print XSL 'ion_series = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'ion_series\']/@value"/>';
print XSL '<p/>';
print XSL 'fragment_ion_tolerance = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'fragment_ion_tol\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'fragment_ion_tol\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'fragment_ion_tol\'])">0.0</xsl:if>         ; leave at 0.0 unless you have real poor data';
print XSL '<p/>';


print XSL 'num_output_lines = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'num_output_lines\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'num_output_lines\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'num_output_lines\'])">10</xsl:if>                  ; # peptide results to show';
print XSL '<p/>';
print XSL 'num_description_lines = 3              ; # full protein descriptions to show for top N peptides';
print XSL '<p/>';
print XSL 'show_fragment_ions = 0                 ; 0=no, 1=yes';
print XSL '<p/>';
print XSL 'print_duplicate_references = 0         ; 0=no, 1=yes';
print XSL '<p/>';
print XSL 'enzyme_number = ';
print XSL '<xsl:choose><xsl:when test="not(pepx:search_summary/pepx:enzymatic_search_constraint)">0</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'tryptic\'">1</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'chymotryptic\'">2</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'clostripain\'">3</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'CNBr\'">4</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'iodosobenzoate\'">5</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'proline_endopept\'">6</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'staph_protease\'">7</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'tryptic_k\'">8</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'tryptic_r\'">9</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'AspN\'">10</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'chymotrypic/modified\'">11</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'elastase\'">12</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'elastase/trypic/chymotryptic\'">13</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'nonspecific\'">0</xsl:when>';
print XSL '</xsl:choose><p/>';
print XSL 'NumEnzymeTermini = <xsl:if test="pepx:search_summary/pepx:enzymatic_search_constraint"><xsl:value-of select="pepx:search_summary/pepx:enzymatic_search_constraint/@min_number_termini"/></xsl:if><p/>';
print XSL '<br/>';
print XSL '#';
print XSL '<p/>';
print XSL '# Up to 3 differential searches can be performed.';
print XSL '<p/>';
print XSL '# Amino acids cannot appear in more than one differential search parameter';
print XSL '<p/>';
print XSL '#';
print XSL '<p/>';
print XSL 'diff_search_options = ';

for(my $index = 0; $index < @variable_mods; $index++) {
    print XSL '<xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $variable_mods[$index] . '\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $variable_mods[$index] . '\']/@massdiff"/><xsl:text> </xsl:text><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $variable_mods[$index] . '\']/@aminoacid"/><xsl:text> </xsl:text></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $variable_mods[$index] . '\'])">0.0 X </xsl:if>';
}




print XSL '<p/>';
print XSL '<br/>';


print XSL 'max_num_differential_AA_per_mod = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'max_num_differential_AA_per_mod\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'max_num_differential_AA_per_mod\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'max_num_differential_AA_per_mod\'])">4</xsl:if>   ; max # of modified AA per diff. mod in a peptide';
print XSL '<p/>';

print XSL 'nucleotide_reading_frame = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'nucleotide_reading_frame\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'nucleotide_reading_frame\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'nucleotide_reading_frame\'])">0</xsl:if>          ; 0=proteinDB, 1-6, 7=forward three, 8=reverse three, 9=all six';
print XSL '<p/>';



print XSL 'mass_type_parent = <xsl:if test="pepx:search_summary/@precursor_mass_type=\'average\'">0</xsl:if><xsl:if test="pepx:search_summary/@precursor_mass_type=\'monoisotopic\'">1</xsl:if>                  ; 0=average masses, 1=monoisotopic masses';
print XSL '<p/>';
print XSL 'mass_type_fragment = <xsl:if test="pepx:search_summary/@fragment_mass_type=\'average\'">0</xsl:if><xsl:if test="pepx:search_summary/@fragment_mass_type=\'monoisotopic\'">1</xsl:if>                ; 0=average masses, 1=monoisotopic masses';
print XSL '<p/>';

print XSL 'remove_precursor_peak = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'remove_precursor_peak\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'remove_precursor_peak\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'remove_precursor_peak\'])">0</xsl:if>                           ; 0=no, 1=yes';
print XSL '<p/>';


print XSL 'ion_cutoff_percentage = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'ion_cutoff_percentage\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'ion_cutoff_percentage\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'ion_cutoff_percentage\'])">0.0</xsl:if>           ; prelim. score cutoff % as a decimal number i.e. 0.30 for 30%';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'match_peak_count = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'match_peak_count\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'match_peak_count\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'match_peak_count\'])">0</xsl:if>                  ; number of auto-detected peaks to try matching (max 5)';
print XSL '<p/>';



print XSL 'match_peak_allowed_error = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'match_peak_allowed_error\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'match_peak_allowed_error\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'match_peak_allowed_error\'])">1</xsl:if>          ; number of allowed errors in matching auto-detected peaks';
print XSL '<p/>';


print XSL 'match_peak_tolerance = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'match_peak_tolerance\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'match_peak_tolerance\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'match_peak_tolerance\'])">1.0</xsl:if>            ; mass tolerance for matching auto-detected peaks';
print XSL '<p/>';



print XSL 'max_num_internal_cleavage_sites = <xsl:if test="pepx:enzymatic_search_constraint"><xsl:value-of select="pepx:enzymatic_search_constraint/@max_num_internal_cleavages"/></xsl:if><xsl:if test="not(pepx:enzymatic_search_constraint)">1</xsl:if>   ; maximum value is 5; for enzyme search';
print XSL '<p/>';
print XSL '<br/>';

print XSL '# partial sequence info ... overrides entries in .dta files';
print XSL '<p/>';
print XSL '#   up to 10 partial sequences ... each must appear in peptides';
print XSL '<p/>';
print XSL '#      analyzed in the forward or reverse directions';
print XSL '<p/>';
print XSL 'partial_sequence = <xsl:for-each select="pepx:search_summary/pepx:sequence_search_constraint"><xsl:value-of select="@sequence"/><xsl:text> </xsl:text></xsl:for-each>';
print XSL '<p/>';
print XSL '<br/>';

print XSL '# protein mass &amp; mass tolerance value i.e. 80000 10%';
print XSL '<p/>';
print XSL '# or protein min &amp; max value i.e. 72000 88000  (0 for both = unused)';
print XSL '<p/>';
print XSL 'protein_mass_filter = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'protein_mass_filter\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'protein_mass_filter\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'protein_mass_filter\'])">0<xsl:text> </xsl:text>0</xsl:if>';
print XSL '<p/>';
print XSL '<br/>';


print XSL '# For sequence_header_filter, enter up to five (5) strings where any one must';
print XSL '<p/>';
print XSL '# be in the header of a sequence entry for that entry to be searched.';
print XSL '<p/>';
print XSL '# Strings are space separated and \'~\' substitutes for a space within a string.';
print XSL '<p/>';
print XSL '# Example:  sequence_header_filter = human homo~sapien trypsin';
print XSL '<p/>';
print XSL 'sequence_header_filter = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'sequence_header_filter\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'sequence_header_filter\']/@value"/></xsl:if>';
print XSL '<p/>';
print XSL '<br/>';

print XSL '# The "add_???" entries are static modifications which modify every occurence';
print XSL '<p/>';
print XSL '# of that residue/terminus';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'add_C_terminus = <xsl:if test="pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'c\']"><xsl:value-of select="pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'c\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'c\'])">0.00</xsl:if>';
print XSL '<p/>';


print XSL 'add_N_terminus = <xsl:if test="pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'n\']"><xsl:value-of select="pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'n\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'n\'])">0.00</xsl:if>';
print XSL '<p/>';

print XSL 'add_G_Glycine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'G\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'G\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'G\'])">0.0000</xsl:if>       ; added to G - avg.  57.0519, mono.  57.02146';
print XSL '<p/>';
print XSL 'add_A_Alanine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[variable=\'N\' and aminoacid=\'A\']"><xsl:value-of select="search_summary/aminoacid_modification[variable=\'N\' and aminoacid=\'A\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\'])">0.0000</xsl:if>       ; added to A - avg.  71.0788, mono.  71.03711';
print XSL '<p/>';
print XSL 'add_S_Serine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\'])">0.0000</xsl:if>        ; added to S - avg.  87.0782, mono.  87.02303';
print XSL '<p/>';
print XSL 'add_P_Proline = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'P\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'P\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'P\'])">0.0000</xsl:if>       ; added to P - avg.  97.1167, mono.  97.05276';
print XSL '<p/>';
print XSL 'add_V_Valine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'V\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'V\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'V\'])">0.0000</xsl:if>        ; added to V - avg.  99.1326, mono.  99.06841';
print XSL '<p/>';


print XSL 'add_T_Threonine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'T\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'T\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'T\'])">0.0000</xsl:if>     ; added to T - avg. 101.1051, mono. 101.04768';
print XSL '<p/>';


print XSL 'add_C_Cysteine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'C\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'C\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'C\'])">0.0000</xsl:if>    ; added to C - avg. 103.1388, mono. 103.00919';
print XSL '<p/>';

print XSL 'add_L_Leucine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'L\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'L\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'L\'])">0.0000</xsl:if>       ; added to L - avg. 113.1594, mono. 113.08406';
print XSL '<p/>';

print XSL 'add_I_Isoleucine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'I\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'I\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'I\'])">0.0000</xsl:if>    ; added to I - avg. 113.1594, mono. 113.08406';
print XSL '<p/>';

print XSL 'add_X_LorI = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'X\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'X\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'X\'])">0.0000</xsl:if>          ; added to X - avg. 113.1594, mono. 113.08406';
print XSL '<p/>';

print XSL 'add_N_Asparagine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'N\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'N\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'N\'])">0.0000</xsl:if>    ; added to N - avg. 114.1038, mono. 114.04293';
print XSL '<p/>';


print XSL 'add_O_Ornithine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'O\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'O\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'O\'])">0.0000</xsl:if>     ; added to O - avg. 114.1472, mono  114.07931';
print XSL '<p/>';

print XSL 'add_B_avg_NandD = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'B\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'B\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'B\'])">0.0000</xsl:if>     ; added to B - avg. 114.5962, mono. 114.53494';
print XSL '<p/>';

print XSL 'add_D_Aspartic_Acid = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'D\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'D\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'D\'])">0.0000</xsl:if> ; added to D - avg. 115.0886, mono. 115.02694';
print XSL '<p/>';


print XSL 'add_Q_Glutamine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Q\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Q\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Q\'])">0.0000</xsl:if>     ; added to Q - avg. 128.1307, mono. 128.05858';
print XSL '<p/>';

print XSL 'add_K_Lysine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'K\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'K\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'K\'])">0.0000</xsl:if>        ; added to K - avg. 128.1741, mono. 128.09496';
print XSL '<p/>';

print XSL 'add_Z_avg_QandE = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Z\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Z\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Z\'])">0.0000</xsl:if>     ; added to Z - avg. 128.6231, mono. 128.55059';
print XSL '<p/>';

print XSL 'add_E_Glutamic_Acid = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'E\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'E\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'E\'])">0.0000</xsl:if> ; added to E - avg. 129.1155, mono. 129.04259';
print XSL '<p/>';


print XSL 'add_M_Methionine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'M\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'M\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'M\'])">0.0000</xsl:if>    ; added to M - avg. 131.1926, mono. 131.04049';
print XSL '<p/>';

print XSL 'add_H_Histidine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'H\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'H\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'H\'])">0.0000</xsl:if>     ; added to H - avg. 137.1411, mono. 137.05891';
print XSL '<p/>';

print XSL 'add_F_Phenyalanine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'F\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'F\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'F\'])">0.0000</xsl:if>  ; added to F - avg. 147.1766, mono. 147.06841';
print XSL '<p/>';

print XSL 'add_R_Arginine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'R\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'R\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'R\'])">0.0000</xsl:if>      ; added to R - avg. 156.1875, mono. 156.10111';
print XSL '<p/>';

print XSL 'add_Y_Tyrosine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Y\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Y\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Y\'])">0.0000</xsl:if>      ; added to Y - avg. 163.1760, mono. 163.06333';
print XSL '<p/>';

print XSL 'add_W_Tryptophan = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'W\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'W\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'W\'])">0.0000</xsl:if>    ; added to W - avg. 186.2132, mono. 186.07931';
print XSL '<p/>';

print XSL '<br/>';


print XSL '#';
print XSL '<p/>';
print XSL '# SEQUEST_ENZYME_INFO _must_ be at the end of this parameters file';
print XSL '<p/>';
print XSL '#';
print XSL '<p/>';
print XSL '[SEQUEST_ENZYME_INFO]';
print XSL '<p/>';
print XSL '0.  No_Enzyme              0      -           -';
print XSL '<p/>';
print XSL '1.  Trypsin                1      KR          P';
print XSL '<p/>';
print XSL '2.  Chymotrypsin           1      FWY         P';
print XSL '<p/>';
print XSL '3.  Clostripain            1      R           -';
print XSL '<p/>';
print XSL '4.  Cyanogen_Bromide       1      M           -';
print XSL '<p/>';
print XSL '5.  IodosoBenzoate         1      W           -';
print XSL '<p/>';
print XSL '6.  Proline_Endopept       1      P           -';
print XSL '<p/>';
print XSL '7.  Staph_Protease         1      E           -';
print XSL '<p/>';
print XSL '8.  Trypsin_K              1      K           P';
print XSL '<p/>';
print XSL '9.  Trypsin_R              1      R           P';
print XSL '<p/>';
print XSL '10. AspN                   0      D           -';
print XSL '<p/>';
print XSL '11. Cymotryp/Modified      1      FWYL        P';
print XSL '<p/>';
print XSL '12. Elastase               1      ALIV        P';
print XSL '<p/>';
print XSL '13. Elastase/Tryp/Chymo    1      ALIVKRWFY   P';
print XSL '<p/>';

print XSL '</xsl:template>';

print XSL '</xsl:stylesheet>';

close(XSL);




}

sub writeCometXSLFile {
(my $temp_xslfile, my $xmlfile) = @_;
open(XSL, ">$temp_xslfile") or die "cannot open $temp_xslfile $!\n";

print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">', "\n";

print XSL '<xsl:template match="pepx:msms_pipeline_analysis">';

print XSL '<xsl:apply-templates select="pepx:msms_run_summary[pepx:search_summary[@search_engine=\'' . $engine . '\' and @base_name=\'' . $basename . '\']]"/>';
print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:msms_run_summary">';

print XSL '# comment lines begin with a \'#\' (pound symbol)';
print XSL '<p/>';

print XSL 'Database = <xsl:value-of select="pepx:search_summary/pepx:search_database/@local_path"/>';
print XSL '<p/>';
print XSL 'EmailAddress = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'email_address\']/@value"/>';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'MassTol = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'peptide_mass_tol\']/@value"/>';
print XSL '<p/>';
print XSL 'UnitsMassTol = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'peptide_mass_tol_units\']/@value"/>';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'MassTypeParent = <xsl:if test="pepx:search_summary/@precursor_mass_type=\'average\'">0</xsl:if><xsl:if test="pepx:search_summary/@precursor_mass_type=\'monoisotopic\'">1</xsl:if>                  # 0=average masses, 1=monoisotopic masses';
print XSL '<p/>';
print XSL 'MassTypeFragment = <xsl:if test="pepx:search_summary/@fragment_mass_type=\'average\'">0</xsl:if><xsl:if test="pepx:search_summary/@fragment_mass_type=\'monoisotopic\'">1</xsl:if>                # 0=average masses, 1=monoisotopic masses';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'IonSeries = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'ion_series\']/@value"/>               # ABCDVWXYZ ions  0 (unused)  or 1 (used)';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'NumOutputLines = <xsl:if test="pepx:search_summary/pepx:parameter[@name=\'num_output_lines\']"><xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'num_output_lines\']/@value"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:parameter[@name=\'num_output_lines\'])">10</xsl:if>                 # num peptide results to show';
print XSL '<p/>';



print XSL 'EnzymeNum = ';
print XSL '<xsl:choose><xsl:when test="not(pepx:search_summary/pepx:enzymatic_search_constraint)">0</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'tryptic\'">1</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'chymotryptic\'">2</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'clostripain\'">3</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'CNBr\'">4</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'iodosobenzoate\'">5</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'proline_endopept\'">6</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'staph_protease\'">7</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'tryptic_k\'">8</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'tryptic_r\'">9</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'AspN\'">10</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'chymotrypic/modified\'">11</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'elastase\'">12</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'elastase/trypic/chymotryptic\'">13</xsl:when>';
print XSL '<xsl:when test="pepx:search_summary/pepx:enzymatic_search_constraint/@enzyme=\'nonspecific\'">0</xsl:when>';
print XSL '</xsl:choose><p/>';
print XSL '<p/>';

print XSL 'NumEnzymeTermini = <xsl:if test="pepx:enzymatic_search_constraint"><xsl:value-of select="pepx:enzymatic_search_constraint/@min_number_termini"/></xsl:if>                 # valid: 1 or 2 (default) i.e. require doubly tryptic or singly tryptic';
print XSL '<p/>';
print XSL 'AllowedMissedCleavages = <xsl:if test="pepx:enzymatic_search_constraint"><xsl:value-of select="pepx:enzymatic_search_constraint/@max_num_internal_cleavages"/></xsl:if><xsl:if test="not(pepx:enzymatic_search_constraint)">1</xsl:if>          # valid: 0 (default), 1, or 2';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'MaxNumVariableModResidues = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'max_num_var_mod_residues\']/@value"/>       # max # of modified AA per diff. mod in a peptide, max 5';
print XSL '<p/>';
print XSL 'RemovePrecursorPeak = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'remove_precursor_peak\']/@value"/>             # 0=no, 1=yes ... removes all signal +/- 1.0 Da of precursor';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'NumDuplicateHeaders = <xsl:value-of select="pepx:search_summary/pepx:parameter[@name=\'num_dup_headers\']/@value"/>             # number of duplicate protein headers to print out, default=0';
print XSL '<p/>';
print XSL '<br/>';

print XSL '# Enter any residues that you require a peptide to have.';
print XSL '<p/>';
print XSL '# If multiple residues are entered, accepted peptides can match any';
print XSL '<p/>';
print XSL '# ... enter multiple residues as one word (i.e. DLR) and not separated (i.e. D L R)';

print XSL '<p/>';
print XSL 'RequireResidues = <xsl:for-each select="pepx:search_summary/pepx:sequence_search_constraint"><xsl:value-of select="@sequence"/><xsl:text> </xsl:text></xsl:for-each>';
print XSL '<p/>';
print XSL '<br/>';



print XSL '<br/>';
print XSL '#';
print XSL '<p/>';
print XSL '# Up to 3 variable modifications can be specified.  Entered masses must be positive numbers.';
print XSL '<p/>';
print XSL '#';
print XSL '<p/>';

for(my $index = 0; $index < @comet_variable_mods; $index++) {
    print XSL 'VariableMod' . ($index+1) . ' = ';
    print XSL '<xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $comet_variable_mods[$index] . '\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $comet_variable_mods[$index] . '\']/@massdiff"/><xsl:text> </xsl:text><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $comet_variable_mods[$index] . '\']/@aminoacid"/><xsl:text> </xsl:text><xsl:choose><xsl:when test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $comet_variable_mods[$index] . '\']/@binary and pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $comet_variable_mods[$index] . '\']/@binary=\'Y\'">1</xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'Y\' and @symbol=\'' . $comet_variable_mods[$index] . '\'])">0.0 X 0</xsl:if>';

    if($index == 0) {
	print XSL '               # &lt;mass&gt; &lt;AAs&gt; &lt;0 or 1&gt;';
    }
    elsif($index == 1) {
	print XSL '               # &lt;0 or 1&gt; defines binary modification, 1=binary, 0=variable';
    }
    elsif($index == 2) {
	print XSL '               # 0.0 mass turns off modification';
   }
    print XSL '<p/>';
}
print XSL '<br/>';


print XSL '#';
print XSL '<p/>';
print XSL '# Variable modifications on N and C terminus of each peptide.';
print XSL '<p/>';
print XSL '# Entered masses must be positive numbers.';
print XSL '<p/>';
print XSL '#';
print XSL '<p/>';
print XSL 'VariableModCTerm = <xsl:if test="pepx:search_summary/pepx:terminal_modification[@variable=\'Y\' and @terminus=\'c\']"><xsl:value-of select="pepx:search_summary/pepx:terminal_modification[@variable=\'Y\' and @terminus=\'c\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:terminal_modification[@variable=\'Y\' and @terminus=\'c\'])">0.00</xsl:if>              # possibly added to C-terminus';;
print XSL '<p/>';
print XSL 'VariableModNTerm = <xsl:if test="pepx:search_summary/pepx:terminal_modification[@variable=\'Y\' and @terminus=\'n\']"><xsl:value-of select="pepx:search_summary/pepx:terminal_modification[@variable=\'Y\' and @terminus=\'n\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:terminal_modification[@variable=\'Y\' and @terminus=\'n\'])">0.00</xsl:if>              # possibly added to N-terminus';;
print XSL '<p/>';
print XSL 'VariableTermModType = 0              # 0=add to N/C-terminus of peptide (default), 1=add to N/C-terminus of protein';
print XSL '<p/>';
print XSL '<br/>';

print XSL '#';
print XSL '<p/>';
print XSL '# Static Modifications.  Entered masses can be negative but must result in positive masses.';
print XSL '<p/>';
print XSL '#';
print XSL '<p/>';



print XSL 'StaticAddCTerm = <xsl:if test="pepx:search_summary/terminal_modification[@variable=\'N\' and @terminus=\'c\']"><xsl:value-of select="pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'c\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'c\'])">0.00</xsl:if>';
print XSL '<p/>';
print XSL 'StaticAddNTerm = <xsl:if test="pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'n\']"><xsl:value-of select="pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'n\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:terminal_modification[@variable=\'N\' and @terminus=\'n\'])">0.00</xsl:if>';
print XSL '<p/>';
print XSL 'StaticTermModType = 0        # 0=add to N/C-terminus of each peptide (default), 1=add to N/C-terminus of protein';
print XSL '<p/>';
print XSL '<br/>';

print XSL 'add_G_Glycine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'G\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'G\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'G\'])">0.0000</xsl:if>       # added to G - avg.  57.0519, mono.  57.02146';
print XSL '<p/>';
print XSL 'add_A_Alanine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[variable=\'N\' and aminoacid=\'A\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[variable=\'N\' and aminoacid=\'A\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\'])">0.0000</xsl:if>       # added to A - avg.  71.0788, mono.  71.03711';
print XSL '<p/>';
print XSL 'add_S_Serine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'A\'])">0.0000</xsl:if>        # added to S - avg.  87.0782, mono.  87.02303';
print XSL '<p/>';
print XSL 'add_P_Proline = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'P\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'P\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'P\'])">0.0000</xsl:if>       # added to P - avg.  97.1167, mono.  97.05276';
print XSL '<p/>';
print XSL 'add_V_Valine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'V\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'V\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'V\'])">0.0000</xsl:if>        # added to V - avg.  99.1326, mono.  99.06841';
print XSL '<p/>';


print XSL 'add_T_Threonine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'T\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'T\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'T\'])">0.0000</xsl:if>     # added to T - avg. 101.1051, mono. 101.04768';
print XSL '<p/>';


print XSL 'add_C_Cysteine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'C\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'C\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'C\'])">0.0000</xsl:if>      # added to C - avg. 103.1388, mono. 103.00919';
print XSL '<p/>';

print XSL 'add_L_Leucine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'L\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'L\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'L\'])">0.0000</xsl:if>       # added to L - avg. 113.1594, mono. 113.08406';
print XSL '<p/>';

print XSL 'add_I_Isoleucine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'I\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'I\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'I\'])">0.0000</xsl:if>    # added to I - avg. 113.1594, mono. 113.08406';
print XSL '<p/>';

print XSL 'add_X_LorI = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'X\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'X\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'X\'])">0.0000</xsl:if>          # added to X - avg. 113.1594, mono. 113.08406';
print XSL '<p/>';

print XSL 'add_N_Asparagine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'N\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'N\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'N\'])">0.0000</xsl:if>    # added to N - avg. 114.1038, mono. 114.04293';
print XSL '<p/>';


print XSL 'add_O_Ornithine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'O\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'O\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'O\'])">0.0000</xsl:if>     # added to O - avg. 114.1472, mono  114.07931';
print XSL '<p/>';

print XSL 'add_B_avg_NandD = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'B\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'B\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'B\'])">0.0000</xsl:if>     # added to B - avg. 114.5962, mono. 114.53494';
print XSL '<p/>';

print XSL 'add_D_Aspartic_Acid = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'D\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'D\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'D\'])">0.0000</xsl:if> # added to D - avg. 115.0886, mono. 115.02694';
print XSL '<p/>';


print XSL 'add_Q_Glutamine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Q\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Q\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Q\'])">0.0000</xsl:if>     # added to Q - avg. 128.1307, mono. 128.05858';
print XSL '<p/>';

print XSL 'add_K_Lysine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'K\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'K\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'K\'])">0.0000</xsl:if>        # added to K - avg. 128.1741, mono. 128.09496';
print XSL '<p/>';

print XSL 'add_Z_avg_QandE = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Z\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Z\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Z\'])">0.0000</xsl:if>     # added to Z - avg. 128.6231, mono. 128.55059';
print XSL '<p/>';

print XSL 'add_E_Glutamic_Acid = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'E\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'E\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'E\'])">0.0000</xsl:if> # added to E - avg. 129.1155, mono. 129.04259';
print XSL '<p/>';


print XSL 'add_M_Methionine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'M\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'M\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'M\'])">0.0000</xsl:if>    # added to M - avg. 131.1926, mono. 131.04049';
print XSL '<p/>';

print XSL 'add_H_Histidine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'H\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'H\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'H\'])">0.0000</xsl:if>     # added to H - avg. 137.1411, mono. 137.05891';
print XSL '<p/>';

print XSL 'add_F_Phenyalanine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'F\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'F\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'F\'])">0.0000</xsl:if>  # added to F - avg. 147.1766, mono. 147.06841';
print XSL '<p/>';

print XSL 'add_R_Arginine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'R\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'R\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'R\'])">0.0000</xsl:if>      # added to R - avg. 156.1875, mono. 156.10111';
print XSL '<p/>';

print XSL 'add_Y_Tyrosine = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Y\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Y\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'Y\'])">0.0000</xsl:if>      # added to Y - avg. 163.1760, mono. 163.06333';
print XSL '<p/>';

print XSL 'add_W_Tryptophan = <xsl:if test="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'W\']"><xsl:value-of select="pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'W\']/@massdiff"/></xsl:if><xsl:if test="not(pepx:search_summary/pepx:aminoacid_modification[@variable=\'N\' and @aminoacid=\'W\'])">0.0000</xsl:if>    # added to W - avg. 186.2132, mono. 186.07931';
print XSL '<p/>';

print XSL '<br/>';



print XSL '#';
print XSL '<p/>';
print XSL '# COMET_ENZYME_DEF must be at the end of this parameters file.';
print XSL '<p/>';
print XSL '# You can define your own enzymes here using the same format ... just add/edit an entry';
print XSL '<p/>';
print XSL '#';
print XSL '<p/>';
print XSL '[COMET_ENZYME_INFO]';
print XSL '<p/>';
print XSL '0.  No_Enzyme              0      -           -';
print XSL '<p/>';
print XSL '1.  Trypsin                1      KR          P';
print XSL '<p/>';
print XSL '2.  Chymotrypsin           1      FWY         P';
print XSL '<p/>';
print XSL '3.  Clostripain            1      R           -';
print XSL '<p/>';
print XSL '4.  Cyanogen_Bromide       1      M           -';
print XSL '<p/>';
print XSL '5.  IodosoBenzoate         1      W           -';
print XSL '<p/>';
print XSL '6.  Proline_Endopept       1      P           -';
print XSL '<p/>';
print XSL '7.  Staph_Protease         1      E           -';
print XSL '<p/>';
print XSL '8.  Trypsin_K              1      K           P';
print XSL '<p/>';
print XSL '9.  Trypsin_R              1      R           P';
print XSL '<p/>';
print XSL '10. AspN                   0      D           -';
print XSL '<p/>';
print XSL '11. Cymotryp-Modified      1      FWYL        P';
print XSL '<p/>';
print XSL '12. Elastase               1      ALIV        P';
print XSL '<p/>';
print XSL '13. Elastase-Tryp-Chymo    1      ALIVKRWFY   P';
print XSL '<p/>';

print XSL '</xsl:template>';

print XSL '</xsl:stylesheet>';

close(XSL);




}

sub writeXSLFile {
(my $temp_xslfile, my $xmlfile) = @_;

my $RESULT_TABLE_PRE = '<table ';
my $RESULT_TABLE = 'cellpadding="0" bgcolor="white" style="font-family: \'Courier New\', Courier, mono; font-size: 10pt;">';
my $RESULT_TABLE_SUF = '</table>';

open(XSL, ">$temp_xslfile") or die "cannot open $temp_xslfile $!\n";

print XSL '<?xml version="1.0"?>', "\n";
print XSL '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:pepx="http://regis-web.systemsbiology.net/pepXML">', "\n";



print XSL '<xsl:template match="pepx:msms_pipeline_analysis">';
print XSL '<xsl:apply-templates select="pepx:msms_run_summary[pepx:search_summary[@search_engine=\'' . $engine . '\' and @base_name=\'' . $basename . '\']]"/>';
print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:msms_run_summary">';
print XSL '<xsl:apply-templates select="pepx:search_summary"/>';

print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:search_summary">';

print XSL '<b>Static modifications:</b><p/>';
print XSL '<br/>';
print XSL '<xsl:apply-templates select="pepx:terminal_modification[@variable=\'N\']"/>';
print XSL '<xsl:apply-templates select="pepx:aminoacid_modification[@variable=\'N\']"/>';

print XSL '<b>Variable modifications:</b><p/>';
print XSL '<br/>';
print XSL '<xsl:apply-templates select="pepx:terminal_modification[@variable=\'Y\']"/>';
print XSL '<xsl:apply-templates select="pepx:aminoacid_modification[@variable=\'Y\']"/>';

print XSL '<b>Enzyme constraints:</b><p/>';
print XSL '<br/>';
print XSL '<xsl:apply-templates select="pepx:enzymatic_search_constraint"/>';

print XSL '<b>Sequence constraints:</b><p/>';
print XSL '<br/>';
print XSL '<xsl:apply-templates select="pepx:sequence_search_constraint"/>';

print XSL '<b>Parameters:</b><p/>';
print XSL '<br/>';
print XSL '<xsl:apply-templates select="pepx:parameter"/>';

print XSL '</xsl:template>';


print XSL '<xsl:template match="pepx:terminal_modification">';
print XSL '<font color="blue">terminal modification: </font><xsl:value-of select="@terminus"/><p/>';
print XSL '<font color="blue">mass: </font><xsl:value-of select="@mass"/><p/>';
print XSL '<font color="blue">massdiff: </font><xsl:value-of select="@massdiff"/><p/>';
print XSL '<xsl:if test="@variable = \'Y\' and @symbol"><font color="blue">symbol: </font><xsl:value-of select="@symbol"/><p/></xsl:if>';
print XSL '<br/>';
print XSL '</xsl:template>';



print XSL '<xsl:template match="pepx:aminoacid_modification">';
print XSL '<font color="blue">aminoacid modification: </font><xsl:value-of select="@aminoacid"/><p/>';
print XSL '<font color="blue">mass: </font><xsl:value-of select="@mass"/><p/>';
print XSL '<font color="blue">massdiff: </font><xsl:value-of select="@massdiff"/><p/>';
print XSL '<xsl:if test="@variable = \'Y\' and @symbol"><font color="blue">symbol: </font><xsl:value-of select="@symbol"/><p/></xsl:if>';
print XSL '<br/>';
print XSL '</xsl:template>';


print XSL '<xsl:template match="pepx:enzymatic_search_constraint">';
print XSL '<font color="blue">enzyme: </font><xsl:value-of select="@enzyme"/><p/>';
print XSL '<font color="blue">max num cleavages: </font><xsl:value-of select="@max_num_internal_cleavages"/><p/>';
print XSL '<font color="blue">min num termini: </font><xsl:value-of select="@min_number_termini"/><p/>';
print XSL '<br/>';
print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:sequence_search_constraint">';
print XSL '<font color="blue">sequence: </font><xsl:value-of select="@sequence"/><p/>';
print XSL '<br/>';
print XSL '</xsl:template>';

print XSL '<xsl:template match="pepx:parameter">';
print XSL '<font color="blue">COUNTER: <xsl:value-of select="@name"/> = </font>';
print XSL '<xsl:value-of select="@value"/><p/>';
print XSL '<br/>';
print XSL '</xsl:template>';


print XSL '</xsl:stylesheet>';

close(XSL);

}


