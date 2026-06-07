#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char base[8]; size_t ret;

    TEST_CASE("call with null pointer", bwrite(NULL, 0, 0, NULL) == 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", bwrite(NULL, 0, 0, buf) == 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with not writable", bwrite(NULL, 0, 0, buf) == 0);
    bclose(buf);

    buf = bopen("Text", 4, "r+");
    TEST_CASE("call with null source pointer", bwrite(NULL, 1, 1, buf) == 0);
    TEST_CASE("call with zero size",  bwrite("", 0, 1, buf) == 0);
    TEST_CASE("call with zero count", bwrite("", 1, 0, buf) == 0);
    bclose(buf);

    buf = bmemopen(base, sizeof base, "w");
    ret = bwrite("ver", 1, 3, buf);
    bvw = bview(buf);
    TEST_CASE("write from start | less than length", ret == 3);
    TEST_CASE("write from start | less than length", BV_LEN(bvw, base, stop) == 3);
    TEST_CASE("write from start | less than length", BV_LEN(bvw, base, head) == 3);
    TEST_CASE("write from start | less than length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("write from start | less than length", "ver", bvw.base, 3);
    bclose(buf);

    buf = bmemopen(base, sizeof base, "w");
    ret = bwrite("deadbeef", 4, 2, buf);
    bvw = bview(buf);
    TEST_CASE("write from start | equal to length", ret == 2);
    TEST_CASE("write from start | equal to length", BV_LEN(bvw, base, stop) == 8);
    TEST_CASE("write from start | equal to length", BV_LEN(bvw, base, head) == 8);
    TEST_CASE("write from start | equal to length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("write from start | equal to length", "deadbeef", bvw.base, 8);
    bclose(buf);

    buf = bmemopen(base, sizeof base, "w");
    ret = bwrite("very long", 3, 3, buf);
    bvw = bview(buf);
    TEST_CASE("write from start | greater than length", ret == 2);
    TEST_CASE("write from start | greater than length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("write from start | greater than length", BV_LEN(bvw, base, head) == 6);
    TEST_CASE("write from start | greater than length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("write from start | greater than length", "very l", bvw.base, 6);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = bwrite("ee", 2, 1, buf);
    bvw = bview(buf);
    TEST_CASE("write from middle | less than length", ret == 1);
    TEST_CASE("write from middle | less than length", BV_LEN(bvw, base, stop) == 8);
    TEST_CASE("write from middle | less than length", BV_LEN(bvw, base, head) == 4);
    TEST_CASE("write from middle | less than length", BV_LEN(bvw, head, stop) == 4);
    TEST_MCMP("write from middle | less than length", "beeeer", bvw.base, 6);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "r+");
    bseek(buf, 2, BSEEK_SET);
    ret = bwrite("havior", 3, 2, buf);
    bvw = bview(buf);
    TEST_CASE("write from middle | equal to length", ret == 2);
    TEST_CASE("write from middle | equal to length", BV_LEN(bvw, base, stop) == 8);
    TEST_CASE("write from middle | equal to length", BV_LEN(bvw, base, head) == 8);
    TEST_CASE("write from middle | equal to length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("write from middle | equal to length", "behavior", bvw.base, 8);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "r+");
    bseek(buf, 4, BSEEK_SET);
    ret = bwrite("areirs", 3, 2, buf);
    bvw = bview(buf);
    TEST_CASE("write from middle | greater than length", ret == 1);
    TEST_CASE("write from middle | greater than length", BV_LEN(bvw, base, stop) == 8);
    TEST_CASE("write from middle | greater than length", BV_LEN(bvw, base, head) == 7);
    TEST_CASE("write from middle | greater than length", BV_LEN(bvw, head, stop) == 1);
    TEST_MCMP("write from middle | greater than length", "beavare", bvw.base, 7);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "a");
    bseek(buf, -2, BSEEK_END); berase(buf, 2);
    ret = bwrite("s", 1, 1, buf);
    bvw = bview(buf);
    TEST_CASE("write from end | less than length", ret == 1);
    TEST_CASE("write from end | less than length", BV_LEN(bvw, base, stop) == 7);
    TEST_CASE("write from end | less than length", BV_LEN(bvw, base, head) == 7);
    TEST_CASE("write from end | less than length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("write from end | less than length", "beavers", bvw.base, 7);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "a");
    bseek(buf, -2, BSEEK_END); berase(buf, 2);
    ret = bwrite("es", 1, 2, buf);
    bvw = bview(buf);
    TEST_CASE("write from end | equal to length", ret == 2);
    TEST_CASE("write from end | equal to length", BV_LEN(bvw, base, stop) == 8);
    TEST_CASE("write from end | equal to length", BV_LEN(bvw, base, head) == 8);
    TEST_CASE("write from end | equal to length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("write from end | equal to length", "beaveres", bvw.base, 8);
    bclose(buf);

    strcpy(base, "beaver");
    buf = bmemopen(base, sizeof base, "a");
    bseek(buf, -2, BSEEK_END); berase(buf, 2);
    ret = bwrite("ift", 3, 1, buf);
    bvw = bview(buf);
    TEST_CASE("write from end | greater than length", ret == 0);
    TEST_CASE("write from end | greater than length", BV_LEN(bvw, base, stop) == 6);
    TEST_CASE("write from end | greater than length", BV_LEN(bvw, base, head) == 6);
    TEST_CASE("write from end | greater than length", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("write from end | greater than length", "beaver", bvw.base, 7);
    bclose(buf);

    return EXIT_SUCCESS;
}