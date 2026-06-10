/* * * * * * * * * * * * * * * * * * * * * *
 * Dynamic buffer with API like as std IO. *
 * Support standard C89 and later.         *
 * * * * * * * * For example * * * * * * * *
 *        fwrite -> write to  file         *
 *        bwrite -> write to buffer        *
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef IOBUFFER_H
#define IOBUFFER_H

#include <stdarg.h>
#include <stddef.h>

/* Version macros */

#define B_STRINGIFY_IMPL(x) # x
#define B_STRINGIFY(x) B_STRINGIFY_IMPL(x)

#define IOBUFFER_VERSION_MAJOR 3
#define IOBUFFER_VERSION_MINOR 0
#define IOBUFFER_VERSION_PATCH 2

#define IOBUFFER_VERSION \
    B_STRINGIFY(IOBUFFER_VERSION_MAJOR) "." \
    B_STRINGIFY(IOBUFFER_VERSION_MINOR) "." \
    B_STRINGIFY(IOBUFFER_VERSION_PATCH)

/* Preprocessor conditions */

#if __STDC_VERSION__ < 199901L
#  define restrict __restrict
#endif

#ifdef __GNUC__
#  define B_ATTR_PRINT_FMT(va_i) __attribute__(( format(printf, 2, va_i) ))
#  define B_ATTR_SCAN__FMT(va_i) __attribute__(( format( scanf, 2, va_i) ))
#  ifdef __clang__
#    define B_ATTR_MALLOC __attribute__(( malloc ))
#  else
#    define B_ATTR_MALLOC __attribute__(( malloc, malloc(bclose, 1) ))
#  endif
#else
#  define B_ATTR_PRINT_FMT(va_i)
#  define B_ATTR_SCAN__FMT(va_i)
#  define B_ATTR_MALLOC
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

BUFFER* bopen   (const void* restrict data, size_t size, const char* restrict mode) B_ATTR_MALLOC;
BUFFER* bmemopen(      void* restrict data, size_t size, const char* restrict mode) B_ATTR_MALLOC;

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

int   bscanf(BUFFER* restrict buffer, const char* restrict format, ...         ) B_ATTR_SCAN__FMT(3);
int  vbscanf(BUFFER* restrict buffer, const char* restrict format, va_list list) B_ATTR_SCAN__FMT(0);

int  bprintf(BUFFER* restrict buffer, const char* restrict format, ...         ) B_ATTR_PRINT_FMT(3);
int vbprintf(BUFFER* restrict buffer, const char* restrict format, va_list list) B_ATTR_PRINT_FMT(0);

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
#define BV_ARG(view, begin, end) (int)BV_LEN(view, begin, end), (const char*)(view).begin
#define BV_LEN(view, begin, end) ((char*)(view).end - (char*)(view).begin)

#ifdef __cplusplus
}
#endif

#endif /* IOBUFFER_H */