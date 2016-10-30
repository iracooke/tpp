#!/usr/bin/perl
#
# Program: TPP HPC Tools
# Author:  Joe Slagel
#
# Copyright (C) 2010 by Joseph Slagel
# 
# This library is free software; you can redistribute it and/or             
# modify it under the terms of the GNU Lesser General Public                
# License as published by the Free Software Foundation; either              
# version 2.1 of the License, or (at your option) any later version.        
#                                                                           
# This library is distributed in the hope that it will be useful,           
# but WITHOUT ANY WARRANTY; without even the implied warranty of            
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         
# General Public License for more details.                                  
#                                                                           
# You should have received a copy of the GNU Lesser General Public          
# License along with this library; if not, write to the Free Software       
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
# 
# Institute for Systems Biology
# 1441 North 34th St.
# Seattle, WA  98103  USA
# jslagel@systemsbiology.org
#
# $Id: $
#

#
# Simple CGI script that can be used to retrieve and delete mascot results. 
# Used in conjunction with qmascot to run mascot searches on a cluster. Install
# it on your mascot server in the cgi folder (typically found at  
# C:\Inetpub\mascot\cgi).
#
use strict;
use CGI;

#-- @GLOBALS -----------------------------------------------------------------#

my $VERSION = ('$Revision: $' =~ m{ \$Revision: \s+ (\S+) }x)[0];

# Default location for mascot 
my $MASCOT_DIR = "C:\\Inetpub\\mascot";
my $HTML_DIR   = "$MASCOT_DIR\\html\\";

my $cgi = new CGI;

#-- @MAIN -------------------------------------------------------------------#

{
   # File folder & name
   my $dat = $cgi->param('dat');
   my $db  = $cgi->param('db');
   $dat = "$HTML_DIR$dat" if ( $dat );
   
   if ( $dat )
      {
      ( -f $dat ) || error( 404, "Dat file $dat not found" );
      open( DAT, $dat ) or error( 500, "Can't open dat file $dat" );
      print $cgi->header( 'text/plain' );
      print while( <DAT> );
      close DAT;
      unlink $dat;
      exit 0;
      }
   elsif ( $db )
      {
      ($db =~ /^$MASCOT_DIR/) || error( 401, "DB file not allowed" );
      ( -f $db ) || error( 404, "DB file $db not found" );
      open( DB, $db ) or error( 500, "Can't open db file $db" );
      print $cgi->header( 'text/plain' );
      print while( <DB> );
      close DB;
      exit 0;
      }
   else
      {
      print $cgi->header( 'text/plain' );
      print "hello\n";
      exit 0;
      }
      
  exit 0;
} 

sub error
{
  print $cgi->header( -status => $_[0], -type => 'text/plain' ), $_[1], "\n";
  exit;
}