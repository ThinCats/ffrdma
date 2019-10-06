include_directories(../include)
link_libraries(fmpi)

add_executable(hello_world hello_world.cc)
add_executable(proc_hello2 process/hello_2.cc)
add_executable(proc_hello4 process/hello_4.cc)
