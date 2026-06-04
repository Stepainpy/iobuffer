#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char* filename(const char* path) {
    const char* loc;

    loc = strrchr(path, '/');
    if (loc) return loc + 1;

    loc = strrchr(path, '\\');
    if (loc) return loc + 1;

    return path;
}

#define TEST_CASE(name, cond) ((void)(           \
    !!(cond) || (                                \
        fprintf(stderr,                          \
            "%s:%i: Failed case: %s\n",          \
            filename(__FILE__), __LINE__, name), \
        exit(EXIT_FAILURE), 0                    \
    )                                            \
))
