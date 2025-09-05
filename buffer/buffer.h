/* * * * * * * * * * * * * * * * * * * * * *
 * Dynamic buffer with API like as std IO. *
 * Support standard C89 and later.         *
 * Functions bscanf and vbscanf not exist. *
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

/* ============ Buffer access ============ */

BUFFER* bopen(void);
void bclose(BUFFER* buffer);
void breset(BUFFER* buffer); /* Extension */
/* Extension. Erase 'count' bytes starting from the current position */
void berase(BUFFER* buffer, size_t count);

/* ========= Buffer positioning ========== */

int bgetpos(BUFFER* restrict buffer,       bpos_t* restrict pos);
int bsetpos(BUFFER*          buffer, const bpos_t*          pos);
long btell(BUFFER* buffer);
int  bseek(BUFFER* buffer, long offset, int origin);
void brewind(BUFFER* buffer);

/* ========= Direct input/output ========= */

size_t bread (      void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);
size_t bwrite(const void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);

/* ====== Unformatted input/output ======= */

int bgetc(BUFFER* buffer);
char* bgets(char* restrict str, int count, BUFFER* restrict buffer);
int bputc(int byte, BUFFER* buffer);
int bputs(const char* restrict string, BUFFER* restrict buffer);
int bungetc(int byte, BUFFER* buffer);
int bpeek(BUFFER* buffer); /* Extension */

/* ======= Formatted input/output ======== */

int  bprintf(BUFFER* restrict buffer, const char* restrict format, ...         ) __bprintf_attr(3);
int vbprintf(BUFFER* restrict buffer, const char* restrict format, va_list list) __bprintf_attr(0);

/* =========== Error handling ============ */

int beob(BUFFER* buffer);

/* =========== View extension ============ */

typedef struct BUFVIEW {
    const void* base;
    const void* head;
    const void* stop;
} BUFVIEW;

#define BV_FMT "%.*s"
#define BV_ARG(view, from) \
    (int)((char*)(view).stop - (char*)(view).from), \
    (const char*)(view).from

BUFVIEW bview(BUFFER* buffer);

#ifdef __cplusplus
}
#endif

#endif /* IO_DYNAMIC_BUFFER_H */