#include "raylib.h"
#include "raymath.h"


// ─────────────────────────────────────
// Global Parameters
// ─────────────────────────────────────

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 1000;

// Grid
const int GRID_COLS = 20;
const int GRID_ROWS = 20;

// Rectangle
const float RECT_SIZE = 30.0f;
const float RECT_ANGLE = 45.0f;

// Color
const Color BACKGROUND_COLOR = {0, 0, 0, 255};
const Color RECT_DARK = {255, 255, 255, 10};
const Color RECT_BRIGHT = {255, 255, 255, 210};

// Mouse influence
const float INFLUENCE_RADIUS = 150.0f;

// ─────────────────────────────────────
// Main
// ─────────────────────────────────────

int main(void)
{
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Rectangle Grid");

  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    Vector2 mouse = GetMousePosition();

    BeginDrawing();

    ClearBackground(BACKGROUND_COLOR);

    // Grid Spacing
    float cellWidth = (float)WINDOW_WIDTH / GRID_COLS;
    float cellHeight = (float)WINDOW_HEIGHT / GRID_ROWS;

    for (int row = 0; row < GRID_ROWS; row++)
    {
      for (int col = 0; col < GRID_COLS; col++)
      {
        // Rectangle center
        float x = col * cellWidth + cellWidth * 0.5f;
        float y = row * cellHeight + cellHeight * 0.5f;

        Vector2 center = {x, y};

        // Distance from mouse to rectangle center
        float distance = Vector2Distance(mouse, center);

        // Normalize distance
        float t = distance / INFLUENCE_RADIUS;

        // Clamp 0 ~ 1
        t = Clamp(t, 0.0f, 1.0f);

        // close -> far : 1 -> 0
        t = 1.0f - t;

        // Color interpolation
        Color color = ColorLerp(RECT_DARK, RECT_BRIGHT, t);

        // DrawRectanglePro uses the center as origin,
        // making rotation around the rectangle center possible.
        Rectangle rect = {x, y, RECT_SIZE, RECT_SIZE};

        Vector2 origin = { RECT_SIZE * 0.5f, RECT_SIZE * 0.5f };

        DrawRectanglePro(rect, origin, RECT_ANGLE, color);
      }
    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}