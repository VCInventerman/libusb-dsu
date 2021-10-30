set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.0.0)
project(libusb-dsu VERSION 0.1.0)

#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

if(DEFINED ENV{HOME})
	set(HOME $ENV{HOME})
else()
	set(HOME "C:/Users/nickk")
endif()

if (WIN32)
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /J") # unsigned char
	#include(/dev/vcpkg/scripts/buildsystems/vcpkg.cmake) # vcpkg
endif()

if (APPLE)
	include_directories(libusb-dsu PUBLIC ${HOME}/usr/local/include) # homebrew
endif()

if (MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clang:-std=c++20")
elseif (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
endif ()


#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /P")
#
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd /Qspectre")

add_executable(libusb-dsu main.cpp)


find_package(OpenSSL REQUIRED)
target_link_libraries(libusb-dsu PRIVATE OpenSSL::SSL)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.77.0 COMPONENTS log_setup log thread atomic chrono filesystem) 
include_directories(${Boost_INCLUDE_DIRS}) 
target_link_libraries(libusb-dsu PRIVATE ${Boost_LIBRARIES})		

find_package(libusb CONFIG REQUIRED)
target_include_directories(libusb-dsu PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(libusb-dsu PRIVATE ${LIBUSB_LIBRARIES})

target_include_directories(libusb-dsu PUBLIC ${HOME}/dev/Evergreen/evglib/include)



#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:"libcmt.lib" /NODEFAULTLIB:"libcmtd.lib"")
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /MACHINE:X64 /NODEFAULTLIB:"libcmt.lib" /NODEFAULTLIB:"libcmtd.lib"")
#target_link_libraries(libusb-dsu PRIVATE msvcprtd.lib)
#target_link_libraries(libusb-dsu PUBLIC libcmtd.lib msvcrtd.lib msvcprtd.lib)
#target_link_libraries(libusb-dsu PUBLIC kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib)

#target_link_libraries(libusb-dsu PUBLIC libcmtd.lib)

#target_link_libraries(libusb-dsu PUBLIC user32.lib)