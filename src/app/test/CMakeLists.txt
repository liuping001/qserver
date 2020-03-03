add_executable(server_a
        server_a.cpp
        )
target_link_libraries(server_a common event)

add_executable(server_b
        server_b.cpp
        )
target_link_libraries(server_b common event)

add_executable(test_svr
        test.cpp
        )
target_link_libraries(test_svr
        common
        event
        pthread
        )