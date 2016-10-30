; TPP installer script using NSIS installer
; for use with MinGW or VC8 builds
;
; Copyright (c) 2006,2007 Insilicos, LLC
;
; This library is free software; you can redistribute it and/or
; modify it under the terms of the GNU Lesser General Public
; License as published by the Free Software Foundation; either
; version 2.1 of the License, or (at your option) any later version.
; 
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Lesser General Public License for more details.
; 
; You should have received a copy of the GNU Lesser General Public
; License along with this library; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
; $Author: bpratt $
;
; $DateTime: 2006/08/08 16:44:37 $
;
; NOTES:
;
; TODO:
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

; define this with /D at commandline, probably "../build/mingw" or "../src/Release" for VC8

!include ${PRODUCT_BUILD_DIR}\TPPVersionInfo.nsh  ; generated file, defines product info
!define PRODUCT_VERSION "${PRODUCT_MAJOR_VERSION_NUMBER}.${PRODUCT_MINOR_VERSION_NUMBER}.${PRODUCT_REV_VERSION_NUMBER}${PRODUCT_RELEASE_TYPE}"


;SetCompressor /SOLID /FINAL lzma  ; gives best compression, but it's slow

; HM NIS Edit Wizard helper defines
!define PRODUCT_INSTALL_DIR_PARENT "C:\Inetpub"
!define PRODUCT_INSTALL_DIR "C:\Inetpub\tpp-bin"
!define PRODUCT_INSTALL_DIR_FWDSLASHES "C:/Inetpub/tpp-bin"
!define PRODUCT_VDIRNAME "tpp-bin"
!define PRODUCT_GUI_URL "http://localhost/tpp-bin/tpp_gui.pl"

!define PRODUCT_WEBSERVER_ROOT "c:\Inetpub\wwwroot"
!define PRODUCT_DATA_DIR "ISB"
!define PRODUCT_DATA_FULLPATH "c:\Inetpub\wwwroot\ISB"
!define PRODUCT_TPPGUI_DATA_FULLPATH "c:\Inetpub\wwwroot\ISB\data"
!define PRODUCT_WEBSERVER_ROOT_FWDSLASHES "C:/Inetpub/wwwroot"

!define PRODUCT_PUBLISHER "Institute for Systems Biology"
!define PRODUCT_WEB_SITE "http://www.proteomecenter.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\Trans-Proteomic Pipeline"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define PRODUCT_REGKEY "Software\Trans-Proteomic Pipeline"
!define PRODUCT_REGKEY_ROOT "HKLM"

!include "FileFunc.nsh"
!insertmacro DirState


; MUI 1.67 compatible ------
!include "MUI.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"
!include "Sections.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "TPP.ico"
!define MUI_UNICON "TPP.ico"

!define ALL_USERS 1 ; set path for all users


; Welcome page
!insertmacro MUI_PAGE_WELCOME
; Custom help page (see function below)
Page custom CustomHelpPage
; License page
!insertmacro MUI_PAGE_LICENSE "license.txt"
; Instfiles page
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE CheckRequirements
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
!define MUI_FINISHPAGE_LINK "Click here to start the TPP web user interface."
!define MUI_FINISHPAGE_LINK_LOCATION "http://localhost/tpp-bin/tpp_gui.pl"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; add an XP manifest to the installer
XPStyle on

; MUI end ------


!include "AddToPath.nsh"           
!include "CreateVDir.nsh"
!include "ConfigApache.nsh"  
!include "CheckApacheServer.nsh"
!include "CheckIISServer.nsh"
!include "checkPerl.nsh"


Name "${PRODUCT_FULLNAME}"

; for Vista, warn that we want admin priv for install
RequestExecutionLevel admin

BrandingText "tools.proteomecenter.org"
Caption "${PRODUCT_NAME} ${PRODUCT_VERSION}, ${PRODUCT_BUILD_ID}"

OutFile "${PRODUCT_BUILD_DIR}\TPP_Setup_${PRODUCT_VERSION}.exe"
InstallDir "${PRODUCT_INSTALL_DIR}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Var /GLOBAL bFirstInstall

; we maintain a single list of files for both install and uninstall,
; these macros let us use the list both ways
!macro addFile sourcedir file subdir
  StrCmp "${subdir}" "" +2 
  SetOutPath "$INSTDIR\${subdir}"
  File "${sourcedir}\${file}"
  SetOutPath "$INSTDIR"
!macroend

!macro deleteFile sourcedir file subdir
  StrCmp "${subdir}" "" +4 
  Delete "$INSTDIR\${subdir}\${file}"
  RMDir "$INSTDIR\${subdir}"
  goto +2
  Delete "$INSTDIR\${file}"
!macroend

!include "TPP_files.nsh"		; the list of distro files
!include "TPP_proteowizard.nsh"	; include list of msconvert files

;;
;; Use Apache section
;;
var /GLOBAL webserver
Section "Use Apache" SEC01
  ; Need to have install dir in place to install apache
  ${DirState} "$INSTDIR" $1
  StrCmp $1 "-1" 0 InstallApache

  StrCpy $bFirstInstall "1"
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$INSTDIR"
  CreateDirectory "$INSTDIR\logs"  ; for TPP gui
  !insertmacro LogDetail "creating $INSTDIR"
  
 InstallApache:
   StrCmp $apacheInstalled "installed" configApache 0
   !insertmacro LogDetail "Installing Apache web server"
   SetOutPath "$INSTDIR"
   File "..\extern\apache-w32-2.2.25\apache.msi"
   ExecWait 'MSIEXEC.EXE /I "$INSTDIR\apache.msi" ALLUSERS=1'
   ; Delete "apache.msi"     ; don't delete - leave as cue (and tool) for uninstaller 
   Call checkApacheServer    ; confirm its installed (for config)
   
 ConfigApache:
   !insertmacro LogDetail "Configuring Apache web server"
   Call ConfigApache	
SectionEnd

LangString DESC_Section1 ${LANG_ENGLISH} "Use the Apache web server. If \
   Apache is already installed it will be used otherwise a version will \
   be installed for you."  

;;
;; Use Microsoft IIS section
;;
Section /o "Use Microsoft IIS" SEC02
   !insertmacro LogDetail "User had selected Microsoft IIS web server component"
   Call CreateVDir			; try to configure it
SectionEnd

LangString DESC_Section2 ${LANG_ENGLISH} "Use the Microsoft IIS web server. \
   This web server is not offically supported and if selected a version should \
   already be installed by you on the system."

;;
;; TPP section
;;
Section "${PRODUCT_NAME} (Required)" SEC03
    SectionIn 1 RO                     ; make it required
    
    ; if there's a cygwin directory, we probably put it there in a previous version, help uninstall
    StrCpy $1 ""
    IfFileExists "c:\cygwin" 0 no_cygwin
       StrCpy $1 "$\r$\n$\r$\nYou are *strongly* recommended to completely uninstall any older cygwin-based TPP versions, following these instructions:$\r$\n\
http://tools.proteomecenter.org/wiki/index.php?title=TPP:Windows_Cygwin_Installation#Uninstalling_the_TPP"
no_cygwin:
    SetRebootFlag false
    StrCpy $bFirstInstall ""
    # call userInfo plugin to get user info.  The plugin puts the result in the stack
    userInfo::getAccountType  
    # pop the result from the stack into $0
    pop $0   
    # compare the result with the string "Admin" to see if the user is admin. 
    strCmp $0 "Admin" admin_ok   
    # if there is not a match, print message and return
    messageBox MB_OKCANCEL "You do not appear to have administrative privilege - this is install may fail.  Proceed anyway?" IDOK pathfix IDCANCEL badpriv
badpriv:
    abort
admin_ok:
  # jmt:
  # first, create tmp dir as C:/tmp
  # TODO: this should be moved into a TPP-specific dir
  # TODO: add to uninstall info
  # TODO: check error
  CreateDirectory "C:\tmp"
  # end jmt
  ${DirState} "$INSTDIR" $1
  StrCmp $1 "-1" 0 create_installdir_done
  !insertmacro LogDetail "creating $INSTDIR"
  StrCpy $bFirstInstall "1"
create_installdir_done:

; TODO: check for R install (in C:\Program Files\R), install R if needed
; then execute these in R:
;source("http://bioconductor.org/biocLite.R")
;biocLite("Biobase")
;biocLite("limma")
;biocLite("maSigPro") 
;biocLite("MLInterfaces")
;biocLite("genefilter") 
;biocLite("cluster") 
;biocLite("nlme") 
;biocLite("MASS") 
;biocLite("statmod") 
;biocLite("XML") 
;biocLite("GDD")
;biocLite("EMV")


  ; add us to the path
pathfix:
  Push $INSTDIR
  Call AddToPath
  Push "WEBSERVER_ROOT"
  Push "${PRODUCT_WEBSERVER_ROOT}"
  call AddToEnvVar

  # for wgnuplot
  Push "GDFONTPATH"
  Push "C:/WINDOWS/FONTS"
  call AddToEnvVar

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$INSTDIR"
  CreateDirectory "$INSTDIR\logs"  ; for TPP gui

  ; clean up any previous installs with typos in the program name:
  Delete "$SMPROGRAMS\Trans Proteomic Pipeline\Uninstall.lnk"
  Delete "$SMPROGRAMS\Trans Proteomic Pipeline\Website.lnk"
  Delete "$DESKTOP\Trans Proteomic Pipeline.lnk"
  Delete "$SMPROGRAMS\Trans Proteomic Pipeline\Trans Proteomic Pipeline.lnk"
  RMDir "$SMPROGRAMS\Trans Proteomic Pipeline"

  CreateDirectory "$SMPROGRAMS\Trans-Proteomic Pipeline"
  CreateShortCut "$SMPROGRAMS\Trans-Proteomic Pipeline\Trans-Proteomic Pipeline.lnk" ${PRODUCT_GUI_URL} "" "$INSTDIR\images\petunia.ico"
  CreateShortCut "$SMPROGRAMS\Trans-Proteomic Pipeline\TPP Data Directory.lnk" ${PRODUCT_DATA_FULLPATH} "" ""
  CreateShortCut "$SMPROGRAMS\Trans-Proteomic Pipeline\Seattle Proteome Center Wiki.lnk" "http://tools.proteomecenter.org/wiki" "" ""
  CreateShortCut "$SMPROGRAMS\Trans-Proteomic Pipeline\TPP Support Newsgroup.lnk" "http://groups.google.com/group/spctools-discuss" "" ""
  CreateShortCut "$SMPROGRAMS\Trans-Proteomic Pipeline\Seattle Proteome Center Website.lnk" "$INSTDIR\${PRODUCT_FULLNAME}.url"

  CreateShortCut "$DESKTOP\Trans-Proteomic Pipeline.lnk" ${PRODUCT_GUI_URL} "" "$INSTDIR\images\petunia.ico"

  CopyFiles /SILENT "$SYSDIR\cmd.exe" "$INSTDIR" ; IIS needs local copy of cmd.exe for pipes


;distrofiles: 
  !insertmacro handleFiles addFile ; install the distro files


  ; create working dir and set permissions while we're still running as admin
  !insertmacro LogDetail "checking directory structure"

  ${DirState} "${PRODUCT_WEBSERVER_ROOT}" $1
  StrCmp $1 "-1" 0 datapath
  !insertmacro LogDetail "creating ${PRODUCT_WEBSERVER_ROOT}"
  ExecWait '$INSTDIR\mkdir "${PRODUCT_WEBSERVER_ROOT}"'
  StrCpy $bFirstInstall "1"

datapath:
  ${DirState} "${PRODUCT_DATA_FULLPATH}" $1
  StrCmp $1 "-1" 0 datapath_exists
	  ; creating "ISB"
	  !insertmacro LogDetail "creating ${PRODUCT_DATA_FULLPATH}"
	  ExecWait '$INSTDIR\mkdir "${PRODUCT_DATA_FULLPATH}"'
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_DATA_FULLPATH}"'

	  ; creating "ISB/data"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}"
	  ExecWait '$INSTDIR\mkdir "${PRODUCT_TPPGUI_DATA_FULLPATH}"'
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}"'

	  ; creating "ISB/data/parameters"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\parameters"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\parameters"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\parameters"'

	  ; creating "ISB/data/MaRiMba"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\MaRiMba"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\MaRiMba"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\MaRiMba"'

	  ; creating "ISB/data/dbase"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase"'

	  ; creating "ISB/data/dbase/speclibs"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase\speclibs"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase\speclibs"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase\speclibs"'



datapath_exists:
  StrCmp $bFirstInstall "1" 0 checkperlandwebserver
  ; is there already a TPP install with faked-up Inetpub dir?
  ${DirState} "$INSTDIR\..\tpp-bin" $1
  StrCmp $1 "1" 0 checkperlandwebserver
  ${DirState} "$INSTDIR\..\AdminScripts" $1
  StrCmp $1 "1" checkperlandwebserver 0
    ; if we find tpp-bin but not AdminScripts, looks like a TPP cyg-apache install
   !insertmacro LogDetail "setting permissions on directory tree ${PRODUCT_INSTALL_DIR_PARENT}"
    ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_INSTALL_DIR_PARENT}"'


createDataDirsAnyway:
	  ; (I don't understand why these aren't usually created.. trying again)
	  ; creating "ISB"
	  !insertmacro LogDetail "creating ${PRODUCT_DATA_FULLPATH}"
	  ExecWait '$INSTDIR\mkdir "${PRODUCT_DATA_FULLPATH}"'
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_DATA_FULLPATH}"'

	  ; creating "ISB/data"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}"
	  ExecWait '$INSTDIR\mkdir "${PRODUCT_TPPGUI_DATA_FULLPATH}"'
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}"'

	  ; creating "ISB/data/parameters"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\parameters"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\parameters"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\parameters"'

	  ; creating "ISB/data/MaRiMba"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\MaRiMba"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\MaRiMba"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\MaRiMba"'

	  ; creating "ISB/data/dbase"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase"'

	  ; creating "ISB/data/dbase/speclibs"
	  !insertmacro LogDetail "creating ${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase\speclibs"
	  CreateDirectory "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase\speclibs"
	  ; windows equivalent of chmod g+w
	  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_TPPGUI_DATA_FULLPATH}\dbase\speclibs"'


fixAllPermissionsAnyway:
  ; jmt: go ahead and do permissions fix for everything,
  ; as something isn't working as-is
  ;   use windows equivalent of chmod g+w:
  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_INSTALL_DIR_PARENT}"'

recordRegistryData:
  ; jmt- some initial registy data, not used at this point
  WriteRegStr ${PRODUCT_REGKEY_ROOT} "${PRODUCT_REGKEY}" "DisplayName" "${PRODUCT_FULLNAME}"
  WriteRegStr ${PRODUCT_REGKEY_ROOT} "${PRODUCT_REGKEY}" "ProductVersion" "${PRODUCT_VERSION}-${PRODUCT_BUILD_ID}"
  WriteRegStr ${PRODUCT_REGKEY_ROOT} "${PRODUCT_REGKEY}" "ProductVersionMajor" "${PRODUCT_MAJOR_VERSION_NUMBER}"
  WriteRegStr ${PRODUCT_REGKEY_ROOT} "${PRODUCT_REGKEY}" "ProductVersionMinor" "${PRODUCT_MINOR_VERSION_NUMBER}"
  WriteRegStr ${PRODUCT_REGKEY_ROOT} "${PRODUCT_REGKEY}" "ProductVersionRev" "${PRODUCT_REV_VERSION_NUMBER}"

setEnvVariables:
  ; jmt- we'll set webserver_tmp just to be safe
  WriteRegStr HKLM "/SYSTEM/CurrentControlSet/Control/Session\ Manager/Environment" "WEBSERVER_TMP" "C:\tmp"
  
  ; set SSRCalc environmental variable
  WriteRegStr HKLM "/SYSTEM/CurrentControlSet/Control/Session\ Manager/Environment" "SSRCalc" "${PRODUCT_INSTALL_DIR}"



checkperlandwebserver:
 ; now see if we need to install Perl and Apache
 ; Call CheckWebServer

; update the xsd and xsl files in web root dir
  ${DirState} "${PRODUCT_DATA_FULLPATH}\schema" $1
  StrCmp $1 "-1" 0 copy_schema
  ExecWait '$INSTDIR\mkdir "${PRODUCT_DATA_FULLPATH}\schema"'
  ExecWait '$INSTDIR\setWin32Permissions "${PRODUCT_DATA_FULLPATH}\schema"'
copy_schema:
  CopyFiles /SILENT $INSTDIR\*.xs? "${PRODUCT_DATA_FULLPATH}\schema"

; if this is our first install, set permissions
  StrCmp $bFirstInstall "1" 0 recheck_path
  ; windows equivalent of chmod g+w
    !insertmacro LogDetail "setting permissions on newly installed files and data directories"
    Exec '$INSTDIR\setWin32Permissions "${PRODUCT_WEBSERVER_ROOT}"'
    Exec '$INSTDIR\setWin32Permissions "$INSTDIR"'

  ; sometimes perl install blows away our path change, try setting again
recheck_path:  
  Push $INSTDIR
  Call AddToPath
  IfSilent no_reboot
  IfRebootFlag 0 no_reboot
    ; we're going to reboot - set up to run installcheck on restart
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "TPP_Install_Test" "$INSTDIR\cmd.exe /C $INSTDIR\tpp_installtest.bat"
    goto main_done

no_reboot:
  SetRebootFlag false
  IfSilent main_done
  !insertmacro LogDetail "running tpp_installtest.bat... waiting for user input"
  ExecWait "$INSTDIR\tpp_installtest.bat"

main_done:
SectionEnd
LangString DESC_Section3 ${LANG_ENGLISH} "TPP component (required)"

;;
;; Proteowizard's msconvert component
;;
LangString DESC_Section4 ${LANG_ENGLISH} "The ProteoWizard's msconvert is a \
   utility for converting instrument output into various formats and \
   contains proprietary vendor libraries with licensing restrictions."
   
Section "msconvert" SEC04
   !insertmacro proteowizardFiles addFile	; install the msconvert files
   !insertmacro LogDetail "msconvert installed: user agreed to licenses"
SectionEnd

; Add component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} $(DESC_Section1)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} $(DESC_Section2)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} $(DESC_Section3)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} $(DESC_Section4)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_FULLNAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Trans-Proteomic Pipeline\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
;  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\xinteract.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FULLNAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  ; copy details window to logfile
  !insertmacro LogDetail "done"
SectionEnd


Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_FULLNAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Trans-Proteomic Pipeline\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd


Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
;  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\xinteract.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FULLNAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  ; copy details window to logfile
  !insertmacro LogDetail "done"


SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "${PRODUCT_FULLNAME} was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove ${PRODUCT_FULLNAME} and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_FULLNAME}.url"
  Delete "$INSTDIR\uninst.exe"

  ; if Apache.msi is in installdir, it means we installed Apache
  IfFileExists "$INSTDIR\Apache.msi" 0 delfiles
  !insertmacro LogDetail "Preparing to remove the Apache web server we installed."
  !insertmacro LogDetail "If you are also using it for other purposes, answer 'No' to the Windows Uninstaller prompt."
  ExecWait 'MSIEXEC.EXE /x "$INSTDIR\Apache.msi"'
  Delete "$INSTDIR\Apache.msi"

delfiles:
  !insertmacro handleFiles deleteFile		; remove the distro files
  !insertmacro proteowizardFiles deleteFile	; remove the msconvert files

  Delete "$SMPROGRAMS\Trans-Proteomic Pipeline\Uninstall.lnk"
  Delete "$SMPROGRAMS\Trans-Proteomic Pipeline\Website.lnk"
  Delete "$DESKTOP\Trans-Proteomic Pipeline.lnk"
  Delete "$SMPROGRAMS\Trans-Proteomic Pipeline\Trans-Proteomic Pipeline.lnk"
  Delete "$SMPROGRAMS\Trans-Proteomic Pipeline\TPP Data Directory.lnk"
  Delete "$SMPROGRAMS\Trans-Proteomic Pipeline\Seattle Proteome Center Wiki.lnk"
  Delete "$SMPROGRAMS\Trans-Proteomic Pipeline\TPP Support Newsgroup.lnk"
  Delete "$SMPROGRAMS\Trans-Proteomic Pipeline\Seattle Proteome Center Website.lnk"

  Delete "$INSTDIR\cmd.exe"
  Delete "$INSTDIR\installer.log"

  RMDir "$SMPROGRAMS\Trans-Proteomic Pipeline"
  RMDir "$INSTDIR"

  Push $INSTDIR
  Call un.RemoveFromPath
  Push "WEBSERVER_ROOT"
  Push "${PRODUCT_WEBSERVER_ROOT}"
  call un.RemoveFromEnvVar
  Call un.DeleteVDir


  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  ; delete SSRCalc environmental variable
  DeleteRegKey HKLM "/SYSTEM/CurrentControlSet/Control/Session\ Manager/Environment/SSRCalc"

  ; [jmt] delete TPP registry data
  DeleteRegKey ${PRODUCT_REGKEY_ROOT} "${PRODUCT_REGKEY}"

  SetAutoClose true
SectionEnd

;;
;; --- Functions --------------------------------------------------------------
;;

;; 
;; Custom installer page containing help on the installation.  
;; 
Var HelpDialog
Var HelpLabel
Function CustomHelpPage
   !insertmacro MUI_HEADER_TEXT "TPP Installation Help" "Getting installation assistance"
   nsDialogs::Create 1018
   Pop $HelpDialog
   ${If} $HelpDialog == error
      Abort
   ${EndIf}
   
   ${NSD_CreateLabel} 0 0 100% 100% \
      "Welcome to the TPP installation process.  If you need assistance with \
      the installation or the TPP in general, please refer to our support and \
      discussion newsgroup at:$\r$\n\
      $\r$\n\
      http://groups.google.com/group/spctools-discuss$\r$\n\
      $\r$\n\
      and the TPP wiki at: $\r$\n\
      $\r$\n\
      http://tools.proteomecenter.org/wiki"
   Pop $HelpLabel
   
   nsDialogs::Show
FunctionEnd

;;
;; Add some callback functions to the component page to ensure that one and 
;; only one webserver component is selected.
;;
Function .onInit
   StrCpy $webserver apache
FunctionEnd

Function .onSelChange
   Push $0
   Push $1
   
   SectionGetFlags ${SEC01} $0 		; is apache selected?
   IntOp $0 $0 & ${SF_SELECTED}
   SectionGetFlags ${SEC02} $1		; is IIS selected? 
   IntOp $1 $1 & ${SF_SELECTED}
   
   ; was previous selected value apache?
   StrCmp $webserver "apache" ApacheWasSelected
  
   IntCmp $0 ${SF_SELECTED} 0 EnableIIS EnableIIS
 DisableIIS:
   StrCpy $webserver "apache"
   SectionGetFlags ${SEC02} $1		; disable IIS choice
   IntOp $1 $1 & ${SECTION_OFF}
   SectionSetFlags ${SEC02} $1
   Goto Done 
 EnableIIS:
   SectionGetFlags ${SEC02} $1		; make sure IIS enabled
   IntOp $1 $1 | ${SF_SELECTED}
   SectionSetFlags ${SEC02} $1
   Goto Done 
  
 ApacheWasSelected: 
   IntCmp $1 ${SF_SELECTED} 0 EnableApache EnableApache
 DisableApache:
   StrCpy $webserver "IIS"
   SectionGetFlags ${SEC01} $0		; disable apache choice
   IntOp $0 $0 & ${SECTION_OFF}
   SectionSetFlags ${SEC01} $0
   Goto Done
 EnableApache:
   SectionGetFlags ${SEC01} $0		; renable apache choice
   IntOp $0 $0 | ${SF_SELECTED}
   SectionSetFlags ${SEC01} $0
   Goto Done
  
 Done:  
   Pop $1
   Pop $0
FunctionEnd

;;
;; Callback function to call when leaving the component page that checks
;; that the required conditions have been met for installation
;;
Function CheckRequirements

   ; If msconvert is selected then make sure user agrees to license
   SectionGetFlags ${SEC04} $0
   IntOp $0 $0 & ${SF_SELECTED}
   IntCmp $0 ${SF_SELECTED} 0 NextCheck NextCheck
   
   MessageBox MB_YESNO "You've choosen to install Proteowizard's msconvert. \
      This program uses proprietary vendor libraries that will also be \
      installed and come with licensing terms and restrictions.  These \
      can be found at:$\r$\n\
      $\r$\n\
      http://proteowizard.sourceforge.net/downloads.shtml$\r$\n\
      $\r$\n\
      Do you agree to the licensing terms and wish to install msconvert?"\
      IDYES NextCheck IDNO 0
   SectionGetFlags ${SEC04} $0		; user said no so deselect it
   IntOp $0 $0 & ${SECTION_OFF}
   SectionSetFlags ${SEC04} $0
   Abort
   
 NextCheck:
   ; check webserver before installing   
   StrCmp $webserver "apache" CheckApache CheckIIS
 CheckApache:
   Call checkApacheServer
   Goto Done
 CheckIIS:
   Call checkIISServer
   
 Done:
 
   ; Finally, check for perl before installing
   Call checkPerl
    
FunctionEnd
