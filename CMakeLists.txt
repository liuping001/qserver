cmake_minimum_required(VERSION 3.13)
project(qserver)
SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)
include_directories(src)
set(SOURCE_FILES
    src/commlib/co/co_task.cpp
    src/commlib/co/coroutine.cpp
    )

add_library(commlib ${SOURCE_FILES})


add_executable(test_co src/test/test_co.cpp)
target_link_libraries(test_co commlib)