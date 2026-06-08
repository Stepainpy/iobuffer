#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_ICMP("without buffer", 0, >, btell(NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("without data", 0, >, btell(buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("get from start" , 0, ==, btell(buf));
    bseek(buf, 2, BSEEK_SET);
    TEST_ICMP("get from middle", 2, ==, btell(buf));
    bseek(buf, 0, BSEEK_END);
    TEST_ICMP("get from end"   , 4, ==, btell(buf));
    bclose(buf);

    return EXIT_SUCCESS;
}