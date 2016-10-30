cd %1
if NOT "%2"=="sloppy" GOTO PREPARE
IF EXIST boost_1_45_0\stage\lib\libboost_zlib-vc90-mt-sgd.lib GOTO DONE
:PREPARE
call VCVARS32.BAT
IF EXIST bjam.exe GOTO DIRCHECK
UnxUtils\usr\local\wbin\unzip.exe bjam.zip
:DIRCHECK
IF EXIST boost_1_45_0 GOTO BUILD
ECHO "extracting boost source code; this may take a few minutes..."
bsdtar-1_2_38.exe -xjf  boost_1_45_0.tar.bz2 2>foo
ECHO "done extracting boost source code"
ECHO
del foo
:BUILD
cd boost_1_45_0
ECHO "buildling boost libraries; this may take some time..." 
..\bjam --toolset=msvc "-sNO_ZLIB=0" -sZLIB_SOURCE="%1/zlib-1.2.5" release debug link=static threading=multi runtime-link=static --with-regex --with-filesystem --with-iostreams --with-thread --with-program_options --with-serialization --with-system --with-date_time stage 
ECHO "done building boost libraries"
ECHO
UnxUtils\usr\local\wbin\rm.exe -rf bin.v2
cd %1
:DONE
