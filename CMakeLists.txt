cmake_minimum_required(VERSION 3.21)
project(iobuffer
    VERSION      2.3.0
    DESCRIPTION "Dynamic memory buffer with API as C standard FILE"
    HOMEPAGE_URL https://github.com/Stepainpy/iobuffer
    LANGUAGES    C
)

add_library(iobuffer STATIC)
add_library(iobuffer::iobuffer ALIAS iobuffer)

set(HEADERS include/iobuffer/iobuffer.h)
set(SOURCES
    src/bidefine.h
    src/iobuffer.c
    src/vbiscanf.c
    src/vbiprintf.c
)

target_sources(iobuffer PRIVATE ${SOURCES})

set_target_properties(iobuffer PROPERTIES
    C_STANDARD          90
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS        OFF
)
set_target_properties(iobuffer PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
    SOVERSION     ${PROJECT_VERSION_MAJOR}
      VERSION     ${PROJECT_VERSION}
)

target_include_directories(iobuffer
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

if(MSVC)
    target_compile_options(iobuffer PUBLIC /O2 /W4)
else()
    target_compile_options(iobuffer PUBLIC -O2 -Wall -Wextra -Wpedantic)
endif()

if(PROJECT_IS_TOP_LEVEL)
    set_target_properties(iobuffer PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()
