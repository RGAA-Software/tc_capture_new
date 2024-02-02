cmake_minimum_required(VERSION 3.26)
project(tc_injector)

add_executable(${PROJECT_NAME}
        inject-helper.c
        inject-library.c
        ../obfuscate.c
)