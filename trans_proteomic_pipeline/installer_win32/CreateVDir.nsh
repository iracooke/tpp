;--------------------------------
; CreateVDir Function
; assumes we've already done checkPerl script

Function CreateVDir
 
;Open a VBScript File in the temp dir for writing
!insertmacro LogDetail "Creating $TEMP\createVDir.vbs";
FileOpen $0 "$TEMP\createVDir.vbs" w
StrCmp $0 0 CreateVDirFailure

;Write the script:



;Create a virtual dir named ${PRODUCT_VDIRNAME} pointing to $INSTDIR with proper attributes
FileWrite $0 "On Error Resume Next$\n"
FileWrite $0 "Set Root = GetObject($\"IIS://LocalHost/W3SVC/1/ROOT$\")$\n"
FileWrite $0 "Set Dir = Root.Create($\"IIsWebVirtualDir$\", $\"${PRODUCT_VDIRNAME}$\")$\n"
FileWrite $0 "If (Err.Number <> 0) Then$\n"
FileWrite $0 "  If (Err.Number <> -2147024713) Then$\n"
FileWrite $0 "    message = $\"Error $\" & Err.Number & $\" $\" & Err.Description $\n"
FileWrite $0 "    message = message & $\" trying to create IIS virtual directory.$\" & chr(13)$\n"
FileWrite $0 "    message = message & $\"Please check your IIS settings, see www.insilicos.com/tpp_iis_config.html for help.$\"$\n"
FileWrite $0 "    MsgBox message, vbCritical, $\"${PRODUCT_NAME}$\"$\n"
FileWrite $0 "    WScript.Quit (Err.Number)$\n"
FileWrite $0 "  End If$\n"
FileWrite $0 "  ' Error -2147024713 means that the virtual directory already exists.$\n"
FileWrite $0 "  returncode = 1$\n"
FileWrite $0 "  ' We will check if the parameters are the same: if so, then OK.$\n"
FileWrite $0 "  ' If not, then fail and display a message box.$\n"
FileWrite $0 "  Set Dir = GetObject($\"IIS://LocalHost/W3SVC/1/ROOT/${PRODUCT_VDIRNAME}$\")$\n"
FileWrite $0 "  If (Dir.Path <> $\"$INSTDIR$\") Then$\n"
FileWrite $0 "    message = $\"Virtual Directory ${PRODUCT_VDIRNAME} already exists in a different folder ($\" + Dir.Path + $\").$\" + chr(13)$\n"
FileWrite $0 "    message = message + $\"Please delete the virtual directory using the IIS console (inetmgr), and install again.$\"$\n"
FileWrite $0 "    MsgBox message, vbCritical, $\"${PRODUCT_NAME}$\"$\n"
FileWrite $0 "    Wscript.Quit (Err.Number)$\n"
FileWrite $0 "  End If$\n"
FileWrite $0 "  If (Dir.AspAllowSessionState <> True  Or  Dir.AccessScript <> True) Then$\n"
FileWrite $0 "    message = $\"Virtual Directory ${PRODUCT_VDIRNAME} already exists and has incompatible parameters.$\" + chr(13)$\n"
FileWrite $0 "    message = message + $\"Please delete the virtual directory using the IIS console (inetmgr), and install again.$\"$\n"
FileWrite $0 "    MsgBox message, vbCritical, $\"${PRODUCT_NAME}$\"$\n"
FileWrite $0 "    Wscript.Quit (Err.Number)$\n"
FileWrite $0 "  End If$\n"
FileWrite $0 "  Wscript.Quit (-2147024713)$\n"
FileWrite $0 "End If$\n"

FileWrite $0 "Dir.Path = $\"$INSTDIR$\"$\n"
FileWrite $0 "Dir.AccessRead = True$\n"
FileWrite $0 "Dir.AccessWrite = False$\n"
FileWrite $0 "Dir.AccessScript = True$\n"
FileWrite $0 "Dir.AccessExecute = True$\n"
FileWrite $0 "Dir.AppFriendlyName = $\"${PRODUCT_VDIRNAME}$\"$\n"
FileWrite $0 "Dir.EnableDirBrowsing = False$\n"
FileWrite $0 "Dir.ContentIndexed = False$\n"
FileWrite $0 "Dir.DontLog = True$\n"
FileWrite $0 "Dir.EnableDefaultDoc = True$\n"
FileWrite $0 "Dir.DefaultDoc = $\"tpp_gui.pl$\"$\n"
FileWrite $0 "Dir.AspBufferingOn = True$\n"
FileWrite $0 "Dir.AspAllowSessionState = True$\n"
FileWrite $0 "Dir.AspSessionTimeout = 30$\n"
FileWrite $0 "Dir.AspScriptTimeout = 900$\n"
FileWrite $0 "Dir.CGITimeout = 1800$\n"
FileWrite $0 "Dir.SetInfo$\n"
FileWrite $0 "Set IISObject = GetObject($\"IIS://LocalHost/W3SVC/1/ROOT/${PRODUCT_VDIRNAME}$\")$\n"
FileWrite $0 "IISObject.AppCreate2(2) 'Create a process-pooled web application$\n"
FileWrite $0 "If (Err.Number <> 0) Then$\n"
FileWrite $0 " message = $\"Error $\" & Err.Number & $\" $\" & Err.Description $\n"
FileWrite $0 " message = message & $\" trying to create the virtual directory at 'IIS://LocalHost/W3SVC/1/ROOT/${PRODUCT_VDIRNAME}'$\" & chr(13)$\n"
FileWrite $0 " message = message & $\"Please check your IIS settings, see www.insilicos.com/tpp_iss_config.html for help.$\" $\n"
FileWrite $0 " MsgBox message, vbCritical, $\"${PRODUCT_NAME}$\"$\n"
FileWrite $0 " WScript.Quit (Err.Number)$\n"
FileWrite $0 "End If$\n"

; now add to the scriptmaps
FileWrite $0 "Set IISObject = GetObject($\"IIS://LocalHost/W3SVC/1/ROOT/${PRODUCT_VDIRNAME}$\")$\n"
FileWrite $0 "NewScriptMaps = IISObject.ScriptMaps$\n"
FileWrite $0 "Redim preserve NewScriptMaps(UBound(NewScriptMaps)+1)$\n"
FileWrite $0 "NewScriptMaps(UBound(NewScriptMaps)) = $\".pl,c:\perl\bin\perl.exe $\"$\"%s$\"$\" %s,5,GET, HEAD, POST$\"$\n"
FileWrite $0 "IISObject.ScriptMaps = NewScriptMaps$\n"
FileWrite $0 "IISObject.SetInfo$\n"

; set read, write, browse access permissions on the data area
FileWrite $0 "Set DataDir = GetObject($\"IIS://LocalHost/${PRODUCT_DATA_DIR}$\")$\n"
FileWrite $0 "DataDir.AccessRead = True$\n"
FileWrite $0 "DataDir.AccessWrite = True$\n"
FileWrite $0 "DataDir.EnableDirBrowsing = True$\n"
FileWrite $0 "DataDir.SetInfo$\n"
FileWrite $0 "If (Err.Number <> 0) Then$\n"
FileWrite $0 " message = $\"cannot set IIS permissions on ${PRODUCT_DATA_FULLPATH}  err=$\" & Err.Number & $\" $\" & Err.Description & chr(13)  & $\" see www.insilicos.com/tpp_iis_config.html for help.$\"$\n"
FileWrite $0 " MsgBox message, vbCritical, $\"${PRODUCT_NAME}$\"$\n"
FileWrite $0 " WScript.Quit (Err.Number)$\n"
FileWrite $0 "End If$\n"
FileWrite $0 "WScript.Quit (0)$\n"

FileClose $0

!insertmacro LogDetail "Executing $TEMP\createVDir.vbs"
nsExec::Exec /TIMEOUT=20000 '"$SYSDIR\cscript.exe" "$TEMP\createVDir.vbs"'
Pop $1
; return code 1 means we didn't have to set it up
StrCmp $1 "-2147024713" VDirAlreadyOK
; return code 0 means we set it up OK
StrCmp $1 "0" 0 CreateVDirFailure
!insertmacro LogDetail "Successfully created IIS virtual directory, suggest reboot"
SetRebootFlag True
goto CreateVDirDone

VDirAlreadyOK:
!insertmacro LogDetail "IIS virtual directory looks OK from previous install"
goto CreateVDirDone

CreateVDirFailure:
!insertmacro LogDetail "Error $1 in CreateVDir.vbs"
!insertmacro LogAbort "Failed to create IIS Virtual Directory"
 
CreateVDirDone:
; keep for support use ; Delete "$TEMP\createVDir.vbs"
FunctionEnd
 
;--------------------------------
; DeleteVDir Function
Function un.DeleteVDir
 
;Open a VBScript File in the temp dir for writing
!insertmacro LogDetail "Creating $TEMP\deleteVDir.vbs";
FileOpen $0 "$TEMP\deleteVDir.vbs" w
 
;Write the script:
;Create a virtual dir named ${PRODUCT_VDIRNAME} pointing to $INSTDIR with proper attributes
FileWrite $0 "On Error Resume Next$\n$\n"
;Delete the application object
FileWrite $0 "Set IISObject = GetObject($\"IIS://LocalHost/W3SVC/1/ROOT/${PRODUCT_VDIRNAME}$\")$\n$\n"
FileWrite $0 "IISObject.AppDelete 'Delete the web application$\n"
FileWrite $0 "If (Err.Number <> 0) Then$\n"
FileWrite $0 " message = Err.Description & $\" Error trying to delete the application at [IIS://LocalHost/W3SVC/1/ROOT/${PRODUCT_VDIRNAME}]$\"$\n"
FileWrite $0 " MsgBox message, vbCritical, $\"${PRODUCT_NAME}$\"$\n"
FileWrite $0 " WScript.Quit (Err.Number)$\n"
FileWrite $0 "End If$\n$\n"
 
FileWrite $0 "Set IISObject = GetObject($\"IIS://LocalHost/W3SVC/1/ROOT$\")$\n$\n"
FileWrite $0 "IIsObject.Delete $\"IIsWebVirtualDir$\", $\"${PRODUCT_VDIRNAME}$\"$\n"
FileWrite $0 "If (Err.Number <> 0) Then$\n"
FileWrite $0 " message = Err.Description & $\" Error trying to delete the virtual directory '${PRODUCT_VDIRNAME}' at 'IIS://LocalHost/W3SVC/1/ROOT'$\" $\n"
FileWrite $0 " MsgBox message, vbCritical, $\"${PRODUCT_NAME}$\"$\n"
FileWrite $0 " Wscript.Quit (Err.Number)$\n"
FileWrite $0 "End If$\n$\n"
 
FileClose $0
 
!insertmacro LogDetail "Executing $TEMP\deleteVDir.vbs"
nsExec::Exec /TIMEOUT=20000 '"$SYSDIR\cscript.exe" "$TEMP\deleteVDir.vbs"'
Pop $1
StrCmp $1 "0" +2
!insertmacro LogDetail "Error $1 in deleteVDir.vbs"
goto DeleteVDirEnd
!insertmacro LogDetail "Virtual Directory ${PRODUCT_VDIRNAME} successfully removed."
; keep for support use ; Delete "$TEMP\deleteVDir.vbs"
DeleteVDirEnd:
FunctionEnd