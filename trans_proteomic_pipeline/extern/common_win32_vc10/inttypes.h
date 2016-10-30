/*
   inttypes.h

   Contributors:
     Created by Marek Michalkiewicz <marekm@linux.org.pl>

   THIS SOFTWARE IS NOT COPYRIGHTED

   This source code is offered for use in the public domain.  You may
   use, modify or distribute it freely.

   This code is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY.  ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
   DISCLAIMED.  This includes but is not limited to warranties of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __INTTYPES_H_
#define __INTTYPES_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#ifdef _STD_USING // trouble with MSVC inttypes being declared in std instead of global
 #undef _STD_USING
#endif
#include <stdint.h>
#else

/* Use [u]intN_t if you need exactly N bits.
   XXX - doesn't handle the -mint8 option.  */

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef int int16_t;
typedef int intptr_t;
typedef unsigned int uint16_t;
typedef unsigned int uintptr_t;

typedef long int32_t;
typedef unsigned long uint32_t;

#ifdef _MSC_VER
#ifndef int64_t
typedef __int64 int64_t;
#endif
typedef unsigned __int64 uint64_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

#endif
#endif