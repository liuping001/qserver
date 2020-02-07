//
// Created by liuping on 2020/2/1.
//
#pragma once
#include <stdlib.h>

#include "app/proto/redis_cmd.pb.h"
#include "commlib/redis/redis_reply.h"

using namespace sw;
using namespace sw::redis;

struct ConvertReply {


static redisReply *PbToReply(const proto::redis::redisReply &pb_reply) {
  auto reply = createReplyObject(pb_reply.type());
  if (reply == nullptr) {
    return reply;
  }
  void *r;
  if (reply->type == REDIS_REPLY_ERROR  ||
      reply->type == REDIS_REPLY_STATUS ||
      reply->type == REDIS_REPLY_STRING) {
    r = createStringObject(reply, pb_reply.str().data(), pb_reply.str().size());
  } else if (reply->type == REDIS_REPLY_INTEGER) {
    createIntegerObject(reply, pb_reply.integer());
  } else if (reply->type == REDIS_REPLY_ARRAY) {
    r = createArrayObject(reply, pb_reply.element_size());
    if (r != nullptr) {
      for (int i = 0; i < pb_reply.element_size(); i++) {
        r = PbToReply(pb_reply.element(i));
        if (r == nullptr) {
          break;
        }
        reply->element[i] = (redisReply *)r;
      }
    }
  }
  if (r == nullptr) {
    freeReplyObject(reply);
    return nullptr;
  }
  return reply;
}

/* Create a reply object */
static redisReply *createReplyObject(int type) {
  redisReply *r = (redisReply *)calloc(1,sizeof(*r));

  if (r == NULL)
    return NULL;

  r->type = type;
  return r;
}

static void *createStringObject(redisReply *reply, const char *str, size_t len) {
  char *buf;

  buf = (char *)malloc(len+1);
  if (buf == NULL) {
    return NULL;
  }

  /* Copy string value */
  memcpy(buf,str,len);
  buf[len] = '\0';
  reply->str = buf;
  reply->len = len;

  return reply;
}

static void *createArrayObject(redisReply *reply, int elements) {
  if (elements > 0) {
    reply->element = (redisReply **)calloc(elements, sizeof(redisReply*));
    if (reply->element == NULL) {
      return NULL;
    }
  }

  reply->elements = elements;

  return reply;
}

static void *createIntegerObject(redisReply *reply, long long value) {

  reply->integer = value;

  return reply;
}

};