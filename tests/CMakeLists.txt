project(iobuffer_test LANGUAGES C)

set(CVERSIONS 90 99 11 23)

set(TESTFILES
    bsetalloc
    bclose
    bopen
    bmemopen
    berase
)

foreach(CVER IN ITEMS ${CVERSIONS})
    set(TESTBASE iobuffer_test_${CVER})
    foreach(TFILE IN ITEMS ${TESTFILES})
        set(TESTNAME ${TESTBASE}_${TFILE})

        add_executable(${TESTNAME} "${TFILE}.c")
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
