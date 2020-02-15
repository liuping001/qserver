//
// Created by liuping on 2020/2/15.
//

#pragma once
#include "singleton.h"
/* rand example: guess the number */
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

struct Random {
  Random() {
    srand (time(NULL));
  }
  // [0,max)
  int RandOne(int max) {
    return rand() % max;
  }
};

using random_util = S<Random>;