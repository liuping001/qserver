add_library(common
        mq_net.cpp
        msg_head.pb.cc
        svr_comm_trans.cpp
)

add_link_options(common amqpcpp commlib )