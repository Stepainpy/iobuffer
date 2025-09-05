# IOBuffer

## Overview

Dynamic buffer with API like standard C files. Support standard C89 (ANSI C).

## Example

``` c
#include <stdio.h>
#include "iobuffer.h"

int main(void) {
    BUFVIEW bv;
    BUFFER* bd = bopen();
    if (!bd) return 1;

    bprintf(bd, "Hello, %s!", "Alex");

    bv = bview(bd);
    printf(BV_FMT"\n", BV_ARG(bv, base));

    bclose(bd);
    return 0;
}
```