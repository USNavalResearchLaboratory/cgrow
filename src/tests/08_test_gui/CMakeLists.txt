find_package( QT NAMES Qt6 Qt5 COMPONENTS Test Widgets PrintSupport Svg REQUIRED )
find_package( Qt${QT_VERSION_MAJOR} COMPONENTS Test Widgets PrintSupport Svg REQUIRED )
find_package( Threads REQUIRED )
if ( QT_VERSION_MAJOR GREATER_EQUAL 6 )
  find_package( Qt${QT_VERSION_MAJOR} COMPONENTS SvgWidgets REQUIRED )
endif()

set( CGROW_GUI_TEST_SOURCES
  main.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/mainwindow.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/mainwindow.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/qcustomplot.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/qcustomplot.h
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/tests_list_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/tests_list_widget.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/test_series_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/test_series_dialog.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/spreadsheet.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/spreadsheet.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/test_series.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/test_series.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/marker.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/marker.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/decorated_spinbox.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/decorated_spinbox.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/qcp_results_table.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/qcp_results_table.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/fitting_worker.cpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/fitting_worker.hpp
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/icons.qrc
  ${CMAKE_SOURCE_DIR}/src/apps/cgrow/images.qrc
  )

add_executable( 08_test_gui ${CGROW_GUI_TEST_SOURCES} )
set_target_properties( 08_test_gui PROPERTIES AUTOMOC ON AUTORCC ON CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON )
target_include_directories( 08_test_gui PRIVATE ${CMAKE_SOURCE_DIR}/src/apps/cgrow )
target_link_libraries(
  08_test_gui
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Test
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::PrintSupport
  Qt${QT_VERSION_MAJOR}::Svg
  libcgrow
  nlopt
  Threads::Threads
  )

if ( QT_VERSION_MAJOR GREATER_EQUAL 6 )
  target_link_libraries( 08_test_gui PRIVATE Qt${QT_VERSION_MAJOR}::SvgWidgets )
endif()

add_test( NAME 08_test_gui COMMAND $<TARGET_FILE:08_test_gui> )
# CTest does not inherit Qt Creator's runtime PATH on Windows.  Prepend the
# Qt DLL directory so this target is runnable both locally and in CI.
set_tests_properties( 08_test_gui PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>"
  )
