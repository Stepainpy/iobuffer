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

#if __STDC_VERSION__ < 199901L
#  if defined(__GNUC__) && !defined(__clang__)
#    define restrict __restrict__
#  else
#    define restrict
#  endif
#endif

#ifdef __GNUC__
#  define __bprintf_attr(ftc) __attribute__((format(printf, 2, ftc)))
#else
#  define __bprintf_attr(...)
#endif /* __GNUC__ */

#ifdef __cplusplus
extern "C" {
#endif

#define EOB (-1)
#define BSEEK_SET 0
#define BSEEK_CUR 1
#define BSEEK_END 2

typedef struct BUFFER BUFFER;
typedef size_t bpos_t;

BUFFER* bopen(void);
void bclose(BUFFER* buffer);

int bgetpos(BUFFER* restrict buffer,       bpos_t* restrict pos);
int bsetpos(BUFFER*          buffer, const bpos_t*          pos);
long btell(BUFFER* buffer);
int  bseek(BUFFER* buffer, long offset, int origin);
void brewind(BUFFER* buffer);

size_t bread (      void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);
size_t bwrite(const void* restrict data, size_t size, size_t count, BUFFER* restrict buffer);

int bgetc(BUFFER* buffer);
char* bgets(char* restrict str, int count, BUFFER* restrict buffer);
int bputc(int byte, BUFFER* buffer);
int bputs(const char* restrict string, BUFFER* restrict buffer);
int bungetc(int byte, BUFFER* buffer);

int beob(BUFFER* buffer);

int  bprintf(BUFFER* restrict buffer, const char* restrict format, ...         ) __bprintf_attr(3);
int vbprintf(BUFFER* restrict buffer, const char* restrict format, va_list list) __bprintf_attr(0);

/* Buffer API extension */

int bpeek(BUFFER* buffer);
void breset(BUFFER* buffer);

/* Buffer view extension */

typedef struct BUFVIEW {
    const void* data;
    size_t      size;
} BUFVIEW;

#define BV_FMT "%.*s"
#define BV_ARG(view) (int)(view).size, (const char*)(view).data

BUFVIEW     bview(BUFFER* buffer);
const void* bdata(BUFFER* buffer);
size_t      bsize(BUFFER* buffer);

#ifdef __cplusplus
}
#endif

#endif /* IO_DYNAMIC_BUFFER_H */