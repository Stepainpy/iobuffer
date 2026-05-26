#define IOBUFFER_SOURCE
#include "iobuffer.h"
#include "bidefine.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#define B_SPACE_CHARS " \t\n\r\v\f"

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

typedef struct {
    size_t maxwidth;
    bilenmod_t lenmod;
    bool assign;
} bifmtspec_t;

typedef uint64_t scanset_t[4];

static void bissset(scanset_t ss, uchar value) { ss[value / 64] |= UINT64_C(1) << (value % 64); }
static bool bissget(scanset_t ss, uchar value) { return (ss[value / 64] >> (value % 64)) & 1; }

static bool biisspace(int ch) {
    return memchr(B_SPACE_CHARS, ch, sizeof B_SPACE_CHARS - 1) != NULL;
}

static int bichartodigit(int ch) {
    if ('0' <= ch && ch <= '9') return ch - '0'     ;
    if ('A' <= ch && ch <= 'Z') return ch - 'A' + 10;
    if ('a' <= ch && ch <= 'z') return ch - 'a' + 10;
    return -1;
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

static const char* biparsescanlist(scanset_t ss, bool* inverse, const char* fmtstr) {
    if (*fmtstr == '^') { *inverse =  true; fmtstr += 1; }
    if (*fmtstr == ']') { bissset(ss, ']'); fmtstr += 1; }

    while (*fmtstr != ']' && *fmtstr != '\0') {
        uchar bch = *fmtstr++;
        if (*fmtstr != '-')
            bissset(ss, bch);
        else {
            uchar ech; int i;
            fmtstr += 1;
            ech = *fmtstr++;

            if (ech == '\0') return NULL;
            if (ech == ']') {
                bissset(ss, bch);
                bissset(ss, '-');
                return fmtstr - 1;
            }

            if (bch > ech) return NULL;
            for (i = bch; i <= ech; i++) bissset(ss, i);
        }
    }

    if (*fmtstr == '\0') return NULL;

    return fmtstr;
}

static int bistrtouim(BUFFER* buf, bifmtspec_t* fmt, va_list args, int base, int* total, bool signing) {
    uintmax_t result;
    bool zero_base = false;
    bool is_neg = false;
    int ch, digit;

    while (biisspace(biimmpeek(buf)))
        biimmskip(buf), ++*total;

    if (fmt->maxwidth > 0) {
        ch = biimmpeek(buf);
        /*  */ if (ch == '-') {
            biimmskip(buf), --fmt->maxwidth, ++*total;
            is_neg = true;
        } else if (ch == '+')
            biimmskip(buf), --fmt->maxwidth, ++*total;
    }

    if (base == 0) {
        if (fmt->maxwidth > 0 && biimmpeek(buf) == '0') {
            biimmskip(buf), --fmt->maxwidth, ++*total;

            zero_base = true;
            base = 8;

            if (fmt->maxwidth > 0) {
                ch = biimmpeek(buf);
                if (ch == 'x' || ch == 'X') {
                    biimmskip(buf), --fmt->maxwidth, ++*total;
                    base = 16;
                }
            }
        } else
            base = 10;
    }

    if (fmt->maxwidth > 0) {
        ch = biimmpeek(buf);
        if (ch == EOB) goto prefix_only;

        digit = bichartodigit(ch);
        if (digit < 0 || digit >= base) goto prefix_only;

        biimmskip(buf), --fmt->maxwidth, ++*total;
        result = digit;

        while (fmt->maxwidth > 0 && (ch = biimmpeek(buf)) != EOB) {
            digit = bichartodigit(ch);
            if (digit < 0 || digit >= base) break;
            biimmskip(buf), --fmt->maxwidth, ++*total;

            result = result * base + digit;
        }
    } else
        goto prefix_only;

    goto assigning;

prefix_only:
    if (!zero_base) return B_FAIL;
    result = 0;

assigning:
    if (is_neg) result = -result;

    /**/ if (fmt->assign &&  signing)
        switch (fmt->lenmod) {
            case BLM_NONE: *va_arg(args,       int*) = result; break;
            case BLM_HH  : *va_arg(args,     schar*) = result; break;
            case BLM_H   : *va_arg(args,     short*) = result; break;
            case BLM_L   : *va_arg(args,      long*) = result; break;
            case BLM_LL  : *va_arg(args,    sllong*) = result; break;
            case BLM_J   : *va_arg(args,  intmax_t*) = result; break;
            case BLM_Z   : *va_arg(args,    size_t*) = result; break;
            case BLM_T   : *va_arg(args, ptrdiff_t*) = result; break;
            case BLM_L_UPPER:       /* plug for switch */      break;
        }
    else if (fmt->assign && !signing)
        switch (fmt->lenmod) {
            case BLM_NONE: *va_arg(args,      uint*) = result; break;
            case BLM_HH  : *va_arg(args,     uchar*) = result; break;
            case BLM_H   : *va_arg(args,    ushort*) = result; break;
            case BLM_L   : *va_arg(args,     ulong*) = result; break;
            case BLM_LL  : *va_arg(args,    ullong*) = result; break;
            case BLM_J   : *va_arg(args, uintmax_t*) = result; break;
            case BLM_Z   : *va_arg(args,    size_t*) = result; break;
            case BLM_T   : *va_arg(args, ptrdiff_t*) = result; break;
            case BLM_L_UPPER:       /* plug for switch */      break;
        }

    return B_OKEY;
}

int vbiscanf(BUFFER* buf, const char* fmt, va_list args) {
    int total_count = 0;
    int total_len   = 0;

    while (true) {
        size_t cmp_len = strcspn(fmt, B_SPACE_CHARS"%");

        if (biimmcmp(fmt, cmp_len, buf, &total_len)) goto error;

        if (fmt[cmp_len]) {
            const char* fmtstr = fmt + cmp_len;
            if (fmtstr[0] == '%' && fmtstr[1] == '%') {
                if (biimmcmp("%", 1, buf, &total_len)) goto error;
                fmtstr += 1;
            } else if (*fmtstr == '%') {
                bifmtspec_t fmt = {0};
                fmt.assign = true;
                fmtstr += 1;

                if (*fmtstr == '*') {
                    fmt.assign = false;
                    fmtstr += 1;
                }

                if ('0' <= *fmtstr && *fmtstr <= '9') {
                    fmt.maxwidth = bistrtoint(fmtstr, &fmtstr);
                    if (fmt.maxwidth <= 0) goto error;
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
                        if (fmt.assign)
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
                            }
                        break;

                    case 'c':
                    if (fmt.lenmod != BLM_NONE) goto error;
                    if (fmt.maxwidth == 0) fmt.maxwidth = 1;
                    {
                        char* dest = NULL;
                        if (fmt.assign)
                            dest = va_arg(args, char*);

                        while (fmt.maxwidth > 0) {
                            int ch = biimmpeek(buf);
                            if (ch == EOB) break;

                            biimmskip(buf);
                            total_len += 1;
                            fmt.maxwidth -= 1;

                            if (fmt.assign) *dest++ = ch;
                        }

                        if (fmt.assign) total_count += 1;
                    } break;

                    case 's':
                    if (fmt.lenmod != BLM_NONE) goto error;
                    if (fmt.maxwidth == 0) fmt.maxwidth = SIZE_MAX;
                    {
                        char* dest = NULL;
                        if (fmt.assign)
                            dest = va_arg(args, char*);

                        while (fmt.maxwidth > 0) {
                            int ch = biimmpeek(buf);
                            if (ch == EOB || biisspace(ch)) break;

                            biimmskip(buf);
                            total_len += 1;
                            fmt.maxwidth -= 1;

                            if (fmt.assign) *dest++ = ch;
                        }

                        if (fmt.assign) {
                            *dest = '\0';
                            total_count += 1;
                        }
                    } break;

                    case '[':
                    if (fmt.lenmod != BLM_NONE) goto error;
                    if (fmt.maxwidth == 0) fmt.maxwidth = SIZE_MAX;
                    {
                        scanset_t ss = {0};
                        bool inverse = false;
                        char* dest = NULL;
                        if (fmt.assign)
                            dest = va_arg(args, char*);

                        fmtstr = biparsescanlist(ss, &inverse, fmtstr + 1);
                        if (!fmtstr) goto error;

                        while (fmt.maxwidth > 0) {
                            int ch = biimmpeek(buf);
                            if (ch == EOB || (inverse == bissget(ss, ch))) break;

                            biimmskip(buf);
                            total_len += 1;
                            fmt.maxwidth -= 1;

                            if (fmt.assign) *dest++ = ch;
                        }

                        if (fmt.assign) {
                            *dest = '\0';
                            total_count += 1;
                        }
                    } break;

                    case 'i':
                        if (fmt.lenmod == BLM_L_UPPER) goto error;
                        if (fmt.maxwidth == 0) fmt.maxwidth = SIZE_MAX;
                        if (bistrtouim(buf, &fmt, args, 0, &total_len, true)) goto error;
                        if (fmt.assign) total_count += 1;
                        break;

                    default: goto error;
                }
            } else {
                while (biisspace(fmtstr[1])) ++fmtstr;
                if (biisspace(biimmpeek(buf))) {
                    biimmskip(buf);
                    total_len += 1;
                    while (biisspace(biimmpeek(buf))) {
                        biimmskip(buf);
                        total_len += 1;
                    }
                } else
                    goto error;
            }
            fmt = fmtstr + 1;
        } else
            break;
    }

error:
    return total_count;
}