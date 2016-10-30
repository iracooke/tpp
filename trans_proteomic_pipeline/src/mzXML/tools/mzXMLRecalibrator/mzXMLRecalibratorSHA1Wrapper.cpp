#include "mzXMLRecalibratorSHA1Wrapper.hpp"
#include "mzXMLRecalibratorSHA1.hpp"
#include <cstdlib>


int sha1_hashFile( char* szFileName , char *szReport )
{
  CSHA1 sha1;

  sha1.Reset();
  if( !(sha1.HashFile( szFileName )) )
    {
      printf( "Cannot open file %s for sha-1 calculation\n", szFileName);
      return 1;		// Cannot open file
    }
  sha1.Final();
  sha1.ReportHash( szReport, CSHA1::REPORT_HEX );
  
  return 0;	// No errors
}
