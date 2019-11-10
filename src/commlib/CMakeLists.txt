set(SOURCE_FILES
        co/co_task.cpp
        co/coroutine.cpp
        trans/trans.cpp
        )

add_library(commlib ${SOURCE_FILES})

add_library(co
        co/co_task.cpp
        co/coroutine.cpp
        )