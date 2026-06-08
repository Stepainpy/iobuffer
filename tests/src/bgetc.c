#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_ICMP("call with null pointer", EOB, ==, bgetc(NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", EOB, ==, bgetc(buf));
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_ICMP("call with not readable", EOB, ==, bgetc(buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("extract first"        , 'T', ==, bgetc(buf));
    TEST_ICMP("extract middle"       , 'e', ==, bgetc(buf));
    TEST_ICMP("extract middle"       , 'x', ==, bgetc(buf));
    TEST_ICMP("extract last"         , 't', ==, bgetc(buf));
    TEST_ICMP("extract after last"   , EOB, ==, bgetc(buf));
    TEST_ICMP("extract one more time", EOB, ==, bgetc(buf));
    bclose(buf);

    return EXIT_SUCCESS;
}