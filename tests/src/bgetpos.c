#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; bpos_t pos;

    TEST_CASE("without buffer and pos", bgetpos(NULL, NULL) != 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("without data and pos", bgetpos(buf, NULL) != 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("without pos", bgetpos(buf, NULL) != 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("get from start", bgetpos(buf, &pos) == 0);
    TEST_CASE("get from start", pos == 0);
    bseek(buf, 2, BSEEK_CUR);
    TEST_CASE("get from middle", bgetpos(buf, &pos) == 0);
    TEST_CASE("get from middle", pos == 2);
    bseek(buf, 0, BSEEK_END);
    TEST_CASE("get from end", bgetpos(buf, &pos) == 0);
    TEST_CASE("get from end", pos == 4);
    bclose(buf);

    return EXIT_SUCCESS;
}