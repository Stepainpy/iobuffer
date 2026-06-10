#include <iobuffer/iobuffer.h>
#include "test.h"

#include <float.h>

void test_format(const char* fmt, int exp_count, int exp_pos, const char* input, ...) {
    BUFFER* buf; BUFVIEW bvw;
    int ret; va_list args;
    char tcname[64];

    va_start(args, input);
    buf = bopen(input, strlen(input), "r");
    sprintf(tcname, "check format string \"%s\"", fmt);

    ret = vbscanf(buf, fmt, args);
    bvw = bview(buf);
    TEST_ICMP(tcname, exp_count, ==, ret);
    TEST_ICMP(tcname, exp_pos, ==, BV_LEN(bvw, base, head));

    bclose(buf);
    va_end(args);
}

int main(void) {
    BUFFER* buf;

    TEST_ICMP("call with null pointer", 0, >, bscanf(NULL, NULL));

    buf = bopen(NULL, 0, "w");
    TEST_ICMP("call with not format"  , 0, >, bscanf(buf, NULL));
    TEST_ICMP("call with not readable", 0, >, bscanf(buf, "io"));
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
    TEST_ICMP("check char spec", 'A', ==, ca[0]);
    test_format("%c", 1, 1, "ABC", &ca);
    TEST_ICMP("check char spec", 'A', ==, ca[0]);
    test_format("%*c", 0, 1, "ABC");

    test_format("%5c", 1, 5, "Two words", &ca);
    TEST_MCMP("check char spec", "Two wo", ca, 5);
    test_format("%*5c", 0, 5, "Two words");

    }

    /* String specifier */

    {
    char ca[16];

    test_format("%s", 1, 4, "word", &ca);
    TEST_SCMP("check str spec", "word", ca);
    test_format("%s", 1, 3, "Two words", &ca);
    TEST_SCMP("check str spec", "Two", ca);

    test_format("%*s", 0, 4, "word", &ca);
    test_format("%*s", 0, 3, "Two words", &ca);

    test_format("%3s", 1, 3, "word", &ca);
    TEST_SCMP("check str spec", "wor", ca);
    test_format("%8s", 1, 3, "Two words", &ca);
    TEST_SCMP("check str spec", "Two", ca);

    test_format("%*3s", 0, 3, "word", &ca);
    test_format("%*8s", 0, 3, "Two words", &ca);

    }

    /* Scanset specifier */

    {
    char ca[16];

    test_format("%[cab]", 1, 3, "abcdef", &ca);
    TEST_SCMP("chech ss spec", "abc", ca);
    test_format("%[^cab]", 1, 3, "234cdef", &ca);
    TEST_SCMP("chech ss spec", "234", ca);
    test_format("%[][0-9]", 1, 8, "[69][42]cdef", &ca);
    TEST_SCMP("chech ss spec", "[69][42]", ca);

    test_format("8(%*[0-9])-%*[0-9]-%*[0-9]-%*[0-9]", 0, 16, "8(800)-555-35-35");

    }

    /* Integer specifier */
    /* and pointer specifier */

    {
    int x;

    test_format("%i", 1, 6, "  +123", &x);
    TEST_ICMP("check int spec", 123, ==, x);
    test_format("%i", 1, 6, "  -123", &x);
    TEST_ICMP("check int spec", -123, ==, x);

    test_format("%i", 1, 3, "0b123", &x);
    TEST_ICMP("check bin spec", 1, ==, x);
    test_format("%i", 1, 4, "0123", &x);
    TEST_ICMP("check oct spec", 83, ==, x);
    test_format("%i", 1, 5, "0x123", &x);
    TEST_ICMP("check hex spec", 291, ==, x);

    test_format("%3i", 1, 3, "12345", &x);
    TEST_ICMP("check bounds", 123, ==, x);
    test_format("%3i", 1, 3, "0x12345", &x);
    TEST_ICMP("check bounds", 1, ==, x);
    test_format("%3i", 1, 2, "00x12345", &x);
    TEST_ICMP("check bounds", 0, ==, x);
    test_format("%3i", 1, 3, "+0x12345", &x);
    TEST_ICMP("check bounds", 0, ==, x);

    test_format("%*d", 0, 5, " -123");
    test_format("%*b", 0, 3, " +123");
    test_format("%*b", 0, 5, "0b101");
    test_format("%*o", 0, 4, " +149");
    test_format("%*x", 0, 5, " +1f9");
    test_format("%*x", 0, 5, "0x1f9");

    }

    /* Float-point number specifier */

    {
    /* expect IEEE 754, float and uint is 32-bit */
    union { float x; unsigned n; } u;

    test_format("%g", 1, 5, "31.25", &u.x);
    TEST_VCMP("check float spec", 31.25, ==, u.x, double, "%g");
    test_format("%f", 1, 9, "31.250000", &u.x);
    TEST_VCMP("check float spec", 31.25, ==, u.x, double, "%g");
    test_format("%e", 1, 11, "3.125000e+1", &u.x);
    TEST_VCMP("check float spec", 31.25, ==, u.x, double, "%g");
    test_format("%a", 1, 9, "0x1.f4p+4", &u.x);
    TEST_VCMP("check float spec", 31.25, ==, u.x, double, "%g");

    test_format("%f", 1, 4, "+nan", &u.x);
    TEST_VCMP("check float nan", (u.x != u.x)   , ==, 1, int, "%i");
    TEST_VCMP("check float nan", (!!(u.n >> 31)), ==, 0, int, "%i");
    test_format("%f", 1, 4, "-nan", &u.x);
    TEST_VCMP("check float nan", (u.x != u.x)   , ==, 1, int, "%i");
    TEST_VCMP("check float nan", (!!(u.n >> 31)), ==, 1, int, "%i");

    test_format("%f", 1, 4, "+inf", &u.x);
    TEST_VCMP("check float inf", (u.x < -FLT_MAX || FLT_MAX < u.x), ==, 1, int, "%i");
    TEST_VCMP("check float inf", u.x, >, 0, double, "%g");
    test_format("%f", 1, 4, "-inf", &u.x);
    TEST_VCMP("check float inf", (u.x < -FLT_MAX || FLT_MAX < u.x), ==, 1, int, "%i");
    TEST_VCMP("check float inf", u.x, <, 0, double, "%g");

    }

    /* Number of characters read */

    {
    int n;

    test_format("%*i%n", 0, 8, "  -80085  ", &n);
    TEST_ICMP("check NoCR", 8, ==, n);

    }

    return EXIT_SUCCESS;
}