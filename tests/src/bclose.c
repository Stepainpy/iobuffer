#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    char chbuf[8];

    BUFFER*    alloc = bopen("", 0, "w");
    BUFFER* fix      = bmemopen(chbuf, sizeof chbuf, "w");
    BUFFER* fixalloc = bmemopen(NULL,  sizeof chbuf, "w");

    TEST_CASE("close null pointer"        , bclose(NULL    ) == EOB);
    TEST_CASE("close flexible allocated"  , bclose(   alloc) ==  0 );
    TEST_CASE("close fixed foreign buffer", bclose(fix     ) ==  0 );
    TEST_CASE("close fixed allocated"     , bclose(fixalloc) ==  0 );

    return EXIT_SUCCESS;
}