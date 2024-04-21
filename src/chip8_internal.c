#include "chip8_internal.h"
#include "raylib/raylib.h"
#include "src/chip8.h"
#include "src/util.h"
#include <string.h>

extern const u64 FONT_INDEX[0xF];

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

void emulate_alu(C8 *c, u8 opcode, u8 x, u8 y, u8 n, u8 nn, u16 nnn) {
  u8 flag;
  switch (n) {
  case 0x0: // 8xy0: set vx to vy
    V(x) = V(y);
    break;

  case 0x1: // 8xy1: logical or
    V(x) |= V(y);
    break;

  case 0x2: // 8xy2: logical and
    V(x) &= V(y);
    break;

  case 0x3: // 8xy3: logical xor
    V(x) ^= V(y);
    break;

  case 0x4: // 8xy4: add
    u16 result = V(x) + V(y);
    V(x) = result;
    V(0xF) = result > U8MAX;
    break;

  case 0x5: // 8xy5: subtract vx = vx - vy
    flag = V(x) >= V(y);
    V(x) -= V(y);
    V(0xF) = flag;
    break;

  case 0x7: // 8xy7: subtract vx = vy - vx
    flag = V(y) >= V(x);
    V(x) = V(y) - V(x);
    V(0xF) = flag;
    break;

  case 0x6: // 8xy6: bitshift to the right
            // TODO: impl quirks (original behaviour at the moment)
    flag = V(y) & 1;
    V(x) = V(y) >> 1;
    V(0xF) = flag;
    break;

  case 0xE: // 8xyE: bitshift to the left
    flag = (V(y) & 0x80) >> 7;
    V(x) = V(y) << 1; // TODO: impl quirks
    V(0xF) = flag;
    break;
  }
}

void emulate_ram(C8 *c, u8 opcode, u8 x, u8 y, u8 n, u8 nn, u16 nnn) {
  switch (nn) {
  case 0x07: // Fx07: set VX to value of delay timer
    V(x) = c->delay_timer;
    break;

  case 0xA: // Fx0A: block until key is pressed. key gets stored in VX
    for (int i = 0; i < 0x10; i++) {
      if (IsKeyPressed(KEYS[i])) {
        V(x) = i;
        goto found;
      }
    }
    c->pc -= 2;
  found:
    break;

  case 0x15: // Fx15: set the delay timer to VX
    c->delay_timer = V(x);
    break;

  case 0x18: // Fx18: set the sound timer to VX
    c->sound_timer = V(x);
    break;

  case 0x29: // Fx29: set I to xth character
    c->address = FONT + FONT_INDEX[x];
    break;

  case 0x55: // Fx55: write registers v0 - vx to memory at i
    memcpy(c->memory + c->address, c->registers, x + 1);
    break;

  case 0x65: // Fx65: load registers v0 - vx from memory at i
    memcpy(c->registers, c->memory + c->address, x + 1);
    break;

  case 0x33: // Fx33: store binary coded decimal number from vx at I, I+1 &
             // I+2
    u8 num = V(x);
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
    c->address += V(x);
    break;
  }
}

void draw(C8 *c, u8 opcode, u8 x, u8 y, u8 n, u8 nn, u16 nnn) {
  u16 nx = V(x) % WIDTH;
  u16 ny = V(y) % HEIGHT;
  V(0xF) = 0;
  for (int i = 0; i < n; i++) {
    u8 byte = c->memory[c->address + i];
    for (int b = 0; b < 8; b++) {
      u8 v = NTHBIT(byte, b);
      if (v == 1) {
        if (c->screen[INDEX(nx + b, ny + i)] == 1) {
          c->screen[INDEX(nx + b, ny + i)] = 0;
          V(0xF) = 1;
        } else {
          c->screen[INDEX(nx + b, ny + i)] = 1;
        }
      }
    }
  }
}
