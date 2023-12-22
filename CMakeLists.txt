cmake_minimum_required(VERSION 3.26)
project(tc_capture)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    ## capture
    set(CAPTURE_SRCS
            tc_capture.cpp
            win/audio/audio_file_saver.cpp
            win/audio/wasapi_audio_capture.cpp
            win/audio/audio_capture_factory.cpp
    )
    add_library(tc_capture ${CAPTURE_SRCS})
    target_link_libraries(tc_capture Winmm)

    ## capture dll
    set(DLL_SRCS
            win/dll_main.cpp
    )
    add_library(tc_capture_library SHARED ${DLL_SRCS})
    target_link_libraries(tc_capture_library tc_common EasyHook64)

elseif (APPLE)

elseif (UNIX)

endif ()

add_subdirectory(win/tests)

#add_custom_command(TARGET tc_capture_library POST_BUILD
#
#)