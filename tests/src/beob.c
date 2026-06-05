#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_CASE("call with null pointer", beob(NULL) == 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", beob(buf) == 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("check EOB", beob(buf) == 0);
    bseek(buf, -1, BSEEK_END);
    TEST_CASE("check EOB", beob(buf) == 0);
    bseek(buf,  0, BSEEK_END);
    TEST_CASE("check EOB", beob(buf) == 1);
    bclose(buf);

    return EXIT_SUCCESS;
}