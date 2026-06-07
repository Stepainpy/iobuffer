#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; int ret;

    /* Wrong usage */

    TEST_CASE("call with null pointer", berase(NULL, 0) != 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", berase(buf, 0) != 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with not writable", berase(buf, 2) != 0);
    bclose(buf);

    /* Correct usage, start */

    buf = bopen("beaver", 6, "r+");
    ret = berase(buf, 3);
    bvw = bview(buf);
    TEST_CASE("remove from start | less than length", ret == 0);
    TEST_CASE("remove from start | less than length", BV_LEN(bvw, base, stop) == 3);
    TEST_CASE("remove from start | less than length", BV_LEN(bvw, base, head) == 0);
    TEST_CASE("remove from start | less than length", BV_LEN(bvw, head, stop) == 3);
    TEST_MCMP("remove from start | less than length", "ver", bvw.base, 3);
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    ret = berase(buf, 6);
    bvw = bview(buf);
    TEST_CASE("remove from start | equal to length", ret == 0);
    TEST_CASE("remove from start | equal to length", BV_LEN(bvw, base, stop) == 0);
    TEST_CASE("remove from start | equal to length", BV_LEN(bvw, base, head) == 0);
    TEST_CASE("remove from start | equal to length", BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    ret = berase(buf, 10);
    bvw = bview(buf);
    TEST_CASE("remove from start | greater than length", ret == 0);
    TEST_CASE("remove from start | greater than length", BV_LEN(bvw, base, stop) == 0);
    TEST_CASE("remove from start | greater than length", BV_LEN(bvw, base, head) == 0);
    TEST_CASE("remove from start | greater than length", BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    /* Correct usage, middle */

    buf = bopen("beaver", 6, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = berase(buf, 2);
    bvw = bview(buf);
    TEST_CASE("remove from middle | less than length", ret == 0);
    TEST_CASE("remove from middle | less than length", BV_LEN(bvw, base, stop) == 4);
    TEST_CASE("remove from middle | less than length", BV_LEN(bvw, base, head) == 2);
    TEST_CASE("remove from middle | less than length", BV_LEN(bvw, head, stop) == 2);
    TEST_MCMP("remove from middle | less than length", "beer", bvw.base, 4);
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = berase(buf, 4);
    bvw = bview(buf);
    TEST_CASE("remove from middle | equal to length", ret == 0);
    TEST_CASE("remove from middle | equal to length", BV_LEN(bvw, base, stop) == 2);
    TEST_CASE("remove from middle | equal to length", BV_LEN(bvw, base, head) == 2);
    TEST_CASE("remove from middle | equal to length", BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    buf = bopen("beaver", 6, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = berase(buf, 10);
    bvw = bview(buf);
    TEST_CASE("remove from middle | greater than length", ret == 0);
    TEST_CASE("remove from middle | greater than length", BV_LEN(bvw, base, stop) == 2);
    TEST_CASE("remove from middle | greater than length", BV_LEN(bvw, base, head) == 2);
    TEST_CASE("remove from middle | greater than length", BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    /* Correct usage, end */

    buf = bopen("beaver", 6, "a+");
    ret = berase(buf, 0);
    bvw = bview(buf);
    TEST_CASE("remove from end | zero length", ret == 0);
    TEST_CASE("remove from end | zero length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("remove from end | zero length", BV_LEN(bvw, base, head) == 6);
    TEST_CASE("remove from end | zero length", BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    buf = bopen("beaver", 6, "a+");
    ret = berase(buf, 10);
    bvw = bview(buf);
    TEST_CASE("remove from end | nonzero length", ret == 0);
    TEST_CASE("remove from end | nonzero length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("remove from end | nonzero length", BV_LEN(bvw, base, head) == 6);
    TEST_CASE("remove from end | nonzero length", BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    return EXIT_SUCCESS;
}