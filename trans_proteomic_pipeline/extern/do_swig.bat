cd %1
IF EXIST swigwin-1.3.39\swig.exe GOTO RUNSWIG
UnxUtils\usr\local\wbin\unzip.exe swigwin-1.3.39.zip
:RUNSWIG
call VCVARS32.BAT
swigwin-1.3.39\swig.exe %2 %3 %4 %5 %6 %7 %8 %9
