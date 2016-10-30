rem usage: install-win32 <vc6|vc71|vc8> <build directory>

set VC_VER=%1
copy /y ..\..\win_lib\STLport-4.6.2\lib\stlport_%VC_VER%46.dll %2
copy /y ..\..\win_lib\zlib123-dll\zlib1.dll %2
copy /y ..\..\win_lib\UnxUtils\tpp\* %2
copy /y ..\..\win_lib\bsdtar-1.2.38-bin\bin\* %2
set VC_VER=