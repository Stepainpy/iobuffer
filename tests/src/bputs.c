#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; BUFVIEW bvw;
    char buffer[4];

    TEST_CASE("call with null pointer", bputs(NULL, NULL) == EOB);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with not writable", bputs(NULL, buf) == EOB);
    bclose(buf);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with null data", bputs(NULL, buf) == EOB);
    bclose(buf);

    buf = bmemopen(buffer, sizeof buffer, "w");

    TEST_CASE("put string", bputs("Tex", buf) >= 0);
    TEST_CASE("put string", bputs("ts.", buf) <  0);

    bvw = bview(buf);
    TEST_CASE("after put", BV_LEN(bvw, base, stop) == 3);
    TEST_CASE("after put", BV_LEN(bvw, base, head) == 3);
    TEST_CASE("after put", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("after put", "Tex", buffer, 3);

    bseek(buf, 1, BSEEK_SET);

    TEST_CASE("put string", bputs("ort", buf) >= 0);
    bvw = bview(buf);
    TEST_CASE("after put", BV_LEN(bvw, base, stop) == 4);
    TEST_CASE("after put", BV_LEN(bvw, base, head) == 4);
    TEST_CASE("after put", BV_LEN(bvw, head, stop) == 0);
    TEST_MCMP("after put", "Tort", buffer, 4);

    bclose(buf);

    return EXIT_SUCCESS;
}