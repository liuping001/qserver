cmake_minimum_required(VERSION 3.2)
project(qserver)
SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(./third)
include_directories(./third/redis-plus-plus/src/sw)
add_subdirectory(commlib)

set (AMQP-CPP_LINUX_TCP ON CACHE BOOL "x")
add_subdirectory(third/AMQP-CPP)

add_subdirectory(third)
add_subdirectory(test)
