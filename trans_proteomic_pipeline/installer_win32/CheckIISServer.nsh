;--------------------------------
; Check for IIS
;

;!include "CreateVDir.nsh"

Function checkIISServer

   ClearErrors
	
   ; This is built off MSFT's required keys for IIS
   ; (info at http://nsis.sf.net/wiki)
   ; and the NSIS Wiki (http://nsis.sf.net/wiki).
   ReadRegDWORD $0 HKLM "SOFTWARE\Microsoft\InetStp" "MajorVersion"
   ReadRegDWORD $1 HKLM "SOFTWARE\Microsoft\InetStp" "MinorVersion"
   IfErrors MissingIIS 0		; do we see IIS?

   !insertmacro LogDetail "Found Microsoft Internet Information Server v$0.$1"
   
   ; ask if the user really wants to use IIS
   MessageBox MB_YESNO|MB_ICONEXCLAMATION \
   	   "Microsoft's IIS webserver appears to be installed on this computer.$\r$\n\
   	   $\r$\n\
       Please note that the supported ${PRODUCT_NAME} webserver is Apache. \
       If you are sure that you would like to use the non-supported IIS webserver \
       press Yes to continue otherwise press No to choose the supported \
       ${PRODUCT_NAME} webserver, Apache." IDNO Cancelled 
 
   ; check IIS version >= 5
   IntCmp $0 5 0 IISMajVerLT5 0
   !insertmacro LogDetail "Microsoft IIS webserver looks OK"
   Goto Done 
   
 IISMajVerLT5:
   !insertmacro LogAbort "Please update IIS to v5 or later then run this installer again."
 
 MissingIIS:
   	!insertmacro LogDetail "No Microsoft IIS webserver found"
   	IfSilent Done 
   	MessageBox MB_YESNO|MB_ICONEXCLAMATION \
   	   "The ${PRODUCT_NAME} installer could not find a Microsoft IIS webserver \
   	   on this computer.$\r$\n$\r$\n\
       Select 'yes' to continue the installation without installing and/or \
       configuring a webserver. If you choose to proceed you will have to \
       manually install and configure it yourself.  Please see the following \
       links for more information: $\r$\n\
           $r$\n\
           http://www.insilicos.com/tpp_iis_config.html$\r$\n\
           http://www.insilicos.com/tpp_apache_config.html$\r$\n\
           $r$\n\
       Choose 'no; if you would like to rather install/configure \
       the Apache webserver." IDNO Cancelled 
   !insertmacro LogDetail "User choosing to skip webserver install/configuration"
    Goto Done

 Cancelled:
   Abort
   
 Goback: 
   Abort
   
 Done:
   ClearErrors
FunctionEnd
