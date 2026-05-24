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
    bool not_assign;
} bifmtspec_t;

static bool biisspace(char ch) {
    return memchr(B_SPACE_CHARS, ch, sizeof B_SPACE_CHARS - 1) != NULL;
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
                fmtstr += 1;

                if (*fmtstr == '*') {
                    fmt.not_assign = true;
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
                        if (!fmt.not_assign)
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