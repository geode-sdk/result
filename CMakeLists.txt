cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
cmake_policy(SET CMP0097 NEW)

project(GeodeResult VERSION 1.0.0 LANGUAGES C CXX)

add_library(GeodeResult INTERFACE)

target_compile_features(GeodeResult INTERFACE cxx_std_20)

target_include_directories(GeodeResult INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(test)
endif()