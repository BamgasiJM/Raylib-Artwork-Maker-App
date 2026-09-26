#include "raylib.h"

int main(void)
{
  InitWindow(800, 600, "Raylib Generative Art");
  SetTargetFPS(60);
  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawCircle(400, 300, 100, RAYWHITE);
    EndDrawing();
  }
  CloseWindow();

  return 0;
}