cmake_minimum_required(VERSION 3.10)
project(qserver)
SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR})
add_subdirectory(commlib)
add_subdirectory(third/AMQP-CPP)
add_subdirectory(test)