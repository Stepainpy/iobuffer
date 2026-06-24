cmake_minimum_required(VERSION 3.21)
project(iobuffer
    VERSION      3.0.3
    DESCRIPTION "Dynamic memory buffer with API as C standard FILE"
    HOMEPAGE_URL https://github.com/Stepainpy/iobuffer
    LANGUAGES    C
)

## External settings

option(IOBUFFER_BUILD_TESTS "Build test targets"    ${PROJECT_IS_TOP_LEVEL})
option(IOBUFFER_INSTALL     "Create install target" ${PROJECT_IS_TOP_LEVEL})

if(DEFINED IOBUFFER_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${IOBUFFER_SHARED_LIBS})
endif()

## Create library

add_library(iobuffer)
add_library(iobuffer::iobuffer ALIAS iobuffer)

## Generate export macro

include(GenerateExportHeader)

generate_export_header(iobuffer
    EXPORT_FILE_NAME   biexport.h
    EXPORT_MACRO_NAME  B_API
    INCLUDE_GUARD_NAME IOBUFFER_EXPORT_H
)

## Setup library

# Set headers and sources

set(HEADERS
    include/iobuffer/iobuffer.h
    "${CMAKE_CURRENT_BINARY_DIR}/biexport.h"
)
set(SOURCES
    src/bidefine.h
    src/iobuffer.c
    src/vbiscanf.c
    src/vbiprintf.c
)

# Set flags for compiling

set(FLAGS -O2 -Wall -Wextra -Wpedantic)

if(MSVC)
    set(FLAGS /W4 /wd4267 /wd4244 /wd4146 /wd4457 /wd4456 /wd4701)
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        list(APPEND FLAGS /Od)
    else()
        list(APPEND FLAGS /O2)
    endif()
else()
    list(APPEND FLAGS -Wno-long-long)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang$")
    list(APPEND FLAGS -Wno-newline-eof)
endif()

# Set target properties

target_sources(iobuffer PRIVATE ${SOURCES})

set_target_properties(iobuffer PROPERTIES
    C_STANDARD          90
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS        OFF
    C_VISIBILITY_PRESET hidden

    VISIBILITY_INLINES_HIDDEN ON

    PUBLIC_HEADER "${HEADERS}"

    SOVERSION ${PROJECT_VERSION_MAJOR}
      VERSION ${PROJECT_VERSION}
)

target_compile_options(iobuffer PUBLIC ${FLAGS})
target_include_directories(iobuffer
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Platform-depend settings

if(NOT WIN32)
    target_link_libraries(iobuffer PRIVATE m)
endif()

## Standalone build settings

# Output directory

if(PROJECT_IS_TOP_LEVEL)
    set_target_properties(iobuffer PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    )
endif()

# Setup tests

if(IOBUFFER_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
    enable_testing()
endif()

# Installation

if(IOBUFFER_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS iobuffer
        EXPORT  iobuffer_targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/iobuffer"
    )

    install(
        EXPORT      iobuffer_targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/iobuffer"
        NAMESPACE   iobuffer::
        FILE        iobuffer-targets.cmake
    )

    configure_package_config_file(
        cmake/iobuffer-config.cmake.in iobuffer-config.cmake
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/iobuffer"
        PATH_VARS CMAKE_INSTALL_LIBDIR
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/iobuffer-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/iobuffer-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/iobuffer-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/iobuffer"
    )
endif()
