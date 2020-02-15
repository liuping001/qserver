set(SOURCE_FILES
        co/co_task.cpp
        co/coroutine.cpp
        trans/trans.cpp
        logging.cpp
        )

add_library(commlib ${SOURCE_FILES})
target_link_libraries(commlib
        pthread
        )
add_library(co
        co/co_task.cpp
        co/coroutine.cpp
        )

add_library(redis_handler
        redis/command.cpp
        redis/command_options.cpp
        redis/redis.cpp
        redis/sds.cpp
        redis/connection.cpp
        redis/reply.cpp)