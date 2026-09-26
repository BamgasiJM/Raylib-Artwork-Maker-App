#include "raylib.h"      // raylib 그래픽 라이브러리 (창 생성, 그리기, 입력 등)
#include <math.h>        // 수학 함수 (sinf, cosf, sqrtf 등)
#include <stdlib.h>      // 표준 라이브러리 (malloc, free, rand, srand)
#include <time.h>        // 시간 함수 (time()으로 시드 설정)

// ===== 화면 및 환경 설정 상수 =====
#define SCREEN_WIDTH 800      // 창 가로 크기 (픽셀)
#define SCREEN_HEIGHT 800     // 창 세로 크기 (픽셀)
#define COLS 60               // 플로우 필드의 가로 그리드 개수 (60x60 격자)
#define ROWS 60               // 플로우 필드의 세로 그리드 개수
#define MAX_PARTICLES 10000   // 최대 파티클 개수 (10,000개)

// ===== 파티클 구조체 정의 =====
typedef struct
{
    Vector2 position;        // 현재 위치 (x, y)
    Vector2 velocity;        // 현재 속도 (x, y)
    Vector2 acceleration;    // 현재 가속도 (x, y) - 플로우 필드 방향으로 설정됨
    float maxSpeed;          // 최대 속도 제한값 (2.0 ~ 5.0)
    Color color;             // 파티클 색상 (네온 느낌의 녹색~청록색 계열)
} Particle;

// ===== 파티클 초기화 함수 =====
void InitParticle(Particle *p)
{
    // ---- 위치: 화면 내 랜덤 위치 ----
    p->position.x = (float)(rand() % SCREEN_WIDTH);
    p->position.y = (float)(rand() % SCREEN_HEIGHT);

    // ---- 속도와 가속도는 0으로 초기화 (플로우 필드가 움직임을 결정) ----
    p->velocity.x = 0.0f;
    p->velocity.y = 0.0f;
    p->acceleration.x = 0.0f;
    p->acceleration.y = 0.0f;

    // ---- 최대 속도: 2.0 ~ 5.0 사이 랜덤 (파티클마다 다른 속도감 부여) ----
    p->maxSpeed = (float)(rand() % 3 + 2); // rand()%3 = 0,1,2 → +2 = 2,3,4 → 실수 변환

    // ---- 네온 느낌의 색상: 초록색(100~255) + 청록색(100~255) 조합 ----
    p->color.r = 0;                                          // 빨간색 성분 없음 (순수 초록/청록 계열)
    p->color.g = (unsigned char)(rand() % 155 + 100);        // 녹색: 100~254
    p->color.b = (unsigned char)(rand() % 155 + 100);        // 파란색: 100~254
    p->color.a = 255;                                        // 불투명 (완전히 보임)
}

// ===== 메인 함수 =====
int main(void)
{
    // ===== 난수 시드 초기화 =====
    srand((unsigned int)time(NULL));   // 현재 시간으로 시드 설정 (매번 다른 패턴)

    // ===== raylib 윈도우 초기화 =====
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib - Pure C Flow Field");
    SetTargetFPS(60);    // 초당 60프레임 고정

    // ============================================================
    // 1. 파티클 배열 동적 할당 (힙 메모리)
    // ============================================================
    Particle *particles = (Particle *)malloc(sizeof(Particle) * MAX_PARTICLES);
    // 모든 파티클 초기화
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        InitParticle(&particles[i]);   // 각 파티클에 초기값 설정
    }

    // ============================================================
    // 2. 플로우 필드(그리드 벡터) 배열 정의
    // ============================================================
    Vector2 flowField[COLS][ROWS];     // 60x60 = 3600개의 방향 벡터를 저장할 2D 배열
    float cellWidth = (float)SCREEN_WIDTH / COLS;   // 각 셀의 가로 크기 (약 13.33픽셀)
    float cellHeight = (float)SCREEN_HEIGHT / ROWS; // 각 셀의 세로 크기 (약 13.33픽셀)

    float timeStep = 0.0f;   // 시간 누적 변수 (플로우 필드가 시간에 따라 변화하도록)

    // ============================================================
    // 메인 게임 루프
    // ============================================================
    while (!WindowShouldClose())   // 창 닫기 버튼 누를 때까지 반복
    {
        timeStep += 0.005f;        // 매 프레임마다 시간 증가 (0.005씩 → 부드러운 변화)

        // ============================================================
        // 1. 플로우 필드 업데이트 (사인/코사인 파형 조합)
        // ============================================================
        for (int x = 0; x < COLS; x++)
        {
            for (int y = 0; y < ROWS; y++)
            {
                // ---- 부드러운 곡선 흐름을 만들기 위한 각도 계산 ----
                // x, y 좌표에 0.1을 곱해 공간적 주기를 만들고, timeStep을 더해 시간에 따른 변화 추가
                // sin * cos 조합으로 물결 모양의 흐름장 생성
                float angle = sinf(x * 0.1f + timeStep) * cosf(y * 0.1f + timeStep) * PI * 2.0f;

                // ---- 각도를 단위 방향 벡터(cos, sin)로 변환 ----
                flowField[x][y].x = cosf(angle);   // x 방향 성분
                flowField[x][y].y = sinf(angle);   // y 방향 성분
            }
        }

        // ============================================================
        // 2. 파티클 물리 업데이트
        // ============================================================
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            Particle *p = &particles[i];   // 현재 파티클 포인터

            // ---- 현재 파티클 위치를 격자 인덱스로 변환 ----
            int cellX = (int)(p->position.x / cellWidth);   // 몇 번째 열인지
            int cellY = (int)(p->position.y / cellHeight);  // 몇 번째 행인지

            // ---- 화면 그리드 경계 내부인지 확인 후 가속도 설정 ----
            if (cellX >= 0 && cellX < COLS && cellY >= 0 && cellY < ROWS)
            {
                // 해당 셀의 플로우 필드 방향을 가속도로 설정
                p->acceleration = flowField[cellX][cellY];
            }

            // ---- 가속도를 속도에 누적 (F = ma, 여기서는 질량=1) ----
            p->velocity.x += p->acceleration.x * 0.5f;   // 0.5는 가속도 계수 (너무 빠르지 않게)
            p->velocity.y += p->acceleration.y * 0.5f;

            // ---- 최고 속도 제한 (클램핑) ----
            // 속도의 크기(스칼라) 계산: sqrt(vx² + vy²)
            float speed = sqrtf(p->velocity.x * p->velocity.x + p->velocity.y * p->velocity.y);
            if (speed > p->maxSpeed)   // 최대 속도 초과 시
            {
                // 방향은 유지하되 크기만 maxSpeed로 줄임 (정규화 후 곱하기)
                p->velocity.x = (p->velocity.x / speed) * p->maxSpeed;
                p->velocity.y = (p->velocity.y / speed) * p->maxSpeed;
            }

            // ---- 속도를 위치에 더해 이동 ----
            p->position.x += p->velocity.x;
            p->position.y += p->velocity.y;

            // ---- 화면 밖으로 나간 파티클은 재초기화 (새로운 위치에서 다시 시작) ----
            if (p->position.x < 0 || p->position.x > SCREEN_WIDTH ||
                p->position.y < 0 || p->position.y > SCREEN_HEIGHT)
            {
                InitParticle(p);   // 위치를 화면 내 랜덤 위치로 재설정
            }
        }

        // ============================================================
        // 3. 렌더링 (그리기)
        // ============================================================
        BeginDrawing();

        // ---- 잔상 효과: 완전히 지우지 않고 반투명 검은색 사각형으로 덮음 ----
        // 알파값 12로 설정 → 이전 프레임의 파티클이 서서히 사라지는 잔상 효과
        Color fadeColor = {0, 0, 0, 12};   // R=0, G=0, B=0, A=12 (매우 투명한 검은색)
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, fadeColor);

        // ---- 모든 파티클 그리기 ----
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            // 반지름 1.2픽셀의 작은 원으로 파티클을 그림 (색상은 각 파티클의 color 사용)
            DrawCircleV(particles[i].position, 1.2f, particles[i].color);
        }

        // ---- 상태 정보 출력 (FPS) ----
        DrawFPS(10, 10);   // 왼쪽 상단에 초당 프레임 수 표시

        EndDrawing();   // 그리기 종료
    }

    // ============================================================
    // 메모리 해제 및 프로그램 종료
    // ============================================================
    free(particles);   // 동적 할당한 파티클 배열 메모리 해제
    CloseWindow();     // raylib 윈도우 닫기

    return 0;   // 정상 종료
}