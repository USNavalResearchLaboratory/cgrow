add_library( libcgrow INTERFACE )

target_include_directories ( libcgrow INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )

FILE(GLOB_RECURSE cgrow_files cgrow.hpp nelder_mead.hpp )

add_custom_target( cgrow_headers SOURCES ${cgrow_files})

