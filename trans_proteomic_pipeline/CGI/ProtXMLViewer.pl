#!/usr/bin/perl
#############################################################################
# Program       : ProtXMLViewer.pl                                          #
# Author        : Luis Mendoza <lmendoza at systems biology>                #
# Date          : 31.08.13                                                  #
# SVN Info      : $Id: ProtXMLViewer.pl 6601 2014-09-04 02:31:03Z real_procopio $
#                                                                           #
# View and filter protXML result files in web browser                       #
#                                                                           #
# Copyright (C) 2013-2014 Luis Mendoza                                      #
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
# Luis Mendoza                                                              #
# Institute for Systems Biology                                             #
# 401 Terry Avenue North                                                    #
# Seattle, WA  98109  USA                                                   #
#                                                                           #
#############################################################################
#use strict;
# use Getopt::Long;
use XML::Parser;
# use File::Copy;
use File::Basename;
use CGI qw/-nosticky :standard/;
use POSIX; # needed for the wobniar color thingy
use tpplib_perl; # exported TPP lib function points
my $TPPVersionInfo = tpplib_perl::getTPPVersionInfo();

my $www_root = "c:/Inetpub/wwwroot/";   # full path to web server root

my $cgi_query = CGI->new;
my %opts;
my %stats;

my @fields_numeric = qw/Probability Coverage Num_Peptides Pct_Spectrum_ids/;
my @fields_string  = qw/Protein_Name Annotation Peptide_Sequence/;
my @fields_sortable= qw/Index Probability Coverage Num_Peptides Protein_Name Pct_Spectrum_ids/;
my @fields_hideable= qw/Index Annotation Protein_Groups Peptides Coverage Num_Peptides/;

my %group_data;
my %index;
my %pepseqs;
my @pepseqs;
my @prots_data;
my @sortme;
my @libra_channels;
my $errors = 0;
$|++;


my $USAGE=<<"EOU";
Usage:
   $0 file.prot.xml 
EOU

for ($cgi_query->url_param) {
    $cgi_query->param($_, $cgi_query->url_param($_)) unless $cgi_query->param($_); # do not replace
}


$opts{'infile'} = $cgi_query->param('file') || shift || '';
$opts{'format'} = $cgi_query->param('of') ? $cgi_query->param('of') : 'HTML'; # output format: HTML [= full page, default], html (portion), tsv
$opts{'self_url'} = url(-relative=>1);

&openHTML();
&processOptions();
&readIndex() unless ($cgi_query->param('Restore'));  # re-do index on 'Restore Original';

if ($opts{'action'} eq 'displayEntry') {
    unless ($index{'is_good'}) {
	&processProtXML();
	&writeIndex();
    }
    $opts{'show_peps'} = 1;
    &processProtXMLEntry($cgi_query->param('entry'));
	
    if ($opts{'boxPeps'}) {
	&showBoxPeps();
    }
    else {
	&writeProtsFile();
    }
}

elsif ($opts{'action'} eq 'displayAnnotEntry') {
    $group_data{"current_prot"} = 0;
    &processProtXMLEntry($cgi_query->param('entry'),'annot');

}

elsif ($opts{'action'} eq 'displayPeptide') {
    $opts{'show_peps'} = 1;
    &processProtXML();
    &writeProtsFile();
    # &showBoxPeps(); # add as option?
}

elsif ($opts{'action'} =~ /^Export/) {
    &processProtXML();
    &writeIndex();

    &sortEntries() if $opts{'sorting'};
    &writeTSVFile() if ($opts{'action'} eq 'ExportExcel');
    &writeJSONFile() if ($opts{'action'} eq 'ExportJSON');
}

else {
    &processProtXML();
    &writeIndex();

    &sortEntries() if $opts{'sorting'};

    if (@prots_data) {
	&writeProtsFile();
	&writeGaggleData() if ($opts{'write_ggl'});
    } else {
	&reportError("No protein entries passed the filtering criteria");
    }

    &writeStats();
    &addFilterOpts();
}

&closeHTML() if $opts{'format'} eq 'HTML';

exit;

###################################################
sub addFilterOpts {
###################################################
    print &addTabbedPane(label => 'Filter & Sort');

    push @fields_sortable, 'Xpress_Heavy' if $opts{"has_xpress"};
    push @fields_numeric,  'Xpress_Heavy' if $opts{"has_xpress"};

    push @fields_sortable, 'ASAP_Heavy' if $opts{"has_asapratio"};
    push @fields_numeric,  'ASAP_Heavy' if $opts{"has_asapratio"};

    push @fields_sortable, 'ASAP_pvalue' if $opts{"has_asapratio_pvalue"};
    push @fields_numeric,  'ASAP_pvalue' if $opts{"has_asapratio_pvalue"};

    if ($opts{"has_libra"}) {
	for my $ch (@libra_channels) {
	    push @fields_sortable, "Libra_$ch";
	}
    }

    print 
        start_form(-method=>'GET',
                   -action=>url(-relative=>1),
		   -style =>'display: inline;'),
	br;

    my @logic = qw/contains starts_with ends_with is_equal_to/;

    print "<span class='tgray' style='width:100%;'> FILTER</span>\n<br/>";
    print "<span style='position:relative;left:50px;'>";
    foreach my $field (@fields_numeric) {
	print
	    "<span style='width:150;display:inline-block;text-align:left'>$field</span>",
	    "<span class='small'> Min:</span>",
	    textfield(-name=>"min_$field",
		      -size=>5,
		      -maxlength=>10),
	    "<span class='small'> Max:</span>",
	    textfield(-name=>"max_$field",
		      -size=>5,
		      -maxlength=>10);

	if ( ($field eq 'Xpress_Heavy') || ($field eq 'ASAP_Heavy') ) {
	    print
		"&nbsp"x3,
		checkbox(-name=>"xcl_$field",
			 -label=>'',
			 -value=>'true'),
		"<span class='small'>exclude without $field</span>";
	}

	print br;
    }

    print br;

    foreach my $field (@fields_string) {
	print
	    "<span style='width:150;display:inline-block;text-align:left'>$field</span>",
	    checkbox(-name=>"not_logic_$field",
		     -label=>'',
		     -value=>'true'),
	    "<span class='small'>does not </span>",
	    popup_menu(-name=>"logic_$field",
		       -values=>\@logic),
	    textfield(-name=>"filter_$field",
		      -size=>25,
		      -maxlength=>25),
	    br;
    }
    print "</span>\n<br/>";

    print "<span class='tgray' style='width:100%;'> SORT</span>\n<br/>";
    print
	"<span class='small' style='position:relative;left:50px;'>",
	popup_menu(-name=>"sort_by",
		   -values=>(\@fields_sortable)),
	radio_group(-name=>"sort_direction",
		    -values=>['Ascending','Descending'],
		    ),
	"</span>",
	br,br;


    print "<span class='tgray' style='width:100%;'> DISPLAY</span>\n<br/>";
    print "<span style='position:relative;left:50px;'>";

    if (0) { # not now...
	foreach my $field (@fields_hideable) {
	    print
		"<span style='width:150;display:inline-block;text-align:left'>$field</span>",
		"<span class='small'>",
		radio_group(-name=>"display_$field",
			    -values=>['Show','Hide'],
			    ),
		"</span>",
		br;
	}
    }

    print
	'Highlight decoy entries, whose names start with: ',
	textfield(-name=>"decoys_start_with",
		  -size=>25,
		  -maxlength=>25),
	br,
	checkbox(-name=>"display_peptides",
		 -label=>'',
		 -value=>'true'),
	" Display peptides (may result in a large page!)",
	br,
	checkbox(-name=>"export_firegoose",
		 -label=>'',
		 -value=>'true'),
	" Broadcast results to Firegoose (need Firefox plugin)",
	br;

    if ($opts{"has_asapratio"}) {
	print
	    "Display ",
	    popup_menu(-name=>"asap_valtype",
		       -values=>['uncorrected','adjusted'],
		       ),
	    " ASAPRatio values",
	    br;
    }

    print "</span>\n<br/>";

    print "<span class='tgray' style='width:100%;'> ACTIONS</span>\n<br/>";
    print
	"<span style='position:relative;left:50px;'>",
	submit(-name=>'FilterSort',
	       -value=>'Filter / Sort'),
	hidden(-name=>'file',
	       -value=>$opts{'infile'}),
	endform,
	"</span>",
	"<span style='position:relative;left:150px;'>",
        start_form(-method=>'GET',
                   -action=>url(-relative=>1),
		   -style =>'display: inline;'),
	submit(-name=>'Restore',
	       -value=>'Restore Original'),
	hidden(-name=>'file',
	       -value=>$opts{'infile'}),
	endform,
	"</span>";

    print &closeTabbedPane();
}

###################################################
sub readIndex {
###################################################
    my $index_file = $opts{infile}.".index";

    return unless (-e $index_file);

    return if ( (-M $opts{infile}) < (-M $index_file) );

    open(INDEX, "$index_file");
    while (<INDEX>) {
	chomp;
	my ($e, $i) = split("\t", $_);
	$index{$e} = $i;
    }
    close INDEX;

    $index{'is_good'} = 1;
}

###################################################
sub writeIndex {
###################################################
    my $index_file = $opts{infile}.".index";

    return if $index{'is_good'};

    open(INDEX, ">$index_file");
    for my $entry (sort keys %index) {
	print INDEX "$entry\t$index{$entry}\n";
    }
    close INDEX;

    $index{'is_good'} = 1;
}

###################################################
sub sortEntries {
###################################################
    my @tmp;

    for my $k (sort @sortme) {
	print "<!-- $k -->\n" if $opts{'DEBUG'};

	$k =~ s/.*_//g; # retrieve original index
	push @tmp, $prots_data[$k];
    }

    @prots_data = ($opts{'sortDir'} eq 'Descending') ? reverse @tmp : @tmp;
}


################################################### use css span tags to align content
sub showBoxPeps {
###################################################
    print &addTabbedPane(label => 'All Proteins') if $opts{'format'} eq 'HTML';

    print<<END;
<span style='z-index:9990; width:100%;position:fixed;font-weight:bold;font-size:10;background-color:white'>
<span style='width:50;display:inline-block'>#</span>
<span style='width:200;display:inline-block'>Main Entry Accession</span>
<span style='width:30;display:inline-block'>#prots</span>
<span style='width:40;display:inline-block'>Prob</span>
<span style='width:100;display:inline-block;text-align:right'>Peptide Sequences</span>
</span><br/>
END

    foreach my $pref (@prots_data) {
	my $class = '';  # style string

	my $extra = $$pref{inds} > 0 ? " +$$pref{inds}" : '&nbsp;';
	my $elink = $$pref{name} ? "<a href=\"javascript:getEntryInfo('$$pref{entry}','peps');\">$$pref{entry}</a>" : "<a href=\"javascript:getEntryInfo('$$pref{entry}','box');\">$$pref{entry}</a>";
	my $plink = $$pref{name} ? "<a href=\"javascript:protlink('$$pref{name}','$$pref{peps}');\">$$pref{name}</a>" : "<font color='#666666'>Group # $$pref{psdo}</font>";

	my $align = $$pref{name} ? 'right' : 'left';

	if (!$$pref{name}) {
	    $class = "style='background: #cccccc;'";	
	}
	elsif ($$pref{name} =~ /^$opts{'decoyText'}/) {
	    $class = "style='background: #eecccc;'";
	    $stats{'Number of known decoy entries:'}++;
	}
	else {
	    $stats{'Number of entries displayed:'}++;
	}

	print<<END;
 <span class="prothead" $class>
 <span class="entry" $class>$elink</span>
 <span class="protsm" $class>$plink</span>
 <span class="inds" $class>$extra</span>
 <span class="probsm" title='$$pref{prob}'>$$pref{prob}</span>
END

#	for my $seq (sort keys %pepseqs) {
	for my $seq (@pepseqs) {
#	    print "key: $seq len: $pepseqs{$seq}<br/>\n";
	    next if ($seq =~ /^seen_/);

	    my $seqlen  = length($seq);
	    my $maxprob = -1;
	    my $maxwght = 0;
	    my $is_evi  = 0;
	    my $is_unq  = 0;
	    my $maxntt  = -1;

	    if ($$pref{name}) {
		my $pepbox = '';
		for my $i (1..1000) {  # SBJ help us if we have more than this...
		    last unless $$pref{"pep${i}_seq"};

		    if ($$pref{"pep${i}_seq"} eq $seq) {
			$is_unq += ($$pref{"pep${i}_unq"} eq 'Y') ? 1 : 0;
			$is_evi += ($$pref{"pep${i}_evi"} eq 'Y') ? 1 : 0;
			$maxntt  = ($$pref{"pep${i}_ntt"} > $maxntt)  ? $$pref{"pep${i}_ntt"} : $maxntt;
			$maxprob = ($$pref{"pep${i}_prb"} > $maxprob) ? $$pref{"pep${i}_prb"} : $maxprob;
			$maxwght = ($$pref{"pep${i}_wgt"} > $maxwght) ? $$pref{"pep${i}_wgt"} : $maxwght;
#			last;
		    }
		}

		if ($maxprob >= 0) {
		    my $bwid = '';
		    my $bsty = '';
		    my $bcol = '';
		    if ($is_evi) {
			$bwid = 'border-width:1px; ';
			$bsty = 'border-style:solid; ';
			$bcol = 'border-color:#ee3333; ';
		    }
		    if ($is_unq) {
			$bwid = 'border-radius:6px; ';
		    }
		    $bsty = ($maxntt == 2) ? 'border-style:solid; ' : ($maxntt == 1) ? 'border-style:dashed; ' : 'border-style:dotted; ';

		    my $col = (&wobniar(-101,"#0064E0","#00FF00"))[int($maxprob*100)];
		    $pepbox = "<span class='histogram$seqlen' style='$bwid $bsty $bcol background-color:$col'></span>";
		}

		print "<span style='width:$seqlen;display:inline-block' title='$seq : maxP=$maxprob (w=$maxwght, ntt=$maxntt)'>$pepbox</span>\n";
	    }
	    else {
		my $npepcolor = $pepseqs{$seq} > 100 ? 100 : int($pepseqs{$seq});
		my $col = (&wobniar(-101,"#0064E0","#00FF00"))[$npepcolor];
		print "<span style='width:$seqlen;display:inline-block' title='$seq : $pepseqs{$seq} spectra'><span class='histogram$seqlen' style='background-color:$col'></span></span>\n";
	    }

	}

	print "</span>\n<br/>\n";
    }
    print &closeTabbedPane(selected=>1) if $opts{'format'} eq 'HTML';
}


###################################################
sub writeGaggleData {
###################################################
    my $div = 1;  # hard-coded for now; an option later on?
    my $gaggle;

    $gaggle = '<div name="gaggle_xml" class="hidden">' if $div;

    $gaggle .= '<gaggleData version="0.1">';
    $gaggle .= "\n<namelist type='direct' name='Protein Names' species='UNKNOWN'>\n";

    foreach my $pref (@prots_data) {
	$gaggle .= $$pref{name} . "\t";
    }

    $gaggle .= "\n</namelist>\n</gaggleData>\n";
    $gaggle .= "</div>\n" if $div;

    print "$gaggle";
}


###################################################
sub writeJSONFile {
###################################################
    my $jsonfile = $cgi_query->param('exportfile') || $opts{'infile'}; #default
    $jsonfile =~ s/xml$/json/i;

    my @json;

    push @json, "var data = [\n";

    foreach my $pref (@prots_data) {
	next unless $$pref{name};

	$$pref{nnds} =~ s/ /,/g;
	$$pref{nnds} =~ s/,$//;
	$$pref{nnds} = ','.$$pref{nnds} if length($$pref{nnds}) > 1;

	push @json, '{',
	'"proteins": "' . $$pref{name} . $$pref{nnds} . '", ',
	'"psm_id_share": ' . $$pref{pcts} . ', ',
	'"probability": ' . $$pref{prob} . ', ',
	'"length": ' . $$pref{plen} . ', ',
	'"coverage": ' . $$pref{pcov} ,
	"},\n";
    }

    push @json, "];\n";

    open(JSON, ">$jsonfile") || return 'Could not write file $jsonfile';
    for (@json) { print JSON $_; }
    close JSON;

    my $webjson = $jsonfile;
    $webjson =~ s/\\/\//g;
    my $webroot = $ENV{'WEBSERVER_ROOT'};
    $webroot =~ s/\\/\//g;
    $webjson =~ s/$webroot//i;
    my $link = "/ISB/html/ProteoGrapher/PG.html?file=$webjson&terms=terms.json";

    print "File $jsonfile written to disk -- Open in <a target='pgwin' href='$link'>ProteoGrapher</a>";
}


###################################################
sub writeTSVFile {
###################################################
    my $xlsfile = $cgi_query->param('exportfile') || $opts{'infile'}; #default
    $xlsfile =~ s/xml$/xls/i;
    # check name?

    my @tsv;

    push @tsv, join("\t",
		    "entry no.",
		    "protein",
		    "protein probability",
		    "protein description",
		    "percent coverage",
		    "tot indep spectra",
		    "percent share of spectrum ids",
		    "peptides"
		    ) . "\n";

    foreach my $pref (@prots_data) {
	push @tsv, join("\t",
			$$pref{entry},
			$$pref{name},
			$$pref{prob},
			$$pref{annt},
			$$pref{pcov},
			$$pref{npep},
			$$pref{pcts},
			$$pref{peps}
			) . "\n";

#	print TSV "$$pref{entry}\t";

#	my $group = $$pref{entry}; #???
#	$group =~ s/[a-z]//g;      #???
#	print TSV "$$pref{prob}\t"; # group...TODO

#	print TSV "$$pref{prob}\t";


    }

    open(TSV, ">$xlsfile") || return 'Could not write file $xlsfile';
    for (@tsv) { print TSV $_; }  # print TSV @tsv; #??
    close TSV;

    print "File $xlsfile written to disk";
}


###################################################
sub writeProtsFile {
###################################################
    my $tablabel = ($opts{'filtering'} ? 'Filtered ' : 'All ') . 'Proteins';
    print &addTabbedPane(label => $tablabel) if $opts{'format'} eq 'HTML';

    print<<END;
<span style='z-index:9990;display:inline-block;white-space:nowrap;width:100%;position:fixed;font-weight:bold;font-size:10;background-color:white'>
<span style='width:50;display:inline-block'>#</span>
<span style='width:500;display:inline-block'>Main Entry Accession</span>
<span style='width:30;display:inline-block'>NTT</span>
<span style='width:140;display:inline-block'>Probability</span>
<span style='width:130;display:inline-block;text-align:right'># Tot PSMs</span>
<span style='width:130;display:inline-block;text-align:right'>% Coverage / Weight</span>
<span style='width:130;display:inline-block;text-align:right'>% Spectrum ids / NSP</span>
END

    my $annot_w = 0;
    if ($opts{"has_xpress"}) {
	print "<span style='width:130;display:inline-block;text-align:right'>XPRESS</span>\n";
	$annot_w += 134;
    }

    if ($opts{"has_asapratio"}) {
	my $label = 'ASAPRatio' . ($opts{"use_asapratio_pvalue"} ? ' (adj)' : '');
	print "<span style='width:130;display:inline-block;text-align:right'>$label</span>\n";
	$annot_w += 134;
    }

    if ($opts{"has_asapratio_pvalue"}) {
	print "<span style='width:45;display:inline-block;text-align:right'>pvalue</span>\n";
	$annot_w += 49;
    }

    if ($opts{"has_libra"}) {
	print "<span style='width:130;display:inline-block;text-align:right'>Libra</span>\n";
	$annot_w += 134;
    }

    print "</span>\n<br/>\n";

    if ($annot_w) {
	print
	    "<style type='text/css'>.annot{width:",
	    (1104+$annot_w),
	    ";display:block;padding-left:30;border-top: 1px solid white;font-size:12px;font-weight:normal;color:#666666;background: #eeeeee;}",
	    "</style>";
    }

    my $prev = 0;

    foreach my $pref (@prots_data) {
	my $class = '';  # style string

	my $anndiv = $opts{'show_peps'} ? 'sann_' : 'fann_';

	my $extra = $$pref{inds} > 0 ? " <a href=\"javascript:getAnnotInfo('$$pref{entry}','$anndiv');\">+$$pref{inds}</a>" : '&nbsp;';
	my $annot = $$pref{annt} ? "&nbsp;&nbsp;&nbsp;<span class='annt'><a href=\"javascript:getAnnotInfo('$$pref{entry}','$anndiv');\">$$pref{annt}</a></span>" : '';
	my $elink = $$pref{name} ? "<a href=\"javascript:getEntryInfo('$$pref{entry}','peps');\">$$pref{entry}</a>" : "<a href=\"javascript:getEntryInfo('$$pref{entry}','box');\">$$pref{entry}</a>";
	my $plink = $$pref{name} ? "<a href=\"javascript:protlink('$$pref{name}','$$pref{peps}');\">$$pref{name}</a>$annot" : "<font color='#666666'>Group # $$pref{psdo}</font>";

	my $align = $$pref{name} ? 'right' : 'left';

	if (!$$pref{name}) {
	    $class = "style='background: #bbbbbb;'";	
	}
	elsif ($$pref{name} =~ /^$opts{'decoyText'}/) {
	    $class = "style='background: #eecccc;'";
	    $stats{'Number of known decoy entries:'}++;
	}
	elsif ($prev eq $$pref{group}) {
	    $class = "style='background: #dddddd;'";
	    $stats{'Number of entries displayed:'}++;
	}
	else {
	    $stats{'Number of entries displayed:'}++;
	}

	if ($$pref{npep} == 1) {
	    $stats{'Number of single hit entries:'}++;
	}
	my $nseq = scalar split '\+', $$pref{peps};
	if ( ($nseq == 1) && ($$pref{npep} >0) ) {
	    $stats{'Number of single sequence entries:'}++;
	}

	my $probcolor = int($$pref{prob}*100);
	my $pcovcolor = int($$pref{pcov});
	my $pctscolor = $opts{max_pcts} ? int(100*$$pref{pcts}/$opts{max_pcts}) : 0;
	my $npepcolor = $$pref{npep} > 100 ? 100 : int($$pref{npep});
	my $npepdisp  = $$pref{npep} > 100 ? "<i>$$pref{npep}</i>" : $$pref{npep};

	print<<END;
<span class="prothead" $class id="e_$$pref{entry}">
 <span class="entry">$elink</span>
 <span class="prot">$plink</span>
 <span class="inds">$extra</span>
 <span class="prob" title='$$pref{prob}'>$$pref{prob} <span class="histogram$probcolor"></span></span>
END

	if ($$pref{name}) {
	    print<<END;
 <span class="nbar" title='$$pref{npep} ($nseq unique sequences)'>$npepdisp <span class="histogram$npepcolor"></span></span>
 <span class="nbar" title='$$pref{pcov}%'>$$pref{pcov}% <span class="histogram$pcovcolor"></span></span>
 <span class="nbar" title='$$pref{pcts}%'>$$pref{pcts}% <span class="histogram$pctscolor"></span></span>
END
 }
	else {
	    print<<END;
<span class="nbar"></span>
<span class="nbar"></span>
<span class="nbar"></span>
END
	}

	if ($opts{"has_xpress"}) {
	    print "<span class='nbar'>\n";

	    if ($$pref{"xhlr"}) {
		print "<a href=\"javascript:xplink('$$pref{name}','$$pref{peps}');\">\n";

		my $hl = $$pref{"xhlr"} / (1 + $$pref{"xhlr"});
		my $lh = 1 - $hl;

		$hl = int($hl*100);
		$lh = int($lh*100);

		print "<span title='$$pref{xhlr}+/-$$pref{xerr} ($$pref{xnum})' class='histogram$hl'></span>\n";
		print "<span title='$$pref{xhlr}+/-$$pref{xerr} ($$pref{xnum})' class='histogram$lh'></span>\n";
		print "</a>";

	    } elsif ($$pref{name}) {
		print "<span title='not calculated' class='na'>-- n/a --</span>\n";

	    }

	    print "</span>\n";
	}

	if ($opts{"has_asapratio"}) {
	    print "<span class='nbar'>\n";

	    if ($$pref{"ahlr"}) {
		print "<a href=\"javascript:asaplink('$$pref{name}','$$pref{entry}');\">\n";

		my $aheavy = $opts{"use_asapratio_pvalue"} ? $$pref{"aahl"} : $$pref{"ahlr"};
		my $aerror = $opts{"use_asapratio_pvalue"} ? $$pref{"aaer"} : $$pref{"aerr"};

		my $hl = $aheavy / (1 + $aheavy);
		my $lh = 1 - $hl;

		$hl = int($hl*100);
		$lh = int($lh*100);

		print "<span title='$aheavy+/-$aerror ($$pref{anum})' class='histogram$hl'></span>\n";
		print "<span title='$aheavy+/-$aerror ($$pref{anum})' class='histogram$lh'></span>\n";
		print "</a>";

	    } elsif ($$pref{"anum"}) {
		print "<a href=\"javascript:asaplink('$$pref{name}','$$pref{entry}');\">\n";
		print "<span title='N/A ($$pref{anum})' class='histogram' style='width:50;background-color:white'></span>\n";
		print "<span title='N/A ($$pref{anum})' class='histogram' style='width:50;background-color:white'></span>\n";
		print "</a>";

	    } elsif ($$pref{name}) {
		print "<span title='not calculated' class='na'>-- n/a --</span>\n";

	    }

	    print "</span>\n";
	}

	if ($opts{"has_asapratio_pvalue"}) {
	    print
		"<span style='width:45;display:inline-block' class='small'>",
		$$pref{apvl} ? $$pref{apvl} : $$pref{name} ? 'n/a' : '',
		"</span>\n";
	}

	if ($opts{"has_libra"}) {
	    my $norm;
	    my $had_ratios = 0;

	    print "<span class='nbar'>\n";

	    for my $ch (@libra_channels) {
		$norm += $$pref{"r$ch"} if $$pref{"r$ch"};
	    }

	    for my $ch (@libra_channels) {
		if ($$pref{"r$ch"}) {
		    $had_ratios++;

		    if ($$pref{"r$ch"} >= 0) {
			my $intratio = int(0.5+$$pref{"r$ch"}*100/$norm);
			print "<span title='ch $ch : $intratio% (".$$pref{"r$ch"}."+/-".$$pref{"e$ch"}.")' class='histogram$intratio'></span>\n";
		    }
		    else {
			my $w = 100 / ($#libra_channels + 1);
			print "<span title='ch $ch: not calculated' class='histogram' style='width:$w;background-color:white'></span>\n";
		    }
		}

	    }
	    print "<span title='not calculated' class='na'>-- n/a --</span>\n" if ($$pref{name} && !$had_ratios);

	    print "</span>\n";
	}

	print "</span><br/>\n";

	# full annotation pane
	print "<div id='$anndiv$$pref{entry}' class='hidden'></div>\n";

	print &displayPeptides($pref) if $opts{'show_peps'};

	$prev = $$pref{group};

    }

    if ($opts{'format'} eq 'HTML') {
	print &closeTabbedPane(selected=>1);

	print &addTabbedPane(label => 'Selected Entries');  #placeholder
	print "<div id='selectedprots'><br/>Details of selected proteins will display here.</div>\n";
	print &closeTabbedPane();
    }

}

###################################################
sub writeAnnots {
###################################################
    my $num = $group_data{"current_prot"};
    my $html = '';

    for my $i (1..$num) {
	my $pname = $group_data{"prot_name_$i"};
	my $pannt = $group_data{"prot_annt_$i"};
	my $ppeps = $group_data{"prot_peps_$i"};

	my $plink = "<a href=\"javascript:protlink('$pname','$ppeps');\">$pname</a>"; # : "<font color='#666666'>Group # $$pref{psdo}</font>";

	my $entry = $group_data{'is_group'} ? $group_data{'grpnum'}.$group_data{"prot_gpid_$i"} : $group_data{'grpnum'};

	$html .= "<span class='annot'>&gt;<b>$plink</b> $pannt</span>\n" if ($cgi_query->param('entry') eq "$entry");
    }

    print $html;
}

###################################################
sub writeStats {
###################################################
    print &addTabbedPane(label => 'File Info');

    print
	start_form('GET',url(-relative=>1)),
	br,
	"<table>\n",
	"<tr>",
	"<td style='text-align:right'>Filepath: </td>",
	"<td class='tgray'>",
	textfield(-name=>'file',
		  -value=>$opts{'infile'},
		  -style=>'background-color:#dddddd',
		  -size=>110,
		  -maxlength=>300),
	submit(-name=>'DispAction',
	       -value=>'Go!')."\n",
	"</td>",
	"</tr>\n";

    for my $key (keys %stats) {
	print
	    "<tr>",
	    "<td style='text-align:right'>$key </td>",
	    "<td class='tgray'>$stats{$key}</td>",
	    "</tr>\n";
    }

    print
	"</table>\n",
	endform,
	br,br;


    if ($stats{'Number of entries displayed:'}) {
	my $exportfile = $opts{'infile'};
	$exportfile =~ s/xml$/xls/i;

	print "<span class='tgray' style='width:100%;'> EXPORT</span>\n<br/>\n\n";

	print
	    "<span style='position:relative;left:50px;'>\n",
	    start_form(-method=>'GET',
		       -action=>url(-query=>1),
		       -onsubmit=>'export_prots(this); return false;',
		       -style =>'display: inline;'),
	    textfield(-name=>'exportfile',
		      -value=>$exportfile,
		      -style=>'background-color:#dddddd',
		      -size=>110,
		      -maxlength=>300),
	    hidden(-name=>'of',
		   -value=>'tsv'),
	    hidden(-name=>'export',
		   -value=>'ExportExcel'),
	    submit(-name=>'ExportExcel',
		   -value=>'Export to XLS'),
	    endform,
	    br,
	    "<span class='prob' style='width:100%' id='tsvexportresult'><br/></span>\n",
	    "</span>\n",
	    br;

	$exportfile =~ s/xls$/json/i;

	print
	    "<span style='position:relative;left:50px;'>\n",
	    start_form(-method=>'GET',
		       -action=>url(-query=>1),
		       -onsubmit=>'export_prots(this); return false;',
		       -style =>'display: inline;'),
	    textfield(-name=>'exportfile',
		      -value=>$exportfile,
		      -style=>'background-color:#dddddd',
		      -size=>110,
		      -maxlength=>300),
	    hidden(-name=>'of',
		   -value=>'json'),
	    hidden(-name=>'export',
		   -value=>'ExportJSON'),
	    submit(-name=>'ExportJSON',
		   -value=>'Export JSON'),
	    endform,
	    br,
	    "<span class='prob' style='width:100%' id='jsonexportresult'><br/></span>\n",
	    "</span>\n",
	    br;
    }

    print &closeTabbedPane();
}

###################################################
sub displayPeptides {
###################################################
    my $prtref = shift;

    my $hiseq = $opts{'filter_Peptide_Sequence'} if $opts{'action'} eq 'displayPeptide';

    my $pep_num = 0;

    my $html;
    my %allpeps;

    for my $i (1..1000) {  # SBJ help us if we have more than this...
	last unless $$prtref{"pep${i}_seq"};
	$allpeps{$$prtref{"pep${i}_seq"}}++;
    }

### sort peptides here!

    for (sort keys %allpeps) {
	for my $i (1..1000) {  # SBJ help us if we have more than this...
	    my $pep = "pep$i";

	    my $seq = "${pep}_seq";
	    next unless $$prtref{$seq} eq $_;

	    my $mod = "${pep}_mod";
	    my $wgt = "${pep}_wgt";
	    my $ntt = "${pep}_ntt"; ## ntt
	    my $nsp = "${pep}_nsp";
	    my $bin = "${pep}_bin";
	    my $ini = "${pep}_ini"; ## pre-nsp prob
	    my $prb = "${pep}_prb";
	    my $chg = "${pep}_chg";
	    my $evi = "${pep}_evi";
	    my $unq = "${pep}_unq";
	    my $num = "${pep}_num";

	    my $disppep = $$prtref{$mod} ? $$prtref{$mod} : $$prtref{$seq};
	    my $pepspan = "$$prtref{$chg}$disppep";

	    if ($hiseq && $$prtref{$seq} =~ /$hiseq/) {
		$html .= "<span class='pephead' style='background:#ddee66'>\n";  # c9dd03 ddddff
	    }
	    else {
		$html .= "<span class='pephead' name='$pepspan' onmouseover='highlight(\"$pepspan\",\"yes\")' onmouseout='highlight(\"$pepspan\",\"no\")'>\n";
	    }

	    $disppep =~ s|\[|<sub>|g;
	    $disppep =~ s|\]|</sub>|g;

	    my $chgtxt = $$prtref{$chg} ? "+$$prtref{$chg}" : '';

	    $html .= "<span class='pepchrg'>$chgtxt&nbsp;&nbsp;&nbsp;</span>\n";

	    $html .= "<a href=\"javascript:peplink('$$prtref{$seq}', '$$prtref{$chg}');\">";
	    $html .= "<span class='pepseq'>$disppep</span></a>\n";

	    my $ntt1 = ($$prtref{$ntt} == 0) ? '#ffffff' : '#bbbbbb';
	    my $ntt2 = ($$prtref{$ntt} == 2) ? '#bbbbbb' : '#ffffff';
	    $html .= "<span style='width:20;display:inline-block'>";
	    $html .= "<span title='ntt=$$prtref{$ntt}' class='histogram' style='width:6;background-color:$ntt2;border-right:0px'></span>";
	    $html .= "<span title='ntt=$$prtref{$ntt}' class='histogram' style='width:6;background-color:$ntt1'></span>";
	    $html .= "</span>\n";


	    my $probcolor = int($$prtref{$prb}*100);
	    my $iniprob   = int($$prtref{$ini}*100);
	    my $probstyle = ($$prtref{$evi} eq 'Y') ? '' : 'style="color:333333";';
	    my $probstyle2= ($$prtref{$evi} eq 'Y') ? 'style="position:relative;"' : 'style="position:relative;background-color:dddddd;"';
#	    $html .= "<span class='prob' $probstyle title='Initial prob = $$prtref{$ini}'>$$prtref{$prb} <span class='histogram$probcolor' $probstyle2></span></span>\n";

	    $html .= "<span class='prob' $probstyle title='Initial prob = $$prtref{$ini}'>$$prtref{$prb} ";
	    $html .= "<span class='histogram$probcolor' $probstyle2>";
	    $html .= "<span style='height:8;display:inline-block;position:absolute;top:-1px;border-right:1px solid black;width:$iniprob'></span>";
	    $html .= "</span>";
	    $html .= "</span>\n";


	    my $npepcolor = $$prtref{$num} > 100 ? 100 : int($$prtref{$num});
	    $html .= "<span class='nbar' title='$$prtref{$num}'>$$prtref{$num} <span class='histogram$npepcolor'></span></span>\n";

	    my $weightcolor = int($$prtref{$wgt}*100);
	    my $weightstyle = ($$prtref{$unq} eq 'Y') ? 'style="color:red"' : '';
	    $html .= "<a href=\"javascript:entriesByPeptide('$$prtref{$seq}');\">";
	    $html .= "<span class='nbar' $weightstyle title='$$prtref{$wgt}'>$$prtref{$wgt} <span class='histogram$weightcolor'></span></span></a>\n";

	    my $nspcolor = $$prtref{$nsp} > 100 ? 100 : int($$prtref{$nsp});
	    $html .= "<a href=\"javascript:nsplink('$$prtref{$seq}', '$$prtref{$chg}', '$$prtref{$bin}', '$$prtref{$nsp}', '$$prtref{name}', '$$prtref{inds}');\">";
	    $html .= "<span class='nbar' title='NSP=$$prtref{$nsp}'>$$prtref{$nsp} <span class='histogram$nspcolor'></span></span></a>\n";

	    $html .= "</span><br/>\n\n";

	    $pep_num += $group_data{$num};  ## ??
	}
    }

    return $html;
}

###################################################
sub writeProtFile {
###################################################
    my $group = $group_data{'grpnum'};

    if ($group_data{'is_group'} eq 'T') {
	my $grpref = {
	    group=> $group,
	    entry=> $group,
	    psdo => $group_data{pseudo},
	    prob => $group_data{prob},
	    name => '',
	    inds => '',
	    pcov => '',
	    plen => '',
	    npep => ''
	    };
	push @prots_data, $grpref unless ($opts{'filtering'} || $opts{'sorting'});
    }

    # deal with sub-groups
    for my $aa ('','a'..'z') {
	for my $a ('a'..'z') {
	    my $cur = "prot_$a$aa";

	    my $nom = "${cur}_name";
	    my $ann = "${cur}_annt";
	    my $all = "${cur}_alla";
	    my $ind = "${cur}_inds";
	    my $prb = "${cur}_prob";
	    my $pep = "${cur}_peps";
	    my $num = "${cur}_npep";
	    my $pct = "${cur}_pcts";
	    my $cov = "${cur}_cvrg";
	    my $len = "${cur}_plen" || 1;
	    # quant
	    my $xhl = "${cur}_xpress_HL_ratio";
	    my $xer = "${cur}_xpress_HL_stdev";
	    my $xnp = "${cur}_xpress_HL_npeps";
	    my $ahl = "${cur}_asap_HL_ratio";
	    my $aer = "${cur}_asap_HL_stdev";
	    my $anp = "${cur}_asap_HL_npeps";
	    my $aah = "${cur}_asap_HL_adj_ratio";
	    my $aae = "${cur}_asap_HL_adj_stdev";
	    my $apv = "${cur}_asap_pvalue";
	    my %lbr;
	    my %lbe;
	    for my $ch (@libra_channels) {
		$lbr{$ch} =  "${cur}_${ch}_ratio";
		$lbe{$ch} =  "${cur}_${ch}_error";
	    }

	    my $grp = $group;

	    last unless $group_data{$nom};

	    $grp .= $a.$aa if $group_data{'is_group'} eq 'T';

	    my $ref = "Ref=$group_data{$nom}";
	    my $i = 0;

	    for my $p (split ' ', $group_data{$ind}) {
		$ref .= "&Ref=$p";
		$i++;
	    }

	    next if ($opts{'oneHits'} && ($group_data{$num} != 1));  # single-hit only?

	    next if (
		     ($group_data{$prb} < $opts{'minProb'}) || ($group_data{$prb} > $opts{'maxProb'}) ||
		     ($group_data{$cov} < $opts{'minPCov'}) || ($group_data{$cov} > $opts{'maxPCov'}) ||
		     ($group_data{$pct} < $opts{'minPSpc'}) || ($group_data{$pct} > $opts{'maxPSpc'}) ||
		     ($group_data{$num} < $opts{'minNPep'}) || ($group_data{$num} > $opts{'maxNPep'}) ||
		     ($group_data{$xhl} < $opts{'minXHLr'}) || ($group_data{$xhl} > $opts{'maxXHLr'}) ||
		     ($group_data{$ahl} < $opts{'minAHLr'}) || ($group_data{$ahl} > $opts{'maxAHLr'}) ||
		     ($group_data{$apv} < $opts{'minPval'}) || ($group_data{$apv} > $opts{'maxPval'}) ||
		     (!$group_data{$xhl} && $opts{'xclXHLr'}) || (!$group_data{$ahl} && $opts{'xclAHLr'})
		     );

	    my $skip = 0;
	    foreach my $field (@fields_string) {
		my $opt = $opts{"filter_$field"} ? $opts{"filter_$field"} : $opts{"not_filter_$field"} ? $opts{"not_filter_$field"} : '';
		next unless $opt;

		my @comp = ($field eq 'Protein_Name') ? (split('\s',"$group_data{$nom} $group_data{$ind}")) : ($field eq 'Annotation') ? (split('~~~~', $group_data{$all})) : ($field eq 'Peptide_Sequence') ? (split('\+', $group_data{$pep})) : ('unk'); 

		my $not_passed = 1;
		for my $comp (@comp) {
		    $not_passed *= (1 - ($comp =~ /$opt/i));
		}
		$skip += $opts{"filter_$field"} ? $not_passed : (1 - $not_passed);

	    }
	    next if $skip;

	    my $protref = {
		group=> $group,
		entry=> $grp,
		prob => $group_data{$prb},
		name => $group_data{$nom},
		annt => $group_data{$ann},
		inds => $i,
		nnds => $group_data{$ind},
		pcov => $group_data{$cov},
		plen => $group_data{$len},
		npep => $group_data{$num},
		peps => $group_data{$pep},
		pcts => $group_data{$pct},
		xhlr => $group_data{$xhl},
		xerr => $group_data{$xer},
		xnum => $group_data{$xnp},
		ahlr => $group_data{$ahl},
		aerr => $group_data{$aer},
		anum => $group_data{$anp},
		aahl => $group_data{$aah},
		aaer => $group_data{$aae},
		apvl => $group_data{$apv}
	    };
	    for my $ch (@libra_channels) {
		$ {$protref} {"r$ch"} = $group_data{$lbr{$ch}};
		$ {$protref} {"e$ch"} = $group_data{$lbe{$ch}};
		$ {$protref} {"libratot"} += $group_data{$lbr{$ch}} if ($group_data{$lbr{$ch}}>=0);
	    }

	    for my $i (1..1000) {  # SBJ help us if we have more than this...
		my $pep = "${cur}_pep$i";

		my $seq = "${pep}_seq";
		my $mod = "${pep}_mod";
		my $wgt = "${pep}_wgt";
		my $ntt = "${pep}_ntt";
		my $nsp = "${pep}_nsp";
		my $bin = "${pep}_bin";
		my $ini = "${pep}_ini";
		my $prb = "${pep}_prb";
		my $chg = "${pep}_chg";
		my $evi = "${pep}_evi";
		my $unq = "${pep}_unq";
		my $num = "${pep}_num";

		last unless $group_data{$seq};

		my $pepkey = $group_data{$mod} ? $group_data{$mod} : $group_data{$seq};

		push @pepseqs, $group_data{$seq} unless $pepseqs{$group_data{$seq}};

		$pepseqs{$group_data{$seq}} = 0 unless $pepseqs{$group_data{$seq}};
		$pepseqs{$group_data{$seq}} += $group_data{$num} unless $pepseqs{"seen_$group_data{$chg}_$pepkey"};

		$pepseqs{"seen_$group_data{$chg}_$pepkey"}++;

		$ {$protref} {"pep${i}_seq"} = $group_data{$seq};
		$ {$protref} {"pep${i}_mod"} = $group_data{$mod};
		$ {$protref} {"pep${i}_wgt"} = $group_data{$wgt};
		$ {$protref} {"pep${i}_ntt"} = $group_data{$ntt};
		$ {$protref} {"pep${i}_nsp"} = $group_data{$nsp};
		$ {$protref} {"pep${i}_bin"} = $group_data{$bin};
		$ {$protref} {"pep${i}_ini"} = $group_data{$ini};
		$ {$protref} {"pep${i}_prb"} = $group_data{$prb};
		$ {$protref} {"pep${i}_chg"} = $group_data{$chg};
		$ {$protref} {"pep${i}_evi"} = $group_data{$evi};
		$ {$protref} {"pep${i}_unq"} = $group_data{$unq};
		$ {$protref} {"pep${i}_num"} = $group_data{$num};
	    }

	    push @prots_data, $protref;

	    if ($opts{'sorting'}) {
		my $sort_val = 
		    ($opts{'sortBy'} eq 'Probability')      ? sprintf("%.4f", ${$protref}{'prob'}) :
		    ($opts{'sortBy'} eq 'Coverage')         ? sprintf("%05d", 100*${$protref}{'pcov'}) :
		    ($opts{'sortBy'} eq 'Pct_Spectrum_ids') ? sprintf("%05d", 100*${$protref}{'pcts'}) :
		    ($opts{'sortBy'} eq 'Num_Peptides')     ? sprintf("%05d", ${$protref}{'npep'}) :
		    ($opts{'sortBy'} eq 'Xpress_Heavy')     ? sprintf("%06d", 100*${$protref}{'xhlr'}) :
		    ($opts{'sortBy'} eq 'ASAP_Heavy')       ? sprintf("%06d", 100*${$protref}{'ahlr'}) :
		    ($opts{'sortBy'} eq 'ASAP_pvalue')      ? sprintf("%.9f", ${$protref}{'apvl'}) :
		    ${$protref}{'entry'};

		if ($opts{'sortBy'} =~ /Libra_(\d*)/) { $sort_val = ${$protref}{"libratot"} ? sprintf("%05d", 10000*${$protref}{"r$1"}/${$protref}{"libratot"}) : '00000'; }

		# secondary sort
		$sort_val .= $opts{'sortBy'} eq 'Probability' ? sprintf("%05d", ${$protref}{'npep'}) : sprintf("%.4f", ${$protref}{'prob'});

		push @sortme, "${sort_val}_$#prots_data";

	    }

	}
    }

}

###################################################
sub processProtXML {
###################################################

    #### Set up the XML parser and parse the returned XML
    my $parser = XML::Parser->new(
				  Handlers => {
				      Start => \&start_protxml_element,
				      End   => \&end_protxml_element,
#				      Char  => \&protxml_chars,
				  },
				  ErrorContext => 2 );
    eval { $parser->parsefile( $opts{'infile'} ); };
    die "ERROR_PARSING_XML:$@" if($@);

    my $quant = '';
    $quant .= 'L' if $opts{"has_libra"};
    $quant .= 'X' if $opts{"has_xpress"};
    $quant .= 'A' if $opts{"has_asapratio"};
    $quant .= 'P' if $opts{"use_asapratio_pvalue"};
    $quant .= 'p' if $opts{"has_asapratio_pvalue"};
    print <<"    END_JS" if ($opts{'format'} =~ /html/i);
    <SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
    <!--
    quant = '$quant';
    // -->
    </SCRIPT>
    END_JS


}

###################################################
sub processProtXMLEntry {
###################################################
    my $entry = shift || 1;
    my $data = shift || 'prot';

    $entry =~ s/[a-z]//g;

    my $byte_offset = $index{$entry};
    my $record_size = ($index{$entry+1} - 1 - $byte_offset);
    my $buffer;

    print "\n\n\n<!--\nbyte_offset::$byte_offset\nrecord_size::$record_size\n-->\n\n" if $opts{'DEBUG'};

    open(PROT, $opts{'infile'});
    binmode PROT;
    seek PROT, $byte_offset, 0;
    read PROT, $buffer, $record_size;
    close PROT;

    print "<!--\n$buffer\n-->\n\n" if $opts{'DEBUG'};
	
    $opts{"max_pcts"} = 0;
    $opts{"max_entry"} = 0;

    #### Set up the XML parser and parse the returned XML
    my $parser;
    if ($data eq 'annot') {
	$parser = XML::Parser->new(
				   Handlers => {
				       Start => \&start_protxml_annot_element,
				       End   => \&end_protxml_annot_element,
				   },
				   ErrorContext => 2 );
    }

    else {
	$parser = XML::Parser->new(
				   Handlers => {
				       Start => \&start_protxml_element,
				       End   => \&end_protxml_element,
#				      Char  => \&protxml_chars,
				   },
				   ErrorContext => 2 );
    }

    eval { $parser->parse( $buffer ); };
    die "ERROR_PARSING_XML:$@" if($@);

}

###################################################
sub start_protxml_annot_element {
###################################################
    my ($handler, $element, %atts) = @_;

    if ($element eq 'protein') {
	my $num = ++$group_data{"current_prot"};

	$group_data{"prot_name_$num"} = $atts{'protein_name'};
	$group_data{"prot_peps_$num"} = $atts{'unique_stripped_peptides'};
	$group_data{"prot_nind"}      = $atts{'n_indistinguishable_proteins'};
	$group_data{"prot_gpid_$num"} = $atts{'group_sibling_id'};
	$group_data{"prot_annt_$num"} = '';
    }

    elsif ($element eq 'annotation') {
	my $num = $group_data{"current_prot"};
	$group_data{"prot_annt_$num"} = "$atts{'protein_description'}";
    }

    elsif ($element eq 'indistinguishable_protein') {
	my $num = ++$group_data{"current_prot"};
	my $prev = $num-1;

	$group_data{"prot_name_$num"} = $atts{'protein_name'};
	$group_data{"prot_gpid_$num"} = $group_data{"prot_gpid_$prev"};
	$group_data{"prot_peps_$num"} = $group_data{"prot_peps_$prev"};
	$group_data{"prot_annt_$num"} = '';
    }

    elsif ($element eq 'protein_group') {
	$group_data{'grpnum'}   = $atts{'group_number'};
	$group_data{'is_group'} = $atts{'pseudo_name'} ? 1:0;
    }

}

###################################################
sub end_protxml_annot_element {
###################################################
    my ($handler, $element) = @_;

    if ($element eq 'protein_group') {
	&writeAnnots();
	undef %group_data;
    }

}


###################################################
sub start_protxml_element {
###################################################
    my ($handler, $element, %atts) = @_;

    unless ($opts{'show_peps'}) {
	if ( ($element eq 'peptide') ||
	     ($element eq 'modification_info') ||
	     ($element eq 'mod_aminoacid_mass') ) {
	    return;
	    #ignore
	}
    }

    if ( ($element eq 'program_details') ||
	 ($element eq 'nsp_information') ||
	 ($element eq 'proteinprophet_details') ||
	 ($element eq 'nsp_distribution') ||
	 ($element eq 'protein_summary_data_filter') ||
	 ($element eq 'peptide_parent_protein') ||
	 ($element eq 'libra_result') ||
	 ($element eq 'analysis_result') ) {
	#ignore
	return;
    }

    elsif ($element eq 'protein_group') {
	$group_data{'grpnum'} = $atts{'group_number'};
	$group_data{'prob'} = $atts{'probability'};
	$group_data{'is_group'} = $atts{'pseudo_name'} ? 'T' : 'F';
	$group_data{'pseudo'} = $atts{'pseudo_name'} ? $atts{'pseudo_name'} : '';

	$index{$atts{'group_number'}} = $handler->current_byte;
	$opts{"max_entry"} = $atts{'group_number'};
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
	$group_data{"${prot}_pcts"} = $atts{'pct_spectrum_ids'} || 0;;
	$group_data{"${prot}_nind"} = $atts{'n_indistinguishable_proteins'};
	$group_data{"${prot}_inds"} = '';
	$group_data{"${prot}_annt"} = '';
	$group_data{"${prot}_alla"} = '';

	$opts{"max_pcts"} = ($atts{'pct_spectrum_ids'} > $opts{"max_pcts"}) ? $atts{'pct_spectrum_ids'} : $opts{"max_pcts"};
    }

    elsif ($element eq 'annotation') {
	my $prot = $group_data{"current_prot"};

	if ($group_data{"${prot}_nind"} == 1) {
	    $group_data{"${prot}_annt"} = "$atts{'protein_description'}";
	}
	$group_data{"${prot}_alla"} .= "~~~~$atts{'protein_description'}";
    }

    elsif ($element eq 'parameter') {
	my $prot = $group_data{"current_prot"};
	if ($atts{'name'} eq 'prot_length') {
	    $group_data{"${prot}_plen"} = $atts{'value'};
	}
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
	$group_data{"${pep}_bin"} = $atts{'n_sibling_peptides_bin'};
	$group_data{"${pep}_ini"} = $atts{'initial_probability'};
	$group_data{"${pep}_prb"} = $atts{'nsp_adjusted_probability'};
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

    elsif ($element eq 'ASAPRatio') {
	my $prot = $group_data{"current_prot"};

	$group_data{"${prot}_asap_HL_ratio"} = ($atts{'heavy2light_ratio_mean'} >= 0.0) ? $atts{'heavy2light_ratio_mean'} : ''; # give some other N/A value?
	$group_data{"${prot}_asap_HL_stdev"} = $atts{'heavy2light_ratio_standard_dev'};
	$group_data{"${prot}_asap_HL_npeps"} = $atts{'ratio_number_peptides'};
    }

    elsif ($element eq 'ASAPRatio_pvalue') {
	my $prot = $group_data{"current_prot"};

	$group_data{"${prot}_asap_HL_adj_ratio"} = ($atts{'heavy2light_adj_ratio_mean'} >= 0.0) ? $atts{'heavy2light_adj_ratio_mean'} : ''; # give some other N/A value?
	$group_data{"${prot}_asap_HL_adj_stdev"} = $atts{'heavy2light_adj_ratio_standard_dev'};
	$group_data{"${prot}_asap_pvalue"}       = $atts{'decimal_pvalue'};
    }

    elsif ($element eq 'XPressRatio') {
	my $prot = $group_data{"current_prot"};

	$group_data{"${prot}_xpress_HL_ratio"} = $atts{'heavy2light_ratio_mean'};
	$group_data{"${prot}_xpress_HL_stdev"} = $atts{'heavy2light_ratio_standard_dev'};
	$group_data{"${prot}_xpress_HL_npeps"} = $atts{'ratio_number_peptides'};
    }

    elsif ($element eq 'intensity') {  # TODO: check that parent tag is libra_result
	my $prot = $group_data{"current_prot"};
	my $lbch = "${prot}_".int(0.5+$atts{'mz'});

	$group_data{"${lbch}_ratio"} = $atts{'ratio'};
	$group_data{"${lbch}_error"} = $atts{'error'};

	push @libra_channels, int(0.5+$atts{'mz'}) unless $opts{"libra_channels_complete"};
    }

    elsif ($element eq 'fragment_masses') {  # TODO: check that parent tag is libra_summary
	push @libra_channels, $atts{'mz'};

    }

    elsif ($element eq 'protein_summary_header') {
	my $db =  $atts{'reference_database'} || 'unknown';
	my $sf =  $atts{'source_files'} || 'unknown';

	print <<"        END_JS" if ($opts{'format'} =~ /html/i);
	<SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
	<!--
	var database = '$db';
	var pepxml_source_files = '$sf';
	// -->
	</SCRIPT>
        END_JS

    }

    elsif ($element eq 'ASAP_prot_analysis_summary') {
	my $rt =  $atts{'reference_isotope'} ? (($atts{'reference_isotope'} eq 'heavy') ? '0' : '1') : '1';

	print <<"        END_JS" if ($opts{'format'} =~ /html/i);
	<SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
	<!--
	var asap_ratio_type = '$rt';
	// -->
	</SCRIPT>
        END_JS

    }

    elsif ($element eq 'analysis_summary') {
	my $anal = $atts{'analysis'} || 'UNK';
	$opts{"has_$anal"} = 1;
    }

    else {
	if ($opts{'DEBUG'}) {
	    print "Did not handle tag <$element>\n";
	}
    }

}

###################################################
sub end_protxml_element {
###################################################
    my ($handler, $element) = @_;

    if ($element eq 'protein_group') {
	if ($opts{'DEBUG'}) {
	    print " GROUP: $group_data{'grpnum'} :: $group_data{'prob'} :: $group_data{'is_group'}\n";
	    for my $key (sort keys(%group_data)) {
		print "......$key == $group_data{$key}\n";
	    }
	}

	&writeProtFile();

	undef %group_data;
    }

    elsif ( ($element eq 'libra_summary') || ($element eq 'protein') ) {
	$opts{"libra_channels_complete"}++;
    }

    elsif ($element eq 'protein_summary') {
	$index{$opts{"max_entry"}+1} = $handler->current_byte;
    }

}

###################################################
sub closeHTML {
###################################################
    print "<br><br><hr size='1' noshade><font color='#999999'>ProtXML Viewer<br>$TPPVersionInfo</font><br><br><br>\n</body>\n</html>";
}

###################################################
sub openHTML {
###################################################
    print "Content-type:text/html\n\n";
    return unless $opts{'format'} eq 'HTML';

    my $css = &printCSS();

    my $filename = $opts{'infile'} ? basename($opts{'infile'}) : 'NO_FILE';

    print<<END;
<html>
<head>
<title>ProtXML Viewer :: $opts{'infile'}</title>
$css
</head>
<body>

<div class="banner">ProteinProphet results for <u>$filename</u>
<br/><br/>
END

    # create empty tabs
    my $pad = '&nbsp;';
    my $table_tag = "<table style='background:#eeeeee' frame='border' rules='all' cellpadding='2'>\n";

    for my $i (0..9) {
	print "<span>$pad</span>\n<span id='navtab$i'></span>\n";
    }

    my $models_file = $opts{'infile'};
    $models_file =~ s/.prot.xml$/.prot-MODELS.html/;

    `tpp_models.pl $opts{'infile'}` if (!-e $models_file);  # attempt, but disregard any errors

    if (-e $models_file) {
	$models_file =~ s/$www_root/\//;
	print<<"END";
	   <span>$pad$pad$pad$pad$pad</span>\n<span class='navtab'><a target='tppmodels' href='$models_file'>Models &gt;</a></span>

	   <SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
	   <!--
	   modelsfile = '$models_file';
	// -->
	   </SCRIPT>
END
    }


    print "</div>\n";
#    print "<div class='opaque'></div>\n";
    print "<div class='blank'>---</div>\n\n";

}

###################################################
sub reportError {
###################################################
    my $errstring = shift;
    my $fatal = shift || 0;

    if (!$opts{'errors'}) {
	print &addTabbedPane(label => 'ERRORS');
	print "<br/><br/><div id='msgs' class='hidden'></div>\n\n";
    }

    print "<SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n";
    print "document.getElementById(\"msgs\").className = 'error';\n";
    print "document.getElementById(\"msgs\").innerHTML += ";

    if (length($errstring) > 0) {
	print "\"<li>$errstring</li>\";\n";
    }
    else {
	print "\"<br/><br/>\";\n";
    }
    print "</SCRIPT>\n";
    print &closeTabbedPane(selected=>1) if (!$opts{'errors'});

    if ($fatal){
	&writeStats();
	&closeHTML();
	exit($fatal);
    }

    $opts{'errors'}++;
}

###################################################
sub processOptions {
###################################################
    $opts{'errors'} = 0;
    $opts{'filtering'} = 0;

    &reportError("I need an input file!",1) if (!$opts{'infile'});
    unless (-e $opts{'infile'}) {
	&reportError("File not found!");
	&reportError("Please check file path and try again:");
	&reportError("$opts{infile}",1);
    }

    # filter opts
    $opts{'minProb'} = $cgi_query->param('min_Probability') ? $cgi_query->param('min_Probability') : 0.0;
    $opts{'maxProb'} = $cgi_query->param('max_Probability') ? $cgi_query->param('max_Probability') : 1.0;
    if ($opts{'minProb'} > $opts{'maxProb'}) {
	&reportError("Min probability filter is higher than max!  Ignoring...");
	$opts{'minProb'} = 0;
	$opts{'maxProb'} = 1;
    }

    $opts{'minPCov'} = $cgi_query->param('min_Coverage') ? $cgi_query->param('min_Coverage') : 0.0;
    $opts{'maxPCov'} = $cgi_query->param('max_Coverage') ? $cgi_query->param('max_Coverage') : 100.0;
    if ($opts{'minPCov'} > $opts{'maxPCov'}) {
	&reportError("Min % coverage filter is higher than max!  Ignoring...");
	$opts{'minPCov'} = 0;
	$opts{'maxPCov'} = 100;
    }

    $opts{'minNPep'} = $cgi_query->param('min_Num_Peptides') ? $cgi_query->param('min_Num_Peptides') : 0;
    $opts{'maxNPep'} = $cgi_query->param('max_Num_Peptides') ? $cgi_query->param('max_Num_Peptides') : 100000;
    if ($opts{'minNPep'} > $opts{'maxNPep'}) {
	&reportError("Min number of peptides filter is higher than max!  Ignoring...");
	$opts{'minNPep'} = 0;
	$opts{'maxNPep'} = 100000;
    }

    $opts{'minPSpc'} = $cgi_query->param('min_Pct_Spectrum_ids') ? $cgi_query->param('min_Pct_Spectrum_ids') : 0.0;
    $opts{'maxPSpc'} = $cgi_query->param('max_Pct_Spectrum_ids') ? $cgi_query->param('max_Pct_Spectrum_ids') : 100.0;
    if ($opts{'minPSpc'} > $opts{'maxPSpc'}) {
	&reportError("Min % spectrum ids filter is higher than max!  Ignoring...");
	$opts{'minPSpc'} = 0;
	$opts{'maxPSpc'} = 100;
    }

    $opts{'minXHLr'} = $cgi_query->param('min_Xpress_Heavy') ? $cgi_query->param('min_Xpress_Heavy') : 0.0;
    $opts{'maxXHLr'} = $cgi_query->param('max_Xpress_Heavy') ? $cgi_query->param('max_Xpress_Heavy') : 9999.0;
    if ($opts{'minXHLr'} > $opts{'maxXHLr'}) {
	&reportError("Min Xpress H/L ratio filter is higher than max!  Ignoring...");
	$opts{'minXHLr'} = 0;
	$opts{'maxXHLr'} = 9999;
    }

    $opts{'minAHLr'} = $cgi_query->param('min_ASAP_Heavy') ? $cgi_query->param('min_ASAP_Heavy') : 0.0;
    $opts{'maxAHLr'} = $cgi_query->param('max_ASAP_Heavy') ? $cgi_query->param('max_ASAP_Heavy') : 9999.0;
    if ($opts{'minAHLr'} > $opts{'maxAHLr'}) {
	&reportError("Min ASAP H/L ratio filter is higher than max!  Ignoring...");
	$opts{'minAHLr'} = 0;
	$opts{'maxAHLr'} = 9999;
    }

    $opts{'minPval'} = $cgi_query->param('min_ASAP_pvalue') ? $cgi_query->param('min_ASAP_pvalue') : 0.0;
    $opts{'maxPval'} = $cgi_query->param('max_ASAP_pvalue') ? $cgi_query->param('max_ASAP_pvalue') : 1.0;
    if ($opts{'minPval'} > $opts{'maxPval'}) {
	&reportError("Min ASAPRatio pvalue filter is higher than max!  Ignoring...");
	$opts{'minPval'} = 0;
	$opts{'maxPval'} = 1;
    }

    $opts{'xclXHLr'}++ if $cgi_query->param("xcl_Xpress_Heavy");
    $opts{'xclAHLr'}++ if $cgi_query->param("xcl_ASAP_Heavy");

    foreach my $field (@fields_string) {
	if ($cgi_query->param("filter_$field")) {
	    my $filter_name = "filter_$field";
	    $filter_name = 'not_'.$filter_name if ($cgi_query->param("not_logic_$field"));

	    $opts{"$filter_name"} = $cgi_query->param("filter_$field");

	    if ( ($cgi_query->param("logic_$field") eq 'starts_with') || ($cgi_query->param("logic_$field") eq 'is_equal_to') ) {
		$opts{"$filter_name"} = '^'.$opts{"$filter_name"};
	    }
	    if ( ($cgi_query->param("logic_$field") eq 'ends_with') || ($cgi_query->param("logic_$field") eq 'is_equal_to') ) {
		$opts{"$filter_name"} .= '$';
	    }
	    $opts{'filtering'}++;
	}
    }

    # sort opts
    $opts{'sortBy'} = $cgi_query->param('sort_by') ? $cgi_query->param('sort_by') : 'Index';
    $opts{'sortDir'} = $cgi_query->param('sort_direction') ? $cgi_query->param('sort_direction') : 'Ascending';
    $opts{'sorting'}++ unless ( ($opts{'sortBy'} eq 'Index') && ($opts{'sortDir'} eq 'Ascending') );


    # display opts
    $opts{'decoyText'} = $cgi_query->param('decoys_start_with') ? $cgi_query->param('decoys_start_with') : 'SUPERCALIFRAGILISTICOESPIRALIDOSO_LUIS';
    $opts{'use_asapratio_pvalue'} = $cgi_query->param("asap_valtype") ? ($cgi_query->param("asap_valtype") eq 'adjusted') : 0;

    if ($cgi_query->param('quant')) {
	my $quant = $cgi_query->param('quant');
	$opts{"has_libra"}            = $quant =~ /L/;
	$opts{"has_xpress"}           = $quant =~ /X/;
	$opts{"has_asapratio"}        = $quant =~ /A/;
	$opts{"use_asapratio_pvalue"} = $quant =~ /P/;
	$opts{"has_asapratio_pvalue"} = $quant =~ /p/;
    }

    $opts{'DEBUG'}     = $cgi_query->param('d') ? 1 : 0;
    $opts{'boxPeps'}   = $cgi_query->param('boxPeps') ? 1 : 0;
    $opts{'show_peps'} = $cgi_query->param('display_peptides') ? 1 : 0;
    $opts{'write_ggl'} = $cgi_query->param('export_firegoose') ? 1 : 0;

    $opts{'filtering'}++ if ($opts{'minProb'} > 0);
    $opts{'filtering'}++ if ($opts{'maxProb'} < 1);
    $opts{'filtering'}++ if ($opts{'minPCov'} > 0);
    $opts{'filtering'}++ if ($opts{'maxPCov'} < 100);
    $opts{'filtering'}++ if ($opts{'minPSpc'} > 0);
    $opts{'filtering'}++ if ($opts{'maxPSpc'} < 100);
    $opts{'filtering'}++ if ($opts{'minNPep'} > 0);
    $opts{'filtering'}++ if ($opts{'maxNPep'} < 100000);
    $opts{'filtering'}++ if ($opts{'minXHLr'} > 0);
    $opts{'filtering'}++ if ($opts{'maxXHLr'} < 9999);
    $opts{'filtering'}++ if ($opts{'minAHLr'} > 0);
    $opts{'filtering'}++ if ($opts{'maxAHLr'} < 9999);
    $opts{'filtering'}++ if ($opts{'minPval'} > 0);
    $opts{'filtering'}++ if ($opts{'maxPval'} < 1);

    # action?
    $opts{'action'} = $cgi_query->param('Action') ? $cgi_query->param('Action') : $cgi_query->param('ExportExcel') ? 'ExportExcel' : $cgi_query->param('ExportJSON') ? 'ExportJSON' : '';

}

###################################################
sub printCSS {
###################################################
    my $css=<<END;
<style type="text/css">
.hidden {display:none}
.visible{display:block}

body    {padding:0px;margin:0px;background-color:#ffffff;font-family: Helvetica,sans-serif; }
table   {border-collapse:collapse; border-color:#000000; font-size: 10pt;}

.blank  {height:75;background-color:white}
.opaque {position:fixed;width:100%;z-index:9990;height:90;filter:alpha(opacity=85);opacity:.85;background-color:white}
.banner {position:fixed;width:100%;z-index:9995;border-bottom: 5px solid white;padding:0px;margin:0px;font-size:20px;color:#ffffff; background-color:#3B5998}
.thead  {padding:0px;margin:0px;color:#ffffff; background-color:#3B5998}
.theader{font-size:12px;text-align:center;font-weight:bold;background-color:#eeaa00;}
.tgray {display:inline-block;border-top: 1px solid black;font-weight:bold;white-space:nowrap;background: #eeeeee;}
.tdecoy{display:inline-block;border-top: 1px solid black;font-weight:bold;white-space:nowrap;background: #eecccc;}
.small {font-size:8px;text-align:right;font-weight:bold;}

.prob{font-size:10px;text-align:left;font-weight:bold;width:140;display:inline-block;color:red}
.probsm{font-size:10px;text-align:left;font-weight:bold;width:40;display:inline-block;color:red}
.nbar{font-size:8px;text-align:right;font-weight:bold;width:130;display:inline-block}

.prothead{display:inline-block;white-space:nowrap;border-top: 1px solid black;background: #eeeeee;}
.annot{width:1104;display:block;padding-left:30;border-top: 1px solid white;font-size:12px;font-weight:normal;color:#666666;background: #eeeeee;}
.entry{width:50;display:inline-block;font-weight:normal;white-space:nowrap;}
.entry a {text-decoration:none}
.prot {width:500;display:inline-block;overflow:hidden;font-weight:bold;white-space:nowrap;}
.protsm{width:200;display:inline-block;overflow:hidden;font-weight:bold;white-space:nowrap;background: #eeeeee;}
.annt {font-size:10px;font-weight:normal;color:#666666}
.annt a {color:#666666}
.na {font-size:10px;text-align:center;width:100;display:inline-block;}

.inds {width:30;display:inline-block;font-size:12px;font-weight:bold;color:#666666;}
.inds a {color:#666666}

.pephead{color:333333;}
.pepchrg{width:50;display:inline-block;font-size:10px;text-align:right;}
.pepseq {width:510;display:inline-block;font-family:"Lucida Console";font-size:10px;color:333333;}
.pepseq a {color:#666666}

.histogram {height:6;display:inline-block;border:1px solid black;}

.navtab {
    padding:0px;
    margin:0px;
    font-size:12px;
    color: #ffffff;
    background:#3B5998;
}
.navtabON {
    padding:6px;
    margin:0px;
    font-size:12px;
    border-top: 2px solid white;
    border-top-left-radius: 9px;
    border-top-right-radius: 9px;
    border-right: 2px solid white;
    border-left: 2px solid white;
    border-bottom: 0px solid white;
    font-weight: bold;
    background: #ffffff;
    color: #3B5998;
}
.navtab a {
    text-decoration:none;
    color:#ffffff;
  }
.navtab a:hover {
  background:#5B79B8;
  color:#ffffff;
}
.navtabON a {
  text-decoration:none;
  background: #ffffff;
  color: #3B5998;
}
.error {
  border: 2px solid #aa0000;
  padding:6px;
  display:block;
  background:#ffaa00;
  color:#aa0000;
}

END

    my $i = 0;
    for my $col (&wobniar(-101,"#0064E0","#00FF00")) {
	$css .= ".histogram$i {width:$i;height:6;display:inline-block;border:1px solid black;background-color:$col}\n";
	$i++;
    }

    $css .= "</style>\n";

  # add javascript function to create and switch tabs
  $css .=<<'  END_JS';
  <SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
  <!--
  var navtabs = new Array();
  var scores = new Object();
  var numselprots = 0;
  var jump_to;
  var quant = '';
  var modelsfile;

  function addPane(tabname) {
      // to close a tabbed pane, simply close a </div> tag
      var index = navtabs.push(tabname) - 1;

      document.getElementById("navtab"+index).innerHTML = "<a href=\"javascript:showPane('navtab" + index + "');\">&nbsp;" + tabname + "&nbsp;</a>";
      document.getElementById("navtab"+index).className = "navtab";

      document.write("<div class=\"hidden\" id=\"navtab"+index+"_content\">");
  }

  function showPane(divId) {
      var x;
      for (x in navtabs) {
	  var elId = "navtab"+x;
	  document.getElementById(elId).className = "navtab";

	  if (document.getElementById(elId+"_content"))
	      document.getElementById(elId+"_content").className="hidden";
      }

      if (document.getElementById(divId+"_content"))
	  document.getElementById(divId+"_content").className="visible";
      document.getElementById(divId).className="navtabON";

      if (jump_to) {
	  window.location.hash = '';
	  window.location.hash = "e_" + jump_to;
	  window.scrollBy(0,-100);
      }
  }

  function highlight(domname,yesno){
      var spanarr = document.getElementsByName(domname);

      if (yesno == 'yes') {
	  new_color = "#f1fd81";
      } else {
	  new_color = "#ffffff";
      }

      for (i=0; i<spanarr.length; i++){
	  spanarr[i].style.background = new_color;
      }
  }

  // All-purpose wrapper for XMLHttpRequest call
    var http_req = new Array();  // 1:protein entry retrieval; 2:annotations; 3:export file

  function executeXHR(index, callback, url) {
      // branch for native XMLHttpRequest object
	  if (window.XMLHttpRequest) {
	      http_req[index] = new XMLHttpRequest();
	      http_req[index].onreadystatechange = callback;
	      http_req[index].open("GET", url, true);
	      http_req[index].send(null);
	  } // branch for IE/Windows ActiveX version
	  else if (window.ActiveXObject) {
	      http_req[index] = new ActiveXObject("Microsoft.XMLHTTP");
	      if (http_req[index]) {
		  http_req[index].onreadystatechange = callback;
		  http_req[index].open("GET", url, true);
		  http_req[index].send();
	      }
	  }

      // warn if we cannot create this object    FIXME
	  if (!http_req[index]) {
	      document.body.style.cursor = "auto";
	  }
  }

  // taken from http://www.activsoftware.com/
  function getQueryVariable(variable) {
      var query = window.location.search.substring(1);
      var vars = query.split("&");
      for (var i=0;i<vars.length;i++) {
	  var pair = vars[i].split("=");
	  if (pair[0] == variable) {
	      return pair[1];
	  }
      } 
      return false;
  }

  END_JS

  $css .=<<"  END_JS";
// Retrieve info on selected protein(s)
    function getEntryInfo(entry, peps) {
	var url;

	if (peps == 'box') {
	    url = \"$opts{'self_url'}?file=$opts{infile}&of=html&boxPeps=1&Action=displayEntry&entry=\"+entry;
	}
	else {
	    url = \"$opts{'self_url'}?file=$opts{infile}&of=html&Action=displayEntry&entry=\"+entry+\"&quant=\"+quant;
	}

	for (x in navtabs) {
	    if (navtabs[x] == 'Selected Entries') {
		showPane("navtab"+x);
	    }
	}
	document.getElementById('selectedprots').innerHTML = "<span class='prob'> Retrieving information...</span>";
	document.body.style.cursor = "wait";

	jump_to = entry;

	var callback = processXHRReply(1,'selectedprots');
	executeXHR(1, callback, url);
    }

// Display specified peptide across all entries
    function entriesByPeptide(pepseq) {
	var url;

	url = \"$opts{'self_url'}?file=$opts{infile}&of=html&Action=displayPeptide&logic_Peptide_Sequence=is_equal_to&filter_Peptide_Sequence=\"+pepseq+\"&quant=\"+quant;

	for (x in navtabs) {
	    if (navtabs[x] == 'Selected Entries') {
		showPane("navtab"+x);
	    }
	}
	document.getElementById('selectedprots').innerHTML = "<span class='prob'> Retrieving information...</span>";
	document.body.style.cursor = "wait";

	var callback = processXHRReply(1,'selectedprots');
	executeXHR(1, callback, url);
    }


// generic function; updates inner HTML of given ID with XHR response
    function processXHRReply(index,htmlID) {
	return function() {
	    // only if req shows 'loaded'
	    if (http_req[index].readyState == 4) {
		// only if 'OK'
		if (http_req[index].status == 200) {
		    document.getElementById(htmlID).innerHTML = http_req[index].responseText;
		    document.body.style.cursor = "auto";
		} else {
		    document.getElementById(htmlID).innerHTML = '<b>There was a problem retrieving the HTML data: ' + http_req[index].statusText + '</b>';
		    document.body.style.cursor = "auto";
		}
	    }
	}
    }


// Retrieve annotation info on selected entry
    function getAnnotInfo(entry,divpre) {
	var annot_html_id = divpre+entry;

	if (document.getElementById(annot_html_id).innerHTML) {
	    var newclass = (document.getElementById(annot_html_id).className == 'visible') ? 'hidden' : 'visible';
	    document.getElementById(annot_html_id).className = newclass;
	}

	else {
	    var url = \"$opts{'self_url'}?file=$opts{infile}&of=html&Action=displayAnnotEntry&entry=\"+entry;

	    document.getElementById(annot_html_id).className = 'visible';
	    document.getElementById(annot_html_id).innerHTML = "<span class='prob'> Retrieving information...</span>";
	    document.body.style.cursor = "wait";

	    var callback = processXHRReply(2,annot_html_id);
	    executeXHR(2, callback, url);
	}
    }


// Export tsv (,etc?)
    function export_prots(form) {
	var file = form.elements.namedItem("exportfile").value;
	var ofmt = form.elements.namedItem("of").value;
	var xprt = form.elements.namedItem("export").value;

	var pos = document.URL.indexOf("#");
	var url = ((pos>0) ? document.URL.substring(0,pos) : document.URL) + "&exportfile=" + file + "&of=" + ofmt + "&" + xprt + "=1";

	var spid = ofmt + 'exportresult';
	document.getElementById(spid).innerHTML = " Saving...<br>"; // + url; // debug

	var callback = processXHRReply(3,spid);
	executeXHR(3, callback, url);
    }


    function protlink( protein, peptides ) {
	var baselink = "comet-fastadb.cgi?Db=" + database;
//	baselink = baselink + "&N-Glyco=1"; // if (glyc);
//	baselink = baselink + "&MarkAA=' . mark_aa . '"; // if(! (mark_aa eq ''));
	baselink = baselink + "&Ref=" + protein;
	baselink = baselink + "&Pep=" + peptides;
	var link = encodeURI(baselink);
	window.open(link,"protseq");
    }

    function asaplink( protein, group ) {
	var baselink = "ASAPCGIDisplay.cgi?";
	baselink = baselink + "protein=" + protein;
	baselink = baselink + "&group_no=" + group;
	baselink = baselink + "&xmlFile=$opts{'infile'}";
	baselink = baselink + "&ratioType=" + asap_ratio_type; // define this!
	var link = encodeURI(baselink);
	window.open(link,"asapprot");
    }

    function xplink( protein, peptides ) {
	var baselink = "XPressCGIProteinDisplayParser.cgi?";
	baselink = baselink + "protein=" + protein;
	baselink = baselink + "&peptide_string=" + peptides;
	baselink = baselink + "&xmlfile=$opts{'infile'}";
	baselink = baselink + "&source_files=" + pepxml_source_files;
	var link = encodeURI(baselink);
	window.open(link,"xpressprot");
    }

    function peplink( pepseq, charge ) {
	var baselink = "PepXMLViewer.cgi?page=1";
	baselink = baselink + "&requiredPeptideText=^" + pepseq + "\$";
	baselink = baselink + "&requiredSpectrumText=" + charge + "\$"; // adjust if no charge for iProphet!
	baselink = baselink + "&sortField=Gcalc_neutral_pep_mass";
	baselink = baselink + "&xmlFileName=" + pepxml_source_files;
	var link = encodeURI(baselink);
	window.open(link,"pepxml");
    }

    function nsplink( peptide, charge, nspbin, nspval, prots, inds ) {
	if (modelsfile) {
	    var baselink = modelsfile;
	    baselink = baselink + "?NSPbin=" + nspbin;
	    baselink = baselink + "&NSPval=" + nspval;
	    baselink = baselink + "&NSPchg=" + charge;
	    baselink = baselink + "&NSPpep=" + peptide;
	    baselink = baselink + "&NSPprt=" + prots;
	    baselink = baselink + "&NSPind=" + inds;
	    var link = encodeURI(baselink);
	    window.open(link,"tppmodels");
	}
      }

  // -->
  </SCRIPT>
  END_JS

    return $css;
}


###############################################################################
sub addTabbedPane {
# adds javascript code to add tab entry and opens corresponding div
# returns javascript in a scalar
# 
###############################################################################
  my %args = @_;

  $args{label} ||= 'tab';
  my $pad = '&nbsp;';

  my $dhtml =<<"  END_JS";
  <SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
  <!--
      addPane('$args{label}');
  // -->
  </SCRIPT>
  END_JS

  return $dhtml;
}

###############################################################################
sub closeTabbedPane {
# close div; add hr; select if requested
# returns html in a scalar
# 
###############################################################################
  my %args = @_;

  $args{selected} ||= '';

  my $dhtml = "</div>\n";

  if ($args{selected}) {
      $dhtml .=<<"      END_DHTML";
      <SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
       <!--
          var mytab = 'navtab' + (navtabs.length - 1);
          document.onload = showPane(mytab);
      // -->
       </SCRIPT>
      END_DHTML
  }

  return $dhtml;
}


### START WOBNIAR

# shiny.pl
# This is a rewrite of a script I wrote 4 years ago to make spectrums of
# colors for web page table tags.  It uses a real simple geometric conversion
# that gets the job done.
#
# It can shade from dark to light, from saturated to dull, and around the
# spectrum all at the same time. It can go thru the spectrum in either
# direction.
#
# The wobniar sub takes 2 or three values:
# $cnt is the size of the array of colors you want back.  Optionally
#   it can be negated if you want the spectrum to rotate in reverse.
#   Thus red->yellow->green reversed gets you red->purple->blue->sky->green
# $col1 can be 000000 to FFFFFF and can optionally have a preceding '#'
# $col2 is optional and will be set to match $col1 if left off.
#
# It will return data as an array or arrayref, it always upcases the color
# values. If $col1 had a "#" preceding it, so will all the output values.
#
# Bugs:
#
#   This should have been a module but I'm soooo lazy.

# @ARGV = ( 25, "#ffff00", "FF00FF" ) if @ARGV==0;
# print join( "\n", wobniar( @ARGV ) ), $/;

sub wobniar {
   die "ColorCount and at least 1 color like #AF32D3 needed\n" if @_ < 2;
   my $cnt = shift;
   my $col1 = shift;
   my $col2 = shift || $col1;
   my @murtceps;
   push @murtceps, uc $col1;

   my $pound = $col1 =~ /^#/ ? "#" : "";
   $col1 =~s/^#//;
   $col2 =~s/^#//;

   my $clockwise = 0;
   $clockwise++ if ( $cnt < 0 );
   $cnt = int( abs( $cnt ) );

   return ( wantarray() ? @murtceps : \@murtceps ) if $cnt == 1;
   return ( wantarray() ? ($col1, $col2) : [$col1, $col2] ) if $cnt == 2;

   # The RGB values need to be on the decimal scale,
   # so we divide em by 255 enpassant.
   my ( $h1, $s1, $i1 ) =
      rgb2hsi( map { hex() / 255 } unpack( 'a2a2a2', $col1 ) );
   my ( $h2, $s2, $i2 ) =
      rgb2hsi( map { hex() / 255 } unpack( 'a2a2a2', $col2 ) );
   $cnt--;
   my $sd = ( $s2 - $s1 ) / $cnt;
   my $id = ( $i2 - $i1 ) / $cnt;
   my $hd = $h2 - $h1;
   if ( uc( $col1 ) eq uc( $col2 ) ) {
      $hd = ( $clockwise ? -1 : 1 ) / $cnt;
   } else {
      $hd = ( ( $hd < 0 ? 1 : 0 ) + $hd - $clockwise) / $cnt;
   }

   while (--$cnt) {
      $s1 += $sd;
      $i1 += $id;
      $h1 += $hd;
      $h1 -= 1 if $h1>1;
      $h1 += 1 if $h1<0;
      push @murtceps, sprintf "$pound%02X%02X%02X",
         map { int( $_ * 255 +.5) }
            hsi2rgb( $h1, $s1, $i1 );
   }
   push @murtceps, uc "$pound$col2";
   return wantarray() ? @murtceps : \@murtceps;
}

sub rgb2hsi {
   my ( $r, $g, $b ) = @_;
   my ( $h, $s, $i ) = ( 0, 0, 0 );

   $i = ( $r + $g + $b ) / 3;
   return ( $h, $s, $i ) if $i == 0;

   my $x = $r - 0.5 * ( $g + $b );
   my $y = 0.866025403 * ( $g - $b );
   $s = ( $x ** 2 + $y ** 2 ) ** 0.5;
        return ( $h, $s, $i ) if $s == 0;

   $h = POSIX::atan2( $y , $x ) / ( 2 * 3.1415926535 );
   return ( $h, $s, $i );
}

sub hsi2rgb {
   my ( $h, $s, $i ) =  @_;
   my ( $r, $g, $b ) = ( 0, 0, 0 );

   # degenerate cases. If !intensity it's black, if !saturation it's grey
        return ( $r, $g, $b ) if ( $i == 0 );
        return ( $i, $i, $i ) if ( $s == 0 );

   $h = $h * 2 * 3.1415926535;
   my $x = $s * cos( $h );
   my $y = $s * sin( $h );

   $r = $i + ( 2 / 3 * $x );
   $g = $i - ( $x / 3 ) + ( $y / 2 / 0.866025403 );
   $b = $i - ( $x / 3 ) - ( $y / 2 / 0.866025403 );

   # limit 0<=x<=1  ## YUCK but we go outta range without it.
   ( $r, $b, $g ) = map { $_ < 0 ? 0 : $_ > 1 ? 1 : $_ } ( $r, $b, $g );

   return ( $r, $g, $b );
}
### END WOBNIAR
