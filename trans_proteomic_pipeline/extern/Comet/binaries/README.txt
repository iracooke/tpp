2014/09/18

Comet version "2014.02 rev. 2".
This is a maintenance release.

Release notes:
- Fixes issue with header SQT output when static modifications are specified
  but no variable modifications are specified.  In this situation, a
  StaticMod header line is repeatedly printed out in a loop with garbage.


Comet version "2014.02 rev. 1".
This is a maintenance release.

Release notes:
- Report missing variable modifications in pep.xml header.


2014/09/16

Comet version "2014.02 rev. 0".
This is a full release of Comet.
http://comet-ms.sourceforge.net

Release notes:
- This release implements a change to how variable terminal modifications are
  specified. Both n- and c-terminal variable modifications are now specified the
  same way as amino acid modifications by using the letters 'n' and 'c'
  respectively i.e. "42.010565 nK" for acetylation. When reporting results, the
  peptide strings will look something like "K.n*DIGSESTEDK.A" for an n-terminal
  modification and "K.DIGSESTEDKc*.A" for a c-terminal modification. Previously,
  an n- or c-terminal modification was denoted by replacing the separation
  periods with brackets (] and [ respectively). Definitely look at the parameter
  help page for any of the variable mods (for example "variable_mod01") to see a
  description of the new options in these parameters as well as example uses.
- When searching a small database or using restrictive parameters such that only
  a small number of candidate peptides are analyzed, Comet will now require (and
  generate) 1000 xcorr scores for the E-value calculation, up from 500. This
  corrects a couple of reported examples where poor scoring identifications from
  sparse searches received artificially low E-values.
- Implement a memory pool that's shared between threads for the re-use of an
  array (pbDuplFragment). This change gives upwards of ~35% performance
  improvement in addition to better memory use from constantly creating and
  destroying the arrays. Implemented by T. Jahan.
- When auto-detecting the number of threads to spawn, Comet will now set this
  number to the value stored in the environment variable NSLOTS if that
  environment variable is set. This environment variable can be used by cluster
  management software such as SGE. Code changes submitted by J. Egertson.
- Extended the number of variable modifications from 6 to 9.
- Added minor Makefile changes to support compiling under msys/mingw;
  contributed by J. Slagel.
- Bug fix: correct initialization in custom amino acids (B, J, U, X, Z).
- Bug fix: Removed an unnecessary array initialization introduced in version
  2014.01.1. This caused searches to be upwards of 2X slower; most noticeable
  when using small "fragment_bin_tol" values.
- Bug fix: addressed a couple of esoteric pep.xml output issues (paths and such)
  when using the -N<name> command line option.
- Bug fix: fix the StaticMod output string in SQT format header (H) line; was
  not printing out the modification residue and this header lines were not
  present in the decoy output.
- Bug fix: fix the "override_charge", parameter. The charge ranges were being
  searched but these were in addition to the existing charge states specified in
  the input file. The logic has been corrected so that only the specified charge
  range will be searched.
- New parameters: All of the variable_modXX parameters (for example
  "variable_mod01") have been modified to add two new options for an optional
  terminal distance constraint and which terminus that distance constraint is
  applied to. And the nubmer of variable mods has been extended to 9. The
  modification character codes for mods 1 through 9 are: *#@^~$%!+
- New parameter "output_percolator". Percolator, I believe as of version 2.08,
  no longer supports the Percolator-in or pin.xml format. The supported input
  format is now a tab-delimited file hence the parameter name change.
- Removed the parameter "output_pinxml". See the replacement parameter
  "output_percolator".
- Removed the parameter "precursor_tolerance_type". The implementation of this
  parameter had a bug and it turns out this parameter was simply not needed.
- Update MSToolkit to version r72.


Comet is an open source MS/MS database search engine released under the
Apache 2.0 license.

Current supported input formats are mzXML, mzML, ms2, and cms2.
Current supported output formats are pepXML, Percolator-IN XML, SQT,
tab-delimited text, and .out files.

To run a search on an input file requires the Comet binary, a search
parameters file (comet.params), an input file, and a protein sequence
database in FASTA format.  The syntax of a search:

   comet.exe input.mzXML
   comet.exe input.ms2

Search parameters, such as which sequence database to query, modifications to
consider, mass tolerances, output format, etc. are defined in the comet.params
file.

One can generate a parameters file with the command

   comet.exe -p

Rename the created file "comet.params.new" to "comet.params".

Windows and linux command line binaries are included with this release.
