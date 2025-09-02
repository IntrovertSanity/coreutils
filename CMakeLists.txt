cmake_minimum_required(VERSION 3.15)

project(coreutils_insane LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("include")
add_executable(echo src/echo.c)
add_executable(cat src/cat.c)