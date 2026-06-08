#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw; bpos_t pos;

    TEST_ICMP("without buffer and pos", 0, !=, bsetpos(NULL, NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("without data and pos", 0, !=, bsetpos(buf, NULL));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("without pos", 0, !=, bsetpos(buf, NULL));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    pos = 0;          TEST_ICMP("set to start"    , 0, ==, bsetpos(buf, &pos));
    bvw = bview(buf); TEST_ICMP("set to start"    , 0, ==, BV_LEN(bvw, base, head));
    pos = 2;          TEST_ICMP("set to middle"   , 0, ==, bsetpos(buf, &pos));
    bvw = bview(buf); TEST_ICMP("set to middle"   , 2, ==, BV_LEN(bvw, base, head));
    pos = 4;          TEST_ICMP("set to end"      , 0, ==, bsetpos(buf, &pos));
    bvw = bview(buf); TEST_ICMP("set to end"      , 4, ==, BV_LEN(bvw, base, head));
    pos = 5;          TEST_ICMP("set to after end", 0, !=, bsetpos(buf, &pos));
    bvw = bview(buf); TEST_ICMP("set to after end", 4, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    return EXIT_SUCCESS;
}