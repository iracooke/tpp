Project: wiffqs11lib
This project is created solely to generate the .tlh and .tli files via the 
inclusion of "analystqs11tlbs.h" with the preprocessor _IMPORT_PROCESSING_ 
defined so that the #import lines (in the header file)get processed.  
Consequently, there is only a "Debug" project configuration.


Directory:
trans_proteomic_pipeline\src\mzXML\converters\mzWiff\libs\analystqs-1.1\ReadMe.txt

This directory contains the AnalystQS 1.1 DLLs to be used by #import directive
via a relative path to generate .tlh and .tli files.

These DLLs are:

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

