#include <iobuffer/iobuffer.h>
#include "test.h"

void test_format(const char* fmt, int exp_count, int exp_pos, const char* input, ...) {
    BUFFER* buf; BUFVIEW bvw;
    int ret; va_list args;
    char tcname[64];

    va_start(args, input);
    buf = bopen(input, strlen(input), "r");
    sprintf(tcname, "check format string \"%s\"", fmt);

    ret = vbscanf(buf, fmt, args);
    bvw = bview(buf);
    TEST_CASE(tcname, ret == exp_count);
    TEST_CASE(tcname, BV_LEN(bvw, base, head) == exp_pos);

    bclose(buf);
    va_end(args);
}

int main(void) {
    BUFFER* buf;

    TEST_CASE("call with null pointer", bscanf(NULL, NULL) < 0);

    buf = bopen(NULL, 0, "w");
    TEST_CASE("call with not format"  , bscanf(buf, NULL) < 0);
    TEST_CASE("call with not readable", bscanf(buf, "io") < 0);
    bclose(buf);

    /* Plain text */

    test_format("", 0, 0, "");
    test_format("abc", 0, 3, "abc");
    test_format("abc", 0, 3, "abcdef");
    test_format("abcdef", 0, 3, "abcxyz");

    test_format("%%", 0, 1, "%");
    test_format("50%%\tsale", 0, 12, "50%     sale");

    /* Character specifier */

    {
    char ca[8];

    test_format("%c", 1, 1, "A", &ca);
    TEST_CASE("check char spec", ca[0] == 'A');
    test_format("%c", 1, 1, "ABC", &ca);
    TEST_CASE("check char spec", ca[0] == 'A');
    test_format("%*c", 0, 1, "ABC");

    test_format("%5c", 1, 5, "Two words", &ca);
    TEST_MCMP("check char spec", "Two wo", ca, 5);
    test_format("%*5c", 0, 5, "Two words");

    }

    /* String specifier */

    {
    char ca[16];

    test_format("%s", 1, 4, "word", &ca);
    TEST_CASE("check str spec", strcmp(ca, "word") == 0);
    test_format("%s", 1, 3, "Two words", &ca);
    TEST_CASE("check str spec", strcmp(ca, "Two") == 0);

    test_format("%*s", 0, 4, "word", &ca);
    test_format("%*s", 0, 3, "Two words", &ca);

    test_format("%3s", 1, 3, "word", &ca);
    TEST_CASE("check str spec", strcmp(ca, "wor") == 0);
    test_format("%8s", 1, 3, "Two words", &ca);
    TEST_CASE("check str spec", strcmp(ca, "Two") == 0);

    test_format("%*3s", 0, 3, "word", &ca);
    test_format("%*8s", 0, 3, "Two words", &ca);

    }

    /* Scanset specifier */

    {
    char ca[16];

    test_format("%[cab]", 1, 3, "abcdef", &ca);
    TEST_CASE("chech ss spec", strcmp(ca, "abc") == 0);
    test_format("%[^cab]", 1, 3, "234cdef", &ca);
    TEST_CASE("chech ss spec", strcmp(ca, "234") == 0);
    test_format("%[][0-9]", 1, 8, "[69][42]cdef", &ca);
    TEST_CASE("chech ss spec", strcmp(ca, "[69][42]") == 0);

    test_format("8(%*[0-9])-%*[0-9]-%*[0-9]-%*[0-9]", 0, 16, "8(800)-555-35-35");

    }

    /* Integer specifier */
    /* and pointer specifier */

    {
    int x;

    test_format("%i", 1, 6, "  +123", &x);
    TEST_CASE("check int spec", x == 123);
    test_format("%i", 1, 6, "  -123", &x);
    TEST_CASE("check int spec", x == -123);

    test_format("%i", 1, 3, "0b123", &x);
    TEST_CASE("check bin spec", x == 1);
    test_format("%i", 1, 4, "0123", &x);
    TEST_CASE("check oct spec", x == 83);
    test_format("%i", 1, 5, "0x123", &x);
    TEST_CASE("check hex spec", x == 291);

    test_format("%3i", 1, 3, "12345", &x);
    TEST_CASE("check bounds", x == 123);
    test_format("%3i", 1, 3, "0x12345", &x);
    TEST_CASE("check bounds", x == 1);
    test_format("%3i", 1, 2, "00x12345", &x);
    TEST_CASE("check bounds", x == 0);
    test_format("%3i", 1, 3, "+0x12345", &x);
    TEST_CASE("check bounds", x == 0);

    test_format("%*d", 0, 5, " -123");
    test_format("%*b", 0, 3, " +123");
    test_format("%*b", 0, 5, "0b101");
    test_format("%*o", 0, 4, " +149");
    test_format("%*x", 0, 5, " +1f9");
    test_format("%*x", 0, 5, "0x1f9");

    }

    /* Float-point number specifier */

    {
    float x;

    test_format("%g", 1, 5, "31.25", &x);
    TEST_CASE("check float spec", x == 31.25);
    test_format("%f", 1, 9, "31.250000", &x);
    TEST_CASE("check float spec", x == 31.25);
    test_format("%e", 1, 11, "3.125000e+1", &x);
    TEST_CASE("check float spec", x == 31.25);
    test_format("%a", 1, 9, "0x1.f4p+4", &x);
    TEST_CASE("check float spec", x == 31.25);

    }

    /* Number of characters read */

    {
    int n;

    test_format("%*i%n", 0, 8, "  -80085  ", &n);
    TEST_CASE("check NoCR", n == 8);

    }

    return EXIT_SUCCESS;
}