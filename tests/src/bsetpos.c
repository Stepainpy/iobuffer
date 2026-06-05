#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; bpos_t pos;

    TEST_CASE("without buffer and pos", bsetpos(NULL, NULL) != 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("without data and pos", bsetpos(buf, NULL) != 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("without pos", bsetpos(buf, NULL) != 0);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    pos = 0;          TEST_CASE("set to start" ,    bsetpos(buf, &pos)      == 0);
    bvw = bview(buf); TEST_CASE("set to start" ,    BV_LEN(bvw, base, head) == 0);
    pos = 2;          TEST_CASE("set to middle",    bsetpos(buf, &pos)      == 0);
    bvw = bview(buf); TEST_CASE("set to middle",    BV_LEN(bvw, base, head) == 2);
    pos = 4;          TEST_CASE("set to end",       bsetpos(buf, &pos)      == 0);
    bvw = bview(buf); TEST_CASE("set to end",       BV_LEN(bvw, base, head) == 4);
    pos = 5;          TEST_CASE("set to after end", bsetpos(buf, &pos)      != 0);
    bvw = bview(buf); TEST_CASE("set to after end", BV_LEN(bvw, base, head) == 4);
    bclose(buf);

    return EXIT_SUCCESS;
}