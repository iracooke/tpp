!ifndef _configApache_nsh
!define _configApache_nsh
 
;--------------------------------
;Configure Apache if it isn't already set up
Var /GLOBAL ApacheConfFile ; set by caller
Var /GLOBAL ApacheExe ; set by caller
Var /GLOBAL ApacheDocumentRoot ; set here
Var /GLOBAL ApachePreconfigured ; set here

Function configApache
	ClearErrors
	StrCpy $ApachePreconfigured ""
	FileOpen $4 $ApacheConfFile a
	ifErrors noconfig 0
	FileRead $4 $1 	
	ifErrors noconfig
	readloop:
		ClearErrors
		FileRead $4 $1 		
		ifErrors readDone

		; look for DocumentRoot setting so we can update xsl and xsd files
		push $1
		push 'DocumentRoot "'
		Call StrStr ; assumes this has been defined, possibly in add_to_path.nsh
		Pop $R1
		StrCmp $R1 "" check_preconf  
		push $1
		push '"'
		Call StrStr ; assumes this has been defined, possibly in add_to_path.nsh
		Pop $2
		StrCpy $3 $2 -3 ; don't copy last three chars ("\r\n)
		StrCpy $ApacheDocumentRoot $3 "" 1 ; don't copy leading quote
		; update the xsd and xsl files in web root dir
		${DirState} "$ApacheDocumentRoot\schema" $1
		StrCmp $1 "-1" 0 copySchema
		ExecWait '$INSTDIR\mkdir "$ApacheDocumentRoot\schema"'
		ExecWait '$INSTDIR\setPermissions "$ApacheDocumentRoot\schema"'
		copySchema:
  	    CopyFiles /SILENT $INSTDIR\*.xs? "$ApacheDocumentRoot\schema"

		; see if we're already configured (looking for slashes both ways, from older installs)
		check_preconf:
		strCmp $1 'Alias /${PRODUCT_VDIRNAME} "$INSTDIR"$\r$\n' 0 tryfwd
		StrCpy $ApachePreconfigured "1"
		tryfwd:
		strCmp $1 'Alias /${PRODUCT_VDIRNAME} "${PRODUCT_INSTALL_DIR_FWDSLASHES}"$\r$\n' 0 readloop
		StrCpy $ApachePreconfigured "1"
		Goto readloop ; stay in loop for htdocs search
	readDone:
	StrCmp $ApachePreconfigured "1" preconfigured
	
	;
	; add our config info
	;  
	; create a backup before we proceed
	Var /GLOBAL BackupConfFile
	StrCpy $BackupConfFile "$ApacheConfFile.preTPP"
	CopyFiles $ApacheConfFile $BackupConfFile
	; now add our config info
	ClearErrors
    FileWrite $4 '$\r$\n'
	ifErrors nowrite
    FileWrite $4 '#$\r$\n'
    FileWrite $4 '# Begin settings for the Trans Proteomic Pipeline$\r$\n'
    FileWrite $4 '#$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 '# Add 5-hour timeout$\r$\n'
    FileWrite $4 'Timeout 18000$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 'Alias /${PRODUCT_VDIRNAME} "${PRODUCT_INSTALL_DIR_FWDSLASHES}"$\r$\n'
    FileWrite $4 '<Directory "${PRODUCT_INSTALL_DIR_FWDSLASHES}">$\r$\n'
    FileWrite $4 '    Options Indexes MultiViews ExecCGI$\r$\n'
    FileWrite $4 '    AllowOverride None$\r$\n'
    FileWrite $4 '    Order allow,deny$\r$\n'
    FileWrite $4 '    Allow from all$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 '    AddHandler cgi-script .cgi .pl$\r$\n'
    FileWrite $4 '    ScriptInterpreterSource Registry$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 '    PassEnv WEBSERVER_ROOT$\r$\n'
    FileWrite $4 '    PassEnv WEBSERVER_TMP$\r$\n'
    FileWrite $4 '</Directory>$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 'Alias /ISB "${PRODUCT_WEBSERVER_ROOT_FWDSLASHES}/ISB"$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 'Alias /schema "${PRODUCT_WEBSERVER_ROOT_FWDSLASHES}/schema"$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 '<Directory "${PRODUCT_WEBSERVER_ROOT_FWDSLASHES}">$\r$\n'
    FileWrite $4 '    Options MultiViews Includes$\r$\n'
    FileWrite $4 '    AllowOverride None$\r$\n'
    FileWrite $4 '    Order allow,deny$\r$\n'
    FileWrite $4 '    Allow from all$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 '    AddType text/html .shtml$\r$\n'
    FileWrite $4 '    AddHandler server-parsed .shtml$\r$\n'
    FileWrite $4 '$\r$\n'
    FileWrite $4 '    PassEnv WEBSERVER_ROOT$\r$\n'
    FileWrite $4 '    PassEnv WEBSERVER_TMP$\r$\n'
    FileWrite $4 '</Directory>$\r$\n'
    FileWrite $4 '#$\r$\n'
    FileWrite $4 '# End settings for the Trans Proteomic Pipeline$\r$\n'
    FileWrite $4 '#$\r$\n'
	FileClose $4

	!insertmacro LogDetail "Your Apache server has been configured for TPP.  We will now restart Apache so these changes can take effect."  
	Exec '$ApacheExe -k restart'
    ; user should probably reboot
    SetRebootFlag true
	Goto configured

  nowrite:
	  !insertmacro LogAbort "cannot write Apache config file $ApacheConfFile"

  preconfigured:
	FileClose $4
    !insertmacro LogDetail "Apache server is preconfigured for TPP, good"
        ; just in case Apache isn't already running
	Exec '$ApacheExe -k start'

  configured:
	Goto done

  noconfig:
	  !insertmacro LogAbort "cannot read Apache config file $ApacheConfFile"

  done:
	ClearErrors
FunctionEnd


!endif ; _configApache_nsh 
