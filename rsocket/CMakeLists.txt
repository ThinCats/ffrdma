cmake_minimum_required(VERSION 3.0.0)

set(SRC_LIST "" PRIVATE)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_library(rsocket ${SRC_LIST} ${HEADER_LIST})
target_include_directories(rsocket PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rsocket PRIVATE rdmacm ibverbs pthread)