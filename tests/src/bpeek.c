#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_CASE("call with null pointer", bpeek(NULL) == EOB);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", bpeek(buf) == EOB);
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_CASE("call with not readable", bpeek(buf) == EOB);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("extract first"     , bpeek(buf) == 'T');
    TEST_CASE("extract first"     , bpeek(buf) == 'T');
    bseek(buf, 2, BSEEK_SET);
    TEST_CASE("extract middle"    , bpeek(buf) == 'x');
    TEST_CASE("extract middle"    , bpeek(buf) == 'x');
    bseek(buf, -1, BSEEK_END);
    TEST_CASE("extract last"      , bpeek(buf) == 't');
    TEST_CASE("extract last"      , bpeek(buf) == 't');
    bseek(buf, 0, BSEEK_END);
    TEST_CASE("extract after last", bpeek(buf) == EOB);
    TEST_CASE("extract after last", bpeek(buf) == EOB);
    bclose(buf);

    return EXIT_SUCCESS;
}