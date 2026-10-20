find_package( Threads REQUIRED )

add_executable( cgrow_basic_fit main.cpp )
set_property( TARGET cgrow_basic_fit PROPERTY CXX_STANDARD 17 )
set_property( TARGET cgrow_basic_fit PROPERTY CXX_STANDARD_REQUIRED ON )

target_link_libraries( cgrow_basic_fit PRIVATE libcgrow Threads::Threads )

add_test( NAME cgrow_basic_fit COMMAND $<TARGET_FILE:cgrow_basic_fit> )
