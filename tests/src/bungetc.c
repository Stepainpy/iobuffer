#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;

    TEST_ICMP("call with null pointer", EOB, ==, bungetc(0, NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", EOB, ==, bungetc(0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_ICMP("call with not readable", EOB, ==, bungetc(0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "a+");

    TEST_ICMP("unget character", 'm', ==, bungetc('m', buf));
    TEST_ICMP("unget character", 'r', ==, bungetc('r', buf));

    bvw = bview(buf);
    TEST_ICMP("check intermedian", 4, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("check intermedian", 2, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("check intermedian", "Term", bvw.base, 4);

    TEST_ICMP("unget character", 'a', ==, bungetc('a', buf));
    TEST_ICMP("unget character", 'H', ==, bungetc('H', buf));
    TEST_ICMP("unget character", EOB, ==, bungetc('>', buf));
    TEST_ICMP("unget character", EOB, ==, bungetc(EOB, buf));

    bvw = bview(buf);
    TEST_ICMP("check intermedian", 4, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("check intermedian", 0, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("check intermedian", "Harm", bvw.base, 4);

    bclose(buf);

    return EXIT_SUCCESS;
}