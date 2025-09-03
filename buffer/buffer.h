/* * * * * * * * * * * * * * * * * * * * * *
 * Dynamic buffer with API like as std IO. *
 * Most function available in C89, but     *
 * bprintf and vbprintf available in C99.  *
 * Functions bscanf and vbscanf not exist. *
 * * * * * * * * For example * * * * * * * *
 *        fwrite -> write to  file         *
 *        bwrite -> write to buffer        *
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef IO_DYNAMIC_BUFFER_H
#define IO_DYNAMIC_BUFFER_H

#include <stddef.h>

#ifdef __GNUC__
#  define __bprintf_gccattr __attribute__((format(printf, 2, 3)))
#else
#  define __bprintf_gccattr
#  if __STDC_VERSION__ >= 199901L
#    define __restrict__ restrict
#  else
#    define __restrict__
#  endif /* C99 */
#endif /* ifdef __GNUC__ */

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

int bgetpos(BUFFER* __restrict__ buffer,       bpos_t* __restrict__ pos);
int bsetpos(BUFFER*              buffer, const bpos_t*              pos);
long btell(BUFFER* buffer);
int  bseek(BUFFER* buffer, long offset, int origin);
void brewind(BUFFER* buffer);

size_t bread (      void* __restrict__ data, size_t size, size_t count, BUFFER* __restrict__ buffer);
size_t bwrite(const void* __restrict__ data, size_t size, size_t count, BUFFER* __restrict__ buffer);

int bgetc(BUFFER* buffer);
char* bgets(char* __restrict__ str, int count, BUFFER* __restrict__ buffer);
int bputc(int byte, BUFFER* buffer);
int bputs(const char* __restrict__ string, BUFFER* __restrict__ buffer);
int bungetc(int byte, BUFFER* buffer);

int beob(BUFFER* buffer);

#if __STDC_VERSION__ >= 199901L
#include <stdarg.h>

int vbprintf(BUFFER* __restrict__ buffer, const char* __restrict__ format, va_list list);
int  bprintf(BUFFER* __restrict__ buffer, const char* __restrict__ format, ...) __bprintf_gccattr;
#endif /* C99 */

/* Buffer API extension */

#if __STDC_VERSION__ >= 199901L
#  define bloop(bytevar, buffer) for (int bytevar; (bytevar = bgetc(buffer)) != EOB;)
#endif /* C99 */

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