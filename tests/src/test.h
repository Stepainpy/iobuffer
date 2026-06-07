#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

#define TEST_CASE(name, cond) ((void)(           \
    !!(cond) || (                                \
        fprintf(stderr,                          \
            "%s:%i: Failed case: %s\n",          \
            filename(__FILE__), __LINE__, name), \
        exit(EXIT_FAILURE), 0                    \
    )                                            \
))
