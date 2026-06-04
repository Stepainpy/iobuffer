#include <iobuffer/iobuffer.h>
#include "test.h"

void test_pair_ptr_size(
    const char* mode,
    const char* text, size_t size,
    ptrdiff_t exp_size, ptrdiff_t exp_pos
) {
    static char tmpstr[1024];
    BUFFER* buf; BUFVIEW bvw;

    buf = bopen(text, size, mode);
    bvw = bview(buf);
    sprintf(tmpstr, "create with mode \"%s\" | not empty with ptr", mode);
    TEST_CASE(tmpstr, buf != NULL);
    TEST_CASE(tmpstr, BV_LEN(bvw, base, stop) == exp_size          );
    TEST_CASE(tmpstr, BV_LEN(bvw, base, head) ==            exp_pos);
    TEST_CASE(tmpstr, BV_LEN(bvw, head, stop) == exp_size - exp_pos);
    bclose(buf);

    buf = bopen(text, 0, mode);
    bvw = bview(buf);
    sprintf(tmpstr, "create with mode \"%s\" | empty with ptr", mode);
    TEST_CASE(tmpstr, buf != NULL);
    TEST_CASE(tmpstr, BV_LEN(bvw, base, stop) == 0);
    TEST_CASE(tmpstr, BV_LEN(bvw, base, head) == 0);
    TEST_CASE(tmpstr, BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    buf = bopen(NULL, 0, mode);
    bvw = bview(buf);
    sprintf(tmpstr, "create with mode \"%s\" | empty without ptr", mode);
    TEST_CASE(tmpstr, buf != NULL);
    TEST_CASE(tmpstr, BV_LEN(bvw, base, stop) == 0);
    TEST_CASE(tmpstr, BV_LEN(bvw, base, head) == 0);
    TEST_CASE(tmpstr, BV_LEN(bvw, head, stop) == 0);
    bclose(buf);

    buf = bopen(NULL, size, mode);
    sprintf(tmpstr, "create with mode \"%s\" | not empty without ptr", mode);
    TEST_CASE(tmpstr, buf == NULL);
}

int main(void) {
    static char fourkb[4096];
    BUFFER* buf;

    memset(fourkb, 0xC3, sizeof fourkb);

    /* Invalid mode */

    buf = bopen(NULL, 0, "m" );
    TEST_CASE("create invalid | full incorrect",     buf == NULL);
    buf = bopen(NULL, 0, "rm");
    TEST_CASE("create invalid | partly incorrect 1", buf == NULL);
    buf = bopen(NULL, 0, "w-");
    TEST_CASE("create invalid | partly incorrect 2", buf == NULL);
    buf = bopen(NULL, 0, "+a");
    TEST_CASE("create invalid | incorrect order",    buf == NULL);

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