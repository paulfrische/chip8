#include "util.h"

typedef union U16 {
  u16 val;
  u8 bytes[2];
} U16;

u16 be16(u16 v) {
  U16 tmp = {.val = 1}; // TODO: faster endian check
  if (tmp.bytes[0] == 1) {
    U16 n = {.val = v};
    return (U16){.bytes = {n.bytes[1], n.bytes[0]}}.val;
  }
  return v;
}
