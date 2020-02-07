//
// Created by liuping on 2020/2/2.
//

#pragma once

namespace sw {

namespace redis {
typedef struct redisReply {
  int type; /* REDIS_REPLY_* */
  long long integer; /* The integer when type is REDIS_REPLY_INTEGER */
  size_t len; /* Length of string */
  char *str; /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
  size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
  struct redisReply **element; /* elements vector for REDIS_REPLY_ARRAY */
} redisReply;

#define REDIS_REPLY_STRING 1
#define REDIS_REPLY_ARRAY 2
#define REDIS_REPLY_INTEGER 3
#define REDIS_REPLY_NIL 4
#define REDIS_REPLY_STATUS 5
#define REDIS_REPLY_ERROR 6

/* Free a reply object */
static void freeReplyObject(void *reply) {
  redisReply *r = (redisReply *) reply;
  size_t j;

  if (r == NULL)
    return;

  switch (r->type) {
    case REDIS_REPLY_INTEGER:break; /* Nothing to free */
    case REDIS_REPLY_ARRAY:
      if (r->element != NULL) {
        for (j = 0; j < r->elements; j++)
          if (r->element[j] != NULL)
            freeReplyObject(r->element[j]);
        free(r->element);
      }
      break;
    case REDIS_REPLY_ERROR:
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING:
      if (r->str != NULL)
        free(r->str);
      break;
  }
  free(r);
}

}
}