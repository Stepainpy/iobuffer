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

/* ======= Preprocessor conditions ======= */

#if __STDC_VERSION__ < 199901L
#  define restrict __restrict
#endif

#ifdef __GNUC__
#  define __bprintf_attr(ftc) __attribute__((format(printf, 2, ftc)))
#else
#  define __bprintf_attr(...)
#endif

#ifdef IOBUFFER_AS_DLL
#  ifdef IOBUFFER_SOURCE
#    define IOBUFFER_API __declspec(dllexport)
#  else
#    define IOBUFFER_API __declspec(dllimport)
#  endif
#else
#  define IOBUFFER_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* =========== Macro constants =========== */

#define EOB (-1)
#define BSEEK_SET 0
#define BSEEK_CUR 1
#define BSEEK_END 2

/* ================ Types ================ */

typedef struct BUFFER BUFFER;
typedef size_t bpos_t;
typedef void* (*balloc_t)(void*, void*, size_t);

/* ============= Allocation ============== */

IOBUFFER_API int bsetalloc(balloc_t alloc_func, void* userdata);

/* ============ Buffer access ============ */

IOBUFFER_API BUFFER* bopen   (const void* restrict data, size_t size, const char* restrict mode);
IOBUFFER_API BUFFER* bmemopen(      void* restrict data, size_t size, const char* restrict mode);
IOBUFFER_API void bclose(BUFFER* buffer);

/* ======== Operations on buffer ========= */

IOBUFFER_API void berase(BUFFER* buffer, size_t count);
IOBUFFER_API void breset(BUFFER* buffer);

/* ========= Buffer positioning ========== */

IOBUFFER_API int bgetpos(BUFFER* restrict buffer,       bpos_t* restrict pos);
IOBUFFER_API int bsetpos(BUFFER*          buffer, const bpos_t*          pos);

IOBUFFER_API long btell(BUFFER* buffer);
IOBUFFER_API int  bseek(BUFFER* buffer, long offset, int origin);

IOBUFFER_API void brewind(BUFFER* buffer);

/* ========= Direct input/output ========= */

IOBUFFER_API size_t bread (      void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);
IOBUFFER_API size_t bwrite(const void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);

/* ====== Unformatted input/output ======= */

IOBUFFER_API int bgetc(BUFFER* buffer);
IOBUFFER_API int bpeek(BUFFER* buffer);
IOBUFFER_API char* bgets(char* restrict str, int count, BUFFER* restrict buffer);

IOBUFFER_API int bputc(int byte, BUFFER* buffer);
IOBUFFER_API int bputs(const char* restrict string, BUFFER* restrict buffer);

IOBUFFER_API int bungetc(int byte, BUFFER* buffer);

/* ======= Formatted input/output ======== */

IOBUFFER_API int  bprintf(BUFFER* restrict buffer, const char* restrict format, ...         ) __bprintf_attr(3);
IOBUFFER_API int vbprintf(BUFFER* restrict buffer, const char* restrict format, va_list list) __bprintf_attr(0);

/* =========== Error handling ============ */

IOBUFFER_API int beob(BUFFER* buffer);

/* =========== View extension ============ */

typedef struct BUFVIEW {
    const void* base;
    const void* head;
    const void* stop;
} BUFVIEW;

IOBUFFER_API BUFVIEW bview(BUFFER* buffer);

#define BV_FMT "%.*s"
#define BV_ARG(view, from) (int)BV_SIZE(view, from), (const char*)(view).from
#define BV_SIZE(view, from) ((char*)(view).stop - (char*)(view).from)

#ifdef __cplusplus
}
#endif

#endif /* IO_DYNAMIC_BUFFER_H */