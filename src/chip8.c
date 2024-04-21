#include "chip8.h"
#include "defines.h"
#define LOGGING
#include "src/util.h"
#include <byteswap.h>
#include <raylib/raygui.h>
#include <raylib/raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX(x, y) ((y) * WIDTH + (x))
#define NEXT c->pc += 2
#define V(N) (c->registers[(N)])

const u8 FONT_DATA[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef union Instruction {
  u16 inst;
  u8 bytes[2];
} Instruction;

typedef struct C8 {
  u8 memory[MEMORY];
  u16 stack[STACK];
  u16 stack_size;
  u8 registers[16];
  u8 screen[WIDTH * HEIGHT];
  u16 address;
  u16 pc;
  u8 delay_timer; // NOTE: timers tick at 60 hertz down to 0
  u8 sound_timer;
  u32 instruction_count;
} C8;

C8 *init_c8() {
  C8 *c = malloc(sizeof(C8));
  memset(c->memory, 0, MEMORY);
  memset(c->stack, 0, STACK * sizeof(c->stack[0]));
  memset(c->registers, 0, 16);
  memset(c->registers, 0, 16);
  memset(c->screen, 0, WIDTH * HEIGHT);
  c->address = 0;
  c->stack_size = 0;
  c->pc = PROGRAM_START;
  c->delay_timer = 0;
  c->sound_timer = 0;
  c->instruction_count = 0;

  memcpy(c->memory + FONT, FONT_DATA, sizeof(FONT_DATA) / sizeof(u8));

  return c;
}

void free_c8(C8 *c) { free(c); }

void draw_c8(C8 *c) {
  for (u8 x = 0; x < WIDTH; ++x) {
    for (u8 y = 0; y < HEIGHT; ++y) {
      DrawRectangle(x * RESOLUTION, y * RESOLUTION, RESOLUTION, RESOLUTION,
                    BLACK);
      if (c->screen[INDEX(x, y)] == 1) {
        DrawRectangle(x * RESOLUTION + 1, y * RESOLUTION + 1, RESOLUTION - 2,
                      RESOLUTION - 2, WHITE);
      }
    }
  }
#ifdef NDEBUG
  // TODO: raygui
  for (int i = 0; i < 16; i++) {
    char msg[0xFF];
    sprintf(msg, "V%x: 0x%x", i, V(i));
    DrawText(msg, 10, 10 + 14 * i, 12, RED);
  }

  char msg[0xFF];
  sprintf(msg, "I: 0x%x", c->address);
  DrawText(msg, 10, HEIGHT * RESOLUTION - 30, 12, RED);
  sprintf(msg, "instructions: %i", c->instruction_count);
  DrawText(msg, 10, HEIGHT * RESOLUTION - 40, 12, RED);
  sprintf(msg, "PC: %i", c->pc);
  DrawText(msg, 10, HEIGHT * RESOLUTION - 60, 12, RED);
#endif
}

void update_timers(C8 *c) {
  if (c->delay_timer > 0) {
    c->delay_timer--;
  }

  if (c->sound_timer > 0) {
    c->sound_timer--;
  }
}

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

void update_c8(C8 *c, u16 input) {
  // FETCH
  Instruction inst = *(Instruction *)(c->memory + c->pc);
  u8 NN = inst.bytes[1];
  u16 NNN = be16(inst.inst) & 0x0FFF; // last three nibbles
  NEXT;
  c->instruction_count += 1;

  // DECODE
  // evil bit fuckery
  u8 optcode = (inst.bytes[0] & 0xF0) >> 4; // first nibble
  u8 X = inst.bytes[0] & 0x0F;              // second nibble
  u8 Y = (NN & 0xF0) >> 4;                  // third nibble
  u8 N = NN & 0x0F;                         // fourth nibble

  switch (optcode) {
  case 0x0:
    switch (NN) {
    case 0xE0: // 00E0: clear screen
    {
      memset(c->screen, 0, WIDTH * HEIGHT);
      /* LOG("clear screen"); */
    } break;
    case 0xEE: // 00EE: return
    {
      /* LOG("return"); */
      c->pc = stack_pop(c);
    } break;
    default:
      ASSERT(
          false,
          "invalid instruction 0x%x (ran %i instructions, pc=%i, address=%i)",
          inst.inst, c->instruction_count, c->pc, c->address);
    }
    break;
  case 0x1: // 1nnn: jump
  {
    c->pc = NNN;
    /* LOG("jump to %x", NNN); */
  } break;
  case 0x2: // 2nnn: call
  {
    /* LOG("call %x", NNN); */
    stack_push(c, c->pc);
    c->pc = NNN;
  } break;
  case 0x3: // 3xnn: skip next opcode if vx == nn
  {
    /* LOG("skip next opcode if V%x (%x) == NN (%x)", X, V(X), NN); */
    if (V(X) == NN) {
      NEXT;
      /* LOG("skip"); */
    }
  } break;
  case 0x4: // 4xnn: skip next opcode if vx != nn
  {
    /* LOG("skip next opcode if V%x (%x) != NN (%x)", X, V(X), NN); */
    if (V(X) != NN) {
      /* LOG("skip"); */
      NEXT;
    }
  } break;
  case 0x5: // 5xy0: skip next opcode if vx == vy
  {
    /* LOG("skip next opcode if V%x (%x) == V%x (%x)", X, V(X), Y, V(Y)); */
    ASSERT(N == 0, "invalid instruction 0x%x", inst.inst);
    if (V(X) == V(Y)) {
      /* LOG("skip"); */
      NEXT;
    }
  } break;
  case 0x6: // 6xnn: set register
  {
    /* LOG("set V%x to %x", X, NN); */
    V(X) = NN;
  } break;
  case 0x7: // 7xnn: add to register
  {
    /* LOG("add %x to V%x", NN, V(X)); */
    V(X) += NN;
  } break;
  case 0x8: // register magic
  {
    switch (N) {
    case 0x0: // 8xy0: set vx to vy
    {
      /* LOG("set V%x (%x) to V%x (%x)", X, V(X), Y, V(Y)); */
      V(X) = V(Y);
    } break;
    case 0x1: // 8xy1: logical or
    {
      /* LOG("or V%x (%x) with V%x (%x)", X, V(X), Y, V(Y)); */
      V(X) |= V(Y);
    } break;
    case 0x2: // 8xy2: logical and
    {
      /* LOG("and V%x (%x) with V%x (%x)", X, V(X), Y, V(Y)); */
      V(X) &= V(Y);
    } break;
    case 0x3: // 8xy3: logical xor
    {
      /* LOG("xor V%x (%x) with V%x (%x)", X, V(X), Y, V(Y)); */
      V(X) ^= V(Y);
    } break;
    case 0x4: // 8xy4: add
    {
      /* LOG("add V%x (%x) to V%x (%x)", X, V(X), Y, V(Y)); */
      u16 result = V(X) + V(Y);
      V(X) = result;
      V(0xF) = result > U8MAX;
      /* LOG("VF = %x", V(0xF)); */
    } break;
    case 0x5: // 8xy5: subtract vx = vx - vy
    {
      u8 flag = V(X) >= V(Y);
      V(X) -= V(Y);
      V(0xF) = flag;
    } break;
    case 0x7: // 8xy7: subtract vx = vy - vx
    {
      LOG("V%x (%x) = V%x (%x) - V%x (%x) = %x", X, V(X), X, V(X), Y, V(Y),
          V(Y) - V(X));

      u8 flag = V(Y) >= V(X);
      V(X) = V(Y) - V(X);
      V(0xF) = flag;
      LOG("Vf = %x", V(0xF));

      /* LOG("Vf = %x", V(0xF)); */
    } break;
    case 0x6: // 8xy6: bitshift to the right
    {
      /* LOG("bitshift V%x (%x) right | store in V%x (%x)", Y, V(Y), X, V(X));
       */
      // TODO: impl quirks (original behaviour at the moment)
      V(X) = V(Y) >> 1;
      V(0xF) = V(Y) & 0x1;
    } break;
    case 0xE: // 8xyE: bitshift to the left
    {
      /* LOG("bitshift V%x (%x) left | store in V%x (%x)", Y, V(Y), X, V(X)); */
      V(X) = V(Y) << 1; // TODO: impl quirks
      V(0xF) = (V(Y) & 0x80) >> 7;
    } break;
    default:
      ASSERT(false, "invalid instruction 0x%x", inst.inst);
    }
  } break;
  case 0x9: // 9xy0: skip next opcode if vx != vy
  {
    /* LOG("skip next opcode if V%x (%x) != V%x (%x)", X, V(X), Y, V(Y)); */
    ASSERT(N == 0, "invalid instruction 0x%x", inst.inst);
    if (V(X) != V(Y)) {
      /* LOG("skip"); */
      NEXT;
    }
  } break;
  case 0xA: // Annn: set address register
  {
    /* LOG("set I to %x", NNN); */
    c->address = NNN;
  } break;
  case 0xD: // Dxyn: draw
  {
    /* LOG("draw"); */
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
  } break;
  case 0xF: // memory magic
  {
    switch (NN) {
    case 0x65: // Fx65: load registers v0 - vx from memory at i
    {
      /* LOG("load registers V0 to V%x from memory at I (%x)", X, c->address);
       */
      for (int i = 0; i <= X; i++) {
        V(i) = c->memory[c->address + i];
      }
    } break;
    case 0x55: // Fx55: write registers v0 - vx to memory at i
    {
      /* LOG("write registers V0 to V%x to memory at I (%x)", X, c->address); */
      for (int i = 0; i <= X; i++) {
        c->memory[c->address + i] = V(i);
      }
    } break;
    case 0x33: // Fx33: store binary coded decimal number from vx at I, I+1 &
               // I+2
    {
      /* LOG("store binary coded decimal of from V%x (0x%x | %i) at %x", X,
       * V(X), */
      /* V(X), c->address); */
      u8 num = V(X);
      u8 third = num % 10;
      num /= 10;
      u8 second = num % 10;
      num /= 10;
      u8 first = num % 10;
      c->memory[c->address + 0] = first;
      c->memory[c->address + 1] = second;
      c->memory[c->address + 2] = third;
    } break;
    case 0x1e: // Fx1E: add vx to I
    {
      /* LOG("add V%x (%x) to I (%x)", X, V(X), c->address); */
      c->address += V(X);
    } break;
    }
  } break;
  default:
    ASSERT(false, "unknown optcode %i", optcode);
  }
}

void load_rom(C8 *c, const char *fn) {
  FILE *fp = fopen(fn, "rb");
  ASSERT(fp != NULL, "file pointer to file %s is NULL", fn);

  fseek(fp, 0, SEEK_END);
  u16 size = ftell(fp);
  rewind(fp);

  fread(c->memory + PROGRAM_START, sizeof(u8), size, fp);

  fclose(fp);
}
