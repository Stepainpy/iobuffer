#include <iobuffer/iobuffer.h>
#include "test.h"

int main(void) {
    BUFFER* buf; char dest[8];

    memset(dest, 0xC3, sizeof dest);

    TEST_PCMP("call with null pointer", NULL, ==, bgets(NULL, 0, NULL));

    buf = bopen(NULL, 0, "w");
    TEST_PCMP("call with not allocated data", NULL, ==, bgets(NULL, 0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "a");
    TEST_PCMP("call with not readable", NULL, ==, bgets(NULL, 0, buf));
    bclose(buf);

    buf = bopen("Text", 4, "r");
    TEST_PCMP("call with null dest"  , NULL, ==, bgets(NULL, 0, buf));
    TEST_PCMP("call with zero length", NULL, ==, bgets(dest, 0, buf));
    TEST_PCMP("call with one length" , dest, ==, bgets(dest, 1, buf));
    TEST_ICMP("check one length", '\0', ==, dest[0]);
    bclose(buf);

    buf = bopen(
        "Alan Turing\n"
        "John von Neumann\n"
        "Alonzo Church\n"
    , 43, "r");

    TEST_PCMP("extract Alan"  , dest, ==, bgets(dest, sizeof dest, buf));
    TEST_SCMP("extract Alan"  , "Alan Tu", dest);
    TEST_PCMP("extract Alan"  , dest, ==, bgets(dest, sizeof dest, buf));
    TEST_SCMP("extract Alan"  , "ring\n", dest);
    TEST_PCMP("extract John"  , dest, ==, bgets(dest, sizeof dest, buf));
    TEST_SCMP("extract John"  , "John vo", dest);
    TEST_PCMP("extract John"  , dest, ==, bgets(dest, sizeof dest, buf));
    TEST_SCMP("extract John"  , "n Neuma", dest);
    TEST_PCMP("extract John"  , dest, ==, bgets(dest, sizeof dest, buf));
    TEST_SCMP("extract John"  , "nn\n", dest);
    TEST_PCMP("extract Alonzo", dest, ==, bgets(dest, sizeof dest, buf));
    TEST_SCMP("extract Alonzo", "Alonzo ", dest);
    TEST_PCMP("extract Alonzo", dest, ==, bgets(dest, sizeof dest, buf));
    TEST_SCMP("extract Alonzo", "Church\n", dest);
    TEST_PCMP("extract nothing", NULL, ==, bgets(dest, sizeof dest, buf));

    bclose(buf);

    return EXIT_SUCCESS;
}