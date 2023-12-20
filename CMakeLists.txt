cmake_minimum_required(VERSION 3.26)
project(tc_capture)

set(CMAKE_CXX_STANDARD 20)

set(DLL_SRCS
        dll_main.cpp
)

add_library(tc_capture tc_capture.cpp)

add_library(tc_capture_dll SHARED ${DLL_SRCS})
target_link_libraries(tc_capture_dll tc_common EasyHook64)