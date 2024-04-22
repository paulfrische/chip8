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

  Shader shader = LoadShader(0, "resources/screen.glsl");
  RenderTexture2D target =
      LoadRenderTexture(WIDTH * RESOLUTION, HEIGHT * RESOLUTION);

  while (!WindowShouldClose()) {
    update_timers(c8);
    for (int i = 0; i < 10; i++) {
      update_c8(c8);
    }

    BeginTextureMode(target);
    BeginDrawing();
    draw_c8(c8);
    EndDrawing();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    BeginShaderMode(shader);
    DrawTextureRec(target.texture,
                   (Rectangle){0, 0, (float)target.texture.width,
                               (float)-target.texture.height},
                   (Vector2){0, 0}, WHITE);
    EndShaderMode();
    draw_debug_c8(c8);
    EndDrawing();
  }

  free_c8(c8);

  UnloadShader(shader);
  CloseWindow();

  return 0;
}
