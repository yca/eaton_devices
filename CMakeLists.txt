cmake_minimum_required(VERSION 3.5)

project(eaton_devices LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(eaton_devices main.cpp
    uuid.h
	device.cpp
	device.h
	measurement.cpp
	measurement.h
	transport.cpp
	transport.h
	hub.cpp
	hub.h
        devices/ambientsensor.cpp
        devices/ambientsensor.h
        devices/pressuresensor.h
        devices/pressuresensor.h
	transports/udptransport.cpp
	transports/udptransport.h
	measurements/temperaturemeasurement.cpp
	measurements/temperaturemeasurement.h
	measurements/pressuremeasurement.cpp
	measurements/pressuremeasurement.h
	measurements/humiditymeasurement.cpp
	measurements/humiditymeasurement.h
    ${PROJECT_ROOT}/3rdparty/loguru/loguru.cpp
    ${PROJECT_ROOT}/3rdparty/loguru/loguru.hpp
    ${PROJECT_ROOT}/3rdparty/loguru/debug.h
    ${PROJECT_ROOT}/3rdparty/loguru/debug.cpp
    )

target_compile_definitions(eaton_devices PUBLIC -DLOGURU_WITH_STREAMS)
target_link_libraries(eaton_devices pthread dl)
