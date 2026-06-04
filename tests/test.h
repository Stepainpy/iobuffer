#include <stdlib.h>
#include <stdio.h>

#define TEST_CASE(name, cond) ((void)(     \
    !!(cond) || (                          \
        fprintf(stderr,                    \
            "%s:%i: Failed assert '%s'\n", \
            __FILE__, __LINE__, name),     \
        exit(EXIT_FAILURE), 0              \
    )                                      \
))
