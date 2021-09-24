cmake_minimum_required( VERSION 3.15 )

project( cgrowfw VERSION 0.9.0 )

unset( QT_QMAKE_EXECUTABLE )

include( FetchContent )

FetchContent_Declare(
  cxxplot
  GIT_REPOSITORY "git@github.com:USNavalResearchLaboratory/cxxplot.git"
  GIT_TAG main
  )

set (OLD_BUILD_SHARED ${BUILD_SHARED_LIBS})
set( BUILD_SHARED_LIBS OFF)

FetchContent_MakeAvailable( cxxplot )

message( "STATUS cxxplot source dir: " ${cxxplot_SOURCE_DIR} )

set(NLOPT_SWIG OFF CACHE INTERNAL "")
set(NLOPT_OCTAVE OFF CACHE INTERNAL "")
set(NLOPT_MATLAB OFF CACHE INTERNAL "")
set(NLOPT_GUILE OFF CACHE INTERNAL "")
set (NLOPT_CXX11 ON CACHE INTERNAL "") # NLOPT DETECTS THIS ONLY FOR GCC. WE MAY WANT OUR OWN CXX11 DETECTION

FetchContent_Declare(
  nlopt
  GIT_REPOSITORY "git://github.com/stevengj/nlopt"
  GIT_TAG "41967f1981d82d8495c0b27151a126bc35ae0d96"
  )

FetchContent_MakeAvailable( nlopt )
# TODO : I think Nlopt interferes with some global variables and it creates issues like for example
# with BUILD_SHARED_LIBS

include( cmake/windeployqt.cmake )

set( CGROW_SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src/third_party/spdlog-1.8.5/include )

set( CGROW_LICENSE_FILE "${CMAKE_CURRENT_LIST_DIR}/license.txt" )
set( CGROW_LICENSES_DIR "${CMAKE_CURRENT_LIST_DIR}/licenses" )
set( CGROW_EXAMPLE_DATA_DIR "${CMAKE_CURRENT_LIST_DIR}/example_data")

add_subdirectory( src )
