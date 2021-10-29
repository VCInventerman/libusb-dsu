set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.0.0)
project(libusb-dsu VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(DEFINED ENV{HOME})
	set(HOME $ENV{HOME})
else()
	set(HOME "C:/Users/nickk")
endif()





add_executable(libusb-dsu main.cpp)

target_include_directories(libusb-dsu PUBLIC ${HOME}/usr/local/include)

target_include_directories(libusb-dsu PUBLIC ${HOME}/dev/evergreen/evglib/include)
