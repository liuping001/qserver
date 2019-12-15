set(SOURCE_FILES
        co/co_task.cpp
        co/coroutine.cpp
        redis_client.cpp
        trans/trans.cpp
        logging.cpp
        )

add_library(commlib ${SOURCE_FILES})

add_library(co
        co/co_task.cpp
        co/coroutine.cpp
        )