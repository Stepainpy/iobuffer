#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;

    bvw = bview(NULL);
    TEST_CASE("call with null pointer", bvw.base == NULL);
    TEST_CASE("call with null pointer", bvw.head == NULL);
    TEST_CASE("call with null pointer", bvw.stop == NULL);

    buf = bopen("Two words", 9, "r");
    bseek(buf, 3, BSEEK_SET);
    bvw = bview(buf);
    TEST_CASE("get pointers", BV_LEN(bvw, base, stop) == 9);
    TEST_CASE("get pointers", BV_LEN(bvw, base, head) == 3);
    TEST_CASE("get pointers", BV_LEN(bvw, head, stop) == 6);
    bclose(buf);

    return EXIT_SUCCESS;
}