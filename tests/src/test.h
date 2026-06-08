#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define _TOSTR(x) # x
#define  TOSTR(x) _TOSTR(x)

#define FILENM filename(__FILE__)
static const char* filename(const char* path) {
    const char* loc;

    loc = strrchr(path, '/');
    if (loc) return loc + 1;

    loc = strrchr(path, '\\');
    if (loc) return loc + 1;

    return path;
}

void memdump(FILE* stream, const void* ptr, size_t size) {
    size_t i; const unsigned char* data = ptr;
    fprintf(stream, "%02X", (unsigned)data[0]);
    for (i = 1; i < size; i++)
        fprintf(stream, " %02X", (unsigned)data[i]);
}

#define TEST_MCMP(name, exp, rec, size) do { \
    const void* lptr = (exp);                \
    const void* rptr = (rec);                \
    const size_t sz = (size);                \
    if (memcmp(lptr, rptr, sz) == 0) break;  \
    fprintf(stderr, "%s:%i: Failed memory compare: %s\n", FILENM, __LINE__, (name)); \
    fprintf(stderr, "  Expected: "); memdump(stderr, lptr, sz); fputc('\n', stderr); \
    fprintf(stderr, "  Received: "); memdump(stderr, rptr, sz); fputc('\n', stderr); \
    exit(EXIT_FAILURE); \
} while (0)

#define TEST_SCMP(name, exp, rec) do {  \
    const char* lptr = (exp);           \
    const char* rptr = (rec);           \
    if (strcmp(lptr, rptr) == 0) break; \
    fprintf(stderr, "%s:%i: Failed string compare: %s\n", FILENM, __LINE__, (name)); \
    fprintf(stderr, "  Expected: %s\n", lptr); \
    fprintf(stderr, "  Received: %s\n", rptr); \
    exit(EXIT_FAILURE); \
} while (0)

#define TEST_VCMP(name, exp, op, rec, type, fmt) do {     \
    const type lval = (const type)(exp);                  \
    const type rval = (const type)(rec);                  \
    if (lval op rval) break;                              \
    fprintf(stderr,                                       \
        "%s:%i: Failed value compare '"TOSTR(op)"': %s\n" \
        , FILENM, __LINE__, (name));                      \
    fprintf(stderr, "  Expected: "fmt"\n", lval);         \
    fprintf(stderr, "  Received: "fmt"\n", rval);         \
    exit(EXIT_FAILURE); \
} while (0)

#define TEST_ICMP(name, exp, op, rec) TEST_VCMP(name, exp, op, rec, int  , "%i")
#define TEST_PCMP(name, exp, op, rec) TEST_VCMP(name, exp, op, rec, void*, "%p")