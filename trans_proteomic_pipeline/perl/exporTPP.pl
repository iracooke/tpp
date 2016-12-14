#!/usr/bin/perl
#
# exporTPP.pl
# Create zip file with TPP results (protein, peptide, and associated spectra) in html format
# 2011 luis@isb

use strict;
use Getopt::Long;
use XML::Parser;
use File::Copy;
use CGI;

# Where things are
my $TPP_WEB  = 'localhost/tpp-bin'; #'https://regis-web.systemsbiology.net/tpp-lmendoza/cgi-bin';
my $comet_url = "$TPP_WEB/comet-fastadb.cgi";
my $pepxml_url= "$TPP_WEB/peptidexml_html2.pl";
my $xslt = 'xsltproc'; #'/usr/bin/xsltproc';
my $lorikeet_dir = 'C:/Inetpub/wwwroot/ISB/html'; #'/proteomics/lmendoza/tpp_TRUNK/trans_proteomic_pipeline/src/Visualization/Comet/plot-msms';
my $readmz_cmd= "C:/Inetpub/tpp-bin/readmzXML"; #"/tools/bin/TPP/tpp-lmendoza/bin/readmzXML";
my $wget_cmd  = 'wget';#'/usr/bin/wget';       # full path to wget

my %aa_masses = # monoisotopic
    (
     'n' => 1.007825,
     'c' => 17.002740,
     'G' => 57.021464,
     'D' => 115.02694,
     'A' => 71.037114,
     'Q' => 128.05858,
     'S' => 87.032029,
     'K' => 128.09496,
     'P' => 97.052764,
     'E' => 129.04259,
     'V' => 99.068414,
     'M' => 131.04048,
     'T' => 101.04768,
     'H' => 137.05891,
     'C' => 103.00919,
     'F' => 147.06841,
     'L' => 113.08406,
     'R' => 156.10111,
     'I' => 113.08406,
     'N' => 114.04293,
     'Y' => 163.06333,
     'W' => 186.07931
     );

my $DEBUG = 1;
my %run_data;
my %group_data;
my @prots_data;
$|++;


my $USAGE=<<"EOU";
Usage:
   $0 file.prot.xml [-p minProb] [-n zipName]

minProb:   ProteinProphet probability cutoff (inclusive)
zipName:   Name of zip file without extension: zipName.zip

EOU


my $infile  = shift || die "I need an input file!\n\n".$USAGE;

my %options;
GetOptions(\%options,'p=s', 'n=s' );

my $minProb = $options{'p'} || 0.0;
my $zipName = $options{'n'} || "TPP_RESULTS_$minProb";


print "-"x79 ."\n";
print &printWithDots("Input file", $infile,1);
die "-"x79 ."\nCannot read input file!\n\n".$USAGE if !-r $infile;

print &printWithDots("Min probability", $minProb,1);
print &printWithDots("Zip name","$zipName.zip",1);
print "-"x79 ."\n";

&createDirs();
&writeAuxFiles();
print "-"x79 ."\n";

&processProtXML();
&writeProtsFile();

&writeIndexFile();

# spectraFiles();

# modelsFiles();
# cytoscapeFiles();

# zipAll();

exit;

###################################################
sub writeIndexFile {
    my $index_file = "$zipName/index.html";
    print "--> Trying to write index file $index_file\n";

    open(INDEX, ">$index_file");

    print INDEX<<END;
<html>
<head>
<title>Exported ProteinProphet Results</title>
<link rel="stylesheet" type="text/css" href="css/tpp.css">
</head>
<body bgcolor="#c0c0c0" link=#0000FF vlink=#0000FF>
<h1>ProteinProphet Results for $zipName</h1>

<h2><a href='prots.html'>Protein List</a> -- (Prob >= $minProb)</h2>

<table border='1'>
<tr class='banner_cid'>
<th colspan='2'>Run Options</th>
</tr>
END

    for my $att (sort keys %run_data) {
	print INDEX "<tr><td align='right' class='graybox2'>$att</td><td align='left' class='graybox'>$run_data{$att}</td></tr>\n"
	    unless ($att =~ /^NSP_/  || $att =~ /^FILTER_/);
    }

    print INDEX "</table>\n";


    # NSP graph using Google Chart
#    my $gchart = 'chs=500x400&chbh=45&cht=bvs&chco=4DBB89,FFBBBB&chm=N*p0*,ffffff,0,,10,,c|N*p0*,000000,1,,10,,c|D,0033FF,2,0,3,1&chxt=x,y&chds=0,1&chd=t2:';

    my $gchart = 'chof=png&chtt=Learned+NSP+Distributions&chs=700x400&chbh=50&cht=bvs&chco=4DBB89,FFBBBB&chxt=x,x,y&chds=0,1&chdlp=|r&chdl=Positive+Freq|Negative+Freq&chxp=1,50&chxs=1,,15';

    my $pos = $run_data{"NSP_bin_0_pos_freq"};
    my $neg = $run_data{"NSP_bin_0_neg_freq"};
    my $rto = "A$run_data{NSP_bin_0_pos_to_neg_ratio},0033FF,0,0,12,-1";
    my $bnd = 'chxl=1:|NSP|0:|<='.$run_data{"NSP_bin_0_nsp_upper_bound_incl"};  # dropped  &le;

    for my $bin (1..50) {
	last unless $run_data{"NSP_bin_".$bin."_bin_no"};

	$pos .= ','.$run_data{"NSP_bin_".$bin."_pos_freq"};
	$neg .= ','.$run_data{"NSP_bin_".$bin."_neg_freq"};
	$rto .= '|A'.$run_data{"NSP_bin_${bin}_pos_to_neg_ratio"}.($run_data{"NSP_bin_${bin}_alt_pos_to_neg_ratio"} ? ' ('.$run_data{"NSP_bin_${bin}_alt_pos_to_neg_ratio"}.')' : '').",0033FF,0,$bin,12,-1";
	$bnd .= '|'. ($run_data{"NSP_bin_".$bin."_nsp_upper_bound_incl"} ? '<='.$run_data{"NSP_bin_".$bin."_nsp_upper_bound_incl"} : '<'.$run_data{"NSP_bin_".$bin."_nsp_upper_bound_excl"});  # dropped &le;  and  &le;
    }
    $gchart .= "&chd=t:$pos|$neg";
    $gchart .= "&chm=N*p1*,ffffff,0,,10,,c|N*p1*,000000,1,,10,,c|$rto";
    $gchart .= "&$bnd";

    my $nsp_img = "models/nsp_graph.png";

#    print INDEX "<a href='http://chart.apis.google.com/chart?$gchart'>NSP Google chart!</a>\n";  # leave for debugging

    my $cmd = "$wget_cmd -q -O $zipName/$nsp_img \"http://chart.apis.google.com/chart?$gchart\"";
    system("$cmd");
    &reportError("there was a problem with wget from google chart: $?\n-->$cmd",1) if ($?);

    print INDEX<<EONSP;
<br/><br/>
<table border='1'>
<tr class='banner_cid'><th>NSP Distributions</th></tr>
<tr><td><img src='$nsp_img'/></td></tr>
</table>
EONSP


    # Sens/Err graph using Google Chart
    $gchart = 'chof=png&chtt=Estimated+Sensitivity+and+Error+Rate&chs=700x400&cht=lxy&chco=4DBB89,FFBBBB&chxt=x,x,y,y&chds=0,1&chdl=Sensitivity|Error&chxp=1,50|3,50&chxs=1,,15|3,,15&chxl=1:|Min+Protein+Prob|3:|Sensitivity+or+Error&chxr=0,0,1,0.1|2,0,1,0.1&chg=10,10,1,5&chm=o,444444,0,-1,3|o,444444,1,-1,3';

    my $prb = $run_data{"FILTER_0_min_probability"};
    my $sen = $run_data{"FILTER_0_sensitivity"};
    my $err = $run_data{"FILTER_0_false_positive_error_rate"};

    for my $num (1..50) {
	last unless $run_data{"FILTER_".$num."_min_probability"};

	$prb .= ','.$run_data{"FILTER_".$num."_min_probability"};
	$sen .= ','.$run_data{"FILTER_".$num."_sensitivity"};
	$err .= ','.$run_data{"FILTER_".$num."_false_positive_error_rate"};
    }
    $gchart .= "&chd=t:$prb|$sen|$prb|$err";

    my $err_img = "models/err_graph.png";

    print INDEX "<a href='http://chart.apis.google.com/chart?$gchart'>Sens/Error Google chart!</a>\n";  # leave for debugging

    $cmd = "$wget_cmd -q -O $zipName/$err_img \"http://chart.apis.google.com/chart?$gchart\"";
    system("$cmd");
    &reportError("there was a problem with wget from google chart: $?\n-->$cmd",1) if ($?);

    print INDEX<<EOERR;
<br/><br/>
<table border='1'>
<tr class='banner_cid'><th colspan="10">Sensitivity and Error Rate Info</th></tr>
<tr><td rowspan="100"><img src='$err_img'/></td></tr>
<tr class='graybox2'><td>Min Prob</td><td>Sensitivity</td><td>Error</td><td># Correct</td><td># Incorrect</td></tr>
EOERR

    for my $num (0..50) {
	last unless $run_data{"FILTER_".$num."_min_probability"};

	print INDEX "<tr class='graybox'>".
	    "<td>".$run_data{"FILTER_".$num."_min_probability"}."</td>".
	    "<td>".$run_data{"FILTER_".$num."_sensitivity"}."</td>".
	    "<td>".$run_data{"FILTER_".$num."_false_positive_error_rate"}."</td>".
	    "<td>".$run_data{"FILTER_".$num."_predicted_num_correct"}."</td>".
	    "<td>".$run_data{"FILTER_".$num."_predicted_num_incorrect"}."</td>".
	    "</tr>\n";

    }
    print INDEX "</table>\n";

    print INDEX "<hr><h6>TPP-results export on ".scalar(localtime)."</h6>\n</body>\n</html>\n";
    close INDEX;
}


sub writeProtsFile {
    my $prot_file = "$zipName/prots.html";
    print "--> Trying to write protein results file $prot_file\n";

    open(PROT, ">$prot_file");

    print PROT<<END;
<html>
<head>
<title>Proteins</title>
<link rel="stylesheet" type="text/css" href="css/tpp.css">
</head>
<body bgcolor="#c0c0c0" link=#0000FF vlink=#0000FF>
<h1>Protein Results for $zipName</h1>

<table border='1'>
<tr class='banner_cid'>
<th>#</th>
<th>Probability</th>
<th>Main entry accession</th>
<th># Peps</th>
<th>% Cov.</th>
</tr>
END

    my $class = 'graybox2';
    my $prev = 0;

    foreach my $pref (@prots_data) {

	my $extra = $$pref{inds} > 0 ? " +$$pref{inds}" : '';
	my $link  = $$pref{name} ? "<a href='proteins/group_$$pref{entry}.html'>$$pref{name}$extra</a>" : '';
	my $align = $$pref{name} ? 'right' : 'left';

	$class = $class eq 'graybox' ? 'graybox2' : 'graybox' unless $$pref{group} eq $prev;

	print PROT<<END;
<tr class="$class">
    <td align='$align'>$$pref{entry}.</td>
    <td align='$align'><font color="AA2222">$$pref{prob}</font></td>
    <td>$link</td>
    <td align='right'>$$pref{npep}</td>
    <td align='right'>$$pref{pcov}</td>
</tr>

END

        $prev = $$pref{group};

    }

    print PROT "</table><hr><h6>TPP-results export on ".scalar(localtime)."</h6>\n";

    print PROT<<END;
</body>
</html>
END

    close PROT;

}

sub displayPeptides {
    my $prt = shift;
    my $ssq = shift;

    my $pep_unq = 1;
    my $pep_num = 0;

    my $html;

    for my $i (1..1000) {  # SBJ help us if we have more than this...
	my $pep = "${prt}_pep$i";

	my $seq = "${pep}_seq";
	my $mod = "${pep}_mod";
	my $wgt = "${pep}_wgt";
	my $ntt = "${pep}_ntt";
	my $ini = "${pep}_ini";
	my $prb = "${pep}_nsp";
	my $chg = "${pep}_chg";
	my $evi = "${pep}_evi";
	my $unq = "${pep}_unq";
	my $num = "${pep}_num";

	last unless $group_data{$seq};

	next unless $group_data{$seq} eq $ssq;

	my $class =  $group_data{$evi} eq 'Y' ? 'markAA' : '';
	my $pepsq = $group_data{$mod} ? $group_data{$mod} : $group_data{$seq};
	$pepsq =~ s|\[|<sub>|g;
	$pepsq =~ s|\]|</sub>|g;

	$html .= "<tr name='${prt}_$ssq' style='display: none;' class='pep_ion'><td>&nbsp;</td><td>&nbsp;</td><td>";
	$html .= "$group_data{$wgt}";
	$html .= "</td><td align='left'>$pepsq</td><td align='left'>+$group_data{$chg}</td><td>$group_data{$num}</td><td class='$class'>$group_data{$prb}</td><td>$group_data{$ini}</td><td>$group_data{$ntt}</td></tr>\n";

	$pep_unq *= ($group_data{$unq} eq 'Y');
	$pep_num += $group_data{$num};
    }

    $pep_unq = $pep_unq ? '*' : '';

    $html = "<td class='markAA' align='right'><tt>$pep_unq</tt></td><td><tt><a href=\"javascript:showclass('${prt}_$ssq')\">$ssq</a></tt></td><td>&nbsp;</td><td align='left'><tt style='font-weight:800'><a href='../peptides/$ssq.html'>$pep_num</a></tt></td></tr>\n" . $html;


    return $html;
}

sub writePepPage {
    my $seq = shift;

    my $pep_file = "$zipName/peptides/$seq.html";

    return if -e $pep_file;

    print "--> Trying to write peptide file $pep_file\n";

    my $cmd = "$wget_cmd -q -O $pep_file.tmp \"${pepxml_url}?xslt=$xslt&prots_display=show_all&Ref=$seq&Infile=$run_data{'source_files'}\"";
    system("$cmd");
    &reportError("there was a problem with wget: $?\n-->$cmd",1) if ($?);

    my (@ions, %all_seqs, %all_probs, $headings);

    open(TMP, "$pep_file.tmp");
    while (<TMP>) {
	for my $row (split /\<tr.+?\>/i, $_) {
	    $row =~ s|\</TR\>||ig;
	    $row =~ s|\</TABLE\>||ig;

	    if ($row =~ m|Prob=(.+?)">.*plot-msms.*\?(.*.dta)"\>.*QUERY=.+?"\>(.+?)\</A\>|i) {  #"
		my $prob = $1;
		my $plot_qs = $2;
		my $modseq = $3;

		$plot_qs =~ s/&amp;/;/g;
		my $query = CGI->new($plot_qs);
#		for my $key ($query->param) {
#		    print ".......key: $key  ::  value: ".$query->param($key)."\n";
#		}

		my $modstring = &buildLorikeetMods( sequence => $seq,
						    qs => $query);

		my $spectrum = $query->param('Dta');

#		print "SPECTRUM($spectrum)::";

		$spectrum =~ m|.*/(.*\.(\d+)\.dta)$|;
		$spectrum = $1;
		my $charge = $2;

#		print "($spectrum) -- CHARGE($charge)\n";		

		$all_probs{$prob}++;
		$all_seqs{"${charge}_$modseq"}++;

		$row =~ s|\<A.+?"\>||ig;  #"
		$row =~ s|\</A\>||ig;

		my $ions_link = &writeSpectrumPage( spectrum => $spectrum,
						    prob => $prob,
						    charge => $charge,
						    seq => $seq,
						    mods => $modstring,
						    modseq => $modseq );

		$row =~ s|(\d+/\d+)|<a href='../spectra/$ions_link'>$1</a>|i;

		my $pepref = {
		    prob => $prob,
		    spec => $spectrum,
		    mseq => $modseq,
		    chrg => $charge,
		    html => $row
		};
		push @ions, $pepref;

	    } elsif ($row =~ /entry.*index.*prob.*spectrum.*peptide.*protein/) {
		$headings = $row;
	    }


	}
    }
    close TMP;
    unlink "$pep_file.tmp";

    open(PEP, ">$pep_file");
    print PEP<<EOHEAD;
<html>
<head>
<title>Spectrum and PSM information for $seq</title>
<link rel='stylesheet' type='text/css' href='../css/tpp.css'>
</head>

<body bgcolor="#c0c0c0" onload="self.focus();" link="#0000FF" vlink="#0000FF">
<h1>Spectrum and PSM information for sequence: <span class='pep_ion'>$seq</span></h1>
EOHEAD

    for my $cs (sort keys %all_seqs) {
	my ($c,$s) = split /_/, $cs;

	print PEP<<EOPEP;
<table cellspacing='0'>
<tr>
<td class='banner_cid'>&nbsp;&nbsp;Ion: <font color='ff8700'>$s&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;+$c</font></td>
</tr>
</table>

<div class="formentry">
<table cellpadding="2">
<tr class='markAA'>$headings</tr>
EOPEP

        for my $p (sort {$b <=> $a} keys %all_probs) {
	    for my $pref (@ions) {
		print PEP "<tr class='pep_ion'>$$pref{html}</tr>\n" if ( ($$pref{chrg} eq $c) && ($$pref{mseq} eq $s) && ($$pref{prob} eq $p) );
	    }
	}

	print PEP "</table>\n</div>\n\n<br/>\n\n";
    }

    print PEP "<hr noshade/><br/>\n<h6>TPP-results export on ".scalar(localtime)."</h6>\n";
    print PEP "</body>\n</html>\n";

    close PEP;

}


sub writeSpectrumPage {
    my %args = @_;

    my $spectrum_file = "$args{spectrum}.html";

    $args{spectrum} =~ /^(.*)\.\d+\.(\d+)\.\d\./;
    my $mzFile = $1;
    my $scan = $2;

    $run_data{'source_files'} =~ m|(.*)/|;
    $mzFile = "$1/$mzFile";  #this is the base path of the pep.xml file

    $mzFile = "$mzFile.mzXML" if -e "$mzFile.mzXML";  # cheap way! need to parse from pepXML at some point...
    $mzFile = "$mzFile.mzML" if -e "$mzFile.mzML";


    open(SPEC, ">$zipName/spectra/$spectrum_file") || &reportError("Could not write spectrum file: $spectrum_file",1);
    print SPEC<<EOHEAD;
<html>
<head><meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title>Spectrum for $args{spectrum}</title>
<link rel="stylesheet" type="text/css" href="../css/tpp.css">
</head>

<body bgcolor="#c0c0c0" onload="self.focus();" link="#0000FF" vlink="#0000FF">

<h1>Spectrum for $args{modseq} <sup>+$args{charge}</sup>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(P=$args{prob})</h1>
EOHEAD

    if (!-e $mzFile) {
	print SPEC "<h1>Could not find file $mzFile</h1>\n";
	
    } elsif (!$scan) {
	print SPEC "<h1>Could not extract scan from $args{spectrum}</h1>\n";

    } else {
	my @spectrum = `$readmz_cmd -r $mzFile $scan`;

	print SPEC<<EOBODY;
        <!--[if IE]><script language="javascript" type="text/javascript" src="../js/excanvas.min.js"></script><![endif]-->
        <script type="text/javascript" src="../js/jquery.min.js"></script>
        <script type="text/javascript" src="../js/jquery-ui.min.js"></script>
        <script type="text/javascript" src="../js/jquery.flot.js"></script>
        <script type="text/javascript" src="../js/jquery.flot.selection.js"></script>
        <script type="text/javascript" src="../js/specview.js"></script>
        <script type="text/javascript" src="../js/peptide.js"></script>
        <script type="text/javascript" src="../js/aminoacid.js"></script>
        <script type="text/javascript" src="../js/ion.js"></script>
        <link rel="stylesheet" type="text/css" href="../css/lorikeet.css">

        <div id="lorikeet"></div>

        <script type="text/javascript">
        \$(document).ready(function () {

            \$("#lorikeet").specview({"sequence":"$args{seq}",
                                      "scanNum":$scan,
                                      "charge":$args{charge},
                                      "precursorMz":0,
                                      "fileName":"$args{spectrum}",
                                      "width": 650,
                                      "height":400,
                                      "showA":[0,0,0],
                                      "showB":[1,1,0],
                                      "showC":[0,0,0],
                                      "showX":[0,0,0],
                                      "showY":[1,1,0],
                                      "showZ":[0,0,0],
				      "variableMods":[ $args{mods} ],
                                      // ToDo: "ntermMod":0,
                                      // ToDo: "ctermMod":0,
                                      "peaks":ms2peaks});
        });


    var ms2peaks = [
EOBODY

	for (@spectrum)  {
	    my ($m, $i) = split ' ', $_;
	    print SPEC "[$m,$i],\n";
	}
	print SPEC "];\n</script>\n\n";
		    
    }

    #footer
    print SPEC "<br><hr size='5' noshade><br>\n</body></html>";
    close SPEC;

    return "$spectrum_file";
}


sub writeProtFile {
    my $group = $group_data{'grpnum'};

#    return unless $group == 155;  #test

    if ($group_data{'is_group'} eq 'T') {
	my $grpref = {
	    group=> $group,
	    entry=> $group,
	    prob => $group_data{prob},
	    name => '',
	    inds => '',
	    pcov => '',
	    npep => ''
	    };
	push @prots_data, $grpref;
    }

    # deal with sub-groups
    for my $aa ('','a'..'z') {
	for my $a ('a'..'z') {
	    my $cur = "prot_$a$aa";

	    my $nom = "${cur}_name";
	    my $ind = "${cur}_inds";
	    my $prb = "${cur}_prob";
	    my $pep = "${cur}_peps";
	    my $num = "${cur}_npep";
	    my $cov = "${cur}_cvrg";
	    my $grp = $group;

	    last unless $group_data{$nom};

	    $grp .= $a.$aa if $group_data{'is_group'} eq 'T';

	    my $ref = "Ref=$group_data{$nom}";
	    my $i = 0;

	    for my $p (split ' ', $group_data{$ind}) {
		$ref .= "&Ref=$p";
		$i++;
	    }

	    my $protref = {
		group=> $group,
		entry=> $grp,
		prob => $group_data{$prb},
		name => $group_data{$nom},
		inds => $i,
		pcov => $group_data{$cov},
		npep => $group_data{$num}
	    };
	    push @prots_data, $protref;

	    my $prot_file = "$zipName/proteins/group_$grp.html";
	    print "--> Trying to write protein file $prot_file\n";

	    my $cmd = "$wget_cmd -q -O $prot_file.tmp \"${comet_url}?Db=$run_data{'reference_database'}&$ref&Pep=$group_data{$pep}\"";
	    system("$cmd");
	    &reportError("there was a problem with wget: $?\n-->$cmd",1) if ($?);

	    open(TEMP, "$prot_file.tmp");
	    open(PROT, ">$prot_file");

	    my $print = 1;

	    while (<TEMP>) {
		if (/<title>/) {
		    print PROT "<title>Sequence and peptide information for Protein Group $grp</title>\n";
		    print PROT "<!-- $cmd -->\n" if $DEBUG;

		} elsif (/<body/) {
		    print PROT $_;
		    print PROT "<h1>Sequence and peptide information for Protein Group $grp (Prob=$group_data{$prb})</h1>\n";

		} elsif (/<style/) {
		    print PROT "<link rel='stylesheet' type='text/css' href='../css/tpp.css'>\n";
		    print PROT<<EOSCRIPT;
<script language="JavaScript">
    function showclass(domname){
	var tr_arr = document.getElementsByName(domname);

	for (i=0; i<tr_arr.length; i++) {
	    if (tr_arr[i].style.display == 'none') {
		new_state = 'table-row';
	    } else {
		new_state = 'none';
	    }
	    tr_arr[i].style.display = new_state;
	}

    }
</script>
EOSCRIPT

		    $print = 0;

		} elsif (m|</style>|) {
		    $print = 1;

		} elsif (/Position.*Mass.*Peptide/) {
		    print PROT "<tr>";
		    foreach my $head (qw(Position Mass Wt Peptide Charge Obs NSP_Adjusted Initial_Probability NTT)) {
			print PROT "<th class='nav'>$head</th>";
		    }
		    print PROT "</tr>\n";

		} elsif (m|<!-- PEPSEQ: (\S*) -->|) {
		    my $seq = $1;
		    print PROT &displayPeptides($cur,$seq);

		    &writePepPage($seq);

		} elsif (/Blast.cgi/) {
		    # do nothing

		} elsif (/In-silico Digestion pane/) {
		    $print = 0;

		} elsif (/End in-silico Digestion pane/) {
		    $print = 1;

		} else {
		    print PROT $_ if $print;

		}

		last if (/page footer/);

	    }

	    print PROT "<h6>TPP-results export on ".scalar(localtime)."</h6>\n";
	    print PROT "</body>\n</html>\n";

	    close TEMP;
	    close PROT;

	    unlink "$prot_file.tmp";
		    
	}
    }

#    exit if ($group == 2);

}

sub processProtXML {

    #### Set up the XML parser and parse the returned XML
    my $parser = XML::Parser->new(
				  Handlers => {
				      Start => \&start_protxml_element,
				      End   => \&end_protxml_element,
#				      Char  => \&protxml_chars,
				  },
				  ErrorContext => 2 );
    eval { $parser->parsefile( $infile ); };
    die "ERROR_PARSING_XML:$@" if($@);

}

sub start_protxml_element {
    my ($handler, $element, %atts) = @_;

    if ($element eq 'protein_group') {
	if ($DEBUG) {
	    print "Group [ ";
	    while (my ($key, $value) = each (%atts)) {
		print "$key :: $value   ";
	    }
	    print "]\n";
	}

	$group_data{'grpnum'} = $atts{'group_number'};
	$group_data{'prob'} = $atts{'probability'};
	$group_data{'is_group'} = $atts{'pseudo_name'} ? 'T' : 'F';
    }

    elsif ($element eq 'protein') {
	my $prot = "prot_".$atts{'group_sibling_id'};
	$group_data{"current_prot"} = $prot;
	$group_data{"current_pept"} = 0;

	$group_data{"${prot}_name"} = $atts{'protein_name'};
	$group_data{"${prot}_prob"} = $atts{'probability'};
	$group_data{"${prot}_cvrg"} = $atts{'percent_coverage'} || 0;
	$group_data{"${prot}_peps"} = $atts{'unique_stripped_peptides'};
	$group_data{"${prot}_npep"} = $atts{'total_number_peptides'};
	$group_data{"${prot}_inds"} = '';
    }

    elsif ($element eq 'indistinguishable_protein') {
	my $prot = $group_data{"current_prot"};

	$group_data{"${prot}_inds"} .= "$atts{'protein_name'} ";
    }

    elsif ($element eq 'peptide') {
	my $prot = $group_data{"current_prot"};
	my $pepn = ++$group_data{"current_pept"};

	my $pep = "${prot}_pep$pepn";

	$group_data{"${pep}_seq"} = $atts{'peptide_sequence'};
	$group_data{"${pep}_chg"} = $atts{'charge'};
	$group_data{"${pep}_wgt"} = $atts{'weight'};
	$group_data{"${pep}_ntt"} = $atts{'n_enzymatic_termini'};
	$group_data{"${pep}_nsp"} = $atts{'n_sibling_peptides'};
	$group_data{"${pep}_ini"} = $atts{'initial_probability'};
	$group_data{"${pep}_nsp"} = $atts{'nsp_adjusted_probability'};
	$group_data{"${pep}_evi"} = $atts{'is_contributing_evidence'};
	$group_data{"${pep}_unq"} = $atts{'is_nondegenerate_evidence'};
	$group_data{"${pep}_num"} = $atts{'n_instances'};
    }

    elsif ($element eq 'modification_info') {
	my $prot = $group_data{"current_prot"};
	my $pepn = $group_data{"current_pept"};

	$group_data{"current_modn"} = 0;

	my $pep = "${prot}_pep$pepn";
	$group_data{"${pep}_mod"} = $atts{'modified_peptide'};
    }

    elsif ($element eq 'mod_aminoacid_mass') {
	my $prot = $group_data{"current_prot"};
	my $pepn = $group_data{"current_pept"};
	my $modn = ++$group_data{"current_modn"};

	my $mod = "${prot}_pep${pepn}_mod$modn";

	$group_data{"${mod}_mass"} = $atts{'mass'};
	$group_data{"${mod}_posn"} = $atts{'position'};
    }

    elsif (  ($element eq 'protein_summary_header') ||
	     ($element eq 'program_details') ||
	     ($element eq 'nsp_information') ||
	     ($element eq 'proteinprophet_details') ) {
	for my $att (keys %atts) {
	    $run_data{$att} = $atts{$att};
	}

	$run_data{"FILTER_NUM"} = 0 unless ($run_data{"FILTER_NUM"});
    }

    elsif ($element eq 'nsp_distribution') {
	my $bin = "NSP_bin_".$atts{'bin_no'};

	for my $att (keys %atts) {
	    $run_data{"${bin}_$att"} = $atts{$att};
	}

    }

    elsif ($element eq 'protein_summary_data_filter') {
	my $num = $run_data{"FILTER_NUM"}++;

	for my $att (keys %atts) {
	    $run_data{"FILTER_${num}_$att"} = $atts{$att};
	}

    }

    else {
	if ($DEBUG) {
	    print "Did not handle tag <$element>\n";
	}
    }


}

sub end_protxml_element {
    my ($handler, $element) = @_;

    if ($element eq 'protein_group') {
	if ($DEBUG) {
	    print " GROUP: $group_data{'grpnum'} :: $group_data{'prob'} :: $group_data{'is_group'}\n";
	    for my $key (sort keys(%group_data)) {
		print "......$key == $group_data{$key}\n";
	    }
	}

	&writeProtFile() if $group_data{'prob'} >= $minProb;
#	    && $group_data{'grpnum'} <0;

	undef %group_data;
    }

}

sub buildLorikeetMods{
    my %args = @_;
    
    my $mods = '';
    for my $key ($args{qs}->param) {
	if ($key =~ /Mod(\d+)/) {
	    my $index = $1;
	    my $amino = substr $args{sequence}, ($index-1), 1;

	    my $massdiff = $args{qs}->param($key) - $aa_masses{$amino};

	    $mods .= "{index:$index , modMass: $massdiff, aminoAcid: \"$amino\"}, ";
	}
    }

    return $mods;
}


sub writeAuxFiles{

    # copy Lorikeet
    foreach my $dir (qw(js css images)) {
	for my $file (glob("$lorikeet_dir/$dir/*")) {
	    print &printWithDots("  +-- $file");

	    my $tdir = $dir eq "images" ? "$dir/lorikeet" : $dir;
	    copy("$file" , "$zipName/$tdir/") || print "NOT ";
	    print "ok\n";
	}
    }

    my $css = "$zipName/css/tpp.css";

    open(CSS, ">$css");

    print CSS<<END;
.hideit {display:none}
.showit {display:table-row}
.accepted {background: #87ff87; font-weight:bold;}
.rejected {background: #ff8700;}
body{font-family: Helvetica, sans-serif; }
h1  {font-family: Helvetica, Arial, Verdana, sans-serif; font-size: 24pt; font-weight:bold; color:#0E207F}
h2  {font-family: Helvetica, Arial, sans-serif; font-size: 20pt; font-weight: bold; color:#0E207F}
h3  {font-family: Helvetica, Arial, sans-serif; font-size: 16pt; color:#FF8700}
h4  {font-family: Helvetica, Arial, sans-serif; font-size: 14pt; color:#0E207F}
h5  {font-family: Helvetica, Arial, sans-serif; font-size: 10pt; color:#AA2222}
h6  {font-family: Helvetica, Arial, sans-serif; font-size:  8pt; color:#333333}
table   {border-collapse: collapse; border-color: #000000;}
.banner_cid   {
                 background: #0e207f;
                 border: 2px solid #0e207f;
                 color: #eeeeee;
                 font-weight:bold;
              }
.markSeq      {
                 color: #0000FF;
                 font-weight:bold;
              }
.markAA       {
                 color: #AA2222;
                 font-weight:bold;
              }
.glyco        {
                 background: #d0d0ff;
                 border: 1px solid black;
              }
.messages     {
                 background: #ffffff;
                 border: 2px solid #FF8700;
                 color: black;
                 padding: 1em;
              }
.formentry    {
               background: #eeeeee;
                 border: 2px solid #0e207f;
                 color: black;
                 padding: 1em;
              }
.nav          {
                 border-bottom: 1px solid black;
                 font-weight:bold;
              }
.graybox      {
                 background: #dddddd;
                 border: 1px solid black;
                 font-weight:bold;
              }

.graybox2     {
                 background: #aaaaaa;
                 border: 1px solid black;
                 font-weight:bold;
              }
.pep_ion      {
                 font-family: monospace;
		 text-align: right;
              }
.seq          {
                 background: #ffaa33;
                 border: 1px solid black;
                 font-weight:bold;
              }
.info         {
                 border-top: 1px solid black;
                 color: #333333;
                 font-size: 10pt;
              }
END
    close CSS;

}


sub createDirs {
    print &printWithDots("Creating dir $zipName");

    if (-d $zipName) {
	print "EXISTS\n";
	die "Directory already exists; exiting (move/delete this directory and try again)\n\n";

    } else {
	mkdir($zipName) || die "failed: $!\n\n";
	print "ok\n";

	foreach my $dir (qw(js css images spectra peptides proteins models)) {
	    print &printWithDots("  +-- $zipName/$dir");
	    mkdir("$zipName/$dir") || print "(error: $!) -- NOT ";
	    print "ok\n";

	}
    }

}


sub printWithDots {
    my $stringLeft  = shift || '';
    my $stringRight = shift || '';
    my $eol = shift() ? "\n" : '';
    my $dots = 38 - length($stringLeft);
    $dots = 3 if ($dots < 3);

    return $stringLeft . "."x$dots . $stringRight . $eol;
}

sub reportError {
    my $errstring = shift;
    my $fatal = shift || 0;

    print "ERROR: $errstring\n";
    exit($fatal) if $fatal;
}
