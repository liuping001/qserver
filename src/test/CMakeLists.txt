
add_executable(test_co test_co.cpp)
target_link_libraries(test_co commlib)

add_executable(test_timer test_timer.cpp)
target_link_libraries(test_timer commlib)

add_executable(test_mq test_mq.cpp)
target_link_libraries(test_mq amqpcpp boost_system pthread)

add_executable(test_mq_consume test_mq_consume.cpp)
target_link_libraries(test_mq_consume amqpcpp boost_system pthread)

#add_executable(async_redis test_async_redis.cpp)
#target_link_libraries(async_redis hiredis-vip event commlib)

#add_executable(toml_cpp toml_make_cpp.cpp)

add_executable(spdlog test_spdlog.cpp)
target_link_libraries(spdlog pthread commlib)

add_executable(test_redispp test_redis++.cpp)
target_link_libraries(test_redispp redis++)

add_executable(get_ip test_get_ip.cpp)

add_executable(consistent_hash test_consistent_hash.cpp)

add_executable(test_zk test_zookeeper.cpp zookeeper_client.cpp)
target_link_libraries(test_zk ${CMAKE_SOURCE_DIR}/third/zookeeper/lib/libzookeeper_st.a co)