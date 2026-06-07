#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char dest[8]; size_t ret;

    TEST_CASE("call with null pointer", bread(NULL, 0, 0, NULL) == 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", bread(NULL, 0, 0, buf) == 0);
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_CASE("call with not readable", bread(NULL, 0, 0, buf) == 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with null dest pointer", bread(NULL, 1, 1, buf) == 0);
    TEST_CASE("call with zero size",  bread(dest, 0, 1, buf) == 0);
    TEST_CASE("call with zero count", bread(dest, 1, 0, buf) == 0);
    bclose(buf);

    buf = bopen("beaver", 6, "r");

    ret = bread(dest, 4, 1, buf);
    bvw = bview(buf);
    TEST_CASE("read from start | less than length", ret == 1);
    TEST_CASE("read from start | less than length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("read from start | less than length", BV_LEN(bvw, base, head) == 4);
    TEST_CASE("read from start | less than length", BV_LEN(bvw, head, stop) == 2);
    TEST_MCMP("read from start | less than length", "beav", dest, 4);

    brewind(buf);
    ret = bread(dest, 2, 3, buf);
    bvw = bview(buf);
    TEST_CASE("read from start | equal to length", ret == 3);
    TEST_CASE("read from start | equal to length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("read from start | equal to length", BV_LEN(bvw, base, head) == 6);
    TEST_CASE("read from start | equal to length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("read from start | equal to length", "beaver", dest, 6);

    brewind(buf);
    ret = bread(dest, 5, 2, buf);
    bvw = bview(buf);
    TEST_CASE("read from start | greater than length", ret == 1);
    TEST_CASE("read from start | greater than length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("read from start | greater than length", BV_LEN(bvw, base, head) == 5);
    TEST_CASE("read from start | greater than length", BV_LEN(bvw, head, stop) == 1);
    TEST_MCMP("read from start | greater than length", "beave", dest, 5);

    bseek(buf, 2, BSEEK_SET);
    ret = bread(dest, 1, 3, buf);
    bvw = bview(buf);
    TEST_CASE("read from middle | less than length", ret == 3);
    TEST_CASE("read from middle | less than length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("read from middle | less than length", BV_LEN(bvw, base, head) == 5);
    TEST_CASE("read from middle | less than length", BV_LEN(bvw, head, stop) == 1);
    TEST_MCMP("read from middle | less than length", "ave", dest, 3);

    bseek(buf, 2, BSEEK_SET);
    ret = bread(dest, 2, 2, buf);
    bvw = bview(buf);
    TEST_CASE("read from middle | equal to length", ret == 2);
    TEST_CASE("read from middle | equal to length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("read from middle | equal to length", BV_LEN(bvw, base, head) == 6);
    TEST_CASE("read from middle | equal to length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("read from middle | equal to length", "aver", dest, 4);

    bseek(buf, 2, BSEEK_SET);
    ret = bread(dest, 3, 2, buf);
    bvw = bview(buf);
    TEST_CASE("read from middle | greater than length", ret == 1);
    TEST_CASE("read from middle | greater than length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("read from middle | greater than length", BV_LEN(bvw, base, head) == 5);
    TEST_CASE("read from middle | greater than length", BV_LEN(bvw, head, stop) == 1);
    TEST_MCMP("read from middle | greater than length", "ave", dest, 3);

    bseek(buf, 0, BSEEK_END);
    ret = bread(dest, 1, 1, buf);
    bvw = bview(buf);
    TEST_CASE("read from end | greater than length", ret == 0);
    TEST_CASE("read from end | greater than length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("read from end | greater than length", BV_LEN(bvw, base, head) == 6);
    TEST_CASE("read from end | greater than length", BV_LEN(bvw, head, stop) == 0);

    bclose(buf);

    return EXIT_SUCCESS;
}