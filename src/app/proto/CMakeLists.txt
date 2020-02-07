find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})#?
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(hello_proto
        msg_head.proto
        say_hello.proto
        redis_cmd.proto
        cmd.proto
        )

PROTOBUF_GENERATE_CPP(hello_SRCS hello_HDRS ${hello_proto})
message("hello_SRCS = ${hello_SRCS}")
message("hello_HDRS = ${hello_HDRS}")
add_library(proto
        ${hello_SRCS}
        ${COM_PB_SRCS}
        )

target_link_libraries(proto protobuf)