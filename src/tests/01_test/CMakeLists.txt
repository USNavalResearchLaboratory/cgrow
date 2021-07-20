set( HSFIT_CURRENT_TARGET_NAME 01_test )

add_executable( ${HSFIT_CURRENT_TARGET_NAME} main.cpp )

set_property(TARGET ${HSFIT_CURRENT_TARGET_NAME} PROPERTY CXX_STANDARD 17)

target_include_directories( ${HSFIT_CURRENT_TARGET_NAME}
    PRIVATE ${nlopt_BINARY_DIR}
    ${nlopt_SOURCE_DIR}/src/api
    ${cppplot_SOURCE_DIR}/include
    ${CGROW_SPDLOG_INCLUDE_DIR}
    )

target_link_libraries(
    ${HSFIT_CURRENT_TARGET_NAME}
        libcgrow
        nlopt
        cxxplot::cxxplot
    )
