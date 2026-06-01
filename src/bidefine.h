#ifndef IOBUFFER_DEFINES_H
#define IOBUFFER_DEFINES_H

#include <limits.h>

#if defined(__GNUC__) && __STDC_VERSION__ < 199901L
#  define B_NO_GCC_WARN_BEGIN() _Pragma(IOBUFFER_STRINGIFY(GCC diagnostic push))
#  define B_NO_GCC_WARN(name)   _Pragma(IOBUFFER_STRINGIFY(GCC diagnostic ignored name))
#  define B_NO_GCC_WARN_END()   _Pragma(IOBUFFER_STRINGIFY(GCC diagnostic pop))
#else
#  define B_NO_GCC_WARN_BEGIN()
#  define B_NO_GCC_WARN(name)
#  define B_NO_GCC_WARN_END()
#endif

/* Boolean and return codes */

#if __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
typedef unsigned char bool;
#  define false ((bool)0)
#  define true  ((bool)1)
#endif

#define B_FAIL 1
#define B_OKEY 0

/* Shortcut typedefs */

typedef   signed char schar;
typedef unsigned char uchar;

typedef unsigned short ushort;
typedef unsigned int   uint  ;
typedef unsigned long  ulong ;

B_NO_GCC_WARN_BEGIN()
B_NO_GCC_WARN("-Wlong-long")
typedef   signed long long sllong;
typedef unsigned long long ullong;
B_NO_GCC_WARN_END()

/* stdint.h for C89 */

#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>
#else
#  if ULONG_MAX >= ULLONG_MAX
typedef   long  intmax_t;
typedef  ulong uintmax_t;
#  else
typedef sllong  intmax_t;
typedef ullong uintmax_t;
#  endif
#endif

#define SIZE_MAX ((size_t)0 - 1)
#define UINTMAX_BITS (sizeof(uintmax_t) * CHAR_BIT)

/* Declarations of immediately functions */

int biimmputc(int ch,                      BUFFER* buf, int* accumulator);
int biimmputs(const char* str, size_t len, BUFFER* buf, int* accumulator);
int biimmrepc(int ch, size_t count,        BUFFER* buf, int* accumulator);

int biimmcmp (const char* str, size_t len, BUFFER* buf, int* accumulator);

int biimmpeek(BUFFER* buf);
int biimmskip(BUFFER* buf);

/* Declarations of formatted io functions */

int vbiscanf (BUFFER* buf, const char* fmt, va_list args);
int vbiprintf(BUFFER* buf, const char* fmt, va_list args);

#endif /* IOBUFFER_DEFINES_H */