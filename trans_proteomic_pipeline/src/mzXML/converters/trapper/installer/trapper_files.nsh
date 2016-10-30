;
; trapper installer script component:
; a list of trapper install files, broken out from rest of script to 
; distinguish content changes from installer logic changes
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
;
; NOTES:
;
; TODO:
;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


!macro handleFiles addOrDelete ; "addOrDelete" is a parameter that defines the action

SetOverwrite on
; converter program
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "trapper.exe" ""
; TODO: postinstall test
;!insertmacro ${addOrDelete} "." "trapper_installtest.bat" ""
; MHDAC library-- note-- cannot be checked into revision control due to licensing;
; adjust paths for your system as required via ${MHDAC_DIR} in trapper_installer.nsi
;
; ls -1 | xargs -i{} echo '!insertmacro ${addOrDelete} ${MHDAC_DIR} "'{}'" ${MHDAC_INSTALL_DIR}'
;
!insertmacro ${addOrDelete} ${MHDAC_DIR} "BaseCommon.dll" ${MHDAC_INSTALL_DIR}
;!insertmacro ${addOrDelete} ${MHDAC_DIR} "BaseCommon.tlb" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "BaseDataAccess.dll" ${MHDAC_INSTALL_DIR}
;!insertmacro ${addOrDelete} ${MHDAC_DIR} "BaseDataAccess.tlb" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "BaseError.dll" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "BaseTof.dll" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "Contents.xsd" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "DefaultMassCal.xsd" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "Devices.xsd" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "MSActualDefs.xsd" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "MSScan_XSpecific.xsd" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "MSTS.xsd" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "MSTS_XSpecific.xsd" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "MassSpecDataReader.dll" ${MHDAC_INSTALL_DIR}
;!insertmacro ${addOrDelete} ${MHDAC_DIR} "MassSpecDataReader.tlb" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "RegisterMassHunterDataAccess.bat" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "UnregisterMassHunterDataAccess.bat" ${MHDAC_INSTALL_DIR}
!insertmacro ${addOrDelete} ${MHDAC_DIR} "agtsampleinforw.dll" ${MHDAC_INSTALL_DIR}
SetOverwrite off
!macroend