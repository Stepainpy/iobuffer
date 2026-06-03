project(iobuffer_test LANGUAGES C)

set(CVERSIONS 90 99 11 23)

set(SOURCES )

foreach(CVER IN ITEMS ${CVERSIONS})
    set(TESTNAME iobuffer_test_${CVER})

    add_executable(${TESTNAME})
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})

    target_sources(${TESTNAME} PRIVATE ${SOURCES})
    target_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${TESTNAME} PROPERTIES
        C_STANDARD          ${CVER}
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS        OFF
    )
endforeach()
