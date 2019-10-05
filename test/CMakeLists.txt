include_directories(../include)

add_executable(hello_world hello_world.cc)
target_link_libraries(hello_world fmpi)