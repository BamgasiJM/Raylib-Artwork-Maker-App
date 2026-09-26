#include "raylib.h"

int main(void)
{
    const int screenWidth = 1000;
    const int screenHeight = 1000;

    InitWindow(screenWidth, screenHeight, "raylib GLSL shader");

    SetTargetFPS(60);

    Shader shader = LoadShader(
        "shaders/basic/vert.glsl",
        "shaders/basic/frag.glsl");

    int timeLoc = GetShaderLocation(shader, "u_time");
    int resolutionLoc = GetShaderLocation(shader, "u_resolution");

    Vector2 resolution = {(float)screenWidth, (float)screenHeight};

    while (!WindowShouldClose())
    {
        float time = (float)GetTime();

        SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, resolutionLoc, &resolution, SHADER_UNIFORM_VEC2);

        BeginDrawing();

        ClearBackground(BLACK);
        BeginShaderMode(shader);
        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
        EndShaderMode();

        EndDrawing();
    }
    UnloadShader(shader);
    CloseWindow();

    return 0;
}