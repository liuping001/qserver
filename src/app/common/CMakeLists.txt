add_library(common
        mq_net.cpp
        msg_head.pb.cc
        svr_comm_trans.cpp
)

target_link_libraries(common amqpcpp commlib protobuf)