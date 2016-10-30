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

#
# gather TPP version info
#
# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath);
use lib realpath(dirname($0));
use tpplib_perl; # exported TPP lib function points
$TPPVersionInfo = tpplib_perl::getTPPVersionInfo();

print "Content-type: text/html\n\n";

%box = &tpplib_perl::read_query_string;      # Read keys and values
$| = 1; # autoflush


my $help_dir = exists $box{'help_dir'} ? $box{'help_dir'} : '';
# chop off terminal / if exists
if($help_dir =~ /^(\S+)\/$/) {
    $help_dir = $1;
}
#print "help dir: $help_dir\n";

print "<HTML>\n";
print "<BODY BGCOLOR=\"white\" OnLoad=\"self.focus()\">\n";
print "<PRE>\n";
print "<HEAD><TITLE>Help for viewing ProteinProphet results (" . $TPPVersionInfo . ")</TITLE></HEAD><table>\n";

print "<tr>\n";
print "<td colspan = \"2\"><font color=\"red\"><b>Help for viewing ProteinProphet<sup><font size=\"3\">&reg;</font></sup> results</b></font>\n";
print "</td>\n";
print "</tr>\n";

print "<tr><td>&nbsp;</td></tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Getting Started</b></td>\n";
print "</tr>\n";



print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help00.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Your previous data filtering is always saved.  Opening the .shtml file in your browser will automatically display the most recent view of the data.  After filtering the data, you can always restore the entire original dataset by clicking on the 'Restore Original Data' button.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Displayed Protein Information</b></td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help.png\" width=\"550\" height=\"350\"/></td><td>\n";
print "1. total peptide coverage of protein sequence<br>";
print "2. weight:  If a peptide is present in multiple sequence entries, this weight indicates the contribution of this peptide among each of those sequence entries.  An asterisk indicates that this peptide is only found in (or is unique to) this protein entry<br>";
print "3. nsp adj prob (number of sibling peptides adjusted probability):  Proteins with multiple peptide identifications are likely more correct than proteins with single peptide identifications.  Different peptides from the same protein are termed sibling peptides.  The number under this column is the adjusted peptide probability after taking into account its number of sibling peptides in this dataset.  These adjusted peptide probabilities are used to calculate the protein probability<br>";
print "4. init prob:  original or initial probability assigned to this peptide by PeptideProphet (without regard to corresponding protein)<br>";
print "5. NTT (number of tolerable (tryptic) termini):  This number indicates whether the peptide sequence exhibits zero, one, or two expected cleavage termini<br>";
print "6. nsp bin: This is the bin (ranging from 0 to 7) reflecting the number of sibling peptides after discretization<br>";print "7. total:  number of instances that this peptide was identified in the underlying dataset(s)<br>";
print "8. pep grp ind (peptide group indicator):  If a peptide was identified from spectra acquired on ions of different precursor charge states, these different identifications of the same peptide are considered independent evidence for that peptide identity.  So the same peptide is listed multiple times for each charge state and the different charge states are indicated in this column\n";

print "</td>\n";
print "</tr>\n";


print "<tr>\n";
print "<td colspan=\"2\"><b>ProteinProphet Analysis Results</b></td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help01.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Model predicted sensitivity and error information displayed as a function of minimum probability filter threshold.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help02.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Details of ProteinProphet analysis, including input file(s), run options, program settings, and learned distributions.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Sorting Data</b></td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help03.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Data can be sorted by index (number), probability, protein name, percent coverage, total number of peptides, share of all spectral identifications, and when relevant, XPRESS Ratio either descending or ascending, and ASAPRatio either descending or ascending. After making selection, push the 'Filter/Sort/Discard checked entries' button.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Filtering Data</b></td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help04.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Data can be filtered by probability and when relevant, by XPRESS or by ASAPRatio quantitation results.  In the latter cases, 'w/o proper XPRESS Ratio' and 'w/o proper ASAPRatio', those entries lacking a ratio or having a negative ratio, can be excluded, and a minimum and maximum ratio can be set.  In addition, when both XPRESS and ASAPRatio data are available, checking 'ASAPRatio consistent' excludes all results for which the two ratios differ by more than one standard deviation.  Displayed peptides can be filtered by probability, number of tolerable termini, precursor ion charge, and/or amino acid content.  Finally, individual entries can be excluded by checking the box to the left of the entry number.  After making selection, push the 'Filter/Sort/Discard checked entries' button. User defined excluded entries will be sustained until cleared by checking the 'clear manual discards/restores' box.  Restoring the original data, by pushing the 'Restore Original' button, will also cancel all manual discards.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Data Views</b></td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help05.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>By checking the appropriate boxes, one can either show or exclude protein groups, annotation, or peptides.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help06.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Entries <i>NOT</i> passing the filtering criteria can be displayed by checking the 'show discarded entries' box.  Checking on the box to the left of a discarded entry number will restore that entry to the filtered list.  Remember to uncheck the 'show discarded entries' box in order to revert to displaying data that <i>passes</i> the filtering criteria.  User defined restored entries will be sustained until cleared by checking the 'clear manual discards/restores' box.  Restoring the original data, by pushing the 'Restore Original' button, will also cancel all manual restores.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Export Data to Excel</b></td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help07.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Check the 'export to excel' box before you Filter/Sort/Discard to write in tab delimited form the filtered dataset.  The spreadsheet that is written should closely mimic the view of the data in your browser.  A link to the written excel spreadsheet is displayed at the top of the protein list.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Writing Displayed Data Subset to File</b></td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help08.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>You can write the filtered data subset to a new file by entering the desired file name and pushing the 'Write Displayed Data Subset to File' button.  Note that only the displayed data will be written to the new file and accessible from there.\n";
print "</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help085.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>Newly written subset data files maintain a record of their 'geneology', which can be accessed by clicking on 'Dataset Derivation Info'.  For all generations of written subset files generated from the original output of ProteinProphet, the name of the parent file and filtering criteria used to generate the child subset file is indicated.  This insures that the current data subset is reproducible. Note: The 'Sensitity/Error Info' is not available for newly written subset data files since non-random filtering can cause the sensitivities and error rates to deviate from the original values predicted by the model for the unfiltered dataset.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Full Menu Data View Options</b></td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help086.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>More display options are available by checking the 'full menu' box prior to filtering.  For example, to have the sensitivity/error information displayed at the top of the protein entries (as it was in the old html output format), check the 'show' box to the right of 'sensitivity/error information'.  For quantitation data, you can select whether or not to display XPRESS and ASAPRatio results.  In a similar manner, whether or not to display the number of unique peptides and total number of peptides for each protein entry can be selected. Note: Be sure to check the 'short menu' box to revert to the default menu.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help087.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>To alter the displayed peptide information, enter a number (1,2,3...) in the text box following each information type that you want to display, and nothing in the boxes following information you don't want.  The numbers reflect the desired display position, starting with 1 at the far left.\n";
print "</td>\n";
print "</tr>\n";


print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help010.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>To alter the displayed annotation for data searched against IPI databases, enter a number (1,2,3...) in the text box following each annotation type that you want to display, and nothing in the boxes following annotation you don't want.  The numbers reflect the desired display position, starting with 1 at the far left.\n";
print "</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td colspan=\"2\"><b>Setting a Customized Data View</b></td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><img src=\"$help_dir/protxml_help0115.png\" width=\"550\" height=\"350\"/></td>\n";
print "<td>You can save display settings so they are used as default for all subsequent runs of ProteinProphet.  To set the customized data view, select all data view options that you want, and check the 'current' box to the right of 'set customized data view'.  If you want to revert the customized data view to the default setting (as if you never set it yourself), check off the 'default' box. Then as usual, push the 'Filter/Sort/Discard checked entries' button.\n";
print "</td>\n";
print "</tr>\n";


print "</table>\n";

print "</PRE>\n";
print "</BODY>\n";
print "</HTML>\n";







