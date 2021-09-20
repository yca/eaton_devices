cmake_minimum_required(VERSION 3.5)

project(eaton_devices LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(eaton_devices main.cpp
    uuid.h
    ${PROJECT_ROOT}/3rdparty/loguru/loguru.cpp
    ${PROJECT_ROOT}/3rdparty/loguru/loguru.hpp
    ${PROJECT_ROOT}/3rdparty/loguru/debug.h
    ${PROJECT_ROOT}/3rdparty/loguru/debug.cpp
    )

target_compile_definitions(eaton_devices PUBLIC -DLOGURU_WITH_STREAMS)
