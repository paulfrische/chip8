#include "chip8.h"
#include "defines.h"
#include <byteswap.h>
#include <endian.h>
#include <raylib/raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX(x, y) ((y) * WIDTH + (x))

typedef struct C8 {
  u8 memory[MEMORY];
  u16 stack[STACK];
  u16 stack_size;
  u8 registers[16];
  u8 screen[WIDTH * HEIGHT]; // NOTE: memory waste
  u16 address; // NOTE: address register (`I`) is actually 12 bits long
  u16 pc;
  u8 delay_timer; // NOTE: timers tick at 60 hertz down to 0
  u8 sound_timer;
  u16 keys; // NOTE: bit field cause I am lazy
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

  return c;
}

void free_c8(C8 *c) { free(c); }

void draw_c8(C8 *c) {
  for (u8 x = 0; x < WIDTH; ++x) {
    for (u8 y = 0; y < HEIGHT; ++y) {
      if (c->screen[INDEX(x, y)] == 0) {
        DrawRectangle(x * RESOLUTION, y * RESOLUTION, RESOLUTION, RESOLUTION,
                      BLACK);
      } else {
        DrawRectangle(x * RESOLUTION, y * RESOLUTION, RESOLUTION, RESOLUTION,
                      WHITE);
      }
    }
  }
#ifndef NDEBUG
  for (int i = 0; i < 16; i++) {
    char msg[0xFF];
    sprintf(msg, "V%x: 0x%x", i, c->registers[i]);
    DrawText(msg, 10, 10 + 14 * i, 12, WHITE);
  }

  char msg[0xFF];
  sprintf(msg, "I: 0x%x", c->address);
  DrawText(msg, 10, HEIGHT * RESOLUTION - 30, 12, WHITE);
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
  c->keys = input;

  // FETCH
  u8 first = *(c->memory + c->pc++);
  u8 NN = *(c->memory + c->pc++);

  /* inst = htobe16(inst); */
  c->pc += 2;

  // DECODE
  // evil bit fuckery
  u8 optcode = (first & 0xF0) >> 4;   // first nibble
  u8 X = first & 0x0F;                // second nibble
  u8 Y = (NN & 0xF0) >> 4;            // third nibble
  u8 N = NN & 0x0F;                   // fourth nibble
  u16 NNN = *(u16 *)(char[2]){X, NN}; // last three nibbles
  /* printf("DEBUG N=%i\n", N); */

  switch (optcode) {
  case 0x0:
    switch (NN) {
    case 0xE0: // clear screen
      memset(c->screen, 0, WIDTH * HEIGHT);
      break;
    case 0xEE: // return
      c->pc = stack_pop(c);
      break;
    }
    break;
  case 0x1: // jump
    c->pc = NNN;
    break;
  case 0x2: // call
    stack_push(c, c->pc);
    c->pc = NNN;
    break;
  case 0x6: // set register
    c->registers[X] = NN;
    break;
  case 0x7: // add to register
    c->registers[X] += NN;
    break;
  case 0xA: // set address register
    c->address = NNN;
    printf("set address register to 0x%x\n", NNN);
    break;
  case 0xD: // draw
    /* printf("draw %i bytes\n", N); */
    u16 x = c->registers[X] % WIDTH;
    u16 y = c->registers[Y] % HEIGHT;
    c->registers[15] = 0;
    for (int i = 0; i < N; i++) {
      u8 byte = c->memory[c->address + i];
      /* printf("byte to draw: %i\n", byte); */
    }
    break;
  default:
    /* ASSERT(false, "unknown optcode %i", optcode); */
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
