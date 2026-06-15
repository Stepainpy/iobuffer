#include <iobuffer/iobuffer.h>
#include "test.h"

#include <math.h>
#include <limits.h>
#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>
#endif

double getinf(void) {
    union { float f; unsigned u; unsigned long ul; } u;
#if ULONG_MAX == 0xFFFFFFFFul
    u.ul = 0x7f800000ul;
#else
    u.u  = 0x7f800000u;
#endif
    return (double)u.f;
}

double getnan(void) {
    union { float f; unsigned u; unsigned long ul; } u;
#if ULONG_MAX == 0xFFFFFFFFul
    u.ul = 0x7fc00000ul;
#else
    u.u  = 0x7fc00000u;
#endif
    return (double)u.f;
}

#define test_format(...) test_format(__LINE__, __VA_ARGS__)

void (test_format)(int line, const char* fmt, const char* exp_out, ...) {
    BUFFER* buf; BUFVIEW bvw;
    int ret; va_list args;

    char tcname[64];
    int exp_out_len = strlen(exp_out);

    va_start(args, exp_out);
    buf = bopen(NULL, 0, "w");
    sprintf(tcname, "check format string \"%s\" on line %i", fmt, line);

    ret = vbprintf(buf, fmt, args);
    bvw = bview(buf);
    TEST_ICMP(tcname, exp_out_len, ==, ret);
    TEST_MCMP(tcname, exp_out, bvw.base, ret);

    bclose(buf);
    va_end(args);
}

int main(void) {
    BUFFER* buf;

    TEST_ICMP("call with null pointer", 0, >, bprintf(NULL, NULL));

    buf = bopen("Text", 4, "r");
    TEST_ICMP("call with not format"  , 0, >, bprintf(buf, NULL));
    TEST_ICMP("call with not writable", 0, >, bprintf(buf, "io"));
    bclose(buf);

    /* Plain text */

    test_format("", "");
    test_format("abc", "abc");
    test_format("%%", "%");
    test_format("%m", "");
    test_format("%10.2m", "");
    test_format("%2.jm", "");
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

    /* Signed integer specifier */

    test_format("%Ld", "");
    test_format("%Li", "");

    test_format("%i", "1", 1);
    test_format("%i", "98765432", 12345679 * 8);
    test_format("%i", "-5", -5);
    test_format("%i", "-13", 27 - 40);

    test_format("%i", "1", 1);
    test_format("%i", "-1", -1);
    test_format("% i", " 1", 1);
    test_format("% i", "-1", -1);
    test_format("%+i", "+1", 1);
    test_format("%+i", "-1", -1);

    test_format("%.i", "15", 15);
    test_format("%.i", "", 0);

    test_format("%0i", "15", 15);
    test_format("%05i", "00015", 15);
    test_format("%-05i", "15   ", 15);
    test_format("%05.3i", "  015", 15);
    test_format("%-05.3i", "015  ", 15);

#if IOBT_FMT_DI_ENABLE_LIMITS
    test_format("%+hhi", "+127", SCHAR_MAX);
    test_format("%+hhi", "-128", SCHAR_MIN);

    test_format("%+hi", "+32767", SHRT_MAX);
    test_format("%+hi", "-32768", SHRT_MIN);

    test_format("%+i", "+2147483647", INT_MAX);
    test_format("%+i", "-2147483648", INT_MIN);

#if LONG_MAX > INT_MAX
    test_format("%+li", "+9223372036854775807", LONG_MAX);
    test_format("%+li", "-9223372036854775808", LONG_MIN);
#else
    test_format("%+li", "+2147483647", LONG_MAX);
    test_format("%+li", "-2147483648", LONG_MIN);
#endif

#if __STDC_VERSION__ >= 199901L
    test_format("%+lli", "+9223372036854775807", LLONG_MAX);
    test_format("%+lli", "-9223372036854775808", LLONG_MIN);

    test_format("%+ji", "+9223372036854775807", INTMAX_MAX);
    test_format("%+ji", "-9223372036854775808", INTMAX_MIN);

    test_format("%+ti", "+9223372036854775807", PTRDIFF_MAX);
    test_format("%+ti", "-9223372036854775808", PTRDIFF_MIN);

    test_format("%+zi", "-1", SIZE_MAX);
    test_format("%+zi", "+0", (size_t)0);
#endif
#endif /* IOBT_FMT_DI_ENABLE_LIMITS */

    /* Unsigned integer specifier */
    /* and pointer specifier (implicit) */

    test_format("%Lb", "");
    test_format("%LB", "");
    test_format("%Lo", "");
    test_format("%Lu", "");
    test_format("%Lx", "");
    test_format("%LX", "");

    test_format("%u", "1", 1);
    test_format("%u", "98765432", 12345679 * 8);
    test_format("%b", "101010", 42);
    test_format("%o", "52", 42);
    test_format("%x", "2a", 42);
    test_format("%X", "2A", 42);

    test_format("%#b", "0b101010", 42);
    test_format("%#B", "0B101010", 42);
    test_format("%#o", "052", 42);
    test_format("%#u", "42", 42);
    test_format("%#x", "0x2a", 42);
    test_format("%#X", "0X2A", 42);

    test_format("%.b", "1111", 15);
    test_format("%.b", "", 0);
    test_format("%.o", "17", 15);
    test_format("%.o", "", 0);
    test_format("%.u", "15", 15);
    test_format("%.u", "", 0);
    test_format("%.x", "f", 15);
    test_format("%.x", "", 0);

    test_format("%06b", "001111", 15);
    test_format("%-06b", "1111  ", 15);
    test_format("%06.3b", "  1111", 15);
    test_format("%-06.3b", "1111  ", 15);
    test_format("%06o", "000017", 15);
    test_format("%-06o", "17    ", 15);
    test_format("%06.3o", "   017", 15);
    test_format("%-06.3o", "017   ", 15);
    test_format("%06u", "000015", 15);
    test_format("%-06u", "15    ", 15);
    test_format("%06.3u", "   015", 15);
    test_format("%-06.3u", "015   ", 15);
    test_format("%06x", "00000f", 15);
    test_format("%-06x", "f     ", 15);
    test_format("%06.3x", "   00f", 15);
    test_format("%-06.3x", "00f   ", 15);

    test_format("%#010b", "0b00001111", 15);
    test_format("%#10.5b", "   0b01111", 15);

    test_format("%#06o", "000017", 15);
    test_format("%#6o", "   017", 15);

    test_format("%#06x", "0x000f", 15);
    test_format("%#6x", "   0xf", 15);

#if IOBT_FMT_BOUX_ENABLE_LIMITS
    test_format("%hhu", "255", UCHAR_MAX);
    test_format("%hu", "65535", USHRT_MAX);
    test_format("%u", "4294967295", UINT_MAX);

#if LONG_MAX > INT_MAX
    test_format("%lu", "18446744073709551615", ULONG_MAX);
#else
    test_format("%lu", "4294967295", ULONG_MAX);
#endif

#if __STDC_VERSION__ >= 199901L
    test_format("%llu", "18446744073709551615", ULLONG_MAX);
    test_format("%ju", "18446744073709551615", UINTMAX_MAX);
    test_format("%tu", "9223372036854775807", PTRDIFF_MAX);
    test_format("%zu", "18446744073709551615", SIZE_MAX);
#endif
#endif /* IOBT_FMT_BOUX_ENABLE_LIMITS */

    /* Float-point number specifier */

    test_format("%hf", "");
    test_format("%je", "");
    test_format("%zg", "");
    test_format("%ta", "");

    test_format("%f", "31.250000", 31.25);
    test_format("%e", "3.125000e+01", 31.25);
    test_format("%g", "31.25", 31.25);
    test_format("%a", "0x1.f4p+4", 31.25);

    test_format("%10.3f", "    31.250", 31.25);
    test_format("%-10.3f", "31.250    ", 31.25);
    test_format("%.0f", "31", 31.25);
    test_format("%.f", "31", 31.25);
    test_format("%#.0f", "31.", 31.25);
    test_format("%#.f", "31.", 31.25);

    test_format("%10.3e", " 3.125e+01", 31.25);
    test_format("%-10.3e", "3.125e+01 ", 31.25);
    test_format("%.0e", "3e+01", 31.25);
    test_format("%.e", "3e+01", 31.25);
    test_format("%#.0e", "3.e+01", 31.25);
    test_format("%#.e", "3.e+01", 31.25);

    test_format("%15.5a", "   0x1.f4000p+4", 31.25);
    test_format("%-10.3a", "0x1.f40p+4", 31.25);
    test_format("%.0a", "0x2p+4", 31.25);
    test_format("%.a", "0x2p+4", 31.25);
    test_format("%#.0a", "0x2.p+4", 31.25);
    test_format("%#.a", "0x2.p+4", 31.25);

    test_format("%f", "inf", getinf());
    test_format("%F", "INF", getinf());
    test_format("%e", "inf", getinf());
    test_format("%E", "INF", getinf());
    test_format("%g", "inf", getinf());
    test_format("%G", "INF", getinf());
    test_format("%a", "inf", getinf());
    test_format("%A", "INF", getinf());

    test_format("%f", "nan", getnan());
    test_format("%F", "NAN", getnan());
    test_format("%e", "nan", getnan());
    test_format("%E", "NAN", getnan());
    test_format("%g", "nan", getnan());
    test_format("%G", "NAN", getnan());
    test_format("%a", "nan", getnan());
    test_format("%A", "NAN", getnan());

    test_format("%g" ,  "1.5",  1.5);
    test_format("%g" , "-1.5", -1.5);
    test_format("% g", " 1.5",  1.5);
    test_format("% g", "-1.5", -1.5);
    test_format("%+g", "+1.5",  1.5);
    test_format("%+g", "-1.5", -1.5);

    test_format("%f" ,  "inf",  getinf());
    test_format("%f" , "-inf", -getinf());
    test_format("% f", " inf",  getinf());
    test_format("% f", "-inf", -getinf());
    test_format("%+f", "+inf",  getinf());
    test_format("%+f", "-inf", -getinf());

    test_format("%f" ,  "nan",  getnan());
    test_format("%f" , "-nan", -getnan());
    test_format("% f", " nan",  getnan());
    test_format("% f", "-nan", -getnan());
    test_format("%+f", "+nan",  getnan());
    test_format("%+f", "-nan", -getnan());

    test_format("%e", "3.125000e+01", 31.25);
    test_format("%E", "3.125000E+01", 31.25);
    test_format("%a", "0x1.f4p+4", 31.25);
    test_format("%A", "0X1.F4P+4", 31.25);

    /* Number of written characters specifier */

    {
    int x, y;

    test_format("abc%ndef%n", "abcdef", &x, &y);
    TEST_ICMP("check NoWC", 3, ==, x);
    TEST_ICMP("check NoWC", 6, ==, y);

    test_format("%n%s%n", "abcdef", &x, "abcdef", &y);
    TEST_ICMP("check NoWC", 0, ==, x);
    TEST_ICMP("check NoWC", 6, ==, y);

    }

    return EXIT_SUCCESS;
}