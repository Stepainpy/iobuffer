#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; int ret;

    TEST_ICMP("call with null pointer", 0, !=, breset(NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", 0, !=, breset(buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("call with not writable", 0, !=, breset(buf));
    bclose(buf);

    buf = bopen("Text", 4, "a");
    ret = breset(buf);
    bvw = bview(buf);
    TEST_ICMP("reset at end", 0, ==, ret);
    TEST_ICMP("reset at end", 0, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("reset at end", 0, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    return EXIT_SUCCESS;
}