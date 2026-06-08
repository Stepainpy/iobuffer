#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_ICMP("call with null pointer", EOB, ==, bpeek(NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", EOB, ==, bpeek(buf));
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_ICMP("call with not readable", EOB, ==, bpeek(buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("extract first"     , 'T', ==, bpeek(buf));
    TEST_ICMP("extract first"     , 'T', ==, bpeek(buf));
    bseek(buf, 2, BSEEK_SET);
    TEST_ICMP("extract middle"    , 'x', ==, bpeek(buf));
    TEST_ICMP("extract middle"    , 'x', ==, bpeek(buf));
    bseek(buf, -1, BSEEK_END);
    TEST_ICMP("extract last"      , 't', ==, bpeek(buf));
    TEST_ICMP("extract last"      , 't', ==, bpeek(buf));
    bseek(buf, 0, BSEEK_END);
    TEST_ICMP("extract after last", EOB, ==, bpeek(buf));
    TEST_ICMP("extract after last", EOB, ==, bpeek(buf));
    bclose(buf);

    return EXIT_SUCCESS;
}