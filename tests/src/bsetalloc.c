#include <iobuffer/iobuffer.h>
#include "test.h"

static void* empty_alloc(void* ptr, size_t size, void* ud) {
    (void)ptr; (void)size; (void)ud;
    return NULL;
}

int main(int argc, char** argv) {
    (void)argc;

    TEST_ICMP("set default allocator"         , 0, ==, bsetalloc(NULL       , NULL));
    TEST_ICMP("set allocator without userdata", 0, ==, bsetalloc(empty_alloc, NULL));
    TEST_ICMP("set allocator with userdata"   , 0, ==, bsetalloc(empty_alloc, argv));
    TEST_ICMP("set incorrect allocator"       , 0, !=, bsetalloc(NULL       , argv));

    return EXIT_SUCCESS;
}