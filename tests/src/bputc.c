#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char buffer[4];

    TEST_ICMP("call with null pointer", EOB, ==, bputc(0, NULL));

    buf = bopen("Text", 4, "r");
    TEST_ICMP("call with not writable", EOB, ==, bputc(0, buf));
    bclose(buf);

    buf = bmemopen(buffer, sizeof buffer, "w");

    TEST_ICMP("put character", 'T', ==, bputc('T', buf));
    TEST_ICMP("put character", 'e', ==, bputc('e', buf));
    TEST_ICMP("put character", 'x', ==, bputc('x', buf));
    TEST_ICMP("put character", 't', ==, bputc('t', buf));
    TEST_ICMP("put character", EOB, ==, bputc('s', buf));
    TEST_ICMP("put character", EOB, ==, bputc('.', buf));

    bvw = bview(buf);
    TEST_ICMP("after put", 4, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("after put", 4, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("after put", "Text", buffer, 4);

    bclose(buf);

    return EXIT_SUCCESS;
}