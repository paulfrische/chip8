#ifndef CHIP8
#define CHIP8

#define MEMORY 0x1000
#define STACK 0xFF

#define WIDTH 0x3f
#define HEIGHT 0x1f

#define RESOLUTION 15

typedef struct C8 C8;
C8 *init_c8();
void free_c8(C8 *c);

void draw_c8(C8 *c);
void draw_c8(C8 *c);

#endif
