#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; int ret;

    TEST_CASE("call with null pointer", breset(NULL) != 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", breset(buf) != 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with not writable", breset(buf) != 0);
    bclose(buf);

    buf = bopen("Text", 4, "a");
    ret = breset(buf);
    bvw = bview(buf);
    TEST_CASE("reset at end", ret == 0);
    TEST_CASE("reset at end", BV_LEN(bvw, base, stop) == 0);
    TEST_CASE("reset at end", BV_LEN(bvw, base, head) == 0);
    TEST_CASE("reset at end", BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    return EXIT_SUCCESS;
}