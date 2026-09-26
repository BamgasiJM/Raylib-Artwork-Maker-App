#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

// ===== 화면 및 환경 설정 =====
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define POINT_COUNT 200      // 각 라인을 구성하는 점의 개수
#define LINE_COUNT 40        // 라인의 개수
#define MAX_AMPLITUDE 280.0f // 최대 진폭 (화면 상단)

// ===== 점 구조체 =====
typedef struct
{
    Vector2 position; // 현재 위치
    Vector2 basePos;  // 기본 위치 (물결이 없을 때의 위치)
    float phase;      // 각 점의 위상 (시간에 따른 변화량)
} Point;

// ===== 라인 구조체 =====
typedef struct
{
    Point *points;            // 점들을 가리키는 포인터 (동적 배열)
    float yOffset;            // 라인의 수직 오프셋 (라인 간 간격)
    float amplitude;          // 각 라인의 진폭 (위치에 따라 점진적으로 변화)
    unsigned char brightness; // 밝기 (HSV에서 V 값, S=0 이므로 그레이스케일)
} Line;

// ===== 라인 초기화 함수 =====
void InitLine(Line *line, float yOffset, float amplitude, unsigned char brightness)
{
    // ---- 점 배열 동적 할당 ----
    line->points = (Point *)malloc(sizeof(Point) * POINT_COUNT);
    line->yOffset = yOffset;
    line->amplitude = amplitude;
    line->brightness = brightness;

    // ---- 각 점 초기화 ----
    for (int i = 0; i < POINT_COUNT; i++)
    {
        Point *p = &line->points[i];

        // x 좌표: 왼쪽에서 오른쪽으로 균등하게 배치
        p->basePos.x = (float)i / (POINT_COUNT - 1) * SCREEN_WIDTH;

        // y 좌표: 기본 위치는 라인의 yOffset
        p->basePos.y = yOffset;

        // 각 점의 위상: x 위치에 비례하여 위상 차이를 둠 (물결이 퍼져나가는 효과)
        p->phase = (float)i / POINT_COUNT * PI * 3.0f;

        // 초기 위치 설정
        p->position = p->basePos;
    }
}

// ===== 메모리 해제 함수 =====
void FreeLine(Line *line)
{
    free(line->points);
    line->points = NULL;
}

int main(void)
{
    // ===== 난수 시드 초기화 =====
    srand((unsigned int)time(NULL));

    // ===== 윈도우 초기화 =====
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wave Lines - Grayscale");
    SetTargetFPS(60);

    // ===== 라인 배열 동적 할당 =====
    Line *lines = (Line *)malloc(sizeof(Line) * LINE_COUNT);

    // ===== 각 라인 초기화 (모두 동일한 속도, 점진적 진폭 변화) =====
    float yStart = 50.0f;
    float yStep = (SCREEN_HEIGHT - 100.0f) / (LINE_COUNT - 1);

    for (int i = 0; i < LINE_COUNT; i++)
    {
        float yOffset = yStart + i * yStep;

        // ===== 진폭: 아래에서 위로 갈수록 커짐 =====
        // i가 0일 때 (화면 상단): 최대 진폭
        // i가 LINE_COUNT-1일 때 (화면 하단): 최소 진폭 (거의 0)
        float progress = 1.0f - (float)i / (LINE_COUNT - 1);      // 1.0 → 0.0 (상단→하단)
        float amplitude = MAX_AMPLITUDE * progress;               // 선형 변화
        // float amplitude = MAX_AMPLITUDE * progress * progress; // 제곱 (부드러운)
        // float amplitude = MAX_AMPLITUDE * powf(progress, 3);   // 세제곱 (더 부드러운)

        // ===== 밝기: 아래에서 위로 갈수록 밝아짐 =====
        unsigned char brightness = (unsigned char)(40 + progress * 215); // 40 ~ 255

        InitLine(&lines[i], yOffset, amplitude, brightness);
    }

    float time = 0.0f;
    float waveSpeed = 0.5f; // 모든 라인이 동일한 속도로 움직임

    // ===== 메인 게임 루프 =====
    while (!WindowShouldClose())
    {
        time += GetFrameTime() * 30.0f; // 부드러운 속도로 증가

        // ===== 업데이트 단계 =====
        for (int i = 0; i < LINE_COUNT; i++)
        {
            Line *line = &lines[i];

            for (int j = 0; j < POINT_COUNT; j++)
            {
                Point *p = &line->points[j];

                // ===== 모든 라인이 동일한 위상과 속도로 물결침 =====
                // 각 점의 위상 + 시간에 따른 변화 (모든 라인이 동일)
                float waveOffset = sinf(p->phase + time * waveSpeed * 0.05f) * line->amplitude;

                // ---- x 위치: 기본 위치 유지 ----
                p->position.x = p->basePos.x;

                // ---- y 위치: 기본 위치 + 물결 오프셋 ----
                p->position.y = p->basePos.y + waveOffset;
            }
        }

        // ===== 렌더링 단계 =====
        BeginDrawing();
        ClearBackground(BLACK);

        // ---- 각 라인 그리기 (선으로 연결) ----
        for (int i = 0; i < LINE_COUNT; i++)
        {
            Line *line = &lines[i];

            // 그레이스케일 색상 생성 (밝기만 다름)
            Color grayColor = {
                line->brightness,
                line->brightness,
                line->brightness,
                255};

            // ---- 점들을 선으로 연결 (두께는 모두 동일하게) ----
            for (int j = 0; j < POINT_COUNT - 1; j++)
            {
                Point *p1 = &line->points[j];
                Point *p2 = &line->points[j + 1];

                DrawLineEx(
                    p1->position,
                    p2->position,
                    3.0f, // 모든 라인 동일한 두께
                    grayColor);
            }
        }

        // ---- FPS 표시 ----
        DrawFPS(10, 10);

        EndDrawing();
    }

    // ===== 메모리 해제 =====
    for (int i = 0; i < LINE_COUNT; i++)
    {
        FreeLine(&lines[i]);
    }
    free(lines);

    CloseWindow();
    return 0;
}