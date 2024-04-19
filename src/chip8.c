#include "chip8.h"
#include "defines.h"
#include <raylib/raylib.h>
#include <stdlib.h>
#include <string.h>

#define INDEX(x, y) ((y) * WIDTH + (x))

typedef struct C8 {
  u8 memory[MEMORY];
  u8 stack[STACK];
  u8 registers[16];
  u8 screen[WIDTH * HEIGHT]; // NOTE: memory waste
  u16 address;     // NOTE: address register (`I`) is actually 12 bits long
  u32 delay_timer; // NOTE: timers tick at 60 hertz down to 0
  u32 sound_timer;
  u16 keys; // NOTE: bit field cause I am lazy
} C8;

C8 *init_c8() {
  C8 *c = malloc(sizeof(C8));
  memset(c->memory, 0, MEMORY);
  memset(c->stack, 0, STACK);
  memset(c->registers, 0, 16);
  memset(c->registers, 0, 16);
  memset(c->screen, 0, WIDTH * HEIGHT);
  c->address = 0;
  c->delay_timer = 0;
  c->sound_timer = 0;
  c->keys = 0;

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
}
