!ifndef _checkPerl_nsh
!define _checkPerl_nsh
 
 ;--------------------------------
;Check for Perl
Function checkPerl

  ; check for ActiveState perl, 5.8.824 or higher


  CheckActivePerlVersion:
    ; MessageBox MB_OK "Checking version of perl"
    ; Use perl's "use version" feature to check version
    ExecWait 'perl -M5.8.8 -e "exit 0"' $0
    ; MessageBox MB_OK "Check perl version test returned $0"
    StrCmp $0 0 FoundGoodVersion BadActivePerlVersion

  FoundGoodVersion:
    ;MessageBox MB_OK "ActivePerl 5.8.8 or higher appears to be installed-- good"
    goto ActivePerlDone

  BadActivePerlVersion:
    MessageBox MB_OK|MB_ICONEXCLAMATION \
       "TPP requires perl version 5.8.8 or greater (available for free \
       from ActiveState, but we are not allowed to distribute it directly). \
       Please install ActivePerl from: $\r$\n \
       $\r$\n \
       http://www.activestate.com/activeperl/downloads $\r$\n \
       $\r$\n \
       and try the TPP installer again." IDOK abort
    
  ActivePerlDone:
    ;MessageBox MB_OK "ActivePerl 5.8.8 or higher appears to be installed-- good!"

  StrCpy $1 "" ; retry loop checker
  StrCpy $2 "" ; retry loop checker
  StrCpy $3 "c:\xampp\Perl\bin\perl.exe" ; least likely perl exe
  StrCpy $4 "c:\xampp\Perl\bin"  ; least likely perl dir
  IfFileExists $3 checkingPerl 0 
    StrCpy $3 "c:\Perl\bin\perl.exe" ; most likely perl exe
    StrCpy $4 "c:\Perl\bin"  ; most likely perl dir
  checkingPerl:
  SearchPath $0 "perl.exe"
  !insertmacro LogDetail "searching for perl.exe... found $0"
  StrCmp $0 "" 0 +2
    StrCpy $0 "perl.exe"
  push $0
  push "cygwin"
  Call StrStr ; assumes this has been defined, possibly in add_to_path.nsh
  Pop $R1
  StrCmp $R1 "" 0 cyg_perl  ; is the perl we found the cygwin version?
  nsExec::Exec '"$0" -v'
  Pop $R1
  StrCmp $R1 0 done no_perl ; exec returned 0 code ie success
  cyg_perl:
    StrCmp $2 "1" no_perl ; been here already?
    !insertmacro LogDetail "Found Cygwin Perl, which is not harmful but not suitable for use with ${PRODUCT_NAME}"
    ; try bumping perl up the path
    Push "c:\Perl\bin\"
    Call RemoveFromPath
    Push $4
    Call RemoveFromPath
    Push $4
    Call AddToPath
    ; and check again
    StrCpy $2 "1" 
    Goto checkingPerl
  no_perl:
    StrCmp $1 "1" last_try_perl ; been here already?
	StrCmp $2 "1" 0 install_perl ; did we hit cygwin?
    IfFileExists $3 0 install_perl
	MessageBox MB_OKCANCEL "It looks like you may have a proper Perl installed, but there is trouble with your PATH and the Cygwin Perl is getting in the way.  Click OK if you actually don't have Perl and want to install it now, or if you do have Perl installed then click Cancel and retry this installer. (This allows the corrected PATH to take effect.)" IDOK install_perl
	MessageBox MB_ICONINFORMATION|MB_OK "Installation is not complete, please rerun this installer.  If you are running this installer from a command line window, close that window to enable the new PATH to take effect."
	!insertmacro LogAbort ""
  install_perl:
    ; is this Vista?
    ReadRegStr $1 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion 
    StrCmp $1 '6.0' 0 vistacheckdone
    ; yes, recommend xampp
    MessageBox MB_OKCANCEL "For Vista we strongly recommend following the instructions at http://www.insilicos.com/IPP_Vista_Instructions.pdf before proceeding with this installation in order to avoid problems with the operation of the web GUI and other Perl-based programs.  Click OK to continue once you have reviewed those instructions."  IDOK vistacheckdone
    !insertmacro LogAbort ""
    vistacheckdone:

  perl_path:
    Push "c:\Perl\bin"
    Call AddToPath
	StrCpy $1 "1" 
	Goto checkingPerl
  last_try_perl:
    IfFileExists "c:\Perl\bin\perl.exe" done 0 ; well, it's there - assume it's going to be OK
    !insertmacro LogAbort "unable to install Perl - try installing from www.activestate.com"

  abort:
    !insertmacro LogAbort "Perl installation/path configuration did not complete"
    Abort

  done:
    !insertmacro LogDetail "Perl installation is OK"




FunctionEnd


!endif ; _checkPerl_nsh 