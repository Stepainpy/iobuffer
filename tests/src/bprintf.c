#include <iobuffer/iobuffer.h>
#include "test.h"

void test_format(const char* fmt, const char* exp_out, ...) {
    BUFFER* buf; BUFVIEW bvw;
    int ret; va_list args;

    char tcname[64];
    int exp_out_len = strlen(exp_out);

    va_start(args, exp_out);
    buf = bopen(NULL, 0, "w");
    sprintf(tcname, "check format string \"%s\"", fmt);

    ret = vbprintf(buf, fmt, args);
    bvw = bview(buf);
    TEST_CASE(tcname, ret == exp_out_len);
    TEST_CASE(tcname, memcmp(bvw.base, exp_out, ret) == 0);

    bclose(buf);
    va_end(args);
}

int main(void) {
    BUFFER* buf;

    TEST_CASE("call with null pointer", bprintf(NULL, NULL) < 0);

    buf = bopen("Text", 4, "r");
    TEST_CASE("call with not format"  , bprintf(buf, NULL) < 0);
    TEST_CASE("call with not writable", bprintf(buf, "io") < 0);
    bclose(buf);

    /* Plain text */

    test_format("", "");
    test_format("abc", "abc");
    test_format("%%", "%");
    test_format("50%% of sale", "50% of sale");
    test_format("%i%% of sale", "50% of sale", 50);

    /* Character specifier */

    test_format("%lc", "");
    test_format("%hhc", "");

    test_format("%c", "A", 'A');
    test_format("%c", "\1", 1);
    test_format("%c", "\xff", -1);
    test_format("%c", "\xff", 255);
    test_format("%c", "\xff", 511);

    test_format("% 1c", "A", 'A');
    test_format("%-1c", "A", 'A');
    test_format("% 5c", "    A", 'A');
    test_format("%-5c", "A    ", 'A');
    test_format("%*c", "    A",  5, 'A');
    test_format("%*c", "A    ", -5, 'A');

    /* String specifier */

    test_format("%ls", "");
    test_format("%hhs", "");

    test_format("%s", "", NULL);
    test_format("%s", "", "");
    test_format("%s", "abc", "abc");
    test_format("%s-%s", "Tio-pan", "Tio", "pan");

    test_format("% 10s", "     hello", "hello");
    test_format("%-10s", "hello     ", "hello");
    test_format("%*s", "     hello",  10, "hello");
    test_format("%*s", "hello     ", -10, "hello");

    test_format("%.6s", "hello", "hello");
    test_format("%.4s", "hell" , "hello");
    test_format("%.1s", "h"    , "hello");
    test_format("%.0s", ""     , "hello");
    test_format("%.s" , ""     , "hello");
    test_format("%.*s", "hello", 6, "hello");
    test_format("%.*s", "hell" , 4, "hello");
    test_format("%.*s", "h"    , 1, "hello");
    test_format("%.*s", ""     , 0, "hello");

    test_format("% 6.4s", "  hell", "hello");
    test_format("%-6.4s", "hell  ", "hello");

    return EXIT_SUCCESS;
}