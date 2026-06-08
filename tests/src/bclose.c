#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    char chbuf[8];

    BUFFER*    alloc = bopen("", 0, "w");
    BUFFER* fix      = bmemopen(chbuf, sizeof chbuf, "w");
    BUFFER* fixalloc = bmemopen(NULL,  sizeof chbuf, "w");

    TEST_ICMP("close null pointer"        , EOB, ==, bclose(NULL));
    TEST_ICMP("close flexible allocated"  ,  0 , ==, bclose(   alloc));
    TEST_ICMP("close fixed foreign buffer",  0 , ==, bclose(fix     ));
    TEST_ICMP("close fixed allocated"     ,  0 , ==, bclose(fixalloc));

    return EXIT_SUCCESS;
}