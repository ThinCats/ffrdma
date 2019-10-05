include_directories(../include)
link_libraries(fmpi)

add_executable(hello hello_world.cc)
add_executable(proc_hello2 process/hello_2.cc)
add_executable(proc_hello3 process/hello_3.cc)
