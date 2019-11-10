#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <memory>

#include "commlib/co/co_task.h"

#include "third/hiredis-vip/hiredis.h"
#include "third/hiredis-vip/async.h"
#include "third/hiredis-vip/adapters/libevent.h"

#include <iostream>
#include <string>
#include <vector>

namespace Redis {
    static void GetCallback(redisAsyncContext *c, void *r, void *privdata);
    static void ConnectCallback(const redisAsyncContext *c, int status);
    static void DisconnectCallback(const redisAsyncContext *c, int status);

    class RedisClient {
        struct event_base &base_;
        CoTask &co_task_;
        redisAsyncContext * context_ = nullptr;
    public:
        RedisClient(struct event_base &base, CoTask &co_task) : base_(base), co_task_(co_task) {}

        redisAsyncContext *Context() { return context_; }
        CoTask & GetCoTask() { return co_task_; }

        int Init(std::string ip, int port) {
            context_ = redisAsyncConnect(ip.c_str(), port);
            if (context_->err) {
                printf("Error: %s\n", context_->errstr);
                return -1;
            }
            auto ret = redisLibeventAttach(context_, &base_);
            if (ret != REDIS_OK) { return ret; }

            ret = redisAsyncSetConnectCallback(context_, ConnectCallback);
            if (ret != REDIS_OK) { return ret; }

            ret = redisAsyncSetDisconnectCallback(context_, DisconnectCallback);
            if (ret != REDIS_OK) { return ret; }
            return 0;
        }
    };

    struct CoInfo {
        CoTask &co_task;
        const CoYield &yield;
    };

    class RedisReplyReader {
    private:
        redisReply *reply_;
    public:
        RedisReplyReader (redisReply *reply) : reply_(reply) {}
        void SetValue(std::string &value) {
            value.assign(reply_->str, reply_->len);
        }
        void SetValue(long long &value) {
            value = reply_->integer;
        }
        template <class T>
        void SetValue(std::vector<T> &value) {
            if (reply_->type != REDIS_REPLY_ARRAY) {
                return;
            }
            for (size_t i = 0; i < reply_->elements; i++) {
                value.emplace_back();
                RedisReplyReader reader(reply_->element[i]);
                reader.SetValue(value.back());
            }
        }

    };

    class RedisCmd {
        struct Ret {
            int code = 0;
            std::string errstr;
            void Clear() {
                code = 0;
                errstr.clear();
            }
        };

        CoTask &co_task_;
        redisAsyncContext *context_;
        const CoYield &yield_;

        Ret Yield(redisReply * &reply) {
            Ret error;
            auto ret = yield_.Yield();
            if (ret != 0) {
                error.code = 1;
                return error;
            }
            auto msg = yield_.GetMsg();
            if (msg == nullptr) {
                error.code = -1;
                return error;
            }

            auto redis_reply = static_cast<redisReply *>(msg);
            if (reply->type == REDIS_REPLY_ERROR) {
                error.errstr.assign(reply->str, reply->len);
                error.code = -1;
                return error;
            }
            reply = redis_reply;
            return error;
        }

        template <class ...T>
        Ret InnerCmd(const std::string &cmd, T&... value) {
            std::cout << "cmd :" <<cmd <<"\n";
            CoInfo co_info{co_task_, yield_};
            redisAsyncCommand(context_, GetCallback, &co_info, cmd.c_str());
            redisReply * reply;
            auto ret = Yield(reply);
            if (ret.code != 0) {
                return ret;
            }
            RedisReplyReader reader(reply);
            reader.SetValue(value...);
            return ret;
        }
    public:

        RedisCmd(CoTask &co_task, redisAsyncContext *context, const CoYield &yield)
                : co_task_(co_task),
                  context_(context),
                  yield_(yield) {}

        Ret Set(const std::string &key, const std::string &value) {
            std::string cmd = "set ";
            cmd.append(key);
            cmd.append(" ").append(value);
            std::cout << "cmd:" << cmd << "\n";
            CoInfo co_info{co_task_, yield_};
            redisAsyncCommand(context_, GetCallback, &co_info, cmd.c_str());
            redisReply * reply;
            return Yield(reply);
        }

        Ret Get(const std::string &key, std::string &value) {
            return InnerCmd("get " + key, value);
        }

        Ret MGet(const std::vector<std::string> &keys, std::vector<std::string> &value) {
            std::string cmd = "mget";
            for (auto &item : keys) {
                cmd.append(" ");
                cmd.append(item);
            }
            return InnerCmd(cmd, value);
        }

        Ret Scan(long long &cursor, std::vector<std::string> &keys) {
            std::string cmd = "scan ";
            cmd.append(std::to_string(cursor));

            CoInfo co_info{co_task_, yield_};
            redisAsyncCommand(context_, GetCallback, &co_info, cmd.c_str());
            redisReply * reply;
            auto ret = Yield(reply);
            if (ret.code != 0) {
                return ret;
            }
            std::cout << "scan  ----:" << reply->element[0]->type << "\n";
            RedisReplyReader reader1(reply->element[0]);
            reader1.SetValue(cursor);
            RedisReplyReader reader2(reply->element[1]);
            reader2.SetValue(keys);
            return ret;
        }
    };

    static void GetCallback(redisAsyncContext *c, void *r, void *privdata) {
        auto info = static_cast<CoInfo *> (privdata);
        if (info != nullptr) {
            info->co_task.ResumeOneWithMsg(info->yield.co_id_, r);
        }
    }

    static void ConnectCallback(const redisAsyncContext *c, int status) {
        if (status != REDIS_OK) {
            printf("Error: %s\n", c->errstr);
            return;
        }
        printf("Connected...\n");
    }

    static void DisconnectCallback(const redisAsyncContext *c, int status) {
        if (status != REDIS_OK) {
            printf("Error: %s\n", c->errstr);
            return;
        }
        printf("Disconnected...\n");
    }
}

void Debug(const std::vector<std::string> &value) {
    for (auto &item : value) {
        std::cout << item << std::endl;
    }
}

using namespace Redis;
int main (int argc, char **argv) {
    struct event_base *base = event_base_new();
    CoTask co_task_;
    RedisClient redis_client(*base, co_task_);
    redis_client.Init("127.0.0.1", 6379);

    co_task_.DoTack([&redis_client](const CoYield &yield) {
        RedisCmd cmd(redis_client.GetCoTask(), redis_client.Context(),  yield);
        std::cout << cmd.Set("key1", "value1").code << std::endl;
        std::cout << cmd.Set("key2", "value2").code << std::endl;

        std::string value1;
        std::cout << cmd.Get("key1", value1).code << std::endl;
        std::cout << "value1:" << value1 << std::endl;
        std::string value3;
        std::cout << cmd.Get("key3", value3).code << std::endl;
        std::cout << "value3:" << value3 << std::endl;


        std::vector<std::string> mvalue;
        std::cout << cmd.MGet({"key1", "key2", "key3"}, mvalue).code << std::endl;;
        Debug(mvalue);

        long long cursor = 0;
        std::vector<std::string> keys;
        std::cout << cmd.Scan(cursor, keys).code;
        Debug(keys);

        redisAsyncDisconnect(redis_client.Context());
    });

    event_base_dispatch(base);
    return 0;
}
