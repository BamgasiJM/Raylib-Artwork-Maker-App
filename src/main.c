#include "raylib.h"

int main(void)
{
  InitWindow(1080, 1080, "Shader Test");
  Shader shader = LoadShader("shaders/basic/vert.glsl", "shaders/basic/frag.glsl");
  int timeLoc = GetShaderLocation(shader, "u_time");

  while (!WindowShouldClose())
  {
    float time = GetTime();
    SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

    BeginDrawing();
    ClearBackground(BLACK);

    BeginShaderMode(shader);

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);

    EndShaderMode();
    EndDrawing();
  }
  UnloadShader(shader);
  CloseWindow();

  return 0;
}