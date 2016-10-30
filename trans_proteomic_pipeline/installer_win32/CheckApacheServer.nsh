;--------------------------------
; Check for Apache 
;

!include "ConfigApache.nsh"  

Var /GLOBAL apacheInstalled  ; set here

Function checkApacheServer

   StrCpy $apacheInstalled ""

   ; first thing is to check the registry - presence of config files proves nothing, could be old
   ClearErrors
   EnumRegKey $0 HKEY_LOCAL_MACHINE  "SOFTWARE\Apache Software Foundation\Apache" 0
   IfErrors NoApache 

   ; Apache 1.x.x
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "%ProgramFiles%\Apache Group\Apache\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe '"%ProgramFiles%\Apache Group\Apache\Apache.exe"'
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; Apache 2.0.x
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "%   rogramFiles%\Apache Group\Apache2\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe '"%ProgramFiles%\Apache Group\Apache2\bin\Apache.exe"'
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; Apache 2.2.x
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "%ProgramFiles%\Apache Software Foundation\Apache2.2\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe '"%ProgramFiles%\Apache Software Foundation\Apache2.2\bin\httpd.exe" -n Apache2.2'
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; Apache 2.3.x (doesn't exist yet, just a guess)
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "%ProgramFiles%\Apache Software Foundation\Apache2.3\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe '"%ProgramFiles%\Apache Software Foundation\Apache2.3\bin\httpd.exe" -n Apache2.3'
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; XAMPP
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "c:\xampp\apache\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe "c:\xampp\apache\bin\httpd.exe"
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; 32 bit Apache 2.0.x on 64 bit machines
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "%ProgramFiles(x86)%\Apache Group\Apache2\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe '"%ProgramFiles(x86)%\Apache Group\Apache2\bin\Apache.exe"'
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; 32 bit Apache 2.2.x on 64 bit machines
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "%ProgramFiles(x86)%\Apache Software Foundation\Apache2.2\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe '"%ProgramFiles(x86)%\Apache Software Foundation\Apache2.2\bin\httpd.exe" -n Apache2.2'
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; 32 bit Apache 2.3.x on 64 bit machines
   ClearErrors
   ExpandEnvStrings $ApacheConfFile "%ProgramFiles(x86)%\Apache Software Foundation\Apache2.3\conf\httpd.conf"
   ExpandEnvStrings $ApacheExe '"%ProgramFiles(x86)%\Apache Software Foundation\Apache2.3\bin\httpd.exe" -n Apache2.3'
   FindFirst $0 $1 $ApacheConfFile
   IfErrors 0 HasApache

   ; no Apache found
   Goto NoApache

HasApache:
   !insertmacro LogDetail "Apache Web server appears to be installed"
   StrCpy $apacheInstalled "installed"
   Goto DoneCheckApache

NoApache:
   !insertmacro LogDetail "Apache webserver is not installed or not right version"
   StrCpy $apacheInstalled "not installed"
   
DoneCheckApache:
FunctionEnd
