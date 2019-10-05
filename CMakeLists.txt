cmake_minimum_required(VERSION 3.0.0)
project(ffrdma VERSION 0.1.0)

option(DEBUG "debug" OFF)
option(BUILD_TEST "Test" ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g2 -O0")
endif(DEBUG)

file(GLOB_RECURSE
     FMPI_SRC_LIST
     ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cc
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
     "src/process.cc")
file(GLOB_RECURSE FMPI_HEADER_LIST
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     "include/*.hpp" "include/**/*.hpp")

add_subdirectory(rsocket)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(fmpi ${FMPI_SRC_LIST} ${FMPI_HEADER_LIST})

target_link_libraries(fmpi rsocket)

if(BUILD_TEST)
  add_subdirectory(test)
endif(BUILD_TEST)

source_group(TREE
             "${CMAKE_CURRENT_SOURCE_DIR}/include"
             PREFIX
             "Header Files"
             FILES
             ${FMPI_HEADER_LIST})
