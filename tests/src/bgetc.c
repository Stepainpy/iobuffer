#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_CASE("call with null pointer", bgetc(NULL) == EOB);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", bgetc(buf) == EOB);
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_CASE("call with not readable", bgetc(buf) == EOB);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("extract first"        , bgetc(buf) == 'T');
    TEST_CASE("extract middle"       , bgetc(buf) == 'e');
    TEST_CASE("extract middle"       , bgetc(buf) == 'x');
    TEST_CASE("extract last"         , bgetc(buf) == 't');
    TEST_CASE("extract after last"   , bgetc(buf) == EOB);
    TEST_CASE("extract one more time", bgetc(buf) == EOB);
    bclose(buf);

    return EXIT_SUCCESS;
}