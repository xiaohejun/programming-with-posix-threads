cmake_minimum_required(VERSION 3.15)

PROJECT(programming-with-posix-threads)

SET(CMAKE_C_FLAGS "-pthread")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/include)

ADD_SUBDIRECTORY(src/ch1)
ADD_SUBDIRECTORY(src/ch2)
ADD_SUBDIRECTORY(src/ch3)