# Microsoft Developer Studio NMAKE File for tweaking perl scripts for VC8
# the meat of it is in perl_paths.inc
# there's a *nix version of this makefile in perl_paths.makefile

QT="
SRC_ROOT=../
XFORM="s/\/cygdrive\/c\/Inetpub\/tpp-bin\//c:\x5c\x5cInetpub\x5c\x5ctpp-bin\x5c\x5c/g;s/\/cygdrive\/c/c\:/g;s/cygpath \-wp //g;s/\/usr\/bin\/perl/c\:\/perl\/bin\/perl/g;s/\x27\/usr\/bin\//\x24\{base_dir\}\.\x27/g;s/\'\/bin\//\'/g;s/\/usr\/bin\///g;s/tpp\/cgi\-bin/tpp\-bin/g;s/tar /bsdtar /g;s/xzOf/\-xzOf/g"

!INCLUDE perl_paths.inc