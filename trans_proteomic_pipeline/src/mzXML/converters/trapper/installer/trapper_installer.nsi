; trapper installer script using NSIS installer
; for use with MinGW or VC8 builds
;
; Copyright (c) 2008, N. Tasman, based on original work (TPP win installer (c) 2006, 2007 B. Pratt, Insilicos, LLC
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
; NOTES:
;
; TODO:
;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;

!define PRODUCT_BUILD_DIR "..\..\..\..\release"

;
; TODO: get version info from file?
;!include ${PRODUCT_BUILD_DIR}\trapperVersionInfo.nsh  ; generated file, define PRODUCT_VERSION
!define PRODUCT_NAME "trapper"
!define PRODUCT_FULLNAME "trapper"
!define PRODUCT_VERSION "version 4.2.1"
!define /date PRODUCT_BUILD_NUMBER "%Y%m%D%H%M"

;SetCompressor /SOLID /FINAL lzma  ; gives best compression, but it's slow

; HM NIS Edit Wizard helper defines
!define PRODUCT_INSTALL_DIR_PARENT $PROGRAMFILES ; predefined constant
!define PRODUCT_INSTALL_DIR "$PROGRAMFILES\trapper"
!define PRODUCT_INSTALL_DIR_FWDSLASHES "$PROGRAMFILES/trapper"

; source (build) location for the bundled MHDAC files
; NOTE: change this for your build system; MHDAC cannot be versioned due to licensing
!define MHDAC_DIR "..\MassHunterDataAccessAssembly_B.02.00_API1.3.1\MassHunterDataAccessAssembly\Bin"

; install location for the bundled MHDAC files
!define MHDAC_INSTALL_DIR "MHDAC_B.02.00_API1.3.1"
!echo "dac install is ${MHDAC_INSTALL_DIR}"


!define PRODUCT_PUBLISHER "Institute for Systems Biology"
!define PRODUCT_WEB_SITE "http://tools.proteomecenter.org/wiki"

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\trapper (Agilent MassHunter format converter)"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define PRODUCT_REGKEY "Software\trapper (Agilent MassHunter format converter)"
!define PRODUCT_REGKEY_ROOT "HKLM"

!include "FileFunc.nsh"
!insertmacro DirState

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
;!define MUI_ICON "TPP.ico"
;!define MUI_UNICON "TPP.ico"

!define ALL_USERS 1 ; set path for all users

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "license.txt"
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
; TODO: add readme?
;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; add an XP manifest to the installer
XPStyle on

; MUI end ------



!include "AddToPath.nsh"           

Function .onInit
FunctionEnd

Name "${PRODUCT_FULLNAME}"

; for Vista, warn that we want admin priv for install
RequestExecutionLevel admin

BrandingText "tools.proteomecenter.org"
Caption "${PRODUCT_NAME} ${PRODUCT_VERSION}, ${PRODUCT_BUILD_NUMBER}"

;OutFile "${PRODUCT_BUILD_DIR}\trapper_Setup_${PRODUCT_VERSION_NOSPACES}.exe"
OutFile "${PRODUCT_BUILD_DIR}\trapper_setup.exe"
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

!include "trapper_files.nsh" ; the list of distro files

Section "${PRODUCT_NAME}" SEC01
    MessageBox MB_OK "Welcome to the trapper installation process.  If you need assistance with the installation, the trapper, or the Trans-proteomic Pipeline in general,$\r$\n\
please refer to our support and discussion newsgroup at http://groups.google.com/group/spctools-discuss$\r$\n\
and the TPP wiki at http://tools.proteomecenter.org/wiki."

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
  ${DirState} "$INSTDIR" $1
  StrCmp $1 "-1" 0 create_installdir_done
  !insertmacro LogDetail "creating $INSTDIR"
  StrCpy $bFirstInstall "1"
create_installdir_done:

  ; add us to the path
pathfix:
  Push $INSTDIR
  Call AddToPath

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\trapper"
  CreateShortCut "$SMPROGRAMS\trapper\uninstall.lnk" $INSTDIR\uninst.exe "" ""
;  CreateShortCut "$DESKTOP\Trans Proteomic Pipeline.lnk" ${PRODUCT_GUI_URL} "" "$INSTDIR\images\TPP.ico"

;  CopyFiles /SILENT "$SYSDIR\cmd.exe" "$INSTDIR" ; IIS needs local copy of cmd.exe for pipes


;distrofiles: 
  !insertmacro handleFiles addFile ; install the distro files
  !echo "installing MHDAC..."
  ;MessageBox MB_OK "installing MHDAC using $INSTDIR\${MHDAC_INSTALL_DIR}\RegisterMassHunterDataAccess.bat"
  ExecWait '"$INSTDIR\${MHDAC_INSTALL_DIR}\RegisterMassHunterDataAccess.bat" "$INSTDIR\${MHDAC_INSTALL_DIR}"'
  !echo "done"



main_done:
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
;  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\xinteract.exe"
;   WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FULLNAME}"
;   WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
;   WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\uninst.exe"
;   WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
;   WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
;   WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
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
   !echo "uninstalling MHDAC..."
   ;MessageBox MB_OK "uninstalling MHDAC using $INSTDIR\${MHDAC_INSTALL_DIR}\UnregisterMassHunterDataAccess.bat"
   ExecWait '"$INSTDIR\${MHDAC_INSTALL_DIR}\UnregisterMassHunterDataAccess.bat" "$INSTDIR\${MHDAC_INSTALL_DIR}"'
   ;ExecWait "$INSTDIR\${MHDAC_INSTALL_DIR}\UnregisterMassHunterDataAccess.bat"
   !echo "done"
   ;  Delete "$INSTDIR\${PRODUCT_FULLNAME}.url"
   Delete "$INSTDIR\uninst.exe"


delfiles:
  !insertmacro handleFiles deleteFile ; remove the distro files

  Delete "$INSTDIR\installer.log"

  Delete "$SMPROGRAMS\trapper\uninstall.lnk"
  RMDir "$SMPROGRAMS\trapper"
  RMDir "$INSTDIR"

  Push $INSTDIR
  Call un.RemoveFromPath


;  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
;  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

;  ; [jmt] delete TPP registry data
;  DeleteRegKey ${PRODUCT_REGKEY_ROOT} "${PRODUCT_REGKEY}"

  SetAutoClose true
SectionEnd