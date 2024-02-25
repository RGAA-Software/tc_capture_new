cmake_minimum_required(VERSION 3.26)
project(tc_capture)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX -DENABLE_JEMALLOC=0)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win/hk_obs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win/hk_obs/d3d8_api)

find_package(libyuv CONFIG REQUIRED)
#find_package(Poco COMPONENTS Foundation REQUIRED)

link_directories(${VCPKG_ROOT}/installed/x64-windows-static/lib)
link_directories(${VCPKG_ROOT}/packages/jemalloc_x64-windows-static/lib)

if (WIN32)

    add_subdirectory(win/hk_obs)

    ## capture
    set(CAPTURE_SRCS
        tc_capture.cpp
        desktop_capture.h
        desktop_capture.cpp
        win/audio/audio_file_saver.cpp
        win/audio/wasapi_audio_capture.cpp
        win/audio/audio_capture_factory.cpp
        ## desktop_capture
        win/desktop_capture/dda_capture.cpp
        win/desktop_capture/win_desktop_capture.cpp
        capture_message_maker.cpp
    )
    add_library(tc_capture ${CAPTURE_SRCS})
    target_link_libraries(tc_capture Winmm)

    ## capture dll
    set(DLL_SRCS
        win/client_manager.cpp
        win/dll_main.cpp
        win/client_ipc_manager.cpp

        ## hk_utils
        win/hk_utils/env.h
        win/hk_utils/module.hpp
        win/hk_utils/shellapi.h

        ## hk_video
        win/hk_video/capture_dxgi.cpp
        win/hk_video/capture_dxgi_d3d11.cpp
        win/hk_video/capture_dxgi_d3d11on12.cpp
        win/hk_video/capture_texture.cpp
        win/hk_video/d3d_utils.cpp
        win/hk_video/hook_dxgi.cpp
        win/hk_video/hook_event.cpp
        win/hk_video/shared_texture.cpp

        ## hk_audio
    )

    add_library(tc_capture_library SHARED ${DLL_SRCS})
    target_link_libraries(tc_capture_library tc_common EasyHook64 yuv Iphlpapi.lib PocoFoundationmt.lib)

elseif (APPLE)

elseif (UNIX)

endif ()

add_subdirectory(win/tests)

#add_custom_command(TARGET tc_capture_library POST_BUILD
#
#)