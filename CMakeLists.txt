cmake_minimum_required(VERSION 3.3)
project(http_parser)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    Parser.hpp
    Parser.cpp
    main.cpp)

add_executable(http_parser ${SOURCE_FILES})
target_link_libraries(http_parser pcap)
