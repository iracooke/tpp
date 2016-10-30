#include "indexmzXMLSHA1Wrapper.hpp"
#include "indexmzXMLSHA1.hpp"
#include <cstdlib>


int sha1_hashFile( char* szFileName , char *szReport )
{
  iCSHA1 sha1;

  sha1.Reset();
  if( !(sha1.HashFile( szFileName )) )
    {
      printf( "Cannot open file %s for sha-1 calculation\n", szFileName);
      return 1;		// Cannot open file
    }
  sha1.Final();
  sha1.ReportHash( szReport, iCSHA1::REPORT_HEX );
  
  return 0;	// No errors
}
