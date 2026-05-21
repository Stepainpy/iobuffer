#define IOBUFFER_SOURCE
#include "iobuffer.h"
#include "bidefine.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <math.h>

#define B_INTBUF_CAPACITY 80
#define B_FLTBUF_CAPACITY 512

#define B_LU_ALPHABET "0123456789abcdef0123456789ABCDEF"

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

int biimmputc(int ch,                      BUFFER* buf, int* accumulator);
int biimmputs(const char* str, size_t len, BUFFER* buf, int* accumulator);
int biimmrepc(int ch, size_t count,        BUFFER* buf, int* accumulator);

static int bimin(int a, int b) { return a < b ? a : b; }
static int bimax(int a, int b) { return a > b ? a : b; }

static bool biisnan(long double x) { return x != x; }
static bool biisinf(long double x) { return x < -LDBL_MAX || LDBL_MAX < x; }

static int bigetexponent(const char* numstr, int point_pos) {
    return point_pos == 1 && numstr[0] == '0'
        ? -1 - (int)strspn(numstr + 2, "0") : point_pos - 1;
}

static int biroundeddigit(char digit, char next) {
    return digit + (next >= '5' && digit != '9');
}

static int biroundedhexdigit(char digit, char next) {
    return digit + (next >= '8' && digit != 'f' && digit != 'F');
}

static int bibasefromch(char ch) {
    switch (ch) {
        case 'b':
        case 'B': return  2;
        case 'o': return  8;
        case 'u': return 10;
        case 'x':
        case 'X': return 16;
        default: return 0;
    }
}

static void bireverse(char* first, char* last) {
    for (--last; first < last; first++, last--) {
        char t = *first;
        *first = *last;
        *last  = t;
    }
}

static int bistrtoint(const char* str, const char** end) {
    int result = 0;
    while ('0' <= *str && *str <= '9') {
        int digit = *str++ - '0';
        if (result > INT_MAX / 10 && digit > INT_MAX % 10) return -1;
        result = 10 * result + digit;
    }
    *end = str;
    return result;
}

static void biinttostr(intmax_t number, char* outbuf) {
    char* end = outbuf;
    if (number < 0) do *end++ = '0' - number % 10; while (number /= 10);
    else            do *end++ = '0' + number % 10; while (number /= 10);
    *end = '\0';
    bireverse(outbuf, end);
}

static void biunttostr(uintmax_t number, char* outbuf, int base, bool up) {
    char* end = outbuf;
    do *end++ = B_LU_ALPHABET[16 * up + number % base]; while (number /= base);
    *end = '\0';
    bireverse(outbuf, end);
}

static void bidbltostr(double number, char* outbuf) {
    double intp, frcp, digit;
    size_t i = 0;

    frcp = modf(number < 0 ? -number : number, &intp);

    do {
        digit = fmod(intp, 10);
        outbuf[i++] = '0' + (int)digit;
        intp = (intp - digit) / 10.0;
    } while (intp && i < B_FLTBUF_CAPACITY - 2);

    bireverse(outbuf, outbuf + i);
    outbuf[i++] = '.';

    while (frcp && i < B_FLTBUF_CAPACITY - 1) {
        frcp = modf(frcp * 10, &digit);
        outbuf[i++] = '0' + (int)digit;
    }

    outbuf[i] = '\0';
}

static void bihfntostr(double number, char* outbuf, int* exp, bool up) {
    union { double dbl; ullong unt; } as;
    int exponent; ullong mantissa;
    bool normal; int i, j;

    as.dbl = number;
    as.unt &= ~((ullong)-1 << 63); /* truncate sign bit */

    exponent = (int)(as.unt >> (DBL_MANT_DIG - 1)) - (DBL_MAX_EXP - 1);
    mantissa = as.unt & (((ullong)1 << (DBL_MANT_DIG - 1)) - 1);
    normal = exponent > -(DBL_MAX_EXP - 1);

    i = 0;
    outbuf[i++] = normal ? '1' : '0';
    outbuf[i++] = '.';
    for (j = 48; j >= 0 && mantissa; mantissa &= ((ullong)1 << j) - 1, j -= 4)
        outbuf[i++] = B_LU_ALPHABET[16 * up + (mantissa >> j & 15)];
    outbuf[i] = '\0';

    *exp = exponent + !normal;
}

static int biputfmt_di(BUFFER* buf, va_list args, bifmtspec_t* fmt, int* total) {
    static char tmpbuf[B_INTBUF_CAPACITY];
    intmax_t received;
    int len, padding;
    bool zerozero;
    bool has_sign;
    bool is_neg;

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

    biinttostr(received, tmpbuf);
    len = strlen(tmpbuf);

    is_neg = received < 0;
    has_sign = fmt->signing >= 0 || is_neg;
    zerozero = fmt->precision == 0 && received == 0;

    if (!zerozero && fmt->precision < 0 && fmt->lead_zero && !fmt->left_just)
        fmt->precision = bimax(1, fmt->fieldwidth) - has_sign;
    if (fmt->precision < 0) fmt->precision = 1;

    padding = bimax(0, fmt->fieldwidth - bimax(fmt->precision, len - zerozero) - has_sign);

    if (!fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    if (is_neg) {
        if (biimmputc('-', buf, total)) return B_FAIL;
    } else if (fmt->signing >= 0) {
        if (biimmputc(fmt->signing > 0 ? '+' : ' ', buf, total)) return B_FAIL;
    }

    if (fmt->precision > len)
        if (biimmrepc('0', fmt->precision - len, buf, total)) return B_FAIL;

    if (!zerozero)
        if (biimmputs(tmpbuf, len, buf, total)) return B_FAIL;

    if ( fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    return B_OKEY;
}

static int biputfmt_boux(BUFFER* buf, va_list args, bifmtspec_t* fmt, int* total, char specch) {
    static char tmpbuf[B_INTBUF_CAPACITY];
    int len, prefix_size, padding, base;
    uintmax_t received;
    bool zerozero;

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

    base = bibasefromch(specch);
    biunttostr(received, tmpbuf, base, specch == 'X');
    len = strlen(tmpbuf);

    prefix_size = fmt->alt_form && received > 0 && (base == 16 || base == 2) ? 2 : 0;
    zerozero = fmt->precision == 0 && received == 0;

    if (!zerozero && fmt->precision < 0 && fmt->lead_zero && !fmt->left_just)
        fmt->precision = bimax(1 + (fmt->alt_form && (base == 16 || base == 2)), fmt->fieldwidth) - prefix_size;
    if (fmt->precision < 0) fmt->precision = 1;

    if (base == 8 && fmt->alt_form && fmt->precision <= len - zerozero)
        fmt->precision = len - zerozero + 1;

    padding = bimax(0, fmt->fieldwidth - bimax(fmt->precision, len - zerozero) - prefix_size);

    if (!fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    if (fmt->alt_form && (base == 16 || base == 2) && received > 0) {
        if (biimmputc(   '0', buf, total)) return B_FAIL;
        if (biimmputc(specch, buf, total)) return B_FAIL;
    }

    if (fmt->precision > len - zerozero)
        if (biimmrepc('0', fmt->precision - len + zerozero, buf, total)) return B_FAIL;

    if (!zerozero)
        if (biimmputs(tmpbuf, len, buf, total)) return B_FAIL;

    if ( fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    return B_OKEY;
}

/* spec values:
 * < 0  =>  fixed    float
 * = 0  =>  general  float
 * > 0  =>  exponent float
 */

static int biputfmt_feg(BUFFER* buf, va_list args, bifmtspec_t* fmt, int* total, bool up, int spec) {
    static char tmpbuf[B_FLTBUF_CAPACITY], expbuf[8];
    long double received;
    bool normal = false;
    bool fixed = false;
    bool is_neg, has_sign;
    int padding, exponent;
    int len, flen, elen, alen;

    switch (fmt->lenmod) {
        case BLM_NONE   : received = va_arg(args,      double); break;
        case BLM_L_UPPER: received = va_arg(args, long double); break;

        default: return B_FAIL;
    }

    /**/ if (biisnan(received))
        memcpy(tmpbuf, up ? "NAN" : "nan", 4);
    else if (biisinf(received))
        memcpy(tmpbuf, up ? "INF" : "inf", 4);
    else
        bidbltostr(received, tmpbuf), normal = true;

    len = strlen(tmpbuf);
    if (normal && received != 0) {
        int point = strchr(tmpbuf, '.') - tmpbuf;
        exponent = bigetexponent(tmpbuf, point);

        if (spec == 0) {
            fixed = -4 <= exponent && exponent < fmt->precision;
            fmt->precision -= fixed ? exponent + 1 : 1;
        } else
            fixed = spec < 0;

        if (fixed)
            flen = len - point - 1;
        else {
            if (exponent < 0) {
                memmove(tmpbuf + 1, tmpbuf + 1 - exponent, len + exponent);
                tmpbuf[0] = tmpbuf[1];
                len = len + exponent + 1;
            } else
                memmove(tmpbuf + 2, tmpbuf + 1, point - 1);

            tmpbuf[point = 1] = '.';
            flen = len - 2;
        }

        if (flen > fmt->precision) {
            char* last = tmpbuf + point + fmt->precision;
            char after_last = last[1]; last[1] = '\0';
            last -= fmt->precision == 0;
            *last = biroundeddigit(*last, after_last);

            flen = fmt->precision;
            len = point + 1 + flen;
        }
    } else if (received == 0) {
        fmt->precision -= spec == 0;
        flen = exponent = 0;
        fixed = true;
    } else
        flen = exponent = -1;

    if (!fixed) {
        biinttostr(exponent, expbuf);
        elen = strlen(expbuf);
        if (elen == 1) {
            expbuf[2] = expbuf[1];
            expbuf[1] = expbuf[0];
            expbuf[0] = '0';
            elen = 2;
        }
    }

    if (spec == 0 && !fmt->alt_form) {
        while (tmpbuf[len - 1] == '0') len--, flen--;
        if    (tmpbuf[len - 1] == '.') len--, flen=0;
    } else if (normal && fmt->precision == 0 && !fmt->alt_form) {
        fmt->precision = flen = 0; len -= 1;
    }

    is_neg = received < 0;
    has_sign = fmt->signing >= 0 || is_neg;

    alen = fixed ? ((spec < 0 || fmt->alt_form) ? fmt->precision - flen : 0) : 2 + elen;
    padding = bimax(0, fmt->fieldwidth - len - has_sign - (normal ? alen : 0));

    if (!(fmt->lead_zero && normal) && !fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    if (is_neg) {
        if (biimmputc('-', buf, total)) return B_FAIL;
    } else if (fmt->signing >= 0) {
        if (biimmputc(fmt->signing > 0 ? '+' : ' ', buf, total)) return B_FAIL;
    }

    if (normal && fmt->lead_zero && !fmt->left_just && padding)
        if (biimmrepc('0', padding, buf, total)) return B_FAIL;

    if (biimmputs(tmpbuf, len, buf, total)) return B_FAIL;

    if (normal) {
        if ((spec != 0 || fmt->alt_form) && fmt->precision > flen)
            if (biimmrepc('0', fmt->precision - flen, buf, total)) return B_FAIL;
        if (!fixed) {
            if (biimmputc(up ? 'E' : 'e', buf, total)) return B_FAIL;
            if (biimmputc(exponent < 0 ? '-' : '+', buf, total)) return B_FAIL;
            if (biimmputs(expbuf, elen, buf, total)) return B_FAIL;
        }
    }

    if (fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    return B_OKEY;
}

static int biputfmt_a(BUFFER* buf, va_list args, bifmtspec_t* fmt, int* total, bool up) {
    static char tmpbuf[B_FLTBUF_CAPACITY], expbuf[8];
    long double received;
    bool normal = false;
    bool is_neg, has_sign;
    int padding, exponent;
    int len, flen, elen;

    switch (fmt->lenmod) {
        case BLM_NONE   : received = va_arg(args,      double); break;
        case BLM_L_UPPER: received = va_arg(args, long double); break;

        default: return B_FAIL;
    }

    /**/ if (biisnan(received))
        memcpy(tmpbuf, up ? "NAN" : "nan", 4);
    else if (biisinf(received))
        memcpy(tmpbuf, up ? "INF" : "inf", 4);
    else
        bihfntostr(received, tmpbuf, &exponent, up), normal = true;

    len = strlen(tmpbuf);
    if (normal && received != 0) {
        flen = len - 2;
        if (fmt->precision >= 0 && flen > fmt->precision) {
            char* last = tmpbuf + 1 + fmt->precision;
            char after_last = last[1]; last[1] = '\0';
            last -= fmt->precision == 0;
            *last = biroundedhexdigit(*last, after_last);

            flen = fmt->precision;
            len = 2 + flen;
        }
    } else if (received == 0)
        flen = exponent = 0;
    else
        flen = exponent = -1;

    biinttostr(exponent, expbuf);
    elen = strlen(expbuf);

    if (fmt->precision < 0 && !fmt->alt_form)
        if (tmpbuf[len - 1] == '.') len--, flen=0;

    is_neg = received < 0;
    has_sign = fmt->signing >= 0 || is_neg;

    padding = bimax(0, fmt->fieldwidth - 2 - len - has_sign - (normal ? 2 + elen : 0));

    if (!fmt->lead_zero && !fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    if (is_neg) {
        if (biimmputc('-', buf, total)) return B_FAIL;
    } else if (fmt->signing >= 0) {
        if (biimmputc(fmt->signing > 0 ? '+' : ' ', buf, total)) return B_FAIL;
    }

    if (normal)
        if (biimmputs(up ? "0X" : "0x", 2, buf, total)) return B_FAIL;

    if (fmt->lead_zero && !fmt->left_just && padding)
        if (biimmrepc('0', padding, buf, total)) return B_FAIL;

    if (biimmputs(tmpbuf, len - (fmt->precision == 0 && !fmt->alt_form), buf, total)) return B_FAIL;
    if (normal) {
        if (fmt->precision >= 0 && fmt->precision > flen)
            if (biimmrepc('0', fmt->precision - flen, buf, total)) return B_FAIL;
        if (biimmputc(up ? 'P' : 'p', buf, total)) return B_FAIL;
        if (biimmputc(exponent < 0 ? '-' : '+', buf, total)) return B_FAIL;
        if (biimmputs(expbuf, elen, buf, total)) return B_FAIL;
    }

    if (fmt->left_just && padding)
        if (biimmrepc(' ', padding, buf, total)) return B_FAIL;

    return B_OKEY;
}

int vbiprintf(BUFFER* buf, const char* fmt, va_list args) {
    int total_len = 0;

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

                if ('0' <= *fmtstr && *fmtstr <= '9') {
                    fmt.fieldwidth = bistrtoint(fmtstr, &fmtstr);
                    if (fmt.fieldwidth <= 0) goto error;
                } else if (*fmtstr == '*') {
                    int received = va_arg(args, int);
                    if (received < 0) {
                        received = -received;
                        fmt.left_just = true;
                    }
                    fmt.fieldwidth = received;
                    fmtstr += 1;
                }

                if (*fmtstr == '.') {
                    fmtstr += 1;
                    if ('0' <= *fmtstr && *fmtstr <= '9') {
                        fmt.precision = bistrtoint(fmtstr, &fmtstr);
                        if (fmt.precision < 0) goto error;
                    } else if (*fmtstr == '*') {
                        fmt.precision = va_arg(args, int);
                        fmtstr += 1;
                    } else
                        fmt.precision = 0;
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

                    case 'p':
                        if (fmt.lenmod != BLM_NONE) goto error;
                        fmt.lenmod = BLM_Z; /* use size_t as uintptr_t */
                        fmt.alt_form = true;
                        if (biputfmt_boux(buf, args, &fmt, &total_len, 'x')) goto error;
                        break;

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

                    case 'b': case 'B':
                    case 'o': case 'u':
                    case 'x': case 'X':
                        if (biputfmt_boux(buf, args, &fmt, &total_len, *fmtstr)) goto error;
                        break;

                    case 'f': case 'F':
                        if (fmt.precision < 0) fmt.precision = 6;
                        if (biputfmt_feg(buf, args, &fmt, &total_len, *fmtstr == 'F', -1)) goto error;
                        break;

                    case 'e': case 'E':
                        if (fmt.precision < 0) fmt.precision = 6;
                        if (biputfmt_feg(buf, args, &fmt, &total_len, *fmtstr == 'E', +1)) goto error;
                        break;

                    case 'g': case 'G':
                        if (fmt.precision <  0) fmt.precision = 6;
                        if (fmt.precision == 0) fmt.precision = 1;
                        if (biputfmt_feg(buf, args, &fmt, &total_len, *fmtstr == 'G',  0)) goto error;
                        break;

                    case 'a': case 'A':
                        if (biputfmt_a(buf, args, &fmt, &total_len, *fmtstr == 'A')) goto error;
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