#ifndef UTIL
#define UTIL

#include "src/defines.h"
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(x, ...)                                                         \
  if (!(x)) {                                                                  \
    printf(__VA_ARGS__);                                                       \
    exit(-1);                                                                  \
  };

#define LOG(...)                                                               \
  {                                                                            \
    printf("[LOG] %s:%i\t", __FILE__, __LINE__);                               \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }

#define NTHBIT(byte, n) ((byte) & (0x80 >> (n))) >> (7 - (n))

u16 be16(u16 v);

#endif
