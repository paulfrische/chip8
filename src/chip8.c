#include "chip8.h"
#include "defines.h"
#include "src/util.h"
#include <byteswap.h>
#include <raylib/raygui.h>
#include <raylib/raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX(x, y) ((y) * WIDTH + (x))

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
  u16 keys; // NOTE: bit field cause I am lazy
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
  c->keys = 0;
  c->instruction_count = 0;

  const u8 font[] = {
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
  memcpy(c->memory + FONT, font, sizeof(font) / sizeof(u8));

  LOG("finished initing ch8");

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
    sprintf(msg, "V%x: 0x%x", i, c->registers[i]);
    DrawText(msg, 10, 10 + 14 * i, 12, WHITE);
  }

  char msg[0xFF];
  sprintf(msg, "I: 0x%x", c->address);
  DrawText(msg, 10, HEIGHT * RESOLUTION - 30, 12, WHITE);
  sprintf(msg, "instructions: %i", c->instruction_count);
  DrawText(msg, 10, HEIGHT * RESOLUTION - 40, 12, WHITE);
  sprintf(msg, "PC: %i", c->pc);
  DrawText(msg, 10, HEIGHT * RESOLUTION - 60, 12, WHITE);
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
  LOG("push %i onto stack", val);
  ASSERT(c->stack_size < STACK, "stack overflow")
  c->stack[c->stack_size] = val;
  c->stack_size++;
}

u16 stack_pop(C8 *c) {
  LOG("pop stack");
  ASSERT(c->stack_size > 0, "stack underflow");
  c->stack_size--;
  return c->stack[c->stack_size];
}

void update_c8(C8 *c, u16 input) { // BUG: big clusterfuck everywhere
  c->keys = input;

  // FETCH
  Instruction inst = *(Instruction *)(c->memory + c->pc);
  u8 NN = inst.bytes[1];
  u16 NNN = be16(inst.inst) & 0x0FFF; // last three nibbles
  c->pc += 2;
  c->instruction_count += 1;

  // DECODE
  // evil bit fuckery
  u8 optcode = (inst.bytes[0] & 0xF0) >> 4; // first nibble
  u8 X = inst.bytes[0] & 0x0F;              // second nibble
  u8 Y = (NN & 0xF0) >> 4;                  // third nibble
  u8 N = NN & 0x0F;                         // fourth nibble

  LOG("inst=0x%x first=0x%x NN=0x%x optcode=0x%x X=0x%x, Y=0x%x, N=0x%x, "
      "NNN=0x%x",
      inst.inst, inst.bytes[0], NN, optcode, X, Y, N, NNN);

  switch (optcode) {
  case 0x0:
    switch (NN) {
    case 0xE0: // clear screen
    {
      LOG("clear screen");
      memset(c->screen, 0, WIDTH * HEIGHT);
      LOG("clearing screen");
      break;
    }
    case 0xEE: // return
    {
      LOG("return");
      c->pc = stack_pop(c);
      break;
    }
    default:
      ASSERT(
          false,
          "invalid instruction 0x%x (ran %i instructions, pc=%i, address=%i)",
          inst.inst, c->instruction_count, c->pc, c->address);
    }
    break;
  case 0x1: // jump
  {
    LOG("jumping to 0x%x", NNN);
    c->pc = NNN;
  } break;
  case 0x2: // call
  {
    LOG("call");
    stack_push(c, c->pc);
    c->pc = NNN;
  } break;
  case 0x3: // 3xnn: skip next opcode if vx == nn
  {
    if (c->registers[X] == NN) {
      c->pc += 2;
    }
  } break;
  case 0x4: // 4xnn: skip next opcode if vx != nn
  {
    if (c->registers[X] != NN) {
      c->pc += 2;
    }
  } break;
  case 0x5: // skip next opcode if vx == vy
  {
    ASSERT(N == 0, "invalid instruction 0x%x", inst.inst);
    if (c->registers[X] == c->registers[Y]) {
      c->pc += 2;
    }
  } break;
  case 0x6: // set register
  {
    LOG("set V%i to 0x%x", X, NN);
    c->registers[X] = NN;
  } break;
  case 0x7: // add to register
  {
    LOG("add %x to register V%x", NN, X);
    c->registers[X] += NN;
  } break;
  case 0x8: //
  {
    switch (N) {
    case 0x0: // set vx to vy
    {
      c->registers[X] = c->registers[Y];
    } break;
    case 0x1: // logical or
    {
      c->registers[X] |= c->registers[Y];
    } break;
    case 0x2: // logical and
    {
      c->registers[X] &= c->registers[Y];
    } break;
    case 0x3: // logical xor
    {
      c->registers[X] ^= c->registers[Y];
    } break;
    case 0x4: // add
    {
      c->registers[X] += c->registers[Y];
    } break;
    case 0x5: // subtract
    {
      c->registers[X] -= c->registers[Y];
    } break;
    case 0x6: // bitshift to the right
    {
      // TODO: impl quirks (original behaviour at the moment)
      c->registers[X] = c->registers[Y] >> 1;
      c->registers[15] = c->registers[Y] & 0x1;
    } break;
    case 0xE: // bitshift to the left
    {
      c->registers[X] = c->registers[Y] << 1; // TODO: impl quirks
      c->registers[15] = c->registers[Y] & 0x80;
    } break;
    case 0x7: // set vx to vy
    {
      c->registers[X] = c->registers[Y] - c->registers[X];
    } break;
    }
  } break;
  case 0x9: // skip next opcode if vx != vy
  {
    ASSERT(N == 0, "invalid instruction 0x%x", inst.inst);
    if (c->registers[X] != c->registers[Y]) {
      c->pc += 2;
    }
  } break;
  case 0xA: // set address register
    LOG("set A to %x", NNN);
    c->address = NNN;
    break;
  case 0xD: // draw
  {
    LOG("draw");
    u16 x = c->registers[X] % WIDTH;
    u16 y = c->registers[Y] % HEIGHT;
    c->registers[15] = 0;
    for (int i = 0; i < N; i++) {
      u8 byte = c->memory[c->address + i];
      /* LOG("byte to draw: %b", byte); */
      for (int b = 0; b < 8; b++) {
        u8 v = NTHBIT(byte, b);
        if (v == 1 && c->screen[INDEX(x + b, y + i)] == 1) {
          c->screen[INDEX(x + b, y + i)] = 0;
          c->registers[15] = 1;
        } else {
          c->screen[INDEX(x + b, y + i)] = v;
        }
      }
    }
  } break;
  case 0xF: // memory magic
  {
    switch (NN) {
    case 0x65: // load registers v0 - vx from memory at i
    {
      for (int i = 0; i <= X; i++) {
        c->registers[i] = *(c->memory + c->address + i);
      }
    } break;
    case 0x55: // load registers v0 - vx from memory at i
    {
      for (int i = 0; i <= X; i++) {
        *(c->memory + c->address + i) = c->registers[i];
      }
    } break;
    case 0x33: // store decimal coded number from vx at I, I+1 & I+2
    {
      u8 num = c->registers[X];
      u8 third = num % 10;
      num /= 10;
      u8 second = num % 10;
      num /= 10;
      u8 first = num % 10;
      num /= 10;
      c->memory[c->address + 0] = first;
      c->memory[c->address + 1] = second;
      c->memory[c->address + 2] = third;
    } break;
    case 0x1e: // add vx to I
    {
      c->address += c->registers[X];
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
