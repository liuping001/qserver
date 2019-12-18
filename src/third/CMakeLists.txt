include_directories(./hiredis-vip)
add_library(hiredis-vip
        hiredis-vip/adlist.c
        hiredis-vip/async.c
        hiredis-vip/command.c
        hiredis-vip/crc16.c
        hiredis-vip/dict.c
        hiredis-vip/hiarray.c
        hiredis-vip/hircluster.c
        hiredis-vip/hiredis.c
        hiredis-vip/hiutil.c
        hiredis-vip/net.c
        hiredis-vip/read.c
        hiredis-vip/sds.c
        )

add_library(hiredis hiredis/async.c
        hiredis/dict.c
        hiredis/hiredis.c
        hiredis/net.c
        hiredis/read.c
        hiredis/sds.c
        )

file(GLOB REDIS_SRC redis-plus-plus/src/sw/redis++/*cpp)
add_library(redis++  ${REDIS_SRC})
target_link_libraries(redis++ hiredis)