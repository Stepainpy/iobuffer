#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf;

    TEST_CASE("without buffer", btell(NULL) < 0L);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("without data", btell(buf) < 0L);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("get from start", btell(buf) == 0L);
    bseek(buf, 2, BSEEK_SET);
    TEST_CASE("get from middle", btell(buf) == 2L);
    bseek(buf, 0, BSEEK_END);
    TEST_CASE("get from end", btell(buf) == 4L);
    bclose(buf);

    return EXIT_SUCCESS;
}