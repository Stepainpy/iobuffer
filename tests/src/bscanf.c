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
    TEST_CASE("check char spec", memcmp(ca, "Two wo", 5) == 0);
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

    return EXIT_SUCCESS;
}