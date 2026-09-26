#include "raylib.h"
#include <math.h>

#define WIDTH 600
#define HEIGHT 600
#define SQUARE_SIZE 200

// RGB 값을 Color 구조체로 변환
Color RGB(unsigned char r, unsigned char g, unsigned char b) {
    return (Color){r, g, b, 255};
}

// HSV를 RGB로 변환 (0~1 범위)
Color HSVtoRGB(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h * 6, 2) - 1));
    float m = v - c;

    float r = 0, g = 0, b = 0;
    if (h < 1.0f/6) { r = c; g = x; b = 0; }
    else if (h < 2.0f/6) { r = x; g = c; b = 0; }
    else if (h < 3.0f/6) { r = 0; g = c; b = x; }
    else if (h < 4.0f/6) { r = 0; g = x; b = c; }
    else if (h < 5.0f/6) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return RGB((unsigned char)((r+m)*255),
               (unsigned char)((g+m)*255),
               (unsigned char)((b+m)*255));
}

int main(void) {
    InitWindow(WIDTH, HEIGHT, "Rotating Square");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // 시간 경과 (초 단위)
        float time = (float)GetTime();

        // 회전 각도 (1초에 60도 회전)
        float rotation = time * 30.0f;

        // Hue 값 변화 (4초마다 한 바퀴)
        float hue = fmod(time / 4.0f, 1.0f);

        // HSV 색상 계산 (S=1.0, V=1.0)
        Color squareColor = HSVtoRGB(hue, 1.0f, 1.0f);

        BeginDrawing();
        ClearBackground(RGB(20, 20, 25));

        // 화면 중앙 좌표
        float centerX = WIDTH / 2.0f;
        float centerY = HEIGHT / 2.0f;

        // 회전된 정사각형 그리기
        DrawRectanglePro(
            (Rectangle){centerX, centerY, SQUARE_SIZE, SQUARE_SIZE},
            (Vector2){SQUARE_SIZE / 2.0f, SQUARE_SIZE / 2.0f},  // 회전 중심
            rotation,
            squareColor
        );

        EndDrawing();
    }

    CloseWindow();
    return 0;
}