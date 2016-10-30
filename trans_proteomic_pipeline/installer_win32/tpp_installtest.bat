@ECHO off
cls
ECHO a quick check to see if the TPP installation process went OK:
cd c:\inetpub\tpp-bin
ECHO checking path... should include "c:\perl(64)?\bin;C:\Inetpub\tpp-bin;"
path > test.txt
grep -iE "perl(64)?\\bin" < test.txt > test2.txt
IF %ERRORLEVEL% == 0 GOTO check1
path
ECHO ---
ECHO path does not look right - where is c:\perl\bin?  
ECHO try a reboot
pause
GOTO end1
:check1
grep -i c\:\\Inetpub\\tpp-bin test.txt  > test2.txt
IF %ERRORLEVEL% == 0 GOTO OK1
path
ECHO ---
ECHO path does not look right - where is C:\Inetpub\tpp-bin?
ECHO try a reboot
pause
GOTO end1
:OK1
ECHO OK
:end1
ECHO ---
ECHO checking perl... should say perl\bin\perl.exe or perl64\bin\perl.exe
which perl.exe > test.txt
grep -iE "perl(64)?\\bin\\perl.exe" test.txt
IF %ERRORLEVEL% == 0 GOTO OK2
ECHO ---
grep -i cygwin test.txt
IF %ERRORLEVEL% == 0 GOTO badcyg
GOTO bad2
:badcyg
ECHO looks like the cygwin perl may be in the way, move c:\cygwin\bin to end of path
:bad2
ECHO perl install looks bad - where is c:\perl\bin\perl.exe?
ECHO try a reboot
pause
GOTO end2
:OK2
ECHO OK
:end2
ECHO ---
ECHO checking webserver... should say "`test.txt' saved"
wget -O test.txt http://localhost/tpp-bin/check_env.pl 2> test2.txt
grep -i saved test2.txt
IF %ERRORLEVEL% == 0 GOTO OK3
type test2.txt
ECHO could not get file from http://localhost/tpp-bin - is webserver running and perl installed?
ECHO if web server is running and perl is installed try a reboot
pause
GOTO end3
:OK3
ECHO OK
:end3
ECHO ---
ECHO checking xinteract installation...
xinteract -installtest | grep OK
IF %ERRORLEVEL% == 0 GOTO end4
xinteract -installtest
ECHO try a reboot
pause
:end4
