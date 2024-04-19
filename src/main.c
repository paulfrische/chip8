#include <raylib/raylib.h>

#define WIDTH 0x3f
#define HEIGHT 0x1f

#define RESOLUTION 15

int main(void) {
  InitWindow(WIDTH * RESOLUTION, HEIGHT * RESOLUTION, "Chip8");

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawText("Hello World!", 10, 10, 20, LIGHTGRAY);

    EndDrawing();
  }

  return 0;
}
