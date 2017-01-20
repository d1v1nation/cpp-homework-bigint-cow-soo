cmake_minimum_required(VERSION 2.8)

project(BIGINT)

include_directories(${BIGINT_SOURCE_DIR})
SET(CMAKE_CXX_COMPILER /usr/bin/g++-5)

add_executable(big_integer_testing
               big_integer_testing.cpp
               big_integer.h
               big_integer.cpp
               gtest/gtest-all.cc
               gtest/gtest.h
               gtest/gtest_main.cc)

add_executable(bi
                big_integer.cpp
                big_integer.h
                main.cpp)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pedantic -fsanitize=address,undefined")
endif()

target_link_libraries(big_integer_testing -lgmp -lgmpxx -lpthread)