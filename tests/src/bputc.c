#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char buffer[4];

    TEST_CASE("call with null pointer", bputc(0, NULL) == EOB);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with not writable", bputc(0, buf) == EOB);
    bclose(buf);

    buf = bmemopen(buffer, sizeof buffer, "w");

    TEST_CASE("put character", bputc('T', buf) == 'T');
    TEST_CASE("put character", bputc('e', buf) == 'e');
    TEST_CASE("put character", bputc('x', buf) == 'x');
    TEST_CASE("put character", bputc('t', buf) == 't');
    TEST_CASE("put character", bputc('s', buf) == EOB);
    TEST_CASE("put character", bputc('.', buf) == EOB);

    bvw = bview(buf);
    TEST_CASE("after put", BV_LEN(bvw, base, stop) == 4);
    TEST_CASE("after put", BV_LEN(bvw, base, head) == 4);
    TEST_CASE("after put", BV_LEN(bvw, head, stop) == 0);
    TEST_CASE("after put", memcmp(buffer, "Text", 4) == 0);

    bclose(buf);

    return EXIT_SUCCESS;
}