#!/tools/bin/perl
#
# Copyright (C) 2013 by Joe Slagel
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
#
# Institute for Systems Biology
# 401 Terry Ave N
# Seattle, WA  98109  USA
#
# $Id: updatepaths.pl 6525 2014-05-28 22:18:40Z slagelwa $

#
# DEVELOPER NOTES
#
# When updating files this program tries to leave the original XML as untouched
# as possible.  This way its possible to easily see what was updated using diff 
# on the original output and the new output.  It does this using a trick of
# diving into XML::Parser's object to get at the raw text parsed and applying
# a simple regex replacement on the original string and the new string.
#
# Updating paths in pep.xml/prot.xml files can be a little tricky. Particularly
# since when pep.xml files are merged InteractParser copies many of the paths 
# from the inputs.  Since the input files can be in different directories it
# rules out simply updating the prefix part of a path.  The best we can do is
# to try to find the file and fix the paths for them.
#
# Also be aware there is another scenario has to do with running InteractParser
# on the results of a single file in order to "fix" the pep.xml.  In this case
# the path may be set to something resembling the filename of the current
# file.
#
use strict;
use warnings;

use Cwd;
use File::Basename;
use File::Copy;
use File::Spec::Functions qw( catpath rel2abs splitpath);
use File::Spec::Win32;
use File::Temp;
use Getopt::Long;
use Pod::Usage;
use XML::Parser;

use lib rel2abs(dirname($0));
use FindBin;
use lib "$FindBin::Bin/../cgi-bin";
use tpplib_perl;


#-- @GLOBALS -----------------------------------------------------------------#

my $VERSION  = tpplib_perl::getTPPVersionInfo() || 'unknown';
my $REVISION = ('$Revision: 6525 $' =~ m{ \$Revision: \s+ (\S+) }x)[0];

my $prog = basename($0);        # Program name
my %opts;                       # Program invocation options

our $outfh;                     # Filehande to write updated results to
our $file;                      # File being updated

our $origfile;                  # Best guess as to the file path
our $origvol;                   # Best guess as to the volume of file
our $origname;                  # Best guess as to the name of file
our $origdir;                   # Best guess as to the directory path of file
our $origbase;                  # Concat of dir + name w/o extension
our $origdb;                    # Original database
our $newfile;                   # New file path
our $newvol;                    # New volume of file
our $newname;                   # New name of file
our $newdir;                    # New directory path of file
our $newbase;                   # Concat of new path + name w/o extension


#-- @MAIN -------------------------------------------------------------------#

$| = 1;                         # Flush STDOUT automatically
{
   parseOptions();

   my $parser = XML::Parser->new( NoExpand => 1, ErrorContext => 3 );
   $parser->setHandlers( Proc    => \&procTag,
                         Start   => \&startTag, 
                         Default => \&defaultTag,
                         End     => \&defaultTag );
   while ( $file = shift @ARGV )
      {
      $outfh = File::Temp->new();

      eval { $parser->parsefile( $file ) };
      if ( $@ ) 
         {
         $@ =~ s/ at \/.*?$//ms;                # Remove module line number
         logdie( "parsing $file\n$@" );
         }

      if ( !$opts{'dry-run'} )                  # Write results?
         {
         if ( $opts{backup} )
            {
            loginfo( "backing up existing file $file$opts{backup}" );
            File::Copy::syscopy( $file, "$file$opts{backup}" )
               or logdie( "can't backup file, $!");
            }
         $outfh->autoflush(1);
         loginfo( "replacing file $file" );
         copy( $outfh->filename(), $file )
               or logdie( "can't write file, $!");
         }
      }

   exit(0);
}

#
# Initialize globals with file parts from what should have been the original
# file name.
#
# @arg  Expected path to file found within its contents
#
sub initparts
   {
   my ( $path ) = @_;

   $origfile = $path;
   (  $origbase = $path ) =~ s/\..*$//;         # strip extension
   if ( $origbase =~ /^[a-z]:/i )               # looks like a Win32 file path?
      {
      ( $origvol, $origdir, $origname ) = File::Spec::Win32->splitpath( $origbase );
      $origdir = File::Spec::Win32->catpath( $origvol, $origdir );
      }
   else
      {
      ( $origvol, $origdir, $origname ) = splitpath( $origbase );
      $origdir = catpath( $origvol, $origdir );
      }

   $newfile = $newbase = $origdir ? rel2abs( $file ) : basename($file);
   $newbase =~ s/\..*$//;                       # strip extension
   ( $newvol, $newdir, $newname ) = splitpath( $newbase );
   $newvol = uc( $newvol );                     # always treat it in uppercase
   $newdir = catpath( $newvol, $newdir );

   loginfo( "file orig: $origfile new: $newfile" ) if ( $opts{verbose} > 1 );
   loginfo( "name orig: $origname new: $newname" ) if ( $opts{verbose} > 1 );
   loginfo( "dir  orig: $origdir  new: $newdir" )  if ( $opts{verbose} > 1 );
   loginfo( "base orig: $origbase new: $newbase" ) if ( $opts{verbose} > 1 );
   }

#
# Returns the value of an attribute in an element
#
sub getattr
   {
   my ( $e, $attr ) = @_;
   ( exists $e->{attrs}->{$attr} ) 
      or logdie( "$attr attribute not found in element $e->{name}" );
   return $e->{attrs}->{$attr};
   }

#
# Subsitute an attribute value with a new value
#
sub setattr
   {
   my ( $e, $attr, $old, $new ) = @_;

   if ( $old eq $new )
      {
      loginfo( "skipping $e->{name}\@$attr, already updated" );
      }
   else
      {
      loginfo( "$e->{name}\@$attr updating");
      loginfo( "... $old => $new" ) if ( $opts{verbose} > 1 );
      ( $e->{txt} =~ s/\Q$old\E/$new/ ) 
         or logwarn( "failed updating $e->{name}\@$attr element" );
      }
   }

#
# Guesstimate what the new path for a file is.  If given a list of extensions
# it will use the base name of the file and try each extension in succession.
# Looks first using a longest common prefix of the old filename and the original
# path ("relative"), then the current directory, then any other directories the
# user told us about, and finally the old location.
#
# @ret  List of paths found for the file
#
sub findfile
   {
   my ( $e, $path, @ext ) = @_;

   # Break the path into its parts
   my ( $vol, $dir, $name );
   if ( $path =~ /^[a-z]:/i )                   # looks like a Win32 file path?
      {
      ( $vol, $dir, $name ) = File::Spec::Win32->splitpath( $path );
      $dir = File::Spec::Win32->catpath( $vol, $dir );
      }
   else
      {
      ( $vol, $dir, $name ) = splitpath( $path );
      $dir = catpath( $vol, $dir );
      }

   if ( @ext )
      { $name =~ s/(\.[^.]+)$//; }              # w/o extension
   else
      { push @ext, ''; }                        # just check filename

   # See if the original path was relative to the original location of this file
   my $rel = '';
   my $lcd = lcd( $dir, $origdir );             # take longest matching prefixes
   if ( $lcd && $lcd ne $dir )
      {
      ($rel = $dir) =~ s/^\Q$lcd\E/$newdir/;    # prepend relative to new dir
      }
   loginfo( "$e->{name} looking for file '$path'" ) if ( $opts{verbose} > 1 );

   my @found;
   foreach my $d ( $rel ? $rel : (), $newdir, @{$opts{paths}}, $dir )
      {
      foreach ( @ext )
         {
         my $file = catpath( '', $d, $name ) . $_; 
         if ( -f $file )
            {
            push @found, $file;
            loginfo( "...found $file" ) if ( $opts{verbose} > 1 );
            }
         elsif ( $opts{verbose} > 1 )
            {
            loginfo( "...checked $file" );
            }
         }
      }

   return @found;
   }


#-- XML::Parser callbacks -----------------------------------------------------# 

#
# Called when a processing instruction is recognized.
#
sub procTag 
   { 
   my ( $xmlp, $target, $data ) = @_; 

   my $txt = $xmlp->original_string();
   if ( $target =~ /xml-stylesheet/i )          # found only in pepXML
      {
      # most cases href seems to be the orig path+name with a .pep.xsl suffix
      if ( my ( $href ) = $data =~ /href="(.*)"/ )
         {
         if ( $href !~ /pepXML_std.xsl$/ )              # *blech* X!Tandem
            {
            my @old = splitpath( $href );
            my $new = $old[1] ? rel2abs($file) : basename($file);
            $href =~ s/\..*$//;                         # w/o extension
            $new  =~ s/\..*$//;                         # w/o extension
            loginfo( "updating xml-stylsheet\@href");
            loginfo( "... $href <=> $new" ) if ( $opts{verbose} > 1 );
            $txt =~ s/\Q$href\E/$new/
              or logwarn( "failed updating xml-stylesheet element" );
            }
         }
      else
         {
         logwarn( "href attribute not seen in xml-stylesheet");
         }
      }

   print $outfh $txt;
   }

#
# Called when a XML start tag is recognized
#
sub startTag 
   {
   my $xml  = shift;
   my $name = shift;
   my $elem = { name => $name, attrs => { @_ }, txt => $xml->original_string() };

   # Treat parameter elements as first class elements, but optional...
   if ( $name eq 'parameter' )
      {
      $elem->{name}     = $elem->{attrs}->{name};
      $elem->{optional} = 1;
      }

   # Recognize element and attribute.  For multiple attributes of the
   # same element encase in ()'s and use "|"
   setnewfile( $elem, 'msms_pipeline_analysis', 'summary_xml' )
   or setbase( $elem, 'msms_run_summary', 'base_name' )
   or setbase( $elem, 'search_summary', 'base_name' )
   or setdb( $elem, 'search_database', 'local_path' )
   or (setfile( $elem, 'interact_summary', 'filename' ) |
       setcwd( $elem, 'interact_summary', 'directory' ))
   or setdb( $elem, 'database_refresh_timestamp', 'database' )
   or setfile( $elem, 'inputfile', 'name' )

   # Update X!Tandem specific <parameter name=?>
   or setfile( $elem, 'output, path', 'value' )
   or setfile( $elem, 'output, sequence path', 'value' )
   or setfile( $elem, 'spectrum, path', 'value' )
   or setfile( $elem, 'list path, taxonomy information', 'value' )
   or setfile( $elem, 'list path, default parameters', 'value' )
   or setdb( $elem, 'list path, sequence source #1', 'value' )
   or warndb( $elem, 'list path, sequence source #2', 'value' )

   # Update Comet specific <parameter name=?>
   or setdb( $elem, 'database_name', 'value' )

   # Update Myrimatch specific <parameter name=?>
   or setdb( $elem, 'Config: ProteinDatabase', 'value' )
   or setcwd( $elem, 'Config: WorkingDirectory', 'value' )

   # .prot.xml elements
   or setnewfile( $elem, 'protein_summary', 'summary_xml' )
   or (setdb( $elem, 'protein_summary_header', 'reference_database' ) |
       setfile( $elem, 'protein_summary_header', 'source_files' ) |
       setfile( $elem, 'protein_summary_header', 'source_files_alt' ))

   or 1;   # done.  use "or 1" to supress "useless use" msg

   print $outfh $elem->{txt};
   }

#
# Default handler to call
#
sub defaultTag 
   {
   my ( $p, $data ) = @_;
   print $outfh $p->original_string();
   }


#-- @SET FUNCTIONS ------------------------------------------------------------#

#
# Replace the attribute of a element with the current file path. Returns whether
# or not the current element was recognized.
#
sub setnewfile
   {
   my ( $e, $name, $attr ) = @_;

   return 0 if ( $e->{name} ne $name );

   my $old = getattr( $e, $attr );
   initparts( $old ) unless ( $origbase );              # already done?
   setattr( $e, $attr, $old, $newfile );
   return 1; 
   }

#
# Replace the attribute of a element to the basename of the file.  Returns
# whether or not the current element was recognized.
#
sub setbase
   {
   my ( $e, $name, $attr ) = @_;

   return 0 if ( $e->{name} ne $name );

   my $old   = getattr( $e, $attr );
   my @paths = findfile( $e, $old, ".mzML", ".mzXML", ".mzML.gz", ".mzXML.gz" );
   if ( !@paths )
      {
      logwarn( "$name\@$attr file not found, not updated" );
      }
   elsif ( $paths[0] eq $old  )
      {
      loginfo( "$name\@$attr skipped, file exists at original path" );
      }
   else
      {
      ( @paths > 1 ) && logwarn( "$name\@$attr found multiple files, using first found" );
      $paths[0] =~ s/\.[^.]+$//;          # w/o extension
      setattr( $e, $attr, $old, $paths[0] );
      }

   return 1;
   }

#
# Replace the attribute of a element with a new file path.  Returns whether or
# not the if the current element was recognized.
#
sub setfile
   {
   my ( $e, $name, $attr ) = @_;

   return 0 if ( $e->{name} ne $name );

   my $oldfile  = getattr( $e, $attr );

   # Special case: was it set to the current file (w/o extension)?
   my @oldparts = splitpath( $oldfile );
   $oldparts[2] =~ s/\..*$//;
   if ( $oldparts[2] eq $origname ) 
      {
      my $new = $oldparts[1] ? rel2abs($file) : basename($file);
      setattr( $e, $attr, $oldfile, $new );
      return 1;
      }

   # Lets go looking
   my @paths = findfile( $e, $oldfile );
   if ( !@paths )
      {
      logwarn( "$name\@$attr not updated, original file not found" );
      }
   elsif ( $paths[0] eq $oldfile )
      {
      loginfo( "$name\@$attr not updated, file exists" );
      }
   else
      {
      setattr( $e, $attr, $oldfile, $paths[0] );
      ( @paths > 1 ) && logwarn( "$name\@$attr found multiple files, using first found" );
      }

   return 1;
   }

#
# Replace a value of an attribute of a element with the new database location.
# Returns true if element was recognized.
#
sub setdb
   {
   my ( $e, $name, $attr ) = @_;

   return 0 if ( $e->{name} ne $name );
   my $db = getattr( $e, $attr );
   if ( $opts{database} )               # user provided a new path to db?
      {
      if ( $origdb && $origdb ne $db )
         {
         logwarn( "$name\@$attr more than one database found, skipping update" );
         }
      else
         {
         setattr( $e, $attr, $db, $opts{database} );
         $origdb = $db if ( !$origdb );
         }
      }
   elsif ( ! -f $db )
      {
      logwarn( "database file '$db' not found, use -d to set" );
      }
   return 1; 
   }

#
# Check for multiple database entries and warn about being unable to
# fix them.
#
sub warndb
   {
   my ( $e, $name, $attr ) = @_;

   return 0 if ( $e->{name} ne $name );

   if ( $opts{database} )
      {
      logwarn( "more that one database source found, only the first will be updated" );
      }
   else
      {
      logwarn( "more that one database source exists" );
      }
   return 1;
   }

#
# Replace the attribute of a element with the current working directory. Returns
# whether or not the current element was recognized.
#
sub setcwd
   {
   my ( $e, $name, $attr ) = @_;

   return 0 if ( $e->{name} ne $name );
   my $old = getattr( $e, $attr );
   setattr( $e, $attr, $old, getcwd() ) if ( $old );
   return 1; 
   }



#-- @GENERAL ------------------------------------------------------------------#

#
# Processes the command line arguments and initialize the global %opts
# variable.
#
sub parseOptions
   {
   # Get options...
   my @flags;
   push @flags, qw( help|h|? man version|V verbose|v+ );        # Standard flags
   push @flags, qw( dry-run|recon|n database|d=s paths|p=s backup|b=s );
   Getopt::Long::Configure( "bundling" );
   GetOptions( \%opts, @flags ) || pod2usage(2);
   $opts{verbose} = 0 unless ( $opts{verbose} );
   $opts{paths} = [ split ':', $opts{paths} ] if ( $opts{paths} );

   # Standard flags
   pod2usage(2)               if ( $opts{help} );
   pod2usage( -verbose => 2 ) if ( $opts{man} );
   printVersion() && exit     if ( $opts{version} );

   # Check for input files
   pod2usage( "$prog error: missing pep.xml/prot.xml file" ) unless @ARGV;
   foreach ( @ARGV )
      {
      pod2usage( "$prog error: expected .pep.xml/.prot.xml input file" )
         unless ( /\.(pep.xml|prot.xml)$/ );
      pod2usage( "$prog error: can't read file $_" ) unless -r $_;
      }

   if ( $opts{database} && ! -f $opts{database} )
      {
      logwarn( "(-d) database file wasn't found" );
      }
   }

#
# Output program version
#
sub printVersion
   {
   print STDOUT "$prog $VERSION (revison ${REVISION})\n";
   }

#
# Outputs information to STDOUT, only if the verbose flag is set
#
sub loginfo
   {
   print STDOUT "$prog info: ", @_, "\n" if ( $opts{verbose} ); 
   }

#
# Calls warn with program context
#
sub logwarn
   {
   warn( "$prog warning: ", @_, "\n" )
   }

#
# Calls die with program context
#
sub logdie
   {
   die( "$prog error: ", @_, "\n" )
   }
   
#
# Find the longest common prefix between strings
#
sub lcd 
   {
   # URLref: http://linux.seindal.dk/2005/09/09/longest-common-prefix-in-perl
   # find longest common prefix of scalar list
   my $prefix = shift;
   for ( @_ ) 
      {
      chop $prefix while ( !/^\Q$prefix\E/ );
      }
   return $prefix;
   }


#-- @DOCUMENTATION ----------------------------------------------------------#

__END__

=head1 NAME

updatepaths - update file paths found in pepXML/protXML files

=head1 DESCRIPTION

This program can be used to update file paths embedded in pepXML and protXML
formatted proteomics files used by TPP.  These file paths can normally refer to
the file itself, input files, output files, database files, parameters,
etc.. These file paths can often get out of sync with the filesystem when
projects are renamed, moved, or otherwise changed.

Some file paths, such as to the current file, are easy to update.  While others
can be exceedingly difficult to correct all occurences, such as the list of
input files to InteractParser, as each file can have different directory parts.
The B<updatepaths> program does its best attempt at deriving the correct path
for each of the file paths to update based on the current file location, the
parameters provided, and whether or not the file/directory exists.

=head1 SYNOPSIS

updatepaths [options] [<file>.pep.xml|<file>.prot.xml]

 Options:
   -h, --help           Print this help
   --man                Display full documentation and exit
   -V, --version        Print the version information and exit
   -v, --verbose        Enable verbose output 
   -n, --dry-run        Don't actually update any paths

   -p, --paths <dirs>   List of directory paths to search for files
   -b, --backup <ext>   Backup existing file with extension <ext>
   -d, --database <db>  Path to database file 

=head1 OPTIONS

=over 5

=item B<-h, --help>

Print a brief help message and exit.

=item B<--man>

Display the full man page documentation and exit.

=item B<-V, --version>

Output version information for this program and exit.

=item B<-v, --verbose>

Verbose mode.  Cause B<updatepaths> to print debugging messages about its 
progress. Multiple -v options increase the verbosity.  The maximum is 2.

=item B<-n, --dry-run>

Dry run.  Don't actually make any updates of the paths in the file(s).

=item B<-p, --paths I<list>>

A colon(":") delimited list of directory paths that should be searched when
trying to derive the correct path of a file to update to.  In cases where
the correct file path isn't easily determined B<updatepaths> will iteratively
try combinations of the paths and the file names (and some cases extensions) 
and use the first file that it finds exists.

=item B<-b, --backup I<extension>>

Saves a backup copy of the original file before updating.  The backup file will
have the extension provided. Only valid when the B<--dry-run> option is not used.

=item B<-d, --database>

Updates the path to the database (if present in the file).  In the case that
there are more than one databases present only the first entry is updated.

=back

=head1 XML ELEMENTS RECOGNIZED

The following is a list of elements and descriptions that B<updatepaths> 
recognizes for updating.

=head2 Elements common to most search engine pepXML files

=over 2

=item * C<xml-stylesheet/href> - full path to the current file but with an 
pep.xsl extension instead of the usual pep.xml.  In the case of X!Tandem this
is either a partial or full URL to the file pepXML_std.xsl.

=item * C<msms_pipeline_analysis/summary_xml> - full path to the current file.

=item * C<msms_run_summary/base_name> - full path to search input file without
the filename extension.  Directory part of base_name may not match directory
part of original file.

=item * C<search_summary/base_name> - full path to the search input file without
the filename extension, except in the case of OMSSA which may also have an 
extension.

=item * C<search_database/local_path> - full path to database file used in
search.

=item * C<interact_summary/filename> - full path to a input file. If
this element appears in output from a seach engine then its likely 
InteractParser was run on it to correct problems with the xml. In this case it
then usually has the extension .pepXML and points to the current file.  This
special case only occured with certain versions of TPP.

=item * C<interact_summary/directory> - blank.  See C<interact_summary/filename>.

=back

=head2 Elements unique to X!Tandem pepXML files

=over 2

=item * C<parameter/name="list path, taxonomy information"> - full path to the
taxonomy file used in the search.

=item * C<parameter/name="output, path"> - usually the full path to the current
file with a .tandem extension.

=item * C<parameter/name="output, sequence path"> - usually the full path to the
current file with a output_sequences extension.

=item * C<parameter/name="spectrum, path"> - the full path to input
mzML/mzXML file.

=item * C<parameter/name="list path, default parameters"> - the full path to
parameters file used in the search.

=item * C<parameter/name="list path, sequence source #1"> - the full path to
first database file searched.  In the case more than one database file was
provided the source number is incremented.

=back

=head2 Elements unique to Comet pepXML files

=over 2

=item * C<parameter/name="database_name"> - full path to the database file used
in the search.

=back

=head2 Elements unique to Myrimatch pepXML files

=over 2

=item * C<parameter/name="Config: ProteinDatabase"> - full path to the database
file used in the search.

=item * C<parameter/name="Config: WorkingDirectory"> - Full path of directory
search was invoked in.

=back

=head2 Elements found in InteractParser pepXML files

Since InteractParser merges one or more pepXML files most of the filename
elements are copied straight from the input pep.xml files and aren't modified by 
InteractParser.  The same holds true for files updated/produced by PeptideProphet
and InterProphet programs.  In the case where certain elements are different
(such as input file tags) each tag is copied resulting in lists of element tags.
This of course presents a challenge to update as its possible that the directory
parts of the file paths are different.

=over 2

=item * C<xml-stylesheet/href> - full path to the current file but with an 
pep.xsl extension instead of the usual pep.xml.

=item * C<database_refresh_timestamp/database> - full path to the database
searched.

=item * C<inputfile/name> - list of full paths to input files without an
extension.

=item * C<interact_summary/filename> - full path to the current file.

=item * C<msms_run_summary/base_name> - list of full paths to inputs without an
extension.

=back

=head2 Elements found in ProteinProphet protXML files

=over 2

=item * C<protein_summary/summary_xml> - full path to the current file.

=item * C<protein_summary_header/reference_database> - full path to database
file.

=item * C<protein_summary_header/source_files> - full path to source input file.

=item * C<protein_summary_header/source_files_alt> - full path to source input
file.

=back

=head1 ACTIONS TAKEN ON ELEMENTS

The following table outlines what actions are performed on each element 
recognized.

                                    comet.pep.xml
                                    | myrimatch.pep.xml
                                    | | omsssa.pep.xml
                                    | | | tandem.pep.xml
                                    | | | | interact-srch-prob.pep.xml
                                    | | | | | interact-srch-ipro.pep.xml
                                    | | | | | | interact-ipro.pep.xml
                                    | | | | | | | interact-ipro.prot.xml
                                    | | | | | | | |
Element                             | | | | | | | |   Action(s)
=============================       ===============   =================
xml-stylesheet/href                 X X X X X         CURFILE
msms_pipeline_analysis/summary_xml  X X X X X X X     CURFILE
msms_run_summary/base_name          X X X X L L L     FINDFILE
search_summary/base_name            X X X X L L L     FINDFILE
search_database/local_path          X X X X L L L     DBFILE
interact_summary/filename+directory O O O   X X X     CURFILE/CWD
inputfile/name                      O O O             CURFILE
inputfile/name                              L L L     FINDFILE
database_refresh_timestamp/database         # # #     DBFILE/DBWARN
-- X!Tandem only --
output, path                              X # # #     FINDFILE
output, sequence path                     X # # #     FINDFILE
output, spectrum path                     X # # #     FINDFILE
list path, taxon information              X # # #     FINDFILE
list path, default parameters             X # # #     FINDFILE
list path, sequence source #1             X # # #     DBFILE 
list path, sequence source #2             X # # #     DBWARN
-- Comet only --
database_name                       X       # # #     DBFILE/DBWARN
-- Myrimatch only --
Config: ProteinDatabase               X     # # #     DBFILE/DBWARN
Config: WorkingDirectory              X     # # #     DBFILE/DBWARN
-- protXML only --
protein_summary/summary_xml                       X   CURFILE
protein_summary/reference_database                X   DBFILE
protein_summary/source_files                      X   FINDFILE

   x - indicates element is found in the file
   O - indicates the element may be found in the file
   L - indicates list of these elements is found in the file
   # - indicates list of these elements may be found in the file

   CURFILE  - Replace with the CURRENT file path
   FINDFILE - Search for the file and replace with first path found
   DBFILE   - Replace with (-d), otherwise warn if the file doesn't exist
   DBWARN   - Warn if more than one database path seen
   CWD      - Replace with current working directory

=head1 EXAMPLES

To correct the paths in a directory containing X!Tandem results invoke the 
command:

=over 5

S<updatepaths xtandem/*.pep.xml>

=back

Example of updating a set of files, verbosely reporting what it is updating,
and saving the original files with a .bak extension:

=over 5

S<updatepaths -vv -b.bak comet/*.pep.xml>

=back

The following command invoked on a InterProphet file:

=over 5

S<updatepaths -p data:/local/databases: interact-ipro.pep.xml>

=back

will update all of the filename paths in the file, searching the directories
data/ and /local/databases for files referenced in the pep.xml


=head1 AUTHORS

Joe Slagel E<lt>jslagel@systemsbiology.orgE<gt>

=cut

X!Tandem Parameter Tags
   list path, taxonomy information    : full path to taxonomy file
   output, path                       : full path to file with .tandem
   output, sequence path              : full path to file with .output_sequences
   spectrum, path                     : full path to .mzML file
   list path, default parameters      : full path to parameters file
   list path, sequence source #1      : full path to database 1st file
   list path, sequence source #2      : full path to database 1st file ...

Myrimatch Parameter Tags
   Config: ProteinDatabase            : full path to database file
   Config: WorkingDirectory           : full directory path (of file?)

InteractParser/PeptideProphet/InterProphet tags:

   xml-stylesheet/href               : full path to pep.xml file with .pep.xsl 
   database_refresh_timestamp/database: full path to database file
   inputfile/name                     : list of full paths to inputs w/o extension
   interact_summary/filename          : full path to pep.xml file
   msms_run_summary/base_name         : list of full paths to inputs w/o extension
   list path, default parameters      : full path to parameters file
   list path, sequence source #1      : full path to database 1st file ...
   list path, taxonomy information    : full path to taxonomy file
                                        copied from X!Tandem searches (more than 1?)
   output, path                       : list of full paths to file with .tandem
                                        copied from X!Tandem searches
   output, sequence path              : list of full paths to file with .output_sequences
                                        copied from X!Tandem searches
   spectrum, path                     : list of full paths to .mzML file
                                        copied from X!Tandem searches
   search_database/local_path         : full path to database file
   search_summary/base_name           : list of full paths to pep.xml file w/o extension

interact-ipro.prot.xml tags:

   protein_summary/summary_xml : full path to file
   protein_summary_header/refence_database : full path to database file file
   protein_summary_header/source_files     : full path to input .pep.xml file 
   protein_summary_header/source_files_alt : full path to input .pep.xml file 

xml-stylesheet/href                : full path to pep.xml file with .pep.xsl 

X!Tandem Parameter Tags
   list path, taxonomy information    : full path to taxonomy file
   output, path                       : full path to file with .tandem
   output, sequence path              : full path to file with .output_sequences
   spectrum, path                     : full path to .mzML file
   list path, default parameters      : full path to parameters file
   list path, sequence source #1      : full path to database 1st file
   list path, sequence source #2      : full path to database 1st file ...

Myrimatch Parameter Tags
   Config: ProteinDatabase            : full path to database file
   Config: WorkingDirectory           : full directory path (of file?)

InteractParser/PeptideProphet/InterProphet tags:

   xml-stylesheet/href               : full path to pep.xml file with .pep.xsl 
   database_refresh_timestamp/database: full path to database file
   inputfile/name                     : list of full paths to inputs w/o extension
   interact_summary/filename          : full path to pep.xml file
   msms_run_summary/base_name         : list of full paths to inputs w/o extension
   list path, default parameters      : full path to parameters file
   list path, sequence source #1      : full path to database 1st file ...
   list path, taxonomy information    : full path to taxonomy file
                                        copied from X!Tandem searches (more than 1?)
   output, path                       : list of full paths to file with .tandem
                                        copied from X!Tandem searches
   output, sequence path              : list of full paths to file with .output_sequences
                                        copied from X!Tandem searches
   spectrum, path                     : list of full paths to .mzML file
                                        copied from X!Tandem searches
   search_database/local_path         : full path to database file
   search_summary/base_name           : list of full paths to pep.xml file w/o extension

interact-ipro.prot.xml tags:

   protein_summary/summary_xml : full path to file
   protein_summary_header/refence_database : full path to database file file
   protein_summary_header/source_files     : full path to input .pep.xml file 
   protein_summary_header/source_files_alt : full path to input .pep.xml file 

xml-stylesheet/href                : full path to pep.xml file with .pep.xsl 
msms_pipeline_analysis/summary_xml : full path to pep.xml file
msms_run_summary/base_name         : full path to pep.xml file w/o extension
search_summary/base_name           : full path to pep.xml file w/o extension
                                   | full path to pep.xml file with .pepXML (OMSSA)
search_database/local_path         : full path to database file
interact_summary/filename          : full path to pep.xml file
                                   (found in xinteract "fixed" files)
inputfile/name                     : full path to pep.xml file 
                                   | full path to pep.xml file with .pepXML
                                   | full path to input files w/o extension
