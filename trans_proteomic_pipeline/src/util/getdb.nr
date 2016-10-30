#!/bin/csh


#
# get nr from NCBI ----------
#
ftp -n <<DONE_WITH_FTP
open ftp.ncbi.nlm.nih.gov
user anonymous email@yourdomain.com
bin
prompt
get /blast/db/FASTA/nr.gz nr.fasta.gz
get /blast/db/blast.txt nr.README
DONE_WITH_FTP

#
# uncompress downloaded file  ----------
#
rm -f nr.fasta
gunzip nr.fasta.gz

#
# add escaping of special chars ', ", & and substitute < to [ and > to ] in def lines
# warning, substitution is expensive aka slow; comment out next two commands
# if you can't afford the time and deal with the special chars yourself
#
sed -e '/^>/s/&/\&amp;/g' \
    -e "/^>/s/'/\&apos;/g" \
    -e '/^>/s/"/\&quot;/g' \
    -e '/^>/s/</[/g' \
    -e '/^>/s/>/]/g' \
    -e 's/^]\(.*\)$/>\1/' < nr.fasta > nr.fasta2
mv -f nr.fasta2 nr.fasta

#
# append date to filenames  ----------
#
set currdate=`date +%Y%m%d`

rm -f nr.fasta.$currdate
mv nr.fasta nr.fasta.$currdate
mv nr.README nr.README.$currdate

