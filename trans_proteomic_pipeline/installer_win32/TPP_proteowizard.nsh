;
; TPP installer script component:
; List of files from Proteowizard to include in TPP.
;

!macro proteowizardFiles addOrDelete	; "addOrDelete" is a parameter for what action to take

SetOverwrite on

!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\msconvert.exe" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\idconvert.exe" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\qtofpeakpicker.exe" ""

!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\cdt.dll" ""

; ABI
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.Compression.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.Data.AnalystDataProvider.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.Data.CommonInterfaces.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.Data.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.Data.WiffReader.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.InternalRawXYProcessing.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.Muni.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.ProjectUtilities.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.RawXYProcessing.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.StructuredStorage.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Clearcore2.Utility.dll" ""

; Agilent
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\agtsampleinforw.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BaseCommon.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BaseDataAccess.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BaseError.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BaseTof.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\MassSpecDataReader.dll" ""

; Bruker
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BDal.CXt.Lc.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BDal.CXt.Lc.Factory.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BDal.CXt.Lc.Interfaces.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BDal.CXt.Lc.UntU2.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BDal.BCO.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\BDal.BCO.interfaces.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\boost_date_time-vc90-mt-1_37-BDAL_20091123.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\boost_regex-vc90-mt-1_37-BDAL_20091123.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\boost_thread-vc90-mt-1_37-BDAL_20091123.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\mkl_sequential.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\msparser.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\msvcr71.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\CompassXtractMS.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\HSReadWrite.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Interop.EDAL.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Interop.HSREADWRITELib.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Interop.EDAL.SxS.manifest" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\Interop.HSREADWRITELib.SxS.manifest" ""

; Thermo
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\fileio.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\fregistry.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\MSFileReader.XRawfile2.dll" ""
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\MSFileReader.XRawfile2.SxS.manifest" ""

; Waters
!insertmacro ${addOrDelete} ${PRODUCT_BUILD_DIR} "pwiz-win32\MassLynxRaw.dll" ""

!macroend
