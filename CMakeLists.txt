cmake_minimum_required(VERSION 3.0.0)
project(ffrdma VERSION 0.1.0)

option(DEBUG "debug" OFF)
option(BUILD_TEST "Test" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g2 -O0")
endif(DEBUG)

file(GLOB FMPI_SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
add_subdirectory(rsocket)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(fmpi ${FMPI_SRC_LIST})

target_link_libraries(fmpi rsocket)

if(BUILD_TESTS)
  add_subdirectory(test)
endif(BUILD_TESTS)


# set(CMAKE_CXX_FLAGS "${CMAK_CXX_FLAGS} -O0 -g2")


# add_executable(mpi-rdma main.cpp)
# add_executable(decoder decoder.cpp)
# target_link_libraries(decoder fmpi)
