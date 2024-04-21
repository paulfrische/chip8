#ifndef CHIP8_INTERNAL
#define CHIP8_INTERNAL

#include "src/chip8.h"
#include "src/defines.h"

#define INDEX(x, y) ((y) * WIDTH + (x))
#define NEXT c->pc += 2
#define V(N) (c->registers[(N)])

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

void stack_push(C8 *c, u16 val);
u16 stack_pop(C8 *c);

void emulate_alu(C8 *c, u8 opcode, u8 X, u8 Y, u8 N, u8 NN, u16 NNN);
void emulate_ram(C8 *c, u8 opcode, u8 X, u8 Y, u8 N, u8 NN, u16 NNN);
void draw(C8 *c, u8 opcode, u8 X, u8 Y, u8 N, u8 NN, u16 NNN);

#endif
