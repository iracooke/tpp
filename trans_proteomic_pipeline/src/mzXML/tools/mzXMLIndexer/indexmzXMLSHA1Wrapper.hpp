/*
#if defined( __SHA1_WRAPPER_H )
#else
#define __SHA1_WRAPPER_H
*/

#if defined __cplusplus
extern "C"
{
#endif

  int sha1_hashFile(char *szFileName , char *szReport);

#if defined __cplusplus
}
#endif

  //#endif
