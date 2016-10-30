
IF EXIST apache-w32-2.2.8 GOTO APACHE-OK
ECHO "extracting apache..."
bsdtar-1_2_38.exe -jxf apache-w32-2.2.8.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting apache"
ECHO
:DONE
:APACHE-OK


IF EXIST bsdtar-1.2.38-bin GOTO BSDTAR-OK
ECHO "extracting bsdtar..."
bsdtar-1_2_38.exe -jxf bsdtar-1.2.38-bin.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting bsdtar"
ECHO
:DONE
:BSDTAR-OK


IF EXIST gdwin32 GOTO GDWIN32-OK
ECHO "extracting gdwin32..."
bsdtar-1_2_38.exe -jxf gdwin32.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting gdwin32"
ECHO
:DONE
:GDWIN32-OK

IF EXIST libw32c GOTO LIBW32C-OK
ECHO "extracting libw32c..."
bsdtar-1_2_38.exe -jxf libw32c.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting libw32c"
ECHO
:DONE
:LIBW32C-OK

IF EXIST PerlSupport GOTO PERLSUPPORT-OK
ECHO "extracting PerlSupport..."
bsdtar-1_2_38.exe -jxf PerlSupport.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting PerlSupport"
ECHO
:DONE
:PERLSUPPORT-OK

IF EXIST setacl-cmdline GOTO SETACL-OK
ECHO "extracting setacl-cmdline..."
bsdtar-1_2_38.exe -jxf setacl-cmdline.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting setacl-cmdline"
ECHO
:DONE
:SETACL-OK

IF EXIST UnxUtils GOTO UNXUTILS-OK
ECHO "extracting UnxUtils..."
bsdtar-1_2_38.exe -jxf UnxUtils.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting UnxUtils"
ECHO
:DONE
:UNXUTILS-OK

IF EXIST xsltproc GOTO XSLTPROC-OK
ECHO "extracting xsltproc..."
bsdtar-1_2_38.exe -jxf xsltproc.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting xsltproc"
ECHO
:DONE
:XSLTPROC-OK

IF EXIST zlib125-dll GOTO ZLIBDLL-OK
ECHO "extracting zlib125-dll..."
bsdtar-1_2_38.exe -jxf zlib125-dll.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting zlib125-dll"
ECHO
:DONE
:ZLIBDLL-OK

IF EXIST zlib-1.2.5 GOTO ZLIB-OK
ECHO "extracting zlib-1.2.5..."
bsdtar-1_2_38.exe -jxf zlib-1.2.5.tar.bz2 2>tar_log
del tar_log
ECHO "done extracting zlib-1.2.5"
ECHO
:DONE
:ZLIB-OK

IF EXIST fann-2.0.0 GOTO FANN-OK
ECHO "extracting fann..."
bsdtar-1_2_38.exe -zxf fann-2.0.0.tar.gz 2>tar_log
del tar_log
ECHO "done extracting fann"
ECHO
:DONE
:FANN-OK

IF EXIST ../build/mingw-i686/gsl-1.14/gsl/gsl_blas.h GOTO GSL-OK
ECHO "Unfortunately you need to do a mingw-based build first to get GSL installed and configured.  That's just how it ships, sorry.  But MinGW is actually the only officially supported windows build system for TPP, anyway.  See README_WIN.txt in the trans_proteomic_pipeline directory for details."
EXIT 1
:DONE
:GSL-OK
IF EXIST ../build/msvc/gsl-1.14/config.h GOTO GSL-CONFIG-OK
mkdir ..\build\msvc
xcopy ..\build\mingw-i686\gsl-1.14 ..\build\msvc\gsl-1.14 /y /q /i /s
c:\perl\bin\perl.exe -pe "s/#define HAVE_INLINE/\/\/#define HAVE_INLINE/g" ../build/mingw-i686/gsl-1.14/config.h > ../build/msvc/gsl-1.14/config.h
echo #ifndef INLINE_DECL  >> ../build/msvc/gsl-1.14/msvc/gsl/gsl_inline.h
echo #define INLINE_DECL  >> ../build/msvc/gsl-1.14/msvc/gsl/gsl_inline.h
echo #endif  >> ../build/msvc/gsl-1.14/msvc/gsl/gsl_inline.h
:GSL-CONFIG-OK

ECHO "win_lib directories ok"


