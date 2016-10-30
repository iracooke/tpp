#!/bin/csh
# Run Mayu on a PeptideProphet or iProphet output file.
# Removes old Mayu output files, changes to the Mayu directory,
# runs Mayu on input pepXML file, moves files to directory containing input file,
# and prints a useful summary.
#
# Terry Farrah  Institute for Systsems Biology  July 18, 2009
# tfarrah@systemsbiology.org

# get database name from pepXML file
if ($#argv != 1) then
  echo "Usage: $0 <pepXML>"
  echo "Runs Mayu protein FDR analysis on a PepPro or iProphet file."
  goto done
endif
set PEPXML=$1
if ( ! $?DATABASE ) then
  set DATABASE=`perl -e 'while (<>) {if ($_ =~ /<search_database local_path="(\S*?)" type="AA"/) {print "$1\n"; exit;}}' < $PEPXML`
endif
if ( $?DATABASE ) then
  set DECOY_PREFIX="DECOY_"
  set BASEDIR=`dirname $1`
  if ( BASEDIR == '.' ) then
    set BASEDIR=$cwd
  endif
  set PEPXML=`basename $PEPXML`
  set DIRNAME=`basename $BASEDIR`
  set MAYU_DIR=/regis/sbeams/bin/Mayu
  echo "Mayu decoy-based protein FDR analysis on $PEPXML"
  /bin/rm -f Mayu.log Mayu_out.txt Mayu_out.csv
  # Mayu must be run from Mayu dir.
  # Name output files (-M option) after working directory to avoid collisions.
  #echo "  Change to ${MAYU_DIR} and run Mayu"
  cd ${MAYU_DIR}
  set cmd="perl Mayu.pl -A ${BASEDIR}/$PEPXML"
  set cmd="$cmd -C ${DATABASE} -E ${DECOY_PREFIX} "
  # -G gives the maximum PSM FDR to consider.
  # -H: at how many intervals to report peptide, protein FDR
  set cmd="$cmd -G 0.003 -H 16 "
  set cmd="$cmd -verbose "
  set cmd="$cmd -PmFDR "
  set cmd="$cmd -M ${DIRNAME} "
  echo "Running the following command:"
  echo "$cmd >&  ${BASEDIR}/Mayu.log"
  $cmd >& ${BASEDIR}/Mayu.log
  #echo "  Move results to working dir"
  /bin/mv ${DIRNAME}*.csv ${BASEDIR}/Mayu_out.csv
  /bin/mv ${DIRNAME}*.txt ${BASEDIR}/Mayu_out.txt
  cd ${BASEDIR}
  # "csv" file is actually tab-delimited. Replace tabs with commas.
  sed -e 's/\t/,/g' -i Mayu_out.csv
  # Print some useful summary info
  echo
  echo "PSM FDR    Protein IDs    Decoys   Protein FDR (lower bound)"
  grep -v "decoy" Mayu_out.csv | awk '{FS=","; printf("%.4f      %8d     %6d      %.4f\n",$3,$14,$16,$19)}'
  echo "True protein ID count may be lower if redundancy in database."
  echo
  echo "Complete results in Mayu_out.csv, Mayu_out.txt, and Mayu.log."
  echo
else
  echo "$0 ERROR: could not extract database from $PEPXML."
endif
