#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_ICMP("call with null pointer", 0, ==, beob(NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", 0, ==, beob(buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("check EOB", 0, ==, beob(buf));
    bseek(buf, -1, BSEEK_END);
    TEST_ICMP("check EOB", 0, ==, beob(buf));
    bseek(buf,  0, BSEEK_END);
    TEST_ICMP("check EOB", 1, ==, beob(buf));
    bclose(buf);

    return EXIT_SUCCESS;
}