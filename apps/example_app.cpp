#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

int main(void)
{
  InitWindow(1080, 720, "imGui App Example");
  SetTargetFPS(60);

  rlImGuiSetup(true);

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    rlImGuiBegin();
    ImGui::ShowDemoWindow();
    rlImGuiEnd();

    EndDrawing();
  }

  rlImGuiShutdown();
  CloseWindow();

  return 0;
}
