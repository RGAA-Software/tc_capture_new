cmake_minimum_required(VERSION 3.26)
project(hk_obs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(injector)
add_subdirectory(graphics)
add_subdirectory(ipc_util)