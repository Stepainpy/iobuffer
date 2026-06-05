project(iobuffer_test LANGUAGES C)

set(CVERSIONS 90 99 11 23)

file(GLOB TESTFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/b*.c")

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

        set_target_properties(${TESTNAME} PROPERTIES
            C_STANDARD          ${CVER}
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS        OFF
        )
    endforeach()
endforeach()
