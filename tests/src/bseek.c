#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; int ret;

    TEST_CASE("without buffer", bseek(NULL, 0, 0) != 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("without data", bseek(buf, 0, 0) != 0);
    bclose(buf);

    buf = bopen("Two words", 9, "a");

    TEST_CASE("pass incorrect base", bseek(buf, 0, -1) != 0);
    TEST_CASE("pass incorrect base", bseek(buf, 0, 10) != 0);

    /* BSEEK_SET */

    ret = bseek(buf, 0, BSEEK_SET);
    bvw = bview(buf);
    TEST_CASE("move to SET/0", ret == 0);
    TEST_CASE("move to SET/0", BV_LEN(bvw, base, head) == 0);

    ret = bseek(buf, 5, BSEEK_SET);
    bvw = bview(buf);
    TEST_CASE("move to SET/5", ret == 0);
    TEST_CASE("move to SET/5", BV_LEN(bvw, base, head) == 5);

    ret = bseek(buf, -5, BSEEK_SET);
    bvw = bview(buf);
    TEST_CASE("move to SET/-5", ret != 0);
    TEST_CASE("move to SET/-5", BV_LEN(bvw, base, head) == 5);

    ret = bseek(buf, 10, BSEEK_SET);
    bvw = bview(buf);
    TEST_CASE("move to SET/10", ret != 0);
    TEST_CASE("move to SET/10", BV_LEN(bvw, base, head) == 5);

    /* BSEEK_END */

    ret = bseek(buf, 0, BSEEK_END);
    bvw = bview(buf);
    TEST_CASE("move to END/0", ret == 0);
    TEST_CASE("move to END/0", BV_LEN(bvw, base, head) == 9);

    ret = bseek(buf, -5, BSEEK_END);
    bvw = bview(buf);
    TEST_CASE("move to END/-5", ret == 0);
    TEST_CASE("move to END/-5", BV_LEN(bvw, base, head) == 4);

    ret = bseek(buf, 5, BSEEK_END);
    bvw = bview(buf);
    TEST_CASE("move to END/5", ret != 0);
    TEST_CASE("move to END/5", BV_LEN(bvw, base, head) == 4);

    ret = bseek(buf, -10, BSEEK_END);
    bvw = bview(buf);
    TEST_CASE("move to END/-10", ret != 0);
    TEST_CASE("move to END/-10", BV_LEN(bvw, base, head) == 4);

    /* BSEEK_CUR */
    bseek(buf, 4, BSEEK_SET);

    ret = bseek(buf, 0, BSEEK_CUR);
    bvw = bview(buf);
    TEST_CASE("move to CUR/0", ret == 0);
    TEST_CASE("move to CUR/0", BV_LEN(bvw, base, head) == 4);

    ret = bseek(buf, 3, BSEEK_CUR);
    bvw = bview(buf);
    TEST_CASE("move to CUR/3", ret == 0);
    TEST_CASE("move to CUR/3", BV_LEN(bvw, base, head) == 7);

    ret = bseek(buf, -3, BSEEK_CUR);
    bvw = bview(buf);
    TEST_CASE("move to CUR/-3", ret == 0);
    TEST_CASE("move to CUR/-3", BV_LEN(bvw, base, head) == 4);

    ret = bseek(buf, 10, BSEEK_CUR);
    bvw = bview(buf);
    TEST_CASE("move to CUR/10", ret != 0);
    TEST_CASE("move to CUR/10", BV_LEN(bvw, base, head) == 4);

    ret = bseek(buf, -10, BSEEK_CUR);
    bvw = bview(buf);
    TEST_CASE("move to CUR/-10", ret != 0);
    TEST_CASE("move to CUR/-10", BV_LEN(bvw, base, head) == 4);

    bclose(buf);

    return EXIT_SUCCESS;
}