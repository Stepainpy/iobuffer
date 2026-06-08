#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; int ret;

    /* Wrong usage */

    TEST_ICMP("call with null pointer", 0, !=, berase(NULL, 0));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", 0, !=, berase(buf, 0));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("call with not writable", 0, !=, berase(buf, 2));
    bclose(buf);

    /* Correct usage, start */

    buf = bopen("beaver", 6, "r+");
    ret = berase(buf, 3);
    bvw = bview(buf);
    TEST_ICMP("remove from start | less than length", 0, ==, ret);
    TEST_ICMP("remove from start | less than length", 3, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from start | less than length", 0, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("remove from start | less than length", "ver", bvw.base, 3);
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    ret = berase(buf, 6);
    bvw = bview(buf);
    TEST_ICMP("remove from start | equal to length", 0, ==, ret);
    TEST_ICMP("remove from start | equal to length", 0, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from start | equal to length", 0, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    ret = berase(buf, 10);
    bvw = bview(buf);
    TEST_ICMP("remove from start | greater than length", 0, ==, ret);
    TEST_ICMP("remove from start | greater than length", 0, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from start | greater than length", 0, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    /* Correct usage, middle */

    buf = bopen("beaver", 6, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = berase(buf, 2);
    bvw = bview(buf);
    TEST_ICMP("remove from middle | less than length", 0, ==, ret);
    TEST_ICMP("remove from middle | less than length", 4, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from middle | less than length", 2, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("remove from middle | less than length", "beer", bvw.base, 4);
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = berase(buf, 4);
    bvw = bview(buf);
    TEST_ICMP("remove from middle | equal to length", 0, ==, ret);
    TEST_ICMP("remove from middle | equal to length", 2, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from middle | equal to length", 2, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = berase(buf, 10);
    bvw = bview(buf);
    TEST_ICMP("remove from middle | greater than length", 0, ==, ret);
    TEST_ICMP("remove from middle | greater than length", 2, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from middle | greater than length", 2, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    /* Correct usage, end */

    buf = bopen("beaver", 6, "a+");
    ret = berase(buf, 0);
    bvw = bview(buf);
    TEST_ICMP("remove from end | zero length", 0, ==, ret);
    TEST_ICMP("remove from end | zero length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from end | zero length", 6, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    buf = bopen("beaver", 6, "a+");
    ret = berase(buf, 10);
    bvw = bview(buf);
    TEST_ICMP("remove from end | nonzero length", 0, ==, ret);
    TEST_ICMP("remove from end | nonzero length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("remove from end | nonzero length", 6, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    return EXIT_SUCCESS;
}