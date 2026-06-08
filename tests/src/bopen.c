#include <iobuffer/iobuffer.h>
#include "test.h"

void test_pair_ptr_size(
    const char* mode,
    const char* text, size_t size,
    ptrdiff_t exp_size, ptrdiff_t exp_pos
) {
    static char tmpstr[64];
    BUFFER* buf; BUFVIEW bvw;

    buf = bopen(text, size, mode);
    bvw = bview(buf);
    sprintf(tmpstr, "create with mode \"%s\" | not empty with ptr", mode);
    TEST_PCMP(tmpstr, NULL, !=, buf);
    TEST_ICMP(tmpstr, exp_size, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP(tmpstr, exp_pos , ==, BV_LEN(bvw, base, head));
    bclose(buf);

    buf = bopen(text, 0, mode);
    bvw = bview(buf);
    sprintf(tmpstr, "create with mode \"%s\" | empty with ptr", mode);
    TEST_PCMP(tmpstr, NULL, !=, buf);
    TEST_ICMP(tmpstr, 0, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP(tmpstr, 0, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    buf = bopen(NULL, 0, mode);
    bvw = bview(buf);
    sprintf(tmpstr, "create with mode \"%s\" | empty without ptr", mode);
    TEST_PCMP(tmpstr, NULL, !=, buf);
    TEST_ICMP(tmpstr, 0, ==, BV_LEN(bvw, base, stop));
    TEST_ICMP(tmpstr, 0, ==, BV_LEN(bvw, base, head));
    bclose(buf);

    buf = bopen(NULL, size, mode);
    sprintf(tmpstr, "create with mode \"%s\" | not empty without ptr", mode);
    TEST_PCMP(tmpstr, NULL, ==, buf);
}

int main(void) {
    static char fourkb[4096];
    BUFFER* buf;

    memset(fourkb, 0xC3, sizeof fourkb);

    /* Invalid mode */

    buf = bopen(NULL, 0, "m" );
    TEST_PCMP("create invalid | full incorrect"    , NULL, ==, buf);
    buf = bopen(NULL, 0, "rm");
    TEST_PCMP("create invalid | partly incorrect 1", NULL, ==, buf);
    buf = bopen(NULL, 0, "w-");
    TEST_PCMP("create invalid | partly incorrect 2", NULL, ==, buf);
    buf = bopen(NULL, 0, "+a");
    TEST_PCMP("create invalid | incorrect order"   , NULL, ==, buf);

    /* Read mode */

    test_pair_ptr_size("r" , "short", 5, 5, 0);
    test_pair_ptr_size("r+", "short", 5, 5, 0);

    test_pair_ptr_size("r" , fourkb, sizeof fourkb, sizeof fourkb, 0);
    test_pair_ptr_size("r+", fourkb, sizeof fourkb, sizeof fourkb, 0);

    /* Write mode */

    test_pair_ptr_size("w" , "short", 5, 0, 0);
    test_pair_ptr_size("w+", "short", 5, 0, 0);

    test_pair_ptr_size("w" , fourkb, sizeof fourkb, 0, 0);
    test_pair_ptr_size("w+", fourkb, sizeof fourkb, 0, 0);

    /* Append mode */

    test_pair_ptr_size("a" , "short", 5, 5, 5);
    test_pair_ptr_size("a+", "short", 5, 5, 5);

    test_pair_ptr_size("a" , fourkb, sizeof fourkb, sizeof fourkb, sizeof fourkb);
    test_pair_ptr_size("a+", fourkb, sizeof fourkb, sizeof fourkb, sizeof fourkb);

    return EXIT_SUCCESS;
}