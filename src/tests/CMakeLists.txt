enable_testing()

add_subdirectory( 02_test_nelder_mead )
add_subdirectory( 07_test_cgrow_regressions )
add_subdirectory( 08_test_gui )

option( CGROW_BUILD_EXPERIMENTAL_TEST_PROGRAMS "Build interactive, non-CTest exploratory programs" OFF )
if ( CGROW_BUILD_EXPERIMENTAL_TEST_PROGRAMS )
  add_subdirectory( 01_test )
  add_subdirectory( 03_test_pertrubed_ordinary )
  add_subdirectory( 04_test_pertrubed_total )
  add_subdirectory( 05_test_general_cuhyso )
  add_subdirectory( 06_test_cgrow_mutliparam )
endif()
