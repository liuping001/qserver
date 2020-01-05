add_executable(server_a
        server_a.cpp
        say_hello.pb.cc)
target_link_libraries(server_a common event)

add_executable(server_b
        server_b.cpp
        say_hello.pb.cc)
target_link_libraries(server_b common event)