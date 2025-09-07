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

## Documentation

### Table of content

- [Macro constants](#macro-constants)
  - [`EOB`](#eob)
  - [`BSEEK_SET`](#bseek_set)
  - [`BSEEK_CUR`](#bseek_cur)
  - [`BSEEK_END`](#bseek_end)
- [Types](#types)
  - [`BUFFER`](#buffer)
  - [`bpos_t`](#bpos_t)
- [Buffer access](#buffer-access)
  - [`bopen`](#buffer-bopenconst-void-restrict-data-size_t-size-const-char-restrict-mode)
  - [`bclose`](#void-bclosebuffer-buffer)
- [Operations on buffer](#operations-on-buffer)
  - [`berase`](#void-berasebuffer-buffer-size_t-count)
  - [`breset`](#void-bresetbuffer-buffer)
- [Buffer positioning](#buffer-positioning)
  - [`bgetpos`](#int-bgetposbuffer-restrict-buffer-bpos_t-restrict-pos)
  - [`bsetpos`](#int-bsetposbuffer-buffer-const-bpos_t-pos)
  - [`btell`](#long-btellbuffer-buffer)
  - [`bseek`](#int-bseekbuffer-buffer-long-offset-int-origin)
  - [`brewind`](#void-brewindbuffer-buffer)
- [Direct input/output](#direct-inputoutput)
  - [`bread`](#size_t-breadvoid-restrict-data-size_t-size-size_t-count-buffer-restrict-buffer)
  - [`bwrite`](#size_t-bwriteconst-void-restrict-data-size_t-size-size_t-count-buffer-restrict-buffer)
- [Unformatted input/output](#unformatted-inputoutput)
  - [`bgetc`](#int-bgetcbuffer-buffer)
  - [`bpeek`](#int-bpeekbuffer-buffer)
  - [`bgets`](#char-bgetschar-restrict-str-int-count-buffer-restrict-buffer)
  - [`bputc`](#int-bputcint-byte-buffer-buffer)
  - [`bputs`](#int-bputsconst-char-restrict-string-buffer-restrict-buffer)
  - [`bungetc`](#int-bungetcint-byte-buffer-buffer)
- [Formatted ~~input~~/output](#formatted-inputoutput)
  - [`bprintf`](#int-bprintfbuffer-restrict-buffer-const-char-restrict-format-)
  - [`vbprintf`](#int-vbprintfbuffer-restrict-buffer-const-char-restrict-format-va_list-list)
- [Error handling](#error-handling)
  - [`beob`](#int-beobbuffer-buffer)
- [View extension](#view-extension)
  - [`BUFVIEW`](#bufview)
  - [`BV_FMT`](#bv_fmt)
  - [`BV_ARG`](#bv_argview-from)
  - [`bview`](#bufview-bviewbuffer-buffer)

## Macro constants

### `EOB`
Integer constant expression of type `int` and negative value.

### `BSEEK_SET`
Argument to `bseek` indicating seeking from beginning of the buffer.

### `BSEEK_CUR`
Argument to `bseek` indicating seeking from the current buffer position.

### `BSEEK_END`
Argument to `bseek` indicating seeking from end of the buffer.

## Types

### `BUFFER`
Object type, capable of holding all information needed to control a buffer.

### `bpos_t`
Non-array complete object type, capable of uniquely specifying a position in buffer.

## Buffer access

### `BUFFER* bopen(const void* restrict data, size_t size, const char* restrict mode)`

Opens a buffer from `data`/`size` and returns a pointer to the buffer. `mode` is used to determine the buffer access mode.  
**Return value**: If successful, returns a pointer to the new buffer. On error, returns a null pointer.

| Mode string | Meaning         | Explanation                    | `data != NULL`<br>`size > 0` | `data == NULL`<br>`size == 0` |
| :---------: | :-------------- | :----------------------------- | :--------------------------- | :---------------------------- |
|    `"r"`    | read            | open a buffer for reading      | read from start              | failure to open               |
|    `"w"`    | write           | create a buffer for writing    | start with empty             | start with empty              |
|    `"a"`    | append          | append to a buffer             | write to end                 | start with empty              |
|    `"r+"`   | read extended   | open a buffer for read/write   | read from start              | start with empty              |
|    `"w+"`   | write extended  | create a buffer for read/write | start with empty             | start with empty              |
|    `"a+"`   | append extended | open a buffer for read/write   | start to end                 | start with empty              |


### `void bclose(BUFFER* buffer)`

Closes the given buffer.  
**Return value**: *none*

## Operations on buffer

### `void berase(BUFFER* buffer, size_t count)`

**[ EXTENSION ]** Erase `count` bytes starting from the current position.  
**Return value**: *none*

### `void breset(BUFFER* buffer)`

**[ EXTENSION ]** Deleting all data in buffer.  
**Return value**: *none*

## Buffer positioning

### `int bgetpos(BUFFER* restrict buffer, bpos_t* restrict pos)`

Obtains the buffer position indicator for the `buffer` and stores them in the object pointed to by `pos`. The value stored is only meaningful as the input to `bsetpos`.  
**Return value**: `0` upon success, nonzero value otherwise.

### `int bsetpos(BUFFER* buffer, const bpos_t* pos)`

Sets the buffer position indicator for the `buffer` according to the value pointed to by `pos`.  
**Return value**: `0` upon success, nonzero value otherwise.

### `long btell(BUFFER* buffer)`

Returns the buffer position indicator for the `buffer`.  
**Return value**: Buffer position indicator on success or `-1L` if failure occurs.

### `int bseek(BUFFER* buffer, long offset, int origin)`

Sets the buffer position indicator for the `buffer` to the value pointed to by `offset`.  
**Return value**: `0` upon success, nonzero value otherwise.

### `void brewind(BUFFER* buffer)`

Moves the buffer position indicator to the beginning of the given buffer.  
**Return value**: *none*

## Direct input/output

### `size_t bread(void* restrict data, size_t size, size_t count, BUFFER* restrict buffer)`

Reads up to `count` objects into the array `data` from the given input buffer `buffer` and storing the results, in the order obtained, into the successive positions of buffer, which is reinterpreted as an array of `unsigned char`. The buffer position indicator is advanced by the number of characters read.  
**Return value**: Number of objects read successfully, which may be less than `count` if an error or end-of-buffer condition occurs.

### `size_t bwrite(const void* restrict data, size_t size, size_t count, BUFFER* restrict buffer)`

Writes `count` of objects from the given array `data` to the output buffer `buffer`. The objects are written as if by reinterpreting each object as an array of `unsigned char` to write those `unsigned char`s into buffer, in order. The buffer position indicator is advanced by the number of characters written.  
**Return value**: The number of objects written successfully, which may be less than `count` if an error occurs.

## Unformatted input/output

### `int bgetc(BUFFER* buffer)`

Reads the next character from the given input buffer.  
**Return value**: On success, returns the obtained character as an `unsigned char` converted to an `int`. On failure, returns `EOB`.

### `int bpeek(BUFFER* buffer)`

**[ EXTENSION ]** Peek the next character from the given input buffer.  
**Return value**: On success, returns the obtained character as an `unsigned char` converted to an `int`. On failure, returns `EOB`.

### `char* bgets(char* restrict str, int count, BUFFER* restrict buffer)`

Reads at most `count - 1` characters from the given buffer and stores them in the character array pointed to by `str`. Parsing stops if a newline character is found (in which case `str` will contain that newline character) or if end-of-buffer occurs. If bytes are read and no errors occur, writes a null character at the position immediately after the last character written to `str`.  
**Return value**: `str` on success, null pointer on failure.

### `int bputc(int byte, BUFFER* buffer)`

Writes a byte `byte` to the given output buffer `buffer`. Internally, the byte is converted to `unsigned char` just before being written.  
**Return value**: On success, returns the written character. On failure, returns `EOB`.

### `int bputs(const char* restrict string, BUFFER* restrict buffer)`

Writes every character from the null-terminated string `string` to the output buffer `buffer`, as if by repeatedly executing `bputc`. The terminating null character from `string` is not written.  
**Return value**: On success, returns a non-negative value. On failure, returns `EOB`.

### `int bungetc(int byte, BUFFER* buffer)`

If `byte` does not equal `EOB`, pushes the byte `byte` (reinterpreted as `unsigned char`) into the buffer `buffer` in such a manner that subsequent read operation from buffer will retrieve that byte.  
**Return value**: On success `byte` is returned. On failure `EOB` is returned and the given buffer remains unchanged.

## Formatted ~~input~~/output

> [!NOTE]
> Why doesn't exist `bscanf`/`vbscanf`? Standard `scanf` return count success parsed values.  
> Not exist common way get read it characters after `scanf`. I don't want to completely create `bscanf`/`vbscanf`.

> [!WARNING]
> In `bprintf`/`vbprintf` use funtion `snprintf`. This function "offical" available in C99, but most compilators
> provide his in C89.

### `int bprintf(BUFFER* restrict buffer, const char* restrict format, ...)`

Loads the data from the given locations, converts them to character string equivalents and writes the results to buffer `buffer`.  
**Return value**: The number of characters written if successful or negative value if an error occurred.

### `int vbprintf(BUFFER* restrict buffer, const char* restrict format, va_list list)`

Loads the data from the locations, defined by `list`, converts them to character string equivalents and writes the results to buffer `buffer`.  
**Return value**: The number of characters written if successful or negative value if an error occurred.

## Error handling

### `int beob(BUFFER* buffer)`

Checks if the end of the given buffer has been reached.  
**Return value**: Nonzero value if the end of the buffer has been reached, otherwise ​`0`.​

## View extension

### `BUFVIEW`
Complete object type with fields `base`, `head`, `stop` with type `const void*`.

### `BV_FMT`
Macro-constant string literal of format specifier for `BUFVIEW`.

### `BV_ARG(view, from)`
Macro-function for create arguments to `printf`'s functions. Valid value of `from`: `base`, `head`.

### `BUFVIEW bview(BUFFER* buffer)`

Create new view object from buffer `buffer`.  
**Return value**: New view object.