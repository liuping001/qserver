
add_executable(test_co test_co.cpp)
target_link_libraries(test_co commlib)

add_executable(test_timer test_timer.cpp)
target_link_libraries(test_timer commlib)

add_executable(test_mq test_mq.cpp)
target_link_libraries(test_mq amqpcpp boost_system pthread)

add_executable(async_redis test_async_redis.cpp)
target_link_libraries(async_redis hiredis-vip event commlib)

add_executable(toml_cpp toml_make_cpp.cpp)

add_executable(spdlog test_spdlog.cpp)
target_link_libraries(spdlog pthread)