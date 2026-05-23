#define IOBUFFER_SOURCE
#include "iobuffer.h"
#include "bidefine.h"

#include <string.h>

#define B_SPACE_CHARS " \t\n\r\v\f"

static bool biisspace(char ch) {
    return memchr(B_SPACE_CHARS, ch, sizeof B_SPACE_CHARS - 1) != NULL;
}

int vbiscanf(BUFFER* buf, const char* fmt, va_list args) {
    int total_count = 0;
    int total_len   = 0;

    while (true) {
        size_t cmp_len = strcspn(fmt, B_SPACE_CHARS"%");

        if (biimmcmp(fmt, cmp_len, buf, &total_len)) goto error;

        if (fmt[cmp_len]) {
            const char* fmtstr = fmt + cmp_len;
            if (*fmtstr == '%') {
                /* TODO: parsing specifier */
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