#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char buffer[4];

    TEST_ICMP("call with null pointer", EOB, ==, bputs(NULL, NULL));

    buf = bopen("Text", 4, "r");
    TEST_ICMP("call with not writable", EOB, ==, bputs(NULL, buf));
    bclose(buf);

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with null data", EOB, ==, bputs(NULL, buf));
    bclose(buf);

    buf = bmemopen(buffer, sizeof buffer, "w");

    TEST_ICMP("put string", 0, <=, bputs("Tex", buf));
    TEST_ICMP("put string", 0, > , bputs("ts.", buf));

    bvw = bview(buf);
    TEST_ICMP("after put", 3, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("after put", 3, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("after put", "Tex", buffer, 3);

    bseek(buf, 1, BSEEK_SET);

    TEST_ICMP("put string", 0, <=, bputs("ort", buf));
    bvw = bview(buf);
    TEST_ICMP("after put", 4, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("after put", 4, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("after put", "Tort", buffer, 4);

    bclose(buf);

    return EXIT_SUCCESS;
}