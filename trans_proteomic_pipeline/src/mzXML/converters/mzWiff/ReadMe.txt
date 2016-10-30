mzWiff is a single executable that uses the library in your computer based on 
your installed Analyst or AnalystQS software to translate your .wiff file to
.mzXML or .mzML

It is not advisable to translate .wiff file generated with AnalystQS with the
Analyst library or vice versa althought it will work sometime.  mzWiff will 
provide warnings but still attempt to translate nevertheless.

Supported Versions:
AnalystQS: v1.0 (does not support peak finding), v1.1, v2.0
Analyst: v1.3 (no longer actively tested), v1.4.x, v1.5 beta

Project Configuration:
"Debug" - Debug build with shared DLLs for C/C++ runtime, MFC, ZLib
"Release" - Release build with shared DLLs for C/C++ runtime, MFC, ZLib
"Static Debug" - Debug build with C/C++ runtime, MFC, ZLib statically linked
"Static Release" - Release build with C/C++ runtime, MFC, ZLib statically linked


Requirements to Run mzWiff
----------------------------
1. mzWiff.exe
2. either Analyst or AnalystQS software installed
3. mzWiff dependent DLLs:
   i) (Statically linked since 08-May-2008, thus no longer needed)
      MSVCR80.DLL, MSVCP80.DLL, MFC80U.DLL  
   ii) WS2_32.DLL - peaklist handling (endian conversion)
       ZLIB1.DLL (Statically linked since 08-May-2008, thus no longer needed)
   iii) MPR.DLL - handle UNC and network mapped drive
   iv) KERNEL32.DLL, ADVAPI32.DLL, OLE32.DLL, OLEAUT32.DLL


Processing Decision
---------------------

Filtering
- When a scan is filtered out, we report a scan of nullified peak list.
  This is meant to keep the scan number intact.


Requirements to Compile mzWiff
--------------------------------

You can compile the project mzWiff without AnalystQS / Analyst installed. The 
generated (via #import) .tlh and .tli files are kept in wiff<version>lib (for 
Analyst library, e.g. wiff14lib for Analyst v1.4.x) and wiffq<version>slib 
(for AnalystQS library, e.g. wiffqs11lib for AnalystQS v1.1).  This is to 
facilitate code modification without the need to install both software on the 
developer machine.  However, to run/test the program, you will need either 
software installed so that the .wiff file can be accessed.

In event that your Analyst/AnalystQS software is newer, you will need to 
refresh these files (.tlh and .tli) if the COM interfaces have changed. You 
should copy the necessary DLLs to libs\analyst-<version> for Analyst software 
(e.g. libs\analyst-1.4 for Analyst v1.4.x) and libs\analystqs-<version> for 
AnalystQS software (e.g. libs\analystqs-1.1 for AnalystQS v1.1).  You should 
build the corresponding wiff<version>lib or wiffqs<version>lib project 
accordingly before performing the compilation of mzWiff.  These DLLs should be 
in libs\analyst-<version> or libs\analystqs-<version>:

    1. AcqMethodSvr.dll
    2. ExploreDataObjects.dll
    3. FileManager.dll
    4. MSMethodSvr.dll
    5. Peak_Finder2.dll
    6. PeakFinderFactory.dll
    7. WIFFTransSvr.dll
    8. ParameterSvr.dll

You should NOT commit these DLLs into the source code repository.

Instead, go thru' each of the .tlh files and change the full path of 
#include "<.tli file>" to a relative path.

Please commit both the .tlh and .tli files that you have generated, so that 
other developers can use them to compile mzWiff.


Performance
-------------
If you wish to squeeze another 5-10% performance, define "NODEBUGDETAIL" in
your project preprocessor definition.  This excludes codes that provides 
detailed debug output, which incur computation cost in loops.  Please note 
that you loose an important helper for diagnosis.

When .wiff file resides on network mapped drive, performance takes a big 
penalty of 7x-20x slower.
