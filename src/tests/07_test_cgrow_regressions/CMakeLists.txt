add_executable( 07_test_cgrow_regressions main.cpp )
set_property( TARGET 07_test_cgrow_regressions PROPERTY CXX_STANDARD 17 )
target_link_libraries( 07_test_cgrow_regressions PRIVATE libcgrow )
add_test( NAME 07_test_cgrow_regressions COMMAND $<TARGET_FILE:07_test_cgrow_regressions> )

find_package( Python3 COMPONENTS Interpreter )
if ( Python3_Interpreter_FOUND )
  add_test(
    NAME 07_test_original_cgrow_bugs
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/verify_original_bugs.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
endif()
