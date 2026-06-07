#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; char dest[8];

    memset(dest, 0xC3, sizeof dest);

    TEST_CASE("call with null pointer", bgets(NULL, 0, NULL) == NULL);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not allocated data", bgets(NULL, 0, buf) == NULL);
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_CASE("call with not readable", bgets(NULL, 0, buf) == NULL);
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with null dest"  , bgets(NULL, 0, buf) == NULL);
    TEST_CASE("call with zero length", bgets(dest, 0, buf) == NULL);
    TEST_CASE("call with one length" , bgets(dest, 1, buf) == dest);
    TEST_CASE("check one length", dest[0] == '\0');
    bclose(buf);

    buf = bopen(
        "Alan Turing\n"
        "John von Neumann\n"
        "Alonzo Church\n"
    , 43, "r");

    TEST_CASE("extract Alan"  , bgets(dest, sizeof dest, buf) == dest);
    TEST_SCMP("extract Alan"  , "Alan Tu", dest);
    TEST_CASE("extract Alan"  , bgets(dest, sizeof dest, buf) == dest);
    TEST_SCMP("extract Alan"  , "ring\n", dest);
    TEST_CASE("extract John"  , bgets(dest, sizeof dest, buf) == dest);
    TEST_SCMP("extract John"  , "John vo", dest);
    TEST_CASE("extract John"  , bgets(dest, sizeof dest, buf) == dest);
    TEST_SCMP("extract John"  , "n Neuma", dest);
    TEST_CASE("extract John"  , bgets(dest, sizeof dest, buf) == dest);
    TEST_SCMP("extract John"  , "nn\n", dest);
    TEST_CASE("extract Alonzo", bgets(dest, sizeof dest, buf) == dest);
    TEST_SCMP("extract Alonzo", "Alonzo ", dest);
    TEST_CASE("extract Alonzo", bgets(dest, sizeof dest, buf) == dest);
    TEST_SCMP("extract Alonzo", "Church\n", dest);
    TEST_CASE("extract nothing", bgets(dest, sizeof dest, buf) == NULL);

    bclose(buf);

    return EXIT_SUCCESS;
}