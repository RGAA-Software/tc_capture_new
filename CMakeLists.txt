cmake_minimum_required(VERSION 3.26)
project(tc_capture)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)

    set(DLL_SRCS
            win/dll_main.cpp
    )
    add_library(tc_capture tc_capture.cpp)
    add_library(tc_capture_library SHARED ${DLL_SRCS})
    target_link_libraries(tc_capture_library tc_common EasyHook64)

elseif (APPLE)

elseif (UNIX)

endif ()

#add_custom_command(TARGET tc_capture_library POST_BUILD
#
#)