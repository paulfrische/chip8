#include "src/chip8.h"
#include "src/defines.h"
#include "src/util.h"
#include <raylib/raylib.h>

int main(int argc, char **argv) {
  ASSERT(argc == 2, "wrong usage");

  C8 *c8 = init_c8();

  load_rom(c8, argv[1]);

  InitWindow(WIDTH * RESOLUTION, HEIGHT * RESOLUTION, "Chip8");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();

    update_timers(c8);
    for (int i = 0; i < 10; i++) {
      /* if (IsKeyPressed(KEY_SPACE) || IsKeyPressedRepeat(KEY_SPACE)) */
      update_c8(c8);
    }

    draw_c8(c8);

    EndDrawing();
  }

  free_c8(c8);

  return 0;
}
