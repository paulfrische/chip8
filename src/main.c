#include "src/chip8.h"
#include "src/defines.h"
#include <raylib/raylib.h>

int main(int argc, char **argv) {
  ASSERT(argc == 2, "wrong usage");

  C8 *c8 = init_c8();

  load_rom(c8, argv[1]);

  InitWindow(WIDTH * RESOLUTION, HEIGHT * RESOLUTION, "Chip8");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();

    u16 input = 0;
    input = input | ((char)IsKeyDown(KEY_X)) << 0;
    input = input | ((char)IsKeyDown(KEY_ONE)) << 1;
    input = input | ((char)IsKeyDown(KEY_TWO)) << 2;
    input = input | ((char)IsKeyDown(KEY_THREE)) << 3;
    input = input | ((char)IsKeyDown(KEY_Q)) << 4;
    input = input | ((char)IsKeyDown(KEY_W)) << 5;
    input = input | ((char)IsKeyDown(KEY_E)) << 6;
    input = input | ((char)IsKeyDown(KEY_A)) << 7;
    input = input | ((char)IsKeyDown(KEY_S)) << 8;
    input = input | ((char)IsKeyDown(KEY_D)) << 9;
    input = input | ((char)IsKeyDown(KEY_Z)) << 10;
    input = input | ((char)IsKeyDown(KEY_C)) << 11;
    input = input | ((char)IsKeyDown(KEY_FOUR)) << 12;
    input = input | ((char)IsKeyDown(KEY_R)) << 13;
    input = input | ((char)IsKeyDown(KEY_F)) << 14;
    input = input | ((char)IsKeyDown(KEY_V)) << 15;

    update_timers(c8);
    /* for (int i = 0; i < 12; i++) { */
    update_c8(c8, input);
    /* } */

    draw_c8(c8);

    EndDrawing();
  }

  free_c8(c8);

  return 0;
}
