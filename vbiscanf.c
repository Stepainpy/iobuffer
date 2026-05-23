#define IOBUFFER_SOURCE
#include "iobuffer.h"
#include "bidefine.h"

int vbiscanf(BUFFER* buf, const char* fmt, va_list args) {
    (void)buf, (void)fmt, (void)args;
    return EOB;
}