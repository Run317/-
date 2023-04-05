cmake_minimum_required(VERSION 3.24)
project(WeChat C)

set(CMAKE_C_STANDARD 99)

include_directories("${PROJECT_SOURCE_DIR}/inc")

find_library(pthread_lib NAMES pthread PATHS ./pthread)

add_executable(WeChat src/main.c inc/usr.h src/usr.c)
