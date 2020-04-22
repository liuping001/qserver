
add_library(common
        mq_net.cpp
        svr_comm_trans.cpp
        redis_actuator.cpp
        svr_list.cpp
        zookeeper_client.cpp
        )

target_link_libraries(common
        amqpcpp
        commlib
        proto
        event
        ${CMAKE_SOURCE_DIR}/third/zookeeper/lib/libzookeeper_st.a
        co
        redis_handler)