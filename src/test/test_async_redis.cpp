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
#include <exception>
#include <sstream>

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

    namespace writer {
        template <class T>
        void Append(std::vector<std::string> &cmd, T &&t) {
            cmd.push_back(std::forward<T>(t));
        }

        template <class T>
        void Append(std::vector<std::string> &cmd, const std::vector<T> &t) {
            for (const auto &item : t) {
                Append(cmd, item);
            }
        }

        template <class H, class ...T>
        void Append(std::vector<std::string> &cmd, H &&h, T&&... t) {
            cmd.push_back(std::forward<H>(h));
            Append(cmd, std::forward<T>(t)...);
        }
        template <class ...T>
        std::vector<std::string> Cmd(T &&...t) {
            std::vector<std::string> cmd;
            Append(cmd, std::forward<T>(t)...);
            return cmd;
        }
    };


    class RedisCmd {

        CoTask &co_task_;
        redisAsyncContext *context_;
        const CoYield &yield_;

        redisReply * Yield() {
            auto ret = yield_.Yield();
            if (ret != 0) {
                throw std::logic_error("yield failed");
            }
            auto msg = yield_.GetMsg();
            if (msg == nullptr) {
                throw std::logic_error("reply null");
            }

            auto redis_reply = static_cast<redisReply *>(msg);
            if (redis_reply->type == REDIS_REPLY_ERROR) {
                throw std::logic_error(std::string(redis_reply->str, redis_reply->len));
            }
            return redis_reply;
        }

        template <class T, class ...Args>
        T InnerCmd(Args &&...args) {
            auto cmd = writer::Cmd(std::forward<Args>(args)...);
            std::vector<const char *> argvs(cmd.size());
            std::vector<size_t> argvlens(cmd.size());
            for (uint32_t i = 0; i < cmd.size(); ++i) {
                argvs[i] = cmd[i].c_str();
                argvlens[i] = cmd[i].length();
            }
            CoInfo co_info{co_task_, yield_};
            redisAsyncCommandArgv(context_, GetCallback, &co_info, argvs.size(), &argvs[0], &argvlens[0]);
            auto reply = Yield();
            RedisReplyReader reader(reply);
            T t;
            reader.SetValue(t);
            return t;
        }
        template <class ...Args>
        void NoRetInnerCmd(Args &&...args) {
            auto cmd = writer::Cmd(std::forward<Args>(args)...);
            std::vector<const char *> argvs(cmd.size());
            std::vector<size_t> argvlens(cmd.size());
            for (uint32_t i = 0; i < cmd.size(); ++i) {
                argvs[i] = cmd[i].c_str();
                argvlens[i] = cmd[i].length();
            }
            CoInfo co_info{co_task_, yield_};
            redisAsyncCommandArgv(context_, GetCallback, &co_info, argvs.size(), &argvs[0], &argvlens[0]);
            Yield();
        }
    public:

        RedisCmd(CoTask &co_task, redisAsyncContext *context, const CoYield &yield)
                : co_task_(co_task),
                  context_(context),
                  yield_(yield) {}

        void Set(const std::string &key, const std::string &value) {
            return NoRetInnerCmd("set", key, value);
        }

        std::string Get(const std::string &key) {
            return InnerCmd<std::string>("get", key);
        }

        std::vector<std::string> MGet(const std::vector<std::string> &keys) {
            return InnerCmd<std::vector<std::string>>("mget", keys);
        }
        long long Incr(const std::string &key) {
            return InnerCmd<long long>("incr", key);
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
        cmd.Set("key1", "value1");
        cmd.Set("key2", "value2");
        char b[6]={0, 1, 2, 122, 9};
        cmd.Set("key3", std::string(b, sizeof(b)));
        std::cout << "value1:" << cmd.Get("key1")<< std::endl;
        std::cout << "value2:" << cmd.Get("key2")<< std::endl;
        std::cout << "value3:" << cmd.Get("key3").size() << std::endl;

        Debug(cmd.MGet({"key1", "key2"}));

        redisAsyncDisconnect(redis_client.Context());
    });

    event_base_dispatch(base);
    return 0;
}
