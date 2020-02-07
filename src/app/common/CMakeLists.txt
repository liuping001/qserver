
add_library(common
        mq_net.cpp
        svr_comm_trans.cpp
        redis_connect.cpp
        )

target_link_libraries(common
        amqpcpp
        commlib
        proto
        event
        redis_handler)