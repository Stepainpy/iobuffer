#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#define B_FAIL 1
#define B_OKEY 0
#define B_INIT_CAP 1024

#define b_min(a, b) ((a) < (b) ? (a) : (b))
#define b_max(a, b) ((a) > (b) ? (a) : (b))

/* taken from GMP:
 * https://github.com/WinBuilds/gmplib/blob/ed48f534df05428c2474c1bde037e84e057a3972/gmp-impl.h#L304
 */
#ifndef va_copy
#  ifdef __va_copy
#    define va_copy(d, s) __va_copy(d, s)
#  else
#    define va_copy(d, s) do memcpy(&(d), &(s), sizeof(va_list)); while (0)
#  endif
#endif

typedef unsigned char uchar;
typedef unsigned long ulong;

struct BUFFER {
    uchar* data;
    size_t count, capacity;
    bpos_t cursor;
};

/* 0 upon success, 1 otherwise
 * takes into account the null-terminator character
 */
static int breserve(BUFFER* buf, size_t add_size) {
    if (buf->cursor + add_size <= buf->capacity)
        return B_OKEY;

    if (buf->capacity == 0) buf->capacity = B_INIT_CAP;
    while (buf->cursor + add_size > buf->capacity)
        buf->capacity += buf->capacity / 2; /* + 50% */
    buf->data = realloc(buf->data, buf->capacity);

    if (!buf->data) memset(buf, 0, sizeof *buf);
    return buf->data == NULL;
}

BUFFER* bopen(void) {
    BUFFER* buf = malloc(sizeof *buf);
    if (buf) memset(buf, 0, sizeof *buf);
    return buf;
}

void bclose(BUFFER* buf) {
    if (buf) free(buf->data);
    free(buf);
}

int bgetpos(BUFFER* restrict buf, bpos_t* restrict pos) {
    if (!buf || !buf->data || !pos) return B_FAIL;
    *pos = buf->cursor;
    return B_OKEY;
}

int bsetpos(BUFFER* buf, const bpos_t* pos) {
    if (!buf || !buf->data || !pos) return B_FAIL;
    if (*pos > buf->count) return B_FAIL;
    buf->cursor = *pos;
    return B_OKEY;
}

long btell(BUFFER* buf) {
    if (!buf || !buf->data) return -1L;
    if (buf->cursor > LONG_MAX) return -1L;
    return buf->cursor;
}

int bseek(BUFFER* buf, long off, int org) {
    if (!buf || !buf->data) return B_FAIL;
    if (org > BSEEK_END || org < BSEEK_SET) return B_FAIL;

    switch (org) {
        case BSEEK_SET: {
            if (off < 0 || (ulong) off > buf->count) return B_FAIL;
            buf->cursor = off;
        } break;
        case BSEEK_CUR: {
            if (off > 0 && (ulong) off > buf->count - buf->cursor) return B_FAIL;
            if (off < 0 && (ulong)-off > buf->cursor) return B_FAIL;
            buf->cursor += off;
        } break;
        case BSEEK_END: {
            if (off > 0 || (ulong)-off > buf->count) return B_FAIL;
            buf->cursor = buf->count + off;
        } break;
    }

    return B_OKEY;
}

void brewind(BUFFER* buf) {
    if (!buf) return;
    buf->cursor = 0;
}

int bgetc(BUFFER* buf) {
    if (!buf || !buf->data) return EOB;
    if (buf->cursor == buf->count) return EOB;
    return buf->data[buf->cursor++];
}

char* bgets(char* restrict str, int count, BUFFER* restrict buf) {
    uchar* newline; size_t minlen, offset;
    if (!buf || !buf->data || !str) return NULL;

    if (buf->count == buf->cursor) return NULL;
    if (count < 1) return NULL;
    if (count == 1) {
        str[0] = '\0';
        return str;
    }

    newline = memchr(buf->data + buf->cursor, '\n', buf->count - buf->cursor);
    if (newline) {
        offset = newline - buf->data - buf->cursor;
        minlen = b_min(offset + 1, (size_t)(count - 1));
    } else
        minlen = b_min(buf->count - buf->cursor, (size_t)(count - 1));

    memcpy(str, buf->data + buf->cursor, minlen);
    buf->cursor += minlen;
    str[minlen] = '\0';

    return str;
}

int bputc(int ch, BUFFER* buf) {
    if (!buf) return EOB;
    if (breserve(buf, 1)) return EOB;

    buf->data[buf->cursor++] = (uchar)ch;
    buf->count = b_max(buf->count, buf->cursor);

    return ch;
}

int bputs(const char* restrict str, BUFFER* restrict buf) {
    size_t len;
    if (!buf || !str) return EOB;

    len = strlen(str);
    if (breserve(buf, len)) return EOB;

    memcpy(buf->data + buf->cursor, str, len);
    buf->cursor += len;
    buf->count = b_max(buf->count, buf->cursor);

    return 0;
}

int bungetc(int ch, BUFFER* buf) {
    if (!buf || !buf->data) return EOB;
    if (ch == EOB) return EOB;

    if (buf->cursor == 0) return EOB;
    buf->data[--buf->cursor] = (uchar)ch;

    return ch;
}

int bprintf(BUFFER* restrict buf, const char* restrict fmt, ...) {
    int len; va_list args; uchar saved;
    if (!buf || !fmt) return -1;

    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (len < 0) return -1;
    if (breserve(buf, len)) return -1;

    saved = buf->data[buf->cursor + len];
    va_start(args, fmt);
    vsprintf((char*)buf->data + buf->cursor, fmt, args);
    va_end(args);

    buf->data[buf->cursor += len] = saved;
    buf->count = b_max(buf->count, buf->cursor);

    return len;
}

int vbprintf(BUFFER* restrict buf, const char* restrict fmt, va_list args) {
    int len; va_list acpy; uchar saved;
    if (!buf || !fmt) return -1;

    va_copy(acpy, args);
    len = vsnprintf(NULL, 0, fmt, acpy);
    va_end(acpy);

    if (len < 0) return -1;
    if (breserve(buf, len)) return -1;

    saved = buf->data[buf->cursor + len];
    va_copy(acpy, args);
    vsprintf((char*)buf->data + buf->cursor, fmt, acpy);
    va_end(acpy);

    buf->data[buf->cursor += len] = saved;
    buf->count = b_max(buf->count, buf->cursor);

    return len;
}

size_t bread(void* restrict data, size_t size, size_t count, BUFFER* restrict buf) {
    size_t read;
    if (!buf || !buf->data) return 0;
    if (!data || !size) return 0;

    read = b_min((buf->count - buf->cursor) / size, count);
    memcpy(data, buf->data + buf->cursor, read * size);
    buf->cursor += read * size;

    return read;
}

size_t bwrite(const void* restrict data, size_t size, size_t count, BUFFER* restrict buf) {
    size_t bytes = size * count;
    if (!buf || bytes == 0) return 0;
    if (breserve(buf, bytes)) return 0;

    memcpy(buf->data + buf->cursor, data, bytes);
    buf->cursor += bytes;
    buf->count = b_max(buf->count, buf->cursor);

    return count;
}

int beob(BUFFER* buf) {
    if (!buf || !buf->data) return 0;
    return buf->cursor == buf->count;
}

/* Buffer API extension */

int bpeek(BUFFER* buf) {
    if (!buf || !buf->data) return EOB;
    if (buf->cursor == buf->count) return EOB;
    return buf->data[buf->cursor];
}

void breset(BUFFER* buf) {
    if (!buf) return;
    buf->cursor = buf->count = 0;
    if (!buf->data) return;
    memset(buf->data, 0, buf->capacity);
}

/* Buffer view extension */

BUFVIEW bview(BUFFER* buf) {
    BUFVIEW view = {0};
    if (buf && buf->data) {
        view.base = buf->data;
        view.head = buf->data + buf->cursor;
        view.stop = buf->data + buf->count;
    }
    return view;
}