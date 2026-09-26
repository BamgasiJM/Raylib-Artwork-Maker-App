#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define NUM_DOTS 20000           // 원점(Index 0) + 일반 점 19999개
#define CONNECT_RADIUS 40.0f  // 연결 가능 반경
#define MAX_CONNECTIONS 10000   // 생성 가능한 최대 선 수
#define GROW_SPEED 1.0f       // 선이 자라는 속도 (프레임당 비율)

// [점 크기 설정]
#define DOT_RADIUS 2.0f        // 일반 점 반지름
#define ORIGIN_RADIUS 4.0f     // 원점 반지름

// 점 데이터 구조
typedef struct {
    Vector2 position;
    Color color;
    bool isConnected;
    int stateTimer;            // 애니메이션 진행 타이머 (프레임 단위)
    int state;                 // 0: 대기, 1: 밝아짐, 2: 유지, 3: 어두워짐, 4: 완료
} Dot;

// 선 연결 데이터 구조
typedef struct {
    int fromIndex;
    int toIndex;
    float growth;              // 0.0f ~ 1.0f (성장 진행도)
    bool active;
    bool childTriggered;       // 자식 점 연쇄 연결 트리거 여부
} Connection;

// 글로벌 상태 변수
static Dot dots[NUM_DOTS];
static Connection connections[MAX_CONNECTIONS];
static int connectionCount = 0;
static bool isTriggered = false;

// 색상 정의
static const Color COLOR_BG = { 0, 0, 0, 255 };
static const Color COLOR_ORIGIN = { 240, 240, 240, 255 };
static const Color COLOR_DARK_DOT = { 10, 10, 10, 255 };

// 초기화 함수
void InitSimulation(void) {
    connectionCount = 0;
    isTriggered = false;

    // 원점 설정 (Center)
    dots[0].position = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    dots[0].color = COLOR_ORIGIN;
    dots[0].isConnected = true;
    dots[0].stateTimer = 0;
    dots[0].state = 0;

    // 랜덤 점 생성
    for (int i = 1; i < NUM_DOTS; i++) {
        dots[i].position = (Vector2){ (float)(rand() % SCREEN_WIDTH), (float)(rand() % SCREEN_HEIGHT) };
        dots[i].color = COLOR_DARK_DOT;
        dots[i].isConnected = false;
        dots[i].stateTimer = 0;
        dots[i].state = 0;
    }
}

// 특정 점에서 주변으로 선을 생성하는 로직
void ExpandConnectionsFrom(int parentIdx) {
    Vector2 parentPos = dots[parentIdx].position;

    if (parentIdx == 0) {
        // [원칙 1] 원점: 반경 내 모든 미연결 점으로 선 방출
        for (int i = 1; i < NUM_DOTS; i++) {
            if (!dots[i].isConnected) {
                float dist = Vector2Distance(parentPos, dots[i].position);
                if (dist <= CONNECT_RADIUS && connectionCount < MAX_CONNECTIONS) {
                    dots[i].isConnected = true;
                    connections[connectionCount++] = (Connection){
                        .fromIndex = parentIdx,
                        .toIndex = i,
                        .growth = 0.0f,
                        .active = true,
                        .childTriggered = false
                    };
                }
            }
        }
    } else {
        // [원칙 2] 일반 점: 반경 내 가장 가까운 미연결 점 1개로만 선 방출
        int closestIdx = -1;
        float minDist = FLT_MAX;

        for (int i = 1; i < NUM_DOTS; i++) {
            if (i == parentIdx || dots[i].isConnected) continue;

            float dist = Vector2Distance(parentPos, dots[i].position);
            if (dist <= CONNECT_RADIUS && dist < minDist) {
                minDist = dist;
                closestIdx = i;
            }
        }

        if (closestIdx != -1 && connectionCount < MAX_CONNECTIONS) {
            dots[closestIdx].isConnected = true;
            connections[connectionCount++] = (Connection){
                .fromIndex = parentIdx,
                .toIndex = closestIdx,
                .growth = 0.0f,
                .active = true,
                .childTriggered = false
            };
        }
    }
}

// 색상 보간 계산
Color ColorLerp(Color c1, Color c2, float factor) {
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;
    return (Color){
        (unsigned char)(c1.r + (c2.r - c1.r) * factor),
        (unsigned char)(c1.g + (c2.g - c1.g) * factor),
        (unsigned char)(c1.b + (c2.b - c1.b) * factor),
        (unsigned char)(c1.a + (c2.a - c1.a) * factor)
    };
}

// 점 색상 및 타이머 업데이트
void UpdateDotAnimation(Dot *dot) {
    if (dot->state == 0) return;

    dot->stateTimer++;

    if (dot->state == 1) { // 10프레임 동안 원점 색으로 밝아짐
        float t = (float)dot->stateTimer / 10.0f;
        dot->color = ColorLerp(COLOR_DARK_DOT, COLOR_ORIGIN, t);
        if (dot->stateTimer >= 10) {
            dot->state = 2;
            dot->stateTimer = 0;
        }
    }
    else if (dot->state == 2) { // 30프레임 동안 유지
        dot->color = COLOR_ORIGIN;
        if (dot->stateTimer >= 30) {
            dot->state = 3;
            dot->stateTimer = 0;
        }
    }
    else if (dot->state == 3) { // 10프레임 동안 원래 색으로 돌아감
        float t = (float)dot->stateTimer / 10.0f;
        dot->color = ColorLerp(COLOR_ORIGIN, COLOR_DARK_DOT, t);
        if (dot->stateTimer >= 10) {
            dot->state = 4;
            dot->color = COLOR_DARK_DOT;
        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Generative Line Network - Raylib 6.0");
    SetTargetFPS(30);

    InitSimulation();

    while (!WindowShouldClose()) {
        // [입력 처리] 스페이스바 트리거 또는 R키로 리셋
        if (IsKeyPressed(KEY_SPACE) && !isTriggered) {
            isTriggered = true;
            ExpandConnectionsFrom(0);
        }
        if (IsKeyPressed(KEY_R)) {
            InitSimulation();
        }

        // [상태 업데이트]
        for (int i = 0; i < connectionCount; i++) {
            if (!connections[i].active) continue;

            // 1. 선 성장
            if (connections[i].growth < 1.0f) {
                connections[i].growth += GROW_SPEED;
                if (connections[i].growth >= 1.0f) {
                    connections[i].growth = 1.0f;

                    // 도달한 점 애니메이션 시작
                    int targetIdx = connections[i].toIndex;
                    dots[targetIdx].state = 1;
                    dots[targetIdx].stateTimer = 0;
                }
            }
            // 2. 선이 끝까지 닿은 후 연쇄 연결 방출
            else if (!connections[i].childTriggered) {
                connections[i].childTriggered = true;
                ExpandConnectionsFrom(connections[i].toIndex);
            }
        }

        // 점 애니메이션 업데이트
        for (int i = 1; i < NUM_DOTS; i++) {
            UpdateDotAnimation(&dots[i]);
        }

        // [렌더링 레이어 순서]
        BeginDrawing();
            ClearBackground(COLOR_BG);

            // 레이어 1: 일반 점 (가장 아래에 배치하여 선이 위에 그려지도록 함)
            for (int i = 1; i < NUM_DOTS; i++) {
                DrawCircleV(dots[i].position, DOT_RADIUS, dots[i].color);
            }

            // 레이어 2: 선 (점 위를 덮으며 연출)
            for (int i = 0; i < connectionCount; i++) {
                Vector2 start = dots[connections[i].fromIndex].position;
                Vector2 endTarget = dots[connections[i].toIndex].position;
                Vector2 currentEnd = Vector2Lerp(start, endTarget, connections[i].growth);

                DrawLineEx(start, currentEnd, 1.0f, (Color){ 180, 180, 180, 150 });
            }

            // 레이어 3: 원점 (선보다도 최상단에 표출)
            DrawCircleV(dots[0].position, ORIGIN_RADIUS, dots[0].color);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
