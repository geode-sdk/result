cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(GeodeResultTest LANGUAGES CXX)

add_executable(GeodeResultTest
    test.cpp
)

target_link_libraries(GeodeResultTest
    PRIVATE
    GeodeResult
)