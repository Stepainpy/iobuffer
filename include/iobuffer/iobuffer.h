/* * * * * * * * * * * * * * * * * * * * * *
 * Dynamic buffer with API like as std IO. *
 * Support standard C89 and later.         *
 * * * * * * * * For example * * * * * * * *
 *        fwrite -> write to  file         *
 *        bwrite -> write to buffer        *
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef IO_DYNAMIC_BUFFER_H
#define IO_DYNAMIC_BUFFER_H

#include <stdarg.h>
#include <stddef.h>

/* Version macros */

#define __IOBUFFER_STRINGIFY(x) # x
#define   IOBUFFER_STRINGIFY(x) __IOBUFFER_STRINGIFY(x)

#define IOBUFFER_VERSION_MAJOR 2
#define IOBUFFER_VERSION_MINOR 3
#define IOBUFFER_VERSION_PATCH 0

#define IOBUFFER_VERSION \
    IOBUFFER_STRINGIFY(IOBUFFER_VERSION_MAJOR) "." \
    IOBUFFER_STRINGIFY(IOBUFFER_VERSION_MINOR) "." \
    IOBUFFER_STRINGIFY(IOBUFFER_VERSION_PATCH)

/* Preprocessor conditions */

#if __STDC_VERSION__ < 199901L
#  define restrict __restrict
#endif

#ifdef __GNUC__
#  define __battrprintfmt(va_i) __attribute__(( format(printf, 2, va_i) ))
#  define __battr_scanfmt(va_i) __attribute__(( format( scanf, 2, va_i) ))
#  ifdef __clang__
#    define __battrmalloc __attribute__(( malloc ))
#  else
#    define __battrmalloc __attribute__(( malloc, malloc(bclose, 1) ))
#  endif
#else
#  define __battrprintfmt(va_i)
#  define __battr_scanfmt(va_i)
#  define __battrmalloc
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Macro constants */

#define EOB (-1)
#define BSEEK_SET 0
#define BSEEK_CUR 1
#define BSEEK_END 2

/* Types */

typedef struct BUFFER BUFFER;
typedef size_t bpos_t;
typedef void* (*balloc_t)(void* ptr, size_t size, void* userdata);

/* Allocation */

int bsetalloc(balloc_t allocator, void* userdata);

/* Buffer access */

int bclose(BUFFER* buffer);

BUFFER* bopen   (const void* restrict data, size_t size, const char* restrict mode) __battrmalloc;
BUFFER* bmemopen(      void* restrict data, size_t size, const char* restrict mode) __battrmalloc;

/* Operations on buffer */

int berase(BUFFER* buffer, size_t count);
int breset(BUFFER* buffer);

/* Buffer positioning */

int bgetpos(BUFFER* restrict buffer,       bpos_t* restrict pos);
int bsetpos(BUFFER*          buffer, const bpos_t*          pos);

long btell(BUFFER* buffer);
int  bseek(BUFFER* buffer, long offset, int origin);

void brewind(BUFFER* buffer);

/* Direct input/output */

size_t bread (      void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);
size_t bwrite(const void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);

/* Unformatted input/output */

int bgetc(BUFFER* buffer);
int bpeek(BUFFER* buffer);

char* bgets(char* restrict str, int count, BUFFER* restrict buffer);

int bputc(int byte, BUFFER* buffer);
int bputs(const char* restrict string, BUFFER* restrict buffer);

int bungetc(int byte, BUFFER* buffer);

/* Formatted input/output */

int   bscanf(BUFFER* restrict buffer, const char* restrict format, ...         ) __battr_scanfmt(3);
int  vbscanf(BUFFER* restrict buffer, const char* restrict format, va_list list) __battr_scanfmt(0);

int  bprintf(BUFFER* restrict buffer, const char* restrict format, ...         ) __battrprintfmt(3);
int vbprintf(BUFFER* restrict buffer, const char* restrict format, va_list list) __battrprintfmt(0);

/* Error handling */

int beob(BUFFER* buffer);

/* View extension */

typedef struct BUFVIEW {
    const void* base;
    const void* head;
    const void* stop;
} BUFVIEW;

BUFVIEW bview(BUFFER* buffer);

#define BV_FMT "%.*s"
#define BV_ARG( view, begin, end) (int)BV_SIZE(view, begin, end), (const char*)(view).begin
#define BV_SIZE(view, begin, end) ((char*)(view).end - (char*)(view).begin)

#ifdef __cplusplus
}
#endif

#endif /* IO_DYNAMIC_BUFFER_H */