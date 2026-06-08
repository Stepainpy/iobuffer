#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char base[8]; size_t ret;

    TEST_ICMP("call with null pointer", 0, ==, bwrite(NULL, 0, 0, NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not allocated data", 0, ==, bwrite(NULL, 0, 0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("call with not writable", 0, ==, bwrite(NULL, 0, 0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "r+");
    TEST_ICMP("call with null source pointer", 0, ==, bwrite(NULL, 1, 1, buf));
    TEST_ICMP("call with zero size"          , 0, ==, bwrite( "" , 0, 1, buf));
    TEST_ICMP("call with zero count"         , 0, ==, bwrite( "" , 1, 0, buf));
    bclose(buf);

    buf = bmemopen(base, sizeof base, "w");
    ret = bwrite("ver", 1, 3, buf);
    bvw = bview(buf);
    TEST_ICMP("write from start | less than length", 3, ==, ret);
    TEST_ICMP("write from start | less than length", 3, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from start | less than length", 3, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from start | less than length", "ver", bvw.base, 3);
    bclose(buf);

    buf = bmemopen(base, sizeof base, "w");
    ret = bwrite("deadbeef", 4, 2, buf);
    bvw = bview(buf);
    TEST_ICMP("write from start | equal to length", 2, ==, ret);
    TEST_ICMP("write from start | equal to length", 8, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from start | equal to length", 8, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from start | equal to length", "deadbeef", bvw.base, 8);
    bclose(buf);

    buf = bmemopen(base, sizeof base, "w");
    ret = bwrite("very long", 3, 3, buf);
    bvw = bview(buf);
    TEST_ICMP("write from start | greater than length", 2, ==, ret);
    TEST_ICMP("write from start | greater than length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from start | greater than length", 6, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from start | greater than length", "very l", bvw.base, 6);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = bwrite("ee", 2, 1, buf);
    bvw = bview(buf);
    TEST_ICMP("write from middle | less than length", 1, ==, ret);
    TEST_ICMP("write from middle | less than length", 8, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from middle | less than length", 4, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from middle | less than length", "beeeer", bvw.base, 6);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = bwrite("havior", 3, 2, buf);
    bvw = bview(buf);
    TEST_ICMP("write from middle | equal to length", 2, ==, ret);
    TEST_ICMP("write from middle | equal to length", 8, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from middle | equal to length", 8, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from middle | equal to length", "behavior", bvw.base, 8);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "r+");
    bseek(buf, 4, BSEEK_SET);
    ret = bwrite("areirs", 3, 2, buf);
    bvw = bview(buf);
    TEST_ICMP("write from middle | greater than length", 1, ==, ret);
    TEST_ICMP("write from middle | greater than length", 8, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from middle | greater than length", 7, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from middle | greater than length", "beavare", bvw.base, 7);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "a");
    bseek(buf, -2, BSEEK_END); berase(buf, 2);
    ret = bwrite("s", 1, 1, buf);
    bvw = bview(buf);
    TEST_ICMP("write from end | less than length", 1, ==, ret);
    TEST_ICMP("write from end | less than length", 7, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from end | less than length", 7, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from end | less than length", "beavers", bvw.base, 7);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "a");
    bseek(buf, -2, BSEEK_END); berase(buf, 2);
    ret = bwrite("es", 1, 2, buf);
    bvw = bview(buf);
    TEST_ICMP("write from end | equal to length", 2, ==, ret);
    TEST_ICMP("write from end | equal to length", 8, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from end | equal to length", 8, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from end | equal to length", "beaveres", bvw.base, 8);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "a");
    bseek(buf, -2, BSEEK_END); berase(buf, 2);
    ret = bwrite("ift", 3, 1, buf);
    bvw = bview(buf);
    TEST_ICMP("write from end | greater than length", 0, ==, ret);
    TEST_ICMP("write from end | greater than length", 6, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP("write from end | greater than length", 6, ==, BV_LEN(bvw, base, head));
    TEST_MCMP("write from end | greater than length", "beaver", bvw.base, 7);
    bclose(buf);

    return EXIT_SUCCESS;
}