add_library( libcgrow INTERFACE )

target_include_directories ( libcgrow INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )
target_compile_features( libcgrow INTERFACE cxx_std_17 )

FILE(GLOB_RECURSE cgrow_files cgrow.hpp nelder_mead.hpp linalg.h )

add_custom_target( cgrow_headers SOURCES ${cgrow_files})

