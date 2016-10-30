// dummy unistd.h file for use with MSVC
#if _MSC_VER < 1400
typedef int32_t ssize_t;
typedef int gid_t;
typedef int uid_t;
typedef int mode_t;
#else
typedef int ssize_t;
#endif
#include <io.h> // gets you write(2) as in real unistd.h
