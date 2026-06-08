#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; bpos_t pos;

    TEST_ICMP("without buffer and pos", 0, !=, bgetpos(NULL, NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("without data and pos", 0, !=, bgetpos(buf, NULL));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("without pos", 0, !=, bgetpos(buf, NULL));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_ICMP("get from start", 0, ==, bgetpos(buf, &pos));
    TEST_ICMP("get from start", 0, ==, pos);
    bseek(buf, 2, BSEEK_CUR);
    TEST_ICMP("get from middle", 0, ==, bgetpos(buf, &pos));
    TEST_ICMP("get from middle", 2, ==, pos);
    bseek(buf, 0, BSEEK_END);
    TEST_ICMP("get from end", 0, ==, bgetpos(buf, &pos));
    TEST_ICMP("get from end", 4, ==, pos);
    bclose(buf);

    return EXIT_SUCCESS;
}