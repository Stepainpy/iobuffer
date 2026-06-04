#include <iobuffer/iobuffer.h>
#include "test.h"

static void* empty_alloc(void* ptr, size_t size, void* ud) {
    (void)ptr; (void)size; (void)ud;
    return NULL;
}

int main(int argc, char** argv) {
    (void)argc;

    TEST_CASE("set default allocator"         , bsetalloc(NULL       , NULL) == 0);
    TEST_CASE("set allocator without userdata", bsetalloc(empty_alloc, NULL) == 0);
    TEST_CASE("set allocator with userdata"   , bsetalloc(empty_alloc, argv) == 0);
    TEST_CASE("set incorrect allocator"       , bsetalloc(NULL       , argv) != 0);

    return EXIT_SUCCESS;
}