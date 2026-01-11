#define IOBUFFER_SOURCE
#include "iobuffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
typedef unsigned char bool;
#  define false ((bool)0)
#  define true  ((bool)1)
#endif

/* taken from GMP: https://github.com/alisw/GMP/blob/master/gmp-impl.h#L305 */
#ifndef va_copy
#  ifdef __va_copy
#    define va_copy(d, s) __va_copy(d, s)
#  else
#    define va_copy(d, s) do memcpy(&(d), &(s), sizeof(va_list)); while (0)
#  endif
#endif

#define B_FAIL 1
#define B_OKEY 0

typedef unsigned char uchar;
typedef unsigned long ulong;

struct BUFFER {
    uchar* data;
    size_t count;
    size_t capacity;
    bpos_t cursor;

    balloc_t alloc;
    void*    udata;

    bool readable;
    bool writable;
    bool allocated;
    bool fixed;
};

static size_t bimin(size_t a, size_t b) { return a < b ? a : b; }
static size_t bimax(size_t a, size_t b) { return a > b ? a : b; }

static void* bidfltalloc(void* ptr, size_t size, void* ud) {
    if (size) return realloc(ptr, size);
    free(ptr); (void)ud;
    return NULL;
}

static balloc_t bialloc = bidfltalloc; /* current allocator function */
static void*    biudata = NULL;        /* userdata for that */

static int birequire(BUFFER* buf, size_t require) {
    size_t newcap = buf->capacity; void* newplace;
    if (buf->cursor + require <= newcap) return B_OKEY;
    if (buf->fixed) return B_FAIL;

    if (newcap == 0) newcap = 1024; /* init */
    while (buf->cursor + require > newcap)
        /* growth by law 'new = ceil(old * phi)', phi ~ 207/128 */
        newcap = (newcap * 207 >> 7) + !!(newcap * 207 & 0x40);

    newplace = buf->alloc(buf->data, newcap, buf->udata);
    if (!newplace) return B_FAIL;

    buf->data = newplace;
    buf->capacity = newcap;
    return B_OKEY;
}

static int biparsemode(const char* mode, BUFFER* buf) {
    if (mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') return B_FAIL;
    if (mode[1] != '+' && mode[1] != '\0') return B_FAIL;
    if (mode[1] == '+' && mode[2] != '\0') return B_FAIL;

    /**/ if (mode[1] == '+') buf->readable = buf->writable = true;
    else if (mode[0] == 'r') buf->readable                 = true;
    else                                     buf->writable = true;

    return B_OKEY;
}

IOBUFFER_API int bsetalloc(balloc_t alloc, void* udata) {
    /*  */ if (alloc) {
        bialloc = alloc;
        biudata = udata;
        return B_OKEY;
    } else if (!udata) {
        bialloc = bidfltalloc;
        biudata = NULL;
        return B_OKEY;
    } else
        return B_FAIL;
}

IOBUFFER_API BUFFER* bopen(const void* restrict data, size_t size, const char* restrict mode) {
    BUFFER* buf = bialloc(NULL, sizeof *buf, biudata);
    if (!buf) return NULL;

    memset(buf, 0, sizeof *buf);
    buf->alloc = bialloc;
    buf->udata = biudata;
    buf->allocated = true;

    if (!data && size > 0) goto error;
    if (!mode || biparsemode(mode, buf)) goto error;

    if (buf->readable || mode[0] == 'a') {
        if (birequire(buf, size)) goto error;
        memcpy(buf->data, data, size);
        buf->count = size;
    }

    if (mode[0] == 'a')
        buf->cursor = buf->count;

    return buf;
error:
    buf->alloc(buf, 0, buf->udata);
    return NULL;
}

IOBUFFER_API BUFFER* bmemopen(void* restrict data, size_t size, const char* restrict mode) {
    BUFFER* buf = bialloc(NULL, sizeof *buf, biudata);
    if (!buf) return NULL;

    memset(buf, 0, sizeof *buf);
    buf->alloc = bialloc;
    buf->udata = biudata;
    buf->fixed = true;

    if (!mode || biparsemode(mode, buf)) goto error;

    buf->capacity = size;
    if (data) {
        buf->data = data;
        buf->count = mode[0] == 'w' ? 0 : size;
    } else if (size > 0) {
        buf->data = buf->alloc(NULL, size, buf->udata);
        if (!buf->data) goto error;
        buf->allocated = true;
    }

    if (mode[0] == 'a')
        buf->cursor = buf->count;

    return buf;
error:
    buf->alloc(buf, 0, buf->udata);
    return NULL;
}

IOBUFFER_API int bclose(BUFFER* buf) {
    if (!buf) return EOB;
    if (buf->allocated)
        buf->alloc(buf->data, 0, buf->udata);
    buf->alloc(buf, 0, buf->udata);
    return B_OKEY;
}

IOBUFFER_API int bgetpos(BUFFER* restrict buf, bpos_t* restrict pos) {
    if (!buf || !buf->data || !pos) return B_FAIL;
    *pos = buf->cursor;
    return B_OKEY;
}

IOBUFFER_API int bsetpos(BUFFER* buf, const bpos_t* pos) {
    if (!buf || !buf->data || !pos) return B_FAIL;
    if (*pos > buf->count) return B_FAIL;
    buf->cursor = *pos;
    return B_OKEY;
}

IOBUFFER_API long btell(BUFFER* buf) {
    if (!buf || !buf->data) return -1L;
    if (buf->cursor > (~0UL >> 1)) return -1L;
    return buf->cursor;
}

IOBUFFER_API int bseek(BUFFER* buf, long off, int org) {
    if (!buf || !buf->data) return B_FAIL;

    switch (org) {
        case BSEEK_SET:
            if (off < 0 || (ulong) off > buf->count) return B_FAIL;
            buf->cursor = off;
            break;
        case BSEEK_CUR:
            if (off > 0 && (ulong) off > buf->count - buf->cursor) return B_FAIL;
            if (off < 0 && (ulong)-off >              buf->cursor) return B_FAIL;
            buf->cursor += off;
            break;
        case BSEEK_END:
            if (off > 0 || (ulong)-off > buf->count) return B_FAIL;
            buf->cursor = buf->count + off;
            break;
        default:
            return B_FAIL;
    }

    return B_OKEY;
}

IOBUFFER_API void brewind(BUFFER* buf) {
    if (!buf) return;
    buf->cursor = 0;
}

IOBUFFER_API int bgetc(BUFFER* buf) {
    if (!buf || !buf->data || !buf->readable) return EOB;
    if (buf->cursor == buf->count) return EOB;
    return buf->data[buf->cursor++];
}

IOBUFFER_API char* bgets(char* restrict str, int count, BUFFER* restrict buf) {
    uchar* newline; size_t minlen, offset;
    if (!buf || !buf->data || !str) return NULL;
    if (!buf->readable) return NULL;

    if (buf->count == buf->cursor) return NULL;
    if (count < 1) return NULL;
    if (count == 1) {
        str[0] = '\0';
        return str;
    }

    newline = memchr(buf->data + buf->cursor, '\n', buf->count - buf->cursor);
    if (newline) {
        offset = newline - buf->data - buf->cursor;
        minlen = bimin(offset + 1, count - 1);
    } else
        minlen = bimin(buf->count - buf->cursor, count - 1);

    memcpy(str, buf->data + buf->cursor, minlen);
    buf->cursor += minlen;
    str[minlen] = '\0';

    return str;
}

IOBUFFER_API int bputc(int ch, BUFFER* buf) {
    if (!buf || !buf->writable) return EOB;
    if (birequire(buf, 1)) return EOB;

    buf->data[buf->cursor++] = (uchar)ch;
    buf->count = bimax(buf->count, buf->cursor);

    return ch;
}

IOBUFFER_API int bputs(const char* restrict str, BUFFER* restrict buf) {
    size_t len;
    if (!buf || !str || !buf->writable) return EOB;

    len = strlen(str);
    if (birequire(buf, len)) return EOB;

    memcpy(buf->data + buf->cursor, str, len);
    buf->count = bimax(buf->count, buf->cursor += len);

    return B_OKEY;
}

IOBUFFER_API int bungetc(int ch, BUFFER* buf) {
    if (!buf || !buf->data) return EOB;
    if (!buf->readable) return EOB;
    if (ch == EOB) return EOB;

    if (buf->cursor == 0) return EOB;
    buf->data[--buf->cursor] = (uchar)ch;

    return ch;
}

IOBUFFER_API int bprintf(BUFFER* restrict buf, const char* restrict fmt, ...) {
    int len; va_list args; uchar saved;
    if (!buf || !fmt || !buf->writable) return EOB;

    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (len < 0 || birequire(buf, len + 1)) return EOB;
    saved = buf->data[buf->cursor + len];

    va_start(args, fmt);
    vsprintf((char*)buf->data + buf->cursor, fmt, args);
    va_end(args);

    buf->data[buf->cursor += len] = saved;
    buf->count = bimax(buf->count, buf->cursor);

    return len;
}

IOBUFFER_API int vbprintf(BUFFER* restrict buf, const char* restrict fmt, va_list args) {
    int len; va_list acpy; uchar saved;
    if (!buf || !fmt || !buf->writable) return EOB;

    va_copy(acpy, args);
    len = vsnprintf(NULL, 0, fmt, acpy);
    va_end(acpy);

    if (len < 0 || birequire(buf, len + 1)) return EOB;
    saved = buf->data[buf->cursor + len];

    va_copy(acpy, args);
    vsprintf((char*)buf->data + buf->cursor, fmt, acpy);
    va_end(acpy);

    buf->data[buf->cursor += len] = saved;
    buf->count = bimax(buf->count, buf->cursor);

    return len;
}

IOBUFFER_API size_t bread(void* restrict data, size_t size, size_t count, BUFFER* restrict buf) {
    size_t read;
    if (!buf || !buf->data || !buf->readable) return 0;
    if (!data || !size || !count) return 0;

    read = bimin((buf->count - buf->cursor) / size, count);
    memcpy(data, buf->data + buf->cursor, read * size);
    buf->cursor += read * size;

    return read;
}

IOBUFFER_API size_t bwrite(const void* restrict data, size_t size, size_t count, BUFFER* restrict buf) {
    if (!buf || !size || !count || !buf->writable) return 0;

    if (birequire(buf, size * count))
        count = (buf->capacity - buf->cursor) / size;

    memcpy(buf->data + buf->cursor, data, size * count);
    buf->count = bimax(buf->count, buf->cursor += size * count);

    return count;
}

IOBUFFER_API int beob(BUFFER* buf) {
    if (!buf || !buf->data) return 0;
    return buf->cursor == buf->count;
}

/* API extension */

IOBUFFER_API int bpeek(BUFFER* buf) {
    if (!buf || !buf->data || !buf->readable) return EOB;
    if (buf->cursor == buf->count) return EOB;
    return buf->data[buf->cursor];
}

IOBUFFER_API int berase(BUFFER* buf, size_t count) {
    if (!buf || !buf->data || !buf->writable) return B_FAIL;
    count = bimin(count, buf->count - buf->cursor);
    memmove(buf->data  + buf->cursor,
            buf->data  + buf->cursor + count,
            buf->count - buf->cursor - count);
    buf->count -= count;
    return B_OKEY;
}

IOBUFFER_API int breset(BUFFER* buf) {
    if (!buf || !buf->data || !buf->writable) return B_FAIL;
    memset(buf->data, 0, buf->capacity);
    buf->cursor = buf->count = 0;
    return B_OKEY;
}

/* View extension */

IOBUFFER_API BUFVIEW bview(BUFFER* buf) {
    BUFVIEW view = {0};
    if (buf && buf->data) {
        view.base = buf->data;
        view.head = buf->data + buf->cursor;
        view.stop = buf->data + buf->count;
    }
    return view;
}