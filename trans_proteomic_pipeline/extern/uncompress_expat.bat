
IF EXIST expat-2.0.1 GOTO DONE

call VCVARS32.BAT
ECHO "extracting expat library source code..."
bsdtar-1_2_38.exe -xzf expat-2.0.1.tar.gz 2>foo
ECHO "done extracting expat source code"
ECHO
del foo

:DONE
