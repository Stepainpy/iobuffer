#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; int ret;

    TEST_ICMP("without buffer", 0, !=, bseek(NULL, 0, 0));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("without data", 0, !=, bseek(buf, 0, 0));
    bclose(buf);

    buf = bopen("Two words", 9, "a");

    TEST_ICMP("pass incorrect base", 0, !=, bseek(buf, 0, -1));
    TEST_ICMP("pass incorrect base", 0, !=, bseek(buf, 0, 10));

    /* BSEEK_SET */

    ret = bseek(buf, 0, BSEEK_SET);
    bvw = bview(buf);
    TEST_ICMP("move to SET/0", 0, ==, ret);
    TEST_ICMP("move to SET/0", 0, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, 5, BSEEK_SET);
    bvw = bview(buf);
    TEST_ICMP("move to SET/5", 0, ==, ret);
    TEST_ICMP("move to SET/5", 5, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, -5, BSEEK_SET);
    bvw = bview(buf);
    TEST_ICMP("move to SET/-5", 0, !=, ret);
    TEST_ICMP("move to SET/-5", 5, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, 10, BSEEK_SET);
    bvw = bview(buf);
    TEST_ICMP("move to SET/10", 0, !=, ret);
    TEST_ICMP("move to SET/10", 5, ==, BV_LEN(bvw, base, head));

    /* BSEEK_END */

    ret = bseek(buf, 0, BSEEK_END);
    bvw = bview(buf);
    TEST_ICMP("move to END/0", 0, ==, ret);
    TEST_ICMP("move to END/0", 9, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, -5, BSEEK_END);
    bvw = bview(buf);
    TEST_ICMP("move to END/-5", 0, ==, ret);
    TEST_ICMP("move to END/-5", 4, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, 5, BSEEK_END);
    bvw = bview(buf);
    TEST_ICMP("move to END/5", 0, !=, ret);
    TEST_ICMP("move to END/5", 4, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, -10, BSEEK_END);
    bvw = bview(buf);
    TEST_ICMP("move to END/-10", 0, !=, ret);
    TEST_ICMP("move to END/-10", 4, ==, BV_LEN(bvw, base, head));

    /* BSEEK_CUR */
    bseek(buf, 4, BSEEK_SET);

    ret = bseek(buf, 0, BSEEK_CUR);
    bvw = bview(buf);
    TEST_ICMP("move to CUR/0", 0, ==, ret);
    TEST_ICMP("move to CUR/0", 4, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, 3, BSEEK_CUR);
    bvw = bview(buf);
    TEST_ICMP("move to CUR/3", 0, ==, ret);
    TEST_ICMP("move to CUR/3", 7, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, -3, BSEEK_CUR);
    bvw = bview(buf);
    TEST_ICMP("move to CUR/-3", 0, ==, ret);
    TEST_ICMP("move to CUR/-3", 4, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, 10, BSEEK_CUR);
    bvw = bview(buf);
    TEST_ICMP("move to CUR/10", 0, !=, ret);
    TEST_ICMP("move to CUR/10", 4, ==, BV_LEN(bvw, base, head));

    ret = bseek(buf, -10, BSEEK_CUR);
    bvw = bview(buf);
    TEST_ICMP("move to CUR/-10", 0, !=, ret);
    TEST_ICMP("move to CUR/-10", 4, ==, BV_LEN(bvw, base, head));

    bclose(buf);

    return EXIT_SUCCESS;
}