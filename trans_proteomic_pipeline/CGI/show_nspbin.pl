#!/usr/bin/perl
#############################################################################
# Program       : show_nspbin.pl                                            #
# Author        : Luis Mendoza <lmendoza at systems biology>                #
# Date          : 11.15.11                                                  #
# SVN Info      : $Id: show_nspbin.pl 6504 2014-05-02 21:10:13Z slagelwa $
#                                                                           #
# ProteinProphet utility                                                    #
#                                                                           #
# Copyright (C) 2011 Luis Mendoza                                           #
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
# Insitute for Systems Biology                                              #
# 401 Terry Avenue North                                                    #
# Seattle, WA  98109  USA                                                   #
#                                                                           #
#############################################################################
use strict;
use XML::Parser;
use CGI;
use tpplib_perl; # exported TPP lib function points
my $TPPVersionInfo = tpplib_perl::getTPPVersionInfo();

my $page_title = "protXML NSP Bin Viewer";
my $cgi_query = CGI->new;
my $has_ni_info = 0; # kludge
$| = 1; # autoflush

&openHTML();
&processProtXML();
&closeHTML();

exit;

#############################################################################
sub openHTML {
    print "Content-type: text/html\n\n";
    print "<html>\n";
    print "<head><title>$page_title</title></head>\n";
    print "<body bgcolor='white' onload='self.focus();'>\n<PRE>\n";

    my $page_head;

    my $charge = $cgi_query->param('charge') ? $cgi_query->param('charge')."_" : '';
    my $pep    = $cgi_query->param('pep')    ? $cgi_query->param('pep') : '';
    my $prot   = $cgi_query->param('prot')   ? $cgi_query->param('prot') : '';
    my $nspval = $cgi_query->param('nsp_val')? $cgi_query->param('nsp_val') : '';

    $page_head .= "<b>Peptide: <font style='color:red'>$charge$pep</font></b>" if $pep;
    $page_head .= "&nbsp;"x5 ."<b>Protein: <font style='color:red'>$prot</font></b>\n" if $prot;
    $page_head .= "<hr size='1' noshade>\n";

    $page_head .= "<b>ProteinProphet<sup><font size='3'>&#174;</font></sup> Estimated Number of Sibling Peptides (nsp): <font style='color:red'>$nspval</font></b>\n" if $nspval;

    print $page_head;

}

sub closeHTML {
    print "<hr size='1' noshade>\n";
    print "<font color='#999999'>$page_title\n$TPPVersionInfo</font>\n";
    print "</PRE>\n</body>\n</html>";

}


sub processProtXML {
    my $infile = $cgi_query->param('xmlfile') || '';

    &reportError("Could not find protXML file: $infile!",1) unless -e $infile;

    #### Set up the XML parser and parse the returned XML
    my $parser = XML::Parser->new(
				  Handlers => {
				      Start => \&start_protxml_element,
				      End   => \&end_protxml_element,
				  },
				  ErrorContext => 2 );
    eval { $parser->parsefile( $infile ); };
    &reportError("ERROR_PARSING_XML:$@",1) if($@);

}

sub start_protxml_element {
    my ($handler, $element, %atts) = @_;

    if ($element eq 'program_details') {
	my $details;

	$details .= "Version: $atts{'version'}\n" if $atts{'version'};
	$details .= "Analysis Date: $atts{'time'}\n" if $atts{'time'};

	print "$details\n\n";
    }

    elsif ($element eq 'nsp_information') {
	print "<b>Learned Number of Sibling Peptide Distributions</b>\n";
	print "Neighboring bin smoothing: $atts{'neighboring_bin_smoothing'}\n" if $atts{'neighboring_bin_smoothing'};

	print "<table style='background:#eeeeee' frame='border' rules='all' cellpadding='2'>\n<tr style='background:#0e207f;color:white'>";
	foreach my $head (qw(bin_number nsp_range positive_freq negative_freq positive/negative_ratio)) {
	    print "<th>$head</th>";
	}
	print "</tr>\n";

    }

    elsif ($element eq 'nsp_distribution') {
	my $tr_style = $cgi_query->param('nsp_bin') == $atts{'bin_no'} ? 'style="background:#ff8800;font-weight:bold"' : '';

	my $tr_data = "<tr $tr_style><td>$atts{'bin_no'}</td>";

	# range
	$tr_data .= "<td>";
	$tr_data .= $atts{'nsp_lower_bound_excl'} ? "$atts{'nsp_lower_bound_excl'} &lt; " : "$atts{'nsp_lower_bound_incl'} &lt;= ";
	$tr_data .= "nsp ";
	$tr_data .= $atts{'nsp_upper_bound_excl'} ? "&lt; $atts{'nsp_upper_bound_excl'}" : "&lt;= $atts{'nsp_upper_bound_incl'}";
	$tr_data .= "</td>";

	$tr_data .= "<td>$atts{'pos_freq'}</td>";
	$tr_data .= "<td>$atts{'neg_freq'}</td>";

	# ratio
	$tr_data .= "<td>$atts{'pos_to_neg_ratio'}";
	$tr_data .= $atts{'alt_pos_to_neg_ratio'} ? " ($atts{'alt_pos_to_neg_ratio'})" : '';
	$tr_data .= "</td>";

	$tr_data .= "</tr>\n";

	print $tr_data;

    }

    elsif ($element eq 'ni_distribution') {
	if ($atts{'bin_no'} eq "0") {  # ugly, but this tag is persistent in protXML files, even when this analysis was not done
	    print "<b>Learned Expected Number of Ion Instances Distribution</b>\n";
	    print "<table style='background:#eeeeee' frame='border' rules='all' cellpadding='2'>\n<tr style='background:#0e207f;color:white'>";
	    foreach my $head (qw(bin_number ni_range positive_freq negative_freq positive/negative_ratio)) {
		print "<th>$head</th>";
	    }
	    print "</tr>\n";
	    $has_ni_info++;
	}

	my $tr_data = "<tr><td>$atts{'bin_no'}</td>";

	# range
	$tr_data .= "<td>";
	$tr_data .= $atts{'ni_lower_bound_excl'} ? "$atts{'ni_lower_bound_excl'} &lt; " : "$atts{'ni_lower_bound_incl'} &lt;= ";
	$tr_data .= "ni ";
	$tr_data .= $atts{'ni_upper_bound_excl'} ? "&lt; $atts{'ni_upper_bound_excl'}" : "&lt;= $atts{'ni_upper_bound_incl'}";
	$tr_data .= "</td>";

	$tr_data .= "<td>$atts{'pos_freq'}</td>";
	$tr_data .= "<td>$atts{'neg_freq'}</td>";

	# ratio
	$tr_data .= "<td>$atts{'pos_to_neg_ratio'}";
	$tr_data .= $atts{'alt_pos_to_neg_ratio'} ? " ($atts{'alt_pos_to_neg_ratio'})" : '';
	$tr_data .= "</td>";

	$tr_data .= "</tr>\n";

	print $tr_data;
    }

}

sub end_protxml_element {
    my ($handler, $element) = @_;

    if ($element eq 'nsp_information') {
	print "</table>\n\n";
    }

    elsif ($element eq 'ni_information') {
	print "</table>\n\n" if $has_ni_info;
    }

    elsif ($element eq 'protein_summary_header') {
	&closeHTML();  # we're done, so stop reading the file
	exit;
    }

}


sub reportError {
    my $errstring = shift;
    my $fatal = shift || 0;

    print "\n\n<font style='border:3px solid red;color:red'>ERROR: $errstring</font>\n\n";

    if ($fatal) {
	&closeHTML();
	exit($fatal);
    }
}
