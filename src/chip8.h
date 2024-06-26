#ifndef CHIP8
#define CHIP8

#define MEMORY 0x1000
#define STACK 0xFF
#define PROGRAM_START 0x200
#define FONT 0x50

#define WIDTH 64
#define HEIGHT 32

#define RESOLUTION 20

typedef struct C8 C8;
C8 *init_c8();
void free_c8(C8 *c);

void update_c8(C8 *c);
void update_timers(C8 *c);
void draw_c8(C8 *c);
void draw_debug_c8(C8 *c);

void load_rom(C8 *c, const char *fn);

#endif
