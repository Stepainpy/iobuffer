# IOBuffer

## Overview

Dynamic buffer with API like standard C files. Support standard C89 (ANSI C).

## Example

``` c
#include <stdio.h>
#include "iobuffer.h"

int main(void) {
    BUFFER* bd; int ch;
    bd = bopen("Hello, ", 7, "a+");
    if (!bd) return 1;

    bputs("Alex", bd);

    brewind(bd);
    while ((ch = bgetc(bd)) != EOB)
        putchar(ch);
    putchar('\n');

    bclose(bd);
    return 0;
}
```