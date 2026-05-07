#define IOBUFFER_SOURCE
#include "iobuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

typedef   signed char  schar;
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

#if defined(__GNUC__) && __STDC_VERSION__ < 199901L
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wlong-long"
#endif
typedef   signed long long sllong;
typedef unsigned long long ullong;
#if defined(__GNUC__) && __STDC_VERSION__ < 199901L
#  pragma GCC diagnostic pop
#endif

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
        newcap = (newcap * 207 + 127) / 128;

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
    int ret; va_list args;
    va_start(args, fmt);
    ret = vbprintf(buf, fmt, args);
    va_end(args);
    return ret;
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

/* Implementation of vbprintf */

typedef enum {
    BLM_NONE = 0,
    BLM_H,
    BLM_L,
    BLM_L_UPPER,
    BLM_HH,
    BLM_LL,
    BLM_J,
    BLM_Z,
    BLM_T
} bilenmod_t;

static int biimmputc(int ch, BUFFER* buf) {
    if (birequire(buf, 1)) return B_FAIL;
    buf->data[buf->cursor++] = (uchar)ch;
    buf->count = bimax(buf->count, buf->cursor);
    return B_OKEY;
}

static int biimmrepc(int ch, size_t count, BUFFER* buf) {
    if (birequire(buf, count)) return B_FAIL;
    memset(buf->data + buf->cursor, ch, count);
    buf->count = bimax(buf->count, buf->cursor += count);
    return B_OKEY;
}

static void biprinti(intmax_t number, char* outbuf) {
    char* end = outbuf;
    if (number < 0) do *end++ = '0' - number % 10; while (number /= 10);
    else            do *end++ = '0' + number % 10; while (number /= 10);
    *end = '\0';
    for (--end; outbuf < end; outbuf++, end--) {
        char tmp = *outbuf; *outbuf = *end; *end = tmp;
    }
}

static void biprintu(uintmax_t number, char* outbuf, int base, bool up) {
    const char* alphabet = up ? "0123456789ABCDEF" : "0123456789abcdef";
    char* end = outbuf;
    do *end++ = alphabet[number % base]; while (number /= base);
    *end = '\0';
    for (--end; outbuf < end; outbuf++, end--) {
        char tmp = *outbuf; *outbuf = *end; *end = tmp;
    }
}

IOBUFFER_API int vbprintf(BUFFER* restrict buf, const char* restrict fmt, va_list args) {
    int total_len = 0;
    if (!buf || !fmt || !buf->writable) return EOB;

    while (true) {
        const char* percent = strchr(fmt, '%');
        size_t len = percent ? (size_t)(percent - fmt) : strlen(fmt);

        if (birequire(buf, len))
            len = buf->capacity - buf->cursor;
        memcpy(buf->data + buf->cursor, fmt, len);
        buf->count = bimax(buf->count, buf->cursor += len);
        total_len += len;

        if (percent) {
            const char* fmtstr = percent + 1;
            if (*fmtstr == '%') {
                if (biimmputc('%', buf)) goto error;
                total_len += 1;
            } else {
                bilenmod_t lenmod = BLM_NONE;
                bool lead_zero = false;
                bool left_just = false;
                bool  alt_form = false;
                int  fld_width =  0;
                int  precision = -1;
                int    signing = -1;

                /* signing values:
                 * < 0  =>  print only minus
                 * = 0  =>  print with space
                 * > 0  =>  print with plus
                 */

                while (*fmtstr == ' ' || *fmtstr == '-' || *fmtstr == '+' ||
                       *fmtstr == '0' || *fmtstr == '#')
                    switch (*fmtstr++) {
                        case '0': lead_zero = true; break;
                        case '-': left_just = true; break;
                        case '#':  alt_form = true; break;
                        case '+':   signing = 1;    break;
                        case ' ':
                            if (signing < 0) signing = 0;
                            break;
                    }

                if ('1' <= *fmtstr && *fmtstr <= '9') {
                    fld_width = atoi(fmtstr);
                    if (fld_width == 0) goto error;
                    while ('0' <= *fmtstr && *fmtstr <= '9') ++fmtstr;
                } else if (*fmtstr == '*') {
                    int received = va_arg(args, int);
                    if (received < 0) {
                        received = -received;
                        left_just = true;
                    }
                    fld_width = received;
                    fmtstr += 1;
                }

                if (fmtstr[0] == '.' && '1' <= fmtstr[1] && fmtstr[1] <= '9') {
                    precision = atoi(++fmtstr);
                    if (precision == 0) goto error;
                    while ('0' <= *fmtstr && *fmtstr <= '9') ++fmtstr;
                } else if (fmtstr[0] == '.' && fmtstr[1] == '*') {
                    precision = va_arg(args, int);
                    fmtstr += 2;
                }

                switch (*fmtstr) {
                    case 'h':
                        lenmod = BLM_H; ++fmtstr;
                        if (*fmtstr != 'h') break;
                        lenmod = BLM_HH; ++fmtstr;
                        break;
                    case 'l':
                        lenmod = BLM_L; ++fmtstr;
                        if (*fmtstr != 'l') break;
                        lenmod = BLM_LL; ++fmtstr;
                        break;
                    case 'j': lenmod = BLM_J; ++fmtstr; break;
                    case 'z': lenmod = BLM_Z; ++fmtstr; break;
                    case 't': lenmod = BLM_T; ++fmtstr; break;
                    case 'L': lenmod = BLM_L_UPPER; ++fmtstr; break;
                }

                switch (*fmtstr) {
                    /* Cases of specifier */
                    case 'c':
                    if (lenmod != BLM_NONE) goto error;
                    {
                        int received = va_arg(args, int);
                        if ( left_just) {
                            if (biimmputc(received, buf)) goto error;
                            ++total_len;
                        }
                        if (fld_width > 1) {
                            if (biimmrepc(' ', fld_width - 1, buf)) goto error;
                            total_len += fld_width - 1;
                        }
                        if (!left_just) {
                            if (biimmputc(received, buf)) goto error;
                            ++total_len;
                        }
                    } break;
                    case 's':
                    if (lenmod != BLM_NONE) goto error;
                    {
                        const char* received = va_arg(args, const char*);
                        int len, maxlen;

                        if (!received) goto error;
                        len = strlen(received);
                        if (precision < 0) precision = len;
                        precision = bimin(precision, len);
                        maxlen = bimax(fld_width, precision);

                        if (birequire(buf, maxlen)) goto error;

                        if ( left_just) {
                            memcpy(buf->data + buf->cursor, received, precision);
                            buf->count = bimax(buf->count, buf->cursor += precision);
                            total_len += precision;
                        }
                        if (precision < maxlen) {
                            int padding = maxlen - precision;
                            if (biimmrepc(' ', padding, buf)) goto error;
                            total_len += padding;
                        }
                        if (!left_just) {
                            memcpy(buf->data + buf->cursor, received, precision);
                            buf->count = bimax(buf->count, buf->cursor += precision);
                            total_len += precision;
                        }
                    } break;
                    case 'd': case 'i': {
                        char tmpbuf[24] = {0};
                        bool is_neg; int len;
                        intmax_t received;

                        switch (lenmod) {
                            case BLM_NONE: received =        va_arg(args, int ); break;
                            case BLM_HH  : received = (schar)va_arg(args, int ); break;
                            case BLM_H   : received = (short)va_arg(args, int ); break;
                            case BLM_L   : received =        va_arg(args, long); break;
                            case BLM_LL  : received = va_arg(args,    sllong); break;
                            case BLM_J   : received = va_arg(args,  intmax_t); break;
                            case BLM_Z   : received = va_arg(args,    size_t); break;
                            case BLM_T   : received = va_arg(args, ptrdiff_t); break;
                            case BLM_L_UPPER: goto error;
                        }

                        biprinti(received, tmpbuf);
                        is_neg = received < 0;
                        len = strlen(tmpbuf);

                        if (precision < 0) {
                            if (lead_zero && !left_just)
                                precision = bimax(1, fld_width) - (signing >= 0 || is_neg);
                            else
                                precision = 1;
                        }

                        if (!left_just && (fld_width > (int)bimax(precision, len) + (signing >= 0 || is_neg))) {
                            size_t padding = fld_width - bimax(precision, len) - (signing >= 0 || is_neg);
                            if (biimmrepc(' ', padding, buf)) goto error;
                            total_len += padding;
                        }

                        if (is_neg) {
                            if (biimmputc('-', buf)) goto error;
                            total_len += 1;
                        } else if (signing >= 0) {
                            if (biimmputc(signing > 0 ? '+' : ' ', buf)) goto error;
                            total_len += 1;
                        }

                        if (precision > len) {
                            if (biimmrepc('0', precision - len, buf)) goto error;
                            total_len += precision - len;
                        }

                        if (birequire(buf, len)) goto error;
                        memcpy(buf->data + buf->cursor, tmpbuf, len);
                        buf->count = bimax(buf->count, buf->cursor += len);
                        total_len += len;

                        if (left_just && (fld_width > (int)bimax(precision, len) + (signing >= 0 || is_neg))) {
                            size_t padding = fld_width - bimax(precision, len) - (signing >= 0 || is_neg);
                            if (biimmrepc(' ', padding, buf)) goto error;
                            total_len += padding;
                        }
                    } break;
                    case 'u': case 'o': case 'x': case 'X': {
                        char tmpbuf[24] = {0};
                        int len, prefix_size;
                        uintmax_t received;
                        bool is_dec = *fmtstr == 'u';
                        bool is_oct = *fmtstr == 'o';
                        bool is_hex = *fmtstr == 'x';
                        bool is_HEX = *fmtstr == 'X';

                        switch (lenmod) {
                            case BLM_NONE: received =         va_arg(args, uint ); break;
                            case BLM_HH  : received = (uchar) va_arg(args, uint ); break;
                            case BLM_H   : received = (ushort)va_arg(args, uint ); break;
                            case BLM_L   : received =         va_arg(args, ulong); break;
                            case BLM_LL  : received = va_arg(args,    ullong); break;
                            case BLM_J   : received = va_arg(args, uintmax_t); break;
                            case BLM_Z   : received = va_arg(args,    size_t); break;
                            case BLM_T   : received = va_arg(args, ptrdiff_t); break;
                            case BLM_L_UPPER: goto error;
                        }

                        biprintu(received, tmpbuf, is_dec ? 10 : is_oct ? 8 : 16, is_HEX);
                        prefix_size = alt_form && received > 0 && (is_hex || is_HEX) ? 2 : 0;
                        len = strlen(tmpbuf);

                        if (precision < 0) {
                            if (lead_zero && !left_just)
                                precision = bimax(1 + (alt_form && (is_hex || is_HEX)), fld_width) - prefix_size;
                            else
                                precision = 1;
                        }
                        if (is_oct && received > 0 && precision <= len)
                            precision = len + 1;

                        if (!left_just && (fld_width > (int)bimax(precision, len) + prefix_size)) {
                            size_t padding = fld_width - bimax(precision, len) - prefix_size;
                            if (biimmrepc(' ', padding, buf)) goto error;
                            total_len += padding;
                        }

                        if ((is_hex || is_HEX) && received > 0) {
                            if (biimmputc('0', buf)) goto error;
                            total_len += 1;
                            if (biimmputc(is_hex ? 'x' : 'X', buf)) goto error;
                            total_len += 1;
                        }

                        if (precision > len) {
                            if (biimmrepc('0', precision - len, buf)) goto error;
                            total_len += precision - len;
                        }

                        if (birequire(buf, len)) goto error;
                        memcpy(buf->data + buf->cursor, tmpbuf, len);
                        buf->count = bimax(buf->count, buf->cursor += len);
                        total_len += len;

                        if (left_just && (fld_width > (int)bimax(precision, len) + prefix_size)) {
                            size_t padding = fld_width - bimax(precision, len) - prefix_size;
                            if (biimmrepc(' ', padding, buf)) goto error;
                            total_len += padding;
                        }
                    } break;
                    case 'p':
                    if (lenmod != BLM_NONE) goto error;
                    if (precision >= 0) goto error;
                    {
                        char tmpbuf[24] = {0}; int len;
                        uintptr_t received = (uintptr_t)va_arg(args, void*);

                        biprintu(received, tmpbuf, 16, false);
                        len = strlen(tmpbuf);
                        memmove(tmpbuf + 2 * sizeof received - len, tmpbuf, len + 1);
                        memset(tmpbuf, '0', 2 * sizeof received - len);
                        len = 2 * sizeof received;

                        if (!left_just && (fld_width > len + 2)) {
                            size_t padding = fld_width - len - 2;
                            if (biimmrepc(' ', padding, buf)) goto error;
                            total_len += padding;
                        }

                        if (biimmputc('0', buf)) goto error; else total_len += 1;
                        if (biimmputc('x', buf)) goto error; else total_len += 1;

                        if (birequire(buf, len)) goto error;
                        memcpy(buf->data + buf->cursor, tmpbuf, len);
                        buf->count = bimax(buf->count, buf->cursor += len);
                        total_len += len;

                        if (left_just && (fld_width > len + 2)) {
                            size_t padding = fld_width - len - 2;
                            if (biimmrepc(' ', padding, buf)) goto error;
                            total_len += padding;
                        }
                    } break;
                    case 'n':
                    switch (lenmod) {
                        case BLM_NONE: *va_arg(args,       int*) = total_len; break;
                        case BLM_HH  : *va_arg(args,     schar*) = total_len; break;
                        case BLM_H   : *va_arg(args,     short*) = total_len; break;
                        case BLM_L   : *va_arg(args,      long*) = total_len; break;
                        case BLM_LL  : *va_arg(args,    sllong*) = total_len; break;
                        case BLM_J   : *va_arg(args,  intmax_t*) = total_len; break;
                        case BLM_Z   : *va_arg(args,    size_t*) = total_len; break;
                        case BLM_T   : *va_arg(args, ptrdiff_t*) = total_len; break;
                        case BLM_L_UPPER: goto error;
                    } break;
                    default: goto error;
                }
            }
            fmt = fmtstr + 1;
        } else
            break;
    }

error:
    return total_len;
}