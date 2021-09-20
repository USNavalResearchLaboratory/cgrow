set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

project( cgrow )

find_package( Qt5
  COMPONENTS
  Widgets
  PrintSupport
  REQUIRED)

qt5_add_resources( resources ${CMAKE_CURRENT_SOURCE_DIR}/icons.qrc ${CMAKE_CURRENT_SOURCE_DIR}/images.qrc)

if( WIN32 )
  set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
endif()

find_package( Threads REQUIRED )

if( WIN32 )
  set( NOCONSOLEWIN WIN32 )
endif()

add_executable( cgrow
  ${NOCONSOLEWIN}
  ${resources}
  ${APP_ICON_RESOURCE_WINDOWS}
  main.cpp
  mainwindow.hpp
  mainwindow.cpp
  qcustomplot.cpp
  qcustomplot.h
  tests_list_widget.cpp
  tests_list_widget.hpp
  test_series_dialog.hpp
  test_series_dialog.cpp
  spreadsheet.hpp
  spreadsheet.cpp
  test_series.hpp
  test_series.cpp
  marker.hpp
  marker.cpp
  decorated_spinbox.hpp
  decorated_spinbox.cpp
  qcp_results_table.hpp
  qcp_results_table.cpp
  fitting_worker.hpp
  fitting_worker.cpp
  )

target_link_libraries( cgrow
  Qt5::Widgets
  Qt5::PrintSupport
  libcgrow
  nlopt
  Threads::Threads
  )

#include( cmake/windeployqt.cmake )

set_target_properties( cgrow
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
  )

set_property(TARGET cgrow PROPERTY CXX_STANDARD 17)

if (WIN32)
  WinDeployQt(TARGET cgrow )

  install(DIRECTORY ${PROJECT_BINARY_DIR}/bin
          DESTINATION .
          COMPONENT applications)

  install(DIRECTORY ${CGROW_LICENSES_DIR} DESTINATION . COMPONENT applications)
  install(DIRECTORY ${CGROW_EXAMPLE_DATA_DIR} DESTINATION . COMPONENT applications)

  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  include(InstallRequiredSystemLibraries)
  install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
          DESTINATION bin
          COMPONENT applications)

  set(CPACK_PACKAGE_NAME "CGROWInstaller")
  set(CPACK_PACKAGE_VENDOR "U.S. NRL, Code 6394")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CGrow crack model identification")
  set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})
  set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "CGROW_${CMAKE_PROJECT_VERSION}")

  set(CPACK_COMPONENTS_ALL applications )

  #set(CPACK_COMPONENTS_ALL applications CTData CLICENSES)
  #set(CPACK_COMPONENT_CTDATA_GROUP "data")
  #set(CPACK_COMPONENT_CLICENSES_GROUP "licenses")

  set(CPACK_NSIS_MENU_LINKS "bin/cgrow.exe" "CGROW v${CMAKE_PROJECT_VERSION}")

  set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "CGROW App v${CMAKE_PROJECT_VERSION}")
  #set(CPACK_COMPONENT_GROUP_DATA_DESCRIPTION "data (CT and instructions)")

  set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/assets/installer_license.txt)
  include(CPack)

endif()
