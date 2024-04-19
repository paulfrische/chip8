#include "src/chip8.h"
#include <raylib/raylib.h>

int main(void) {
  C8 *c8 = init_c8();

  InitWindow(WIDTH * RESOLUTION, HEIGHT * RESOLUTION, "Chip8");

  while (!WindowShouldClose()) {
    BeginDrawing();

    draw_c8(c8);

    EndDrawing();
  }

  free_c8(c8);

  return 0;
}
