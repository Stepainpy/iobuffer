#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;

    TEST_CASE("call with null pointer", bungetc(0, NULL) == EOB);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", bungetc(0, buf) == EOB);
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_CASE("call with not readable", bungetc(0, buf) == EOB);
    bclose(buf);

    buf = bopen("Text", 4, "a+");

    TEST_CASE("unget character", bungetc('m', buf) == 'm');
    TEST_CASE("unget character", bungetc('r', buf) == 'r');

    bvw = bview(buf);
    TEST_CASE("check intermedian", BV_LEN(bvw, base, stop) == 4);
    TEST_CASE("check intermedian", BV_LEN(bvw, base, head) == 2);
    TEST_CASE("check intermedian", BV_LEN(bvw, head, stop) == 2);
    TEST_MCMP("check intermedian", "Term", bvw.base, 4);

    TEST_CASE("unget character", bungetc('a', buf) == 'a');
    TEST_CASE("unget character", bungetc('H', buf) == 'H');
    TEST_CASE("unget character", bungetc('>', buf) == EOB);
    TEST_CASE("unget character", bungetc(EOB, buf) == EOB);

    bvw = bview(buf);
    TEST_CASE("check intermedian", BV_LEN(bvw, base, stop) == 4);
    TEST_CASE("check intermedian", BV_LEN(bvw, base, head) == 0);
    TEST_CASE("check intermedian", BV_LEN(bvw, head, stop) == 4);
    TEST_MCMP("check intermedian", "Harm", bvw.base, 4);

    bclose(buf);

    return EXIT_SUCCESS;
}