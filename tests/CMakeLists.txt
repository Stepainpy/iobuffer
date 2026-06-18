project(iobuffer_test LANGUAGES C)

set(CVERSIONS 90 99 11 23)

file(GLOB TESTFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/b*.c")

option(IOBT_FMT_DI_ENABLE_LIMITS "Enable check output of signed limits" ON)
option(IOBT_FMT_BOUX_ENABLE_LIMITS "Enable check output of unsigned limits" ON)

set(FLAGS -O2 -Wall -Wextra -Wpedantic -Wno-variadic-macros)

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

foreach(CVER IN ITEMS ${CVERSIONS})
    set(TESTBASE iobuffer_test_${CVER})
    foreach(TFILE IN ITEMS ${TESTFILES})
        string(LENGTH ${TFILE} TFLEN)
        math(EXPR TNLEN "${TFLEN} - 7")
        string(SUBSTRING ${TFILE} 5 ${TNLEN} TNAME)
        set(TESTNAME ${TESTBASE}_${TNAME})

        add_executable(${TESTNAME} ${TFILE})
        add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})

        target_link_libraries(${TESTNAME}
            PRIVATE iobuffer::iobuffer)
        target_include_directories(${TESTNAME}
            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_compile_options(${TESTNAME}
            PRIVATE ${FLAGS})

        set_target_properties(${TESTNAME} PROPERTIES
            C_STANDARD          ${CVER}
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS        OFF
        )

        if(IOBT_FMT_DI_ENABLE_LIMITS)
            target_compile_definitions(${TESTNAME}
                PRIVATE IOBT_FMT_DI_ENABLE_LIMITS=1)
        endif()
        if(IOBT_FMT_BOUX_ENABLE_LIMITS)
            target_compile_definitions(${TESTNAME}
                PRIVATE IOBT_FMT_BOUX_ENABLE_LIMITS=1)
        endif()

        if(WIN32)
            target_compile_definitions(${TESTNAME}
                PRIVATE _CRT_SECURE_NO_WARNINGS=1)

            add_custom_command(
                TARGET ${TESTNAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                $<TARGET_FILE_DIR:iobuffer::iobuffer>
                $<TARGET_FILE_DIR:${TESTNAME}>
            )
        endif()
    endforeach()
endforeach()
