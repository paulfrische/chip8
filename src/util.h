#ifndef UTIL
#define UTIL

#include "src/defines.h"
#include <stdio.h>
#include <stdlib.h>

#define LEVEL 0

#define ASSERT(x, ...)                                                         \
  if (!(x)) {                                                                  \
    printf("%s:%i", __FILE__, __LINE__);                                       \
    printf(__VA_ARGS__);                                                       \
    exit(-1);                                                                  \
  };

#define ERROR(...)                                                             \
  if (LEVEL >= 0) {                                                            \
    printf("[ERROR] %s:%i\t", __FILE__, __LINE__);                             \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#define INFO(...)                                                              \
  if (LEVEL >= 1) {                                                            \
    printf("[INFO] %s:%i\t", __FILE__, __LINE__);                              \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#define DEBUG(...)                                                             \
  if (LEVEL >= 2) {                                                            \
    printf("[DEBUG] %s:%i\t", __FILE__, __LINE__);                             \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }

#define NTHBIT(byte, n) ((byte) & (0x80 >> (n))) >> (7 - (n))

u16 be16(u16 v);

#endif
