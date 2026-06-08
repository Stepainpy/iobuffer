#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char dest[8]; size_t ret;

    TEST_ICMP("call with null pointer", 0, ==, bread(NULL, 0, 0, NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", 0, ==, bread(NULL, 0, 0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_ICMP("call with not readable", 0, ==, bread(NULL, 0, 0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("call with null dest pointer", 0, ==, bread(NULL, 1, 1, buf));
    TEST_ICMP("call with zero size"        , 0, ==, bread(dest, 0, 1, buf));
    TEST_ICMP("call with zero count"       , 0, ==, bread(dest, 1, 0, buf));
    bclose(buf);

    buf = bopen("beaver", 6, "r");

    ret = bread(dest, 4, 1, buf);
    bvw = bview(buf);
    TEST_ICMP("read from start | less than length", 1, ==, ret);
    TEST_ICMP("read from start | less than length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("read from start | less than length", 4, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("read from start | less than length", "beav", dest, 4);

    brewind(buf);
    ret = bread(dest, 2, 3, buf);
    bvw = bview(buf);
    TEST_ICMP("read from start | equal to length", 3, ==, ret);
    TEST_ICMP("read from start | equal to length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("read from start | equal to length", 6, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("read from start | equal to length", "beaver", dest, 6);

    brewind(buf);
    ret = bread(dest, 5, 2, buf);
    bvw = bview(buf);
    TEST_ICMP("read from start | greater than length", 1, ==, ret);
    TEST_ICMP("read from start | greater than length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("read from start | greater than length", 5, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("read from start | greater than length", "beave", dest, 5);

    bseek(buf, 2, BSEEK_SET);
    ret = bread(dest, 1, 3, buf);
    bvw = bview(buf);
    TEST_ICMP("read from middle | less than length", 3, ==, ret);
    TEST_ICMP("read from middle | less than length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("read from middle | less than length", 5, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("read from middle | less than length", "ave", dest, 3);

    bseek(buf, 2, BSEEK_SET);
    ret = bread(dest, 2, 2, buf);
    bvw = bview(buf);
    TEST_ICMP("read from middle | equal to length", 2, ==, ret);
    TEST_ICMP("read from middle | equal to length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("read from middle | equal to length", 6, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("read from middle | equal to length", "aver", dest, 4);

    bseek(buf, 2, BSEEK_SET);
    ret = bread(dest, 3, 2, buf);
    bvw = bview(buf);
    TEST_ICMP("read from middle | greater than length", 1, ==, ret);
    TEST_ICMP("read from middle | greater than length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("read from middle | greater than length", 5, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("read from middle | greater than length", "ave", dest, 3);

    bseek(buf, 0, BSEEK_END);
    ret = bread(dest, 1, 1, buf);
    bvw = bview(buf);
    TEST_ICMP("read from end | greater than length", 0, ==, ret);
    TEST_ICMP("read from end | greater than length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("read from end | greater than length", 6, ==, BV_LEN(bvw, base, head));

    bclose(buf);

    return EXIT_SUCCESS;
}