#define IOBUFFER_SOURCE
#include "iobuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <float.h>
#include <math.h>

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

/* signing values:
 * < 0  =>  print only minus
 * = 0  =>  print with space
 * > 0  =>  print with plus
 */

typedef struct {
    bilenmod_t lenmod;
    int fieldwidth;
    int  precision;
    int    signing;
    bool lead_zero;
    bool  alt_form;
    bool left_just;
} bifmtspec_t;

static int biimmputc(int ch, BUFFER* buf, int* accumulator) {
    if (birequire(buf, 1)) return B_FAIL;
    buf->data[buf->cursor++] = (uchar)ch;
    buf->count = bimax(buf->count, buf->cursor);
    *accumulator += 1;
    return B_OKEY;
}

static int biimmputs(const char* str, size_t len, BUFFER* buf, int* accumulator) {
    int rc = B_OKEY;
    if (birequire(buf, len))
        len = buf->capacity - buf->cursor, rc = B_FAIL;
    memcpy(buf->data + buf->cursor, str, len);
    buf->count = bimax(buf->count, buf->cursor += len);
    *accumulator += len;
    return rc;
}

static int biimmrepc(int ch, size_t count, BUFFER* buf, int* accumulator) {
    int rc = B_OKEY;
    if (birequire(buf, count))
        count = buf->capacity - buf->cursor, rc = B_FAIL;
    memset(buf->data + buf->cursor, ch, count);
    buf->count = bimax(buf->count, buf->cursor += count);
    *accumulator += count;
    return rc;
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

static int biputfmt_di(BUFFER* buf, va_list args, bifmtspec_t* fmt, int* total) {
    char tmpbuf[24] = {0};
    bool is_neg; int len, padding;
    intmax_t received;

    switch (fmt->lenmod) {
        case BLM_NONE: received =        va_arg(args, int ); break;
        case BLM_HH  : received = (schar)va_arg(args, int ); break;
        case BLM_H   : received = (short)va_arg(args, int ); break;
        case BLM_L   : received =        va_arg(args, long); break;
        case BLM_LL  : received = va_arg(args,    sllong); break;
        case BLM_J   : received = va_arg(args,  intmax_t); break;
        case BLM_Z   : received = va_arg(args,    size_t); break;
        case BLM_T   : received = va_arg(args, ptrdiff_t); break;

        case BLM_L_UPPER:
        default:
            return B_FAIL;
    }

    biprinti(received, tmpbuf);
    is_neg = received < 0;
    len = strlen(tmpbuf);

    if (fmt->precision < 0) {
        if (fmt->lead_zero && !fmt->left_just)
            fmt->precision = bimax(1, fmt->fieldwidth) - (fmt->signing >= 0 || is_neg);
        else
            fmt->precision = 1;
    }

    padding = fmt->fieldwidth - (int)bimax(fmt->precision, len) - (fmt->signing >= 0 || is_neg);
    padding = padding < 0 ? 0 : padding;

    if (!fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    if (is_neg) {
        if (biimmputc('-', buf, total)) return B_FAIL;
    } else if (fmt->signing >= 0) {
        if (biimmputc(fmt->signing > 0 ? '+' : ' ', buf, total)) return B_FAIL;
    }

    if (fmt->precision > len)
        if (biimmrepc('0', fmt->precision - len, buf, total)) return B_FAIL;

    if (biimmputs(tmpbuf, len, buf, total)) return B_FAIL;

    if ( fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    return B_OKEY;
}

static int biputfmt_uox(BUFFER* buf, va_list args, bifmtspec_t* fmt, int* total, char specch) {
    char tmpbuf[24] = {0};
    int len, prefix_size, padding;
    uintmax_t received;
    bool is_dec = specch == 'u';
    bool is_oct = specch == 'o';
    bool is_hex = specch == 'x';
    bool is_HEX = specch == 'X';

    switch (fmt->lenmod) {
        case BLM_NONE: received =         va_arg(args, uint ); break;
        case BLM_HH  : received = (uchar) va_arg(args, uint ); break;
        case BLM_H   : received = (ushort)va_arg(args, uint ); break;
        case BLM_L   : received =         va_arg(args, ulong); break;
        case BLM_LL  : received = va_arg(args,    ullong); break;
        case BLM_J   : received = va_arg(args, uintmax_t); break;
        case BLM_Z   : received = va_arg(args,    size_t); break;
        case BLM_T   : received = va_arg(args, ptrdiff_t); break;

        case BLM_L_UPPER:
        default:
            return B_FAIL;
    }

    biprintu(received, tmpbuf, is_dec ? 10 : is_oct ? 8 : 16, is_HEX);
    prefix_size = fmt->alt_form && received > 0 && (is_hex || is_HEX) ? 2 : 0;
    len = strlen(tmpbuf);

    if (fmt->precision < 0) {
        if (fmt->lead_zero && !fmt->left_just)
            fmt->precision = bimax(1 + (fmt->alt_form && (is_hex || is_HEX)), fmt->fieldwidth) - prefix_size;
        else
            fmt->precision = 1;
    }
    if (is_oct && received > 0 && fmt->precision <= len)
        fmt->precision = len + 1;

    padding = fmt->fieldwidth - (int)bimax(fmt->precision, len) - prefix_size;
    padding = padding < 0 ? 0 : padding;

    if (!fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    if ((is_hex || is_HEX) && received > 0) {
        if (biimmputc('0', buf, total)) return B_FAIL;
        if (biimmputc(is_hex ? 'x' : 'X', buf, total)) return B_FAIL;
    }

    if (fmt->precision > len)
        if (biimmrepc('0', fmt->precision - len, buf, total)) return B_FAIL;

    if (biimmputs(tmpbuf, len, buf, total)) return B_FAIL;

    if ( fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    return B_OKEY;
}

#define B_FLTBUF_CAPACITY 512

static void biprintd(double number, char* outint, char* frcout) {
    double intp, frcp;
    size_t count, i = 0;

    frcp = modf(number < 0 ? -number : number, &intp);

    do {
        double digit = fmod(intp, 10.0);
        outint[i++] = '0' + (int)digit;
        intp = (intp - digit) / 10.0;
    } while (intp && i < B_FLTBUF_CAPACITY - 1);
    outint[i] = '\0';
    for (count = i, i = 0; i < count / 2; i++) {
        char tmp = outint[i];
        outint[i] = outint[count - i - 1];
        outint[count - i - 1] = tmp;
    }

    for (i = 0; frcp && i < B_FLTBUF_CAPACITY - 1;) {
        double digit;
        modf(frcp *= 10.0, &digit);
        frcout[i++] = '0' + (int)digit;
        frcp -= digit;
    }
    frcout[i] = '\0';
}

static int biroundeddigit(char digit, char next) {
    if (next >= '5' && digit != '9') return digit + 1;
    return digit;
}

static int bisign(long double num) {
    if (num != num) return 0;
    if (num < 0) return 1;
    return 0;
}

static int biputfmt_f(BUFFER* buf, va_list args, bifmtspec_t* fmt, int* total, bool up) {
    static char intpart[B_FLTBUF_CAPACITY];
    static char frcpart[B_FLTBUF_CAPACITY];
    long double received;
    bool normal = false;
    int ilen, flen;
    int padding;

    switch (fmt->lenmod) {
        case BLM_NONE   : received = va_arg(args,      double); break;
        case BLM_L_UPPER: received = va_arg(args, long double); break;

        default: return B_FAIL;
    }

    if (received != received) {
        memcpy(intpart, up ? "NAN" : "nan", 4);
        frcpart[0] = '\0';
    } else if (
        (fmt->lenmod == BLM_NONE    && (received < - DBL_MAX ||  DBL_MAX < received)) ||
        (fmt->lenmod == BLM_L_UPPER && (received < -LDBL_MAX || LDBL_MAX < received))
    ) {
        memcpy(intpart, up ? "INF" : "inf", 4);
        frcpart[0] = '\0';
    } else {
        biprintd(received, intpart, frcpart);
        normal = true;
    }
    ilen = strlen(intpart);
    flen = strlen(frcpart);

    padding = fmt->fieldwidth - ilen - fmt->precision - 1
        - (bisign(received) || fmt->signing >= 0);
    padding = padding < 0 ? 0 : padding;

    if (!fmt->lead_zero && !fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    if (bisign(received)) {
        if (biimmputc('-', buf, total)) return B_FAIL;
    } else if (fmt->signing >= 0) {
        if (biimmputc(fmt->signing > 0 ? '+' : ' ', buf, total)) return B_FAIL;
    }

    if (fmt->lead_zero && !fmt->left_just && padding)
        if (biimmrepc('0', padding, buf, total)) return B_FAIL;

    if (biimmputs(intpart, ilen, buf, total)) return B_FAIL;

    if (normal) {
        if (biimmputc('.', buf, total)) return B_FAIL;

        if (flen < fmt->precision) {
            if (biimmputs(frcpart, flen, buf, total)) return B_FAIL;
            if (biimmrepc('0', fmt->precision - flen, buf, total)) return B_FAIL;
        } else {
            if (biimmputs(frcpart, fmt->precision - 1, buf, total)) return B_FAIL;
            if (biimmputc(biroundeddigit(
                frcpart[fmt->precision - 1], frcpart[fmt->precision]), buf, total)) return B_FAIL;
        }
    }

    if (fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    return B_OKEY;
}

IOBUFFER_API int vbprintf(BUFFER* restrict buf, const char* restrict fmt, va_list args) {
    int total_len = 0;
    if (!buf || !fmt || !buf->writable) return EOB;

    while (true) {
        const char* percent = strchr(fmt, '%');
        size_t len = percent ? (size_t)(percent - fmt) : strlen(fmt);

        if (biimmputs(fmt, len, buf, &total_len)) goto error;

        if (percent) {
            const char* fmtstr = percent + 1;
            if (*fmtstr == '%') {
                if (biimmputc('%', buf, &total_len)) goto error;
            } else {
                bifmtspec_t fmt = {0};
                fmt.precision = -1;
                fmt.signing   = -1;

                while (*fmtstr == ' ' || *fmtstr == '-' || *fmtstr == '+' ||
                       *fmtstr == '0' || *fmtstr == '#')
                    switch (*fmtstr++) {
                        case '0': fmt.lead_zero = true; break;
                        case '-': fmt.left_just = true; break;
                        case '#': fmt.alt_form  = true; break;
                        case '+': fmt.signing   = 1;    break;
                        case ' ':
                            if (fmt.signing < 0) fmt.signing = 0;
                            break;
                    }

                if ('1' <= *fmtstr && *fmtstr <= '9') {
                    fmt.fieldwidth = atoi(fmtstr);
                    if (fmt.fieldwidth == 0) goto error;
                    while ('0' <= *fmtstr && *fmtstr <= '9') ++fmtstr;
                } else if (*fmtstr == '*') {
                    int received = va_arg(args, int);
                    if (received < 0) {
                        received = -received;
                        fmt.left_just = true;
                    }
                    fmt.fieldwidth = received;
                    fmtstr += 1;
                }

                if (fmtstr[0] == '.' && '1' <= fmtstr[1] && fmtstr[1] <= '9') {
                    fmt.precision = atoi(++fmtstr);
                    if (fmt.precision == 0) goto error;
                    while ('0' <= *fmtstr && *fmtstr <= '9') ++fmtstr;
                } else if (fmtstr[0] == '.' && fmtstr[1] == '*') {
                    fmt.precision = va_arg(args, int);
                    if (fmt.precision <= 0) goto error;
                    fmtstr += 2;
                }

                switch (*fmtstr) {
                    case 'h':
                        fmt.lenmod = BLM_H; ++fmtstr;
                        if (*fmtstr != 'h') break;
                        fmt.lenmod = BLM_HH; ++fmtstr;
                        break;
                    case 'l':
                        fmt.lenmod = BLM_L; ++fmtstr;
                        if (*fmtstr != 'l') break;
                        fmt.lenmod = BLM_LL; ++fmtstr;
                        break;
                    case 'j': fmt.lenmod = BLM_J; ++fmtstr; break;
                    case 'z': fmt.lenmod = BLM_Z; ++fmtstr; break;
                    case 't': fmt.lenmod = BLM_T; ++fmtstr; break;
                    case 'L': fmt.lenmod = BLM_L_UPPER; ++fmtstr; break;
                }

                switch (*fmtstr) {
                    case 'n':
                    switch (fmt.lenmod) {
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
                    case 'c':
                    if (fmt.lenmod != BLM_NONE) goto error;
                    {
                        int received = va_arg(args, int);
                        if ( fmt.left_just)
                            if (biimmputc(received, buf, &total_len)) goto error;

                        if (fmt.fieldwidth > 1)
                            if (biimmrepc(' ', fmt.fieldwidth - 1, buf, &total_len)) goto error;

                        if (!fmt.left_just)
                            if (biimmputc(received, buf, &total_len)) goto error;
                    } break;
                    case 's':
                    if (fmt.lenmod != BLM_NONE) goto error;
                    {
                        const char* received = va_arg(args, const char*);
                        int len, maxlen;

                        if (!received) goto error;
                        len = strlen(received);
                        if (fmt.precision < 0) fmt.precision = len;
                        fmt.precision = bimin(fmt.precision, len);
                        maxlen = bimax(fmt.fieldwidth, fmt.precision);

                        if ( fmt.left_just)
                            if (biimmputs(received, fmt.precision, buf, &total_len)) goto error;

                        if (fmt.precision < maxlen)
                            if (biimmrepc(' ', maxlen - fmt.precision, buf, &total_len)) goto error;

                        if (!fmt.left_just)
                            if (biimmputs(received, fmt.precision, buf, &total_len)) goto error;
                    } break;
                    case 'd': case 'i':
                        if (biputfmt_di(buf, args, &fmt, &total_len)) goto error;
                        break;
                    case 'u': case 'o': case 'x': case 'X':
                        if (biputfmt_uox(buf, args, &fmt, &total_len, *fmtstr)) goto error;
                        break;
                    case 'p':
                        if (fmt.lenmod != BLM_NONE) goto error;
                        fmt.lenmod = BLM_Z; /* use size_t as uintptr_t */
                        fmt.alt_form = true;
                        if (biputfmt_uox(buf, args, &fmt, &total_len, 'x')) goto error;
                        break;
                    case 'f': case 'F':
                        if (fmt.precision < 0) fmt.precision = 6;
                        if (biputfmt_f(buf, args, &fmt, &total_len, *fmtstr == 'F')) goto error;
                        break;
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