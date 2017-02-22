cmake_minimum_required(VERSION 3.5)
project(rshd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES rshd.cpp read_writer.cpp read_writer.h client.cpp client.h terminal.cpp terminal.h server.cpp server.h)
add_executable(rshd ${SOURCE_FILES})
