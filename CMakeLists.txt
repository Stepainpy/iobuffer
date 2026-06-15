cmake_minimum_required(VERSION 3.21)
project(iobuffer
    VERSION      3.0.2
    DESCRIPTION "Dynamic memory buffer with API as C standard FILE"
    HOMEPAGE_URL https://github.com/Stepainpy/iobuffer
    LANGUAGES    C
)

## Create and setup library

add_library(iobuffer STATIC)
add_library(iobuffer::iobuffer ALIAS iobuffer)

set(HEADERS
    include/iobuffer/iobuffer.h
)
set(SOURCES
    src/bidefine.h
    src/iobuffer.c
    src/vbiscanf.c
    src/vbiprintf.c
)

set(FLAGS -O2 -Wall -Wextra -Wpedantic)

if(MSVC)
    set(FLAGS /O2 /W4)
else()
    list(APPEND FLAGS -Wno-long-long)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    list(APPEND FLAGS -Wno-newline-eof)
endif()

target_sources(iobuffer PRIVATE ${SOURCES})

set_target_properties(iobuffer PROPERTIES
    C_STANDARD          90
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS        OFF

    PUBLIC_HEADER "${HEADERS}"

    SOVERSION ${PROJECT_VERSION_MAJOR}
      VERSION ${PROJECT_VERSION}
)

target_compile_options(iobuffer PUBLIC ${FLAGS})
target_include_directories(iobuffer
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

# Platform-depend settings

if(NOT WIN32)
    target_link_libraries(iobuffer PRIVATE m)
endif()

## Standalone build settings

# Output directory

if(PROJECT_IS_TOP_LEVEL)
    set_target_properties(iobuffer PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()

# Setup tests

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    add_subdirectory(tests)
    enable_testing()
endif()

# Installation

if (PROJECT_IS_TOP_LEVEL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS iobuffer
        EXPORT  iobuffer_targets
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"
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
