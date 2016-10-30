#!/usr/bin/perl
#############################################################################
# Program       : show_help.pl                                              #
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

my $PEPXRESOURCES;
if ( $^O eq 'linux' ) {  # linux installation
  $PEPXRESOURCES = '/tpp/html/';
} elsif ( ($^O eq 'cygwin' )||($^O eq 'MSWin32' )) { # windows installation
    $PEPXRESOURCES = '/tpp-bin/';
} # end configuration

# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath);
use lib realpath(dirname($0));
use tpplib_perl; # exported TPP lib function points
#
# gather TPP version info
#
$TPPVersionInfo = tpplib_perl::getTPPVersionInfo();
print "Content-type: text/html\n\n";

%box = &tpplib_perl::read_query_string;      # Read keys and values
$| = 1; # autoflush


my $help_dir = exists $box{'help_dir'} ? $box{'help_dir'} : '';
# chop off terminal / if exists
if($help_dir =~ /^(\S+)\/$/) {
    $help_dir = $1;
}
if($PEPXRESOURCES =~ /^(\S+)\/$/) {
    $PEPXRESOURCES = $1;
}
#print "help dir: $help_dir\n";




print <<HTMLEND;

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-US" lang="en-US">
  <head>

    <!--
	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
	-->
    


    <title>PepXML Viewer: Help</title>

    <!-- STYLESHEETS -->
    <link rel="stylesheet" type="text/css" href="$PEPXRESOURCES/css/PepXMLViewer.css" ></link>

  </head>


  <body>

    <div id="PageContainer">
      
      <h1>PepXML Viewer: help</h1>







	<table cellspacing="0" width="100%">
	  <tbody>
	    <tr>
	      <td class="banner_cid">&nbsp;&nbsp;&nbsp;getting started</td>
	      <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;</td>
	    </tr>
	  </tbody>
	</table>
	<div id="summaryDiv" class="formentry">

	 <p>

	This program is used to view PepXML files, which contain
	information about peptides derived from MS/MS (MS level 2)
	data.  In the Trans Proteomic Pipeline, these files are
	iteratively modified by various programs as processing
	progresses.  A basic PepXML file might only contain
	search-engine results, converted, for example, from SEQUEST or
	MASCOT.  A more complex file (or the same file after more
	processing steps) might contain information about Peptide
	Prophet-assigned peptide probabilities or peptide quantitation
	info (derrived from XPRESS, ASAPratio, or LIBRA.)

	</p>

	 
	<p> 

	When the viewer program runs, it creates an "index" file with
	a name similar to the xml file you're viewing.  This file
	contains information for quickly moving through the (larger)
	PepXML file.  You may safely delete these at any time;
	however, the next time you invoke the viewer on this file, you
	may notice a delay as the index file is rebuilt.

	</p>

	</div>

	<br />
	<br />

	<table cellspacing="0" width="100%">
	  <tbody>
	    <tr>
	      <td class="banner_cid">&nbsp;&nbsp;&nbsp;understanding the interface: overview</td>
	      <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;</td>
	    </tr>
	  </tbody>
	</table>
	<div id="summaryDiv" class="formentry">
	   
	  <p>
	    The page is divided into six basic sections.  Note that
	    you can click on "show/hide" to expand or hide
	    the adjacent section.
	  </p>
	  <ul>
	    <li>Summary: general information about the file you're
	    viewing, including stats based on the filtering options
	    you've selected.  Keep an eye here and you see the number
	    of peptides in the current subset change as you apply
	    various filters.
	    <li> general options: 'update' refreshes the display with
	      new options you've selected.  'export to spreadsheet'
	      generates a tab-delimited file closely mimic the view of
	      the data in your browser, corresponding the the current
	      subset.  A link to the generated excel spreadsheet is
	      displayed at the bottom of the 'Summary' section.
	      'Pep3D' runs that program with the current PepXML file
	      as input.  'additional analysis info' provides details
	      of MS/MS analysis, including for each input file, search
	      engine, and record of each analysis.  To the far right,
	      the 'restore original' button will discard any display
	      or filter options and restore the entire subset.
	    <li>Display Options: any option (besides column selection
	    ) that doesn't change the number of peptides in the
	    current subset.  Here you'll find options like number of
	    rows per page to display, sort order, highlighting, and so
	    forth.  Data can be sorted by index (number), spectrum
	    name, peptide, search scores, probability (when
	    relevannt), XPRESS quantitation either descending or
	    ascending (when relevant), and ASAPRatio quanitiation
	    either descending or ascending (when relevant). After
	    making selection, push the "update" button.
	    <li>Column Selection and Ordering: expand this section,
	    and you'll see lists of displayed and undisplayed columns.
	    Click on a column name (hold 'control' and click to select
	    multiple names at one time) and move them up or down, or
	    hide or display them.
	    <li>Filtering Options: here you'll find choices to pare
	    down your currently displayed subset.  The options here,
	    like in Display Options, will change based on the
	    information contained in a specific PepXML file.  Data can
	    be filtered by probability (when relevant), precursor ion
	    charge, search scores, XPRESS quantitation (when
	    relevant), and ASAPRatio quantitation (when relevant).  In
	    the latter cases, 'require valid ASAP Ratio' excluses
	    those entries lacking a ratio, or having a negative ratio;
	    a minimum and maximum ratio can also be set.  In addition,
	    when both XPRESS and ASAPRatio data are available,
	    checking ' Require xpress ratio within asap mean +/-
	    error' excludes all results for which the two ratios
	    differ by more than one standard deviation.  After making
	    selection, push the 'update' button, or press the enter
	    key from a text box. Filter options remain set until the
	    'restore original' button (found in the upper right).
	    <li>the data table: this is where your data is displayed.
	    Each link brings you to another window with more
	    infomation.  See below for more information.
	  </ul>
	    </p>  
	
	</div>
    

	<br />
	<br />
	

	<table cellspacing="0" width="100%">
	  <tbody>
	    <tr>
	      <td class="banner_cid">&nbsp;&nbsp;&nbsp;working with the data</td>
	      <td align="left">&nbsp;&nbsp;&nbsp;&nbsp;</td>
	    </tr>
	  </tbody>
	</table>
	<div id="summaryDiv" class="formentry">
	   
	  <p>
	    Information about most of the typical columns can be found
	    here.  Note that not all of these columns will be
	    available if your PepXML file was not processed with a
	    specific program.  Some entries in the data table also
	    contain links which provide additional infomation.
	  </p>
	  <ul>
	    <li>
	      index (entry number): unique search result id
	    <li>
	      probability: probability that search result is correct
	      (e.g. as determined by PeptideProphet), with link to
	      probability analysis results.  Note that these links
	      will be the same within one PepXML file.
	    <li>
	      spectrum: links to comprehensive search results
	    (including runner up peptides, if avaliable).  You can
	    highlight and filter this text.
	    <li>
	      search scores: specific for each search engine
	      contributing to dataset.
	    <li>
	      matched ions: the fraction of peptide theoretical
	      fragment ions present in spectrum.  Link to MS/MS
	      spectrum with assigned fragment ions.
	    <li>
	      peptide: Best match from search engine for a given
	      spectra.  Linked to Blast launcher.  Note that you can
	      highlight and filter this text.
	    <li> 
	      protein(s): Link to database.  Hover the mouse over
	      this link for a description, if avaliable.  Addtional
	      proteins containing assigned peptide are listed out, if
	      "multiple protein hits" : "list of all hits" is selected
	      in display options.
	    <li>
	      XPRESS: Quantitation with link to ion trace.
	    <li>
	      ASAPRatio: Quantitation with link to ion trace.
	    <li>
	      PeptideProphet F score: composite score incorporating
	      several search scores.
	  </ul>
	    </p>  
	
	</div>
    

	    <br\>
	    <br \>


	<h6>Help text based on original file by Andy Keller.<br>
	    Developed at <a href="http://www.systemsbiology.org/"> Institute for Systems Biology </a> / <a href="http://www.proteomecenter.org/"> Seattle Proteomics Center </a>
	  </h6>


	

    </div>
  </body>
</html>

HTMLEND



# print "<td><img src=\"$help_dir/pipeline_help1.png\" width=\"550\" height=\"350\"/></td>\n";

