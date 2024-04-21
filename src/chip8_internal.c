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
    DEBUG("set V%x (%x) to V%x (%x)", x, V(x), y, V(y));
    V(x) = V(y);
    break;

  case 0x1: // 8xy1: logical or
    DEBUG("V%x (%x) |= V%x (%x)", x, V(x), y, V(y));
    V(x) |= V(y);
    break;

  case 0x2: // 8xy2: logical and
    DEBUG("V%x (%x) &= V%x (%x)", x, V(x), y, V(y));
    V(x) &= V(y);
    break;

  case 0x3: // 8xy3: logical xor
    DEBUG("V%x (%x) ^= V%x (%x)", x, V(x), y, V(y));
    V(x) ^= V(y);
    break;

  case 0x4: // 8xy4: add
    DEBUG("V%x (%x) += V%x (%x)", x, V(x), y, V(y));
    u16 result = V(x) + V(y);
    V(x) = result;
    V(0xF) = result > U8MAX;
    break;

  case 0x5: // 8xy5: subtract vx = vx - vy
    DEBUG("V%x (%x) -= V%x (%x)", x, V(x), y, V(y));
    flag = V(x) >= V(y);
    V(x) -= V(y);
    V(0xF) = flag;
    break;

  case 0x7: // 8xy7: subtract vx = vy - vx
    DEBUG("V%x (%x) = V%x (%x) - V%x (%x)", x, V(x), y, V(y), x, V(x));
    flag = V(y) >= V(x);
    V(x) = V(y) - V(x);
    V(0xF) = flag;
    break;

  case 0x6: // 8xy6: bitshift to the right
            // TODO: impl quirks (original behaviour at the moment)
    DEBUG("V%x (%x) = V%x (%x) >> 1", x, V(x), y, V(y));
    flag = V(y) & 1;
    V(x) = V(y) >> 1;
    V(0xF) = flag;
    break;

  case 0xE: // 8xyE: bitshift to the left
    DEBUG("V%x (%x) = V%x (%x) << 1", x, V(x), y, V(y));
    flag = (V(y) & 0x80) >> 7;
    V(x) = V(y) << 1; // TODO: impl quirks
    V(0xF) = flag;
    break;
  }
}

void emulate_ram(C8 *c, u8 opcode, u8 x, u8 y, u8 n, u8 nn, u16 nnn) {
  switch (nn) {
  case 0x07: // Fx07: set VX to value of delay timer
    DEBUG("set V%x (%x) to delay timer (%x)", x, V(x), c->delay_timer);
    V(x) = c->delay_timer;
    break;

  case 0xA: // Fx0A: block until key is pressed. key gets stored in VX
    DEBUG("block until key is pressed - V%x (%x)", x, V(x));
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
    DEBUG("set delay timer (%x) to V%x (%x)", c->delay_timer, x, V(x));
    c->delay_timer = V(x);
    break;

  case 0x18: // Fx18: set the sound timer to VX
    DEBUG("set sound timer (%x) to V%x (%x)", c->delay_timer, x, V(x));
    c->sound_timer = V(x);
    break;

  case 0x29: // Fx29: set I to xth character
    DEBUG("set I (%x) to character no. V%x (%x)", c->address, x, V(x));
    c->address = FONT + FONT_INDEX[x];
    break;

  case 0x55: // Fx55: write registers v0 - vx to memory at i
    DEBUG("write registers V0 - V%x to 0x%x", x, c->address);
    memcpy(c->memory + c->address, c->registers, x + 1);
    break;

  case 0x65: // Fx65: load registers v0 - vx from memory at i
    DEBUG("load registers V0 - V%x from 0x%x", x, c->address);
    memcpy(c->registers, c->memory + c->address, x + 1);
    break;

  case 0x33: // Fx33: store binary coded decimal number from vx at I, I+1 &
             // I+2
    DEBUG("store binary coded decimal from V%x (%x - %i) at 0x%x", x, V(x),
          V(x), c->address);
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
    DEBUG("add V%x (%x) to I (%x)", x, V(x), c->address);
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
      u16 tx = nx + b;
      u16 ty = ny + i;
      if (v == 1 && tx < WIDTH && ty < HEIGHT && tx > 0 && ty > 0) {
        if (c->screen[INDEX(tx, ty)] == 1) {
          c->screen[INDEX(tx, ty)] = 0;
          V(0xF) = 1;
        } else {
          c->screen[INDEX(tx, ty)] = 1;
        }
      }
    }
  }
}
