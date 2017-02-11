cmake_minimum_required(VERSION 2.8)

project(BIGINT)

include_directories(${BIGINT_SOURCE_DIR})
SET(CMAKE_CXX_COMPILER /usr/bin/g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -std=c++17")
SET(UNDEF_WATCH 1)


if((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND UNDEF_WATCH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wextra -Winline -pedantic -fsanitize=address,undefined")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

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
                cow_soo_vector.h
                cow_soo_vector.cpp
                main.cpp)



target_link_libraries(big_integer_testing -lgmp -lgmpxx -lpthread)
