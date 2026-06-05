#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;

    brewind(NULL);

    buf = bopen("Text", 4, "a");
    brewind(buf);
    bvw = bview(buf);
    TEST_CASE("main test", BV_LEN(bvw, base, head) == 0);
    bclose(buf);

    return EXIT_SUCCESS;
}