#include "chip8.h"
#include "chip8_internal.h"
#include "defines.h"
#include "src/util.h"

#include <raylib/raygui.h>
#include <raylib/raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef union Instruction {
  u16 inst;
  u8 bytes[2];
} Instruction;

extern const u8 FONT_DATA[];
extern const u64 FONT_SIZE;

C8 *init_c8() {
  // NOTE: do array members always get initialized to 0s?
  C8 *c = malloc(sizeof(C8));
  *c = (C8){
      .address = 0,
      .stack_size = 0,
      .pc = PROGRAM_START,
      .delay_timer = 0,
      .sound_timer = 0,
      .instruction_count = 0,
  };

  memcpy(c->memory + FONT, FONT_DATA, FONT_SIZE / sizeof(u8));

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

#ifndef NDEBUG
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

  if (c->sound_timer > 0) { // TODO: play sound as long as > 0
    c->sound_timer--;
  }
}

void update_c8(C8 *c) {
  if (c->pc % 2 != 0)
    ERROR("program counter is broken");
  // FETCH
  Instruction inst = *(Instruction *)(c->memory + c->pc);
  u8 nn = inst.bytes[1];
  u16 nnn = be16(inst.inst) & 0x0FFF; // last three nibbles
  NEXT;
  c->instruction_count += 1;

  // DECODE
  // evil bit *magic*
  u8 opcode = (inst.bytes[0] & 0xF0) >> 4; // first nibble
  u8 x = inst.bytes[0] & 0x0F;             // second nibble
  u8 y = (nn & 0xF0) >> 4;                 // third nibble
  u8 n = nn & 0x0F;                        // fourth nibble

  switch (opcode) {

  case 0x0:
    switch (nn) {

    case 0xE0: // 00E0: clear screen
      DEBUG("clear screen");
      memset(c->screen, 0, WIDTH * HEIGHT);
      break;

    case 0xEE: // 00EE: return
      DEBUG("return")
      c->pc = stack_pop(c);
      break;

    default:
      ASSERT(
          false,
          "invalid instruction 0x%x (ran %i instructions, pc=%i, address=%i)",
          inst.inst, c->instruction_count, c->pc, c->address);
    }
    break;

  case 0x1: // 1nnn: jump
    DEBUG("jump to %x", nnn);
    c->pc = nnn;
    break;

  case 0x2: // 2nnn: call
    DEBUG("call %x", nnn);
    stack_push(c, c->pc);
    c->pc = nnn;
    break;

  case 0x3: // 3xnn: skip next opcode if vx == nn
    DEBUG("skip if V%x (%x) == %x", x, V(x), nn);
    if (V(x) == nn) {
      DEBUG("skip")
      NEXT;
    }
    break;

  case 0x4: // 4xnn: skip next opcode if vx != nn
    DEBUG("skip if V%x (%x) != %x", x, V(x), nn);
    if (V(x) != nn) {
      DEBUG("skip")
      NEXT;
    }
    break;

  case 0x5: // 5xy0: skip next opcode if vx == vy
    DEBUG("skip if V%x (%x) == V%x (%x)", x, V(x), y, V(y));
    ASSERT(n == 0, "invalid instruction 0x%x", inst.inst);
    if (V(x) == V(y)) {
      DEBUG("skip");
      NEXT;
    }
    break;

  case 0x6: // 6xnn: set vx to nn
    DEBUG("set V%x (%x) to %x", x, V(x), nn);
    V(x) = nn;
    break;

  case 0x7: // 7xnn: add to register
    DEBUG("add %x V%x (%x)", nn, x, V(x));
    V(x) += nn;
    break;

  case 0x8: // register magic
    emulate_alu(c, opcode, x, y, n, nn, nnn);
    break;

  case 0x9: // 9xy0: skip next opcode if vx != vy
    DEBUG("skip if V%x (%x) != V%x (%x)", x, V(x), y, V(y));
    ASSERT(n == 0, "invalid instruction 0x%x", inst.inst);
    if (V(x) != V(y)) {
      DEBUG("skip");
      NEXT;
    }
    break;

  case 0xA: // Annn: set address register
    DEBUG("set I to %x", nnn);
    c->address = nnn;
    break;

  case 0xB: // Bnnn: relative jump
    DEBUG("relative jump to NNN (%x) + V%x (%x) = %x", nnn, x, V(x),
          nnn + V(x));
    c->pc = nnn + V(0x0);
    break;

  case 0xC: // Cxnn: random number in VX with mask nn
    srand(time(NULL));
    u8 r = (rand() / RAND_MAX) * U8MAX;
    DEBUG("set V%x (%x) to random number %x", x, V(x), r & nn);
    V(x) = r & nn;

  case 0xD: // Dxyn: draw
    DEBUG("draw");
    draw(c, opcode, x, y, n, nn, nnn);
    break;

  case 0xE: // skip if key
    switch (nn) {
    case 0x9E: // Ex9E: skip if key in VX is down
      DEBUG("check if key %x is down", V(x));
      if (IsKeyDown(KEYS[V(x)])) {
        NEXT;
      }
      break;

    case 0xA1: // ExA1: skip if key in VX is *not* down
      DEBUG("check if key %x is up", V(x));
      if (!IsKeyDown(KEYS[V(x)])) {
        NEXT;
      }
      break;
    }

    break;

  case 0xF: // memory magic
    emulate_ram(c, opcode, x, y, n, nn, nnn);
    break;

  default:
    ASSERT(false, "unknown opcode %i", opcode);
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
