cmake_minimum_required(VERSION 3.30)
project(iobuffer
    VERSION 2.1.3
    DESCRIPTION "Dynamic memory buffer with API as C standard FILE"
    HOMEPAGE_URL https://github.com/Stepainpy/iobuffer
    LANGUAGES C
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

set(CMAKE_C_STANDARD 90) # Also known as C89
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/O2 /W4)
else()
    add_compile_options(-O2 -Wall -Wextra -Wpedantic)
endif()

add_library(${PROJECT_NAME} STATIC "${CMAKE_SOURCE_DIR}/iobuffer.c")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})