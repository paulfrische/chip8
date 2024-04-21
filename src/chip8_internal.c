#include "chip8_internal.h"
#include "src/chip8.h"
#include "src/util.h"
#include <string.h>

void stack_push(C8 *c, u16 val) {
  ASSERT(c->stack_size < STACK, "stack overflow")
  c->stack[c->stack_size] = val;
  c->stack_size++;
}

u16 stack_pop(C8 *c) {
  ASSERT(c->stack_size > 0, "stack underflow");
  c->stack_size--;
  return c->stack[c->stack_size];
}

void emulate_alu(C8 *c, u8 opcode, u8 X, u8 Y, u8 N, u8 NN, u16 NNN) {
  u8 flag;
  switch (N) {
  case 0x0: // 8xy0: set vx to vy
    V(X) = V(Y);
    break;

  case 0x1: // 8xy1: logical or
    V(X) |= V(Y);
    break;

  case 0x2: // 8xy2: logical and
    V(X) &= V(Y);
    break;

  case 0x3: // 8xy3: logical xor
    V(X) ^= V(Y);
    break;

  case 0x4: // 8xy4: add
    u16 result = V(X) + V(Y);
    V(X) = result;
    V(0xF) = result > U8MAX;
    break;

  case 0x5: // 8xy5: subtract vx = vx - vy
    flag = V(X) >= V(Y);
    V(X) -= V(Y);
    V(0xF) = flag;
    break;

  case 0x7: // 8xy7: subtract vx = vy - vx
    flag = V(Y) >= V(X);
    V(X) = V(Y) - V(X);
    V(0xF) = flag;
    break;

  case 0x6: // 8xy6: bitshift to the right
            // TODO: impl quirks (original behaviour at the moment)
    flag = V(Y) & 1;
    V(X) = V(Y) >> 1;
    V(0xF) = flag;
    break;

  case 0xE: // 8xyE: bitshift to the left
    flag = (V(Y) & 0x80) >> 7;
    V(X) = V(Y) << 1; // TODO: impl quirks
    V(0xF) = flag;
    break;
  }
}

void emulate_ram(C8 *c, u8 opcode, u8 X, u8 Y, u8 N, u8 NN, u16 NNN) {
  switch (NN) {
  case 0x65: // Fx65: load registers v0 - vx from memory at i
    memcpy(c->registers, c->memory + c->address, X + 1);
    break;

  case 0x55: // Fx55: write registers v0 - vx to memory at i
    memcpy(c->memory + c->address, c->registers, X + 1);
    break;

  case 0x33: // Fx33: store binary coded decimal number from vx at I, I+1 &
             // I+2
    u8 num = V(X);
    u8 third = num % 10;
    num /= 10;
    u8 second = num % 10;
    num /= 10;
    u8 first = num % 10;
    c->memory[c->address + 0] = first;
    c->memory[c->address + 1] = second;
    c->memory[c->address + 2] = third;
    break;

  case 0x1e: // Fx1E: add vx to I
    c->address += V(X);
    break;
  }
}

void draw(C8 *c, u8 opcode, u8 X, u8 Y, u8 N, u8 NN, u16 NNN) {
  u16 x = V(X) % WIDTH;
  u16 y = V(Y) % HEIGHT;
  V(0xF) = 0;
  for (int i = 0; i < N; i++) {
    u8 byte = c->memory[c->address + i];
    for (int b = 0; b < 8; b++) {
      u8 v = NTHBIT(byte, b);
      if (v == 1) {
        if (c->screen[INDEX(x + b, y + i)] == 1) {
          c->screen[INDEX(x + b, y + i)] = 0;
          V(0xF) = 1;
        } else {
          c->screen[INDEX(x + b, y + i)] = 1;
        }
      }
    }
  }
}
