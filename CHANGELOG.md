# Changelog

## 2.2.0 - 2026-05-22

### Added

- Full implementation of `bprintf`/`vbprintf` functions
- `bidefine.h` and `vbiprintf.c` files

### Removed

- Dependence on `vsnprinf` function

## 2.1.3 - 2026-04-28

### Added

- CMake for compilation of static library

## 2.1.2 - 2026-03-04

### Fixed

- Growth of buffer capacity

## 2.1.1 - 2026-01-28

### Added

- More complex example in `README.md` file

## 2.1.0 - 2026-01-15

### Added

- Version macro-constants to `iobuffer.h` file

## 2.0.0 - 2026-01-11

### Changed

- User-data pointer place in custom allocator
- API of buffer view macro-functions

### Added

- Returns error code in `bclose`, `berase` and `breset` functions

## 1.4.0 - 2026-01-10

### Added

- Macro for function declaration

## 1.3.2 - 2026-01-10

### Changed

- Growth of buffer capacity

## 1.3.1 - 2026-01-09

### Changed

- Behaviour of `bsetalloc` function
- Mode table of `bopen` function in documentation

## 1.3.0 - 2025-12-30

### Added

- Custom allocator for buffer

## 1.2.0 - 2025-09-18

### Changed

- Behaviour of `bwrite` function

### Added

- `bmemopen` function with documentation

## 1.1.1 - 2025-09-27

### Added

- `BV_SIZE` macro-function

## 1.1.0 - 2025-09-07

### Changed

- Semantic of `bopen` function
- Note about `bscanf`/`vbscanf` functions
- Example in `README.md` file

### Added

- Read and write permission for buffer
- Documentation in `README.md` file

### Removed

- Documentation comments from code

## 1.0.0 - 2025-09-06

*First standalone version from code use only for yourself.*
