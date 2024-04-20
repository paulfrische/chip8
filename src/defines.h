#ifndef DEFINES
#define DEFINES

#include <stdint.h>
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

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int16_t s16;
typedef int8_t s8;
typedef int32_t s32;
typedef int64_t s64;

#endif
