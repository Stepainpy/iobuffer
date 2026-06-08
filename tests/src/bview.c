#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;

    bvw = bview(NULL);
    TEST_PCMP("call with null pointer", NULL, ==, bvw.base);
    TEST_PCMP("call with null pointer", NULL, ==, bvw.head);
    TEST_PCMP("call with null pointer", NULL, ==, bvw.stop);

    buf = bopen("Two words", 9, "r");
    bseek(buf, 3, BSEEK_SET);
    bvw = bview(buf);
    TEST_ICMP("get pointers", 9, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("get pointers", 3, ==, BV_LEN(bvw, base, head));
    TEST_ICMP("get pointers", 6, ==, BV_LEN(bvw, head, stop));
    bclose(buf);

    return EXIT_SUCCESS;
}