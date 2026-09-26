// ============================================================
// Particle Flow Field Tuner with ImGui
// ============================================================
// 이 프로그램은 raylib과 Dear ImGui(rlImGui 백엔드)를 사용하여
// 파티클이 플로우 필드를 따라 움직이는 시각화를 만듭니다.
// ImGui 패널로 파라미터를 실시간 조정할 수 있습니다.
// ============================================================

#include "raylib.h"      // raylib: 그래픽, 윈도우, 렌더링 기능
#include "rlImGui.h"     // rlImGui: raylib과 ImGui를 연결하는 브리지
#include "imgui.h"       // Dear ImGui: GUI 위젯 라이브러리 (C++)
#include <math.h>        // 수학 함수: sinf, cosf, sqrtf 등
#include <stdlib.h>      // 표준 라이브러리: malloc, free, rand 등
#include <time.h>        // 시간 함수: time() 난수 시드 초기화용

// ============================================================
// 상수 정의
// ============================================================
#define SCREEN_WIDTH 1000    // 화면 가로 크기 (픽셀)
#define SCREEN_HEIGHT 700    // 화면 세로 크기 (픽셀)
#define COLS 60              // 플로우 필드 그리드 가로 개수
#define ROWS 60              // 플로우 필드 그리드 세로 개수
#define MAX_PARTICLES 5000   // 동시에 관리할 최대 파티클 개수

// ============================================================
// 파티클 구조체
// ============================================================
// 각 파티클(점)은 다음 속성을 갖습니다:
typedef struct
{
    Vector2 position;        // 현재 위치 (x, y 좌표)
    Vector2 velocity;        // 현재 속도 (매 프레임 위치 변화량)
    Vector2 acceleration;    // 현재 가속도 (플로우 필드에서 받는 힘)
    float maxSpeed;          // 최대 속도 제한값 (너무 빨라지는 것을 방지)
    Color color;             // 파티클의 그리기 색상 (R, G, B, A)
} Particle;

// ============================================================
// 파티클 초기화 함수
// ============================================================
// 새로운 파티클 또는 화면을 벗어난 파티클을 초기 상태로 되돌립니다.
void InitParticle(Particle *p)
{
    // 위치: 화면 내 랜덤 좌표
    p->position.x = (float)(rand() % SCREEN_WIDTH);
    p->position.y = (float)(rand() % SCREEN_HEIGHT);

    // 처음에는 움직이지 않음 (플로우 필드가 움직임을 결정)
    p->velocity.x = 0.0f;
    p->velocity.y = 0.0f;
    p->acceleration.x = 0.0f;
    p->acceleration.y = 0.0f;

    // 최대 속도 설정
    p->maxSpeed = 2.5f;

    // 색상: 청록색 계열 (네온 느낌)
    // R=0: 빨간색 성분 없음
    // G, B: 각각 100~254 범위에서 랜덤
    p->color.r = 0;
    p->color.g = (unsigned char)(rand() % 155 + 100);
    p->color.b = (unsigned char)(rand() % 155 + 100);
    p->color.a = 255;  // 완전 불투명
}

// ============================================================
// 메인 함수
// ============================================================
int main(void)
{
    // 난수 생성기 초기화 (매번 다른 패턴 만들기)
    srand((unsigned int)time(NULL));

    // ============================================================
    // 1. raylib 초기화
    // ============================================================
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Particle Flow Field Tuner");
    SetTargetFPS(60);  // 초당 60프레임 고정

    // ImGui 초기화 (어두운 테마 사용)
    rlImGuiSetup(true);

    // ============================================================
    // 2. 파티클 배열 생성 및 초기화
    // ============================================================
    // MAX_PARTICLES개의 파티클을 힙 메모리에 할당
    Particle *particles = (Particle *)malloc(sizeof(Particle) * MAX_PARTICLES);

    // 모든 파티클을 초기 상태로 설정
    for (int i = 0; i < MAX_PARTICLES; i++)
        InitParticle(&particles[i]);

    // ============================================================
    // 3. 플로우 필드 설정
    // ============================================================
    // 플로우 필드: 화면을 60x60 그리드로 나누고, 각 셀마다 방향 벡터를 저장
    // 파티클은 자신이 있는 셀의 방향을 따라 움직입니다.
    Vector2 flowField[COLS][ROWS];

    // 각 셀의 크기 계산
    float cellWidth = (float)SCREEN_WIDTH / COLS;    // 약 16.67 픽셀
    float cellHeight = (float)SCREEN_HEIGHT / ROWS;  // 약 11.67 픽셀

    // 시간 누적 변수 (플로우 필드가 시간에 따라 변하도록)
    float timeStep = 0.0f;

    // ============================================================
    // 4. ImGui 파라미터 (사용자가 슬라이더로 조정 가능)
    // ============================================================
    static float timeSpeed = 0.005f;           // 플로우 필드 변화 속도
    static float accelFactor = 0.5f;           // 가속도 계수 (힘의 크기)
    static float spatialFreq = 0.1f;           // 공간 주파수 (플로우 패턴의 촘촘함)
    static float particleRadius = 2.0f;        // 파티클 그리기 크기
    static float trailAlpha = 12;              // 잔상 투명도 (작을수록 선명)
    static int particleCount = 5000;           // 실제 렌더링할 파티클 개수

    // ============================================================
    // 5. 메인 게임 루프 (ESC 누르거나 창 닫을 때까지 반복)
    // ============================================================
    while (!WindowShouldClose())
    {
        // ---- 시간 진행 ----
        timeStep += timeSpeed;

        // ============================================================
        // 단계 1: 플로우 필드 업데이트
        // ============================================================
        // sin/cos 조합으로 부드러운 물결 무늬 생성
        for (int x = 0; x < COLS; x++)
        {
            for (int y = 0; y < ROWS; y++)
            {
                // 각 셀의 각도 계산
                // - x * spatialFreq, y * spatialFreq: 공간적 주기 생성
                // - + timeStep: 시간에 따른 변화
                // - sin * cos 조합: 복잡하고 부드러운 흐름 패턴
                // - * PI * 2.0f: 각도를 라디안으로 변환 (0 ~ 2π)
                float angle = sinf(x * spatialFreq + timeStep) * cosf(y * spatialFreq + timeStep) * PI * 2.0f;

                // 각도를 (x, y) 방향 벡터로 변환
                // 단위 벡터: 크기는 1, 방향은 각도 angle
                flowField[x][y].x = cosf(angle);  // x 방향 성분
                flowField[x][y].y = sinf(angle);  // y 방향 성분
            }
        }

        // ============================================================
        // 단계 2: 파티클 물리 시뮬레이션 업데이트
        // ============================================================
        for (int i = 0; i < particleCount; i++)
        {
            Particle *p = &particles[i];

            // (a) 파티클이 어느 셀에 있는지 찾기
            int cellX = (int)(p->position.x / cellWidth);
            int cellY = (int)(p->position.y / cellHeight);

            // (b) 셀이 그리드 범위 내에 있으면 플로우 필드 방향으로 가속도 설정
            if (cellX >= 0 && cellX < COLS && cellY >= 0 && cellY < ROWS)
                p->acceleration = flowField[cellX][cellY];

            // (c) 가속도를 속도에 더함 (F = ma, 여기서 질량 = 1)
            //     accelFactor는 힘의 크기를 조절
            p->velocity.x += p->acceleration.x * accelFactor;
            p->velocity.y += p->acceleration.y * accelFactor;

            // (d) 최대 속도 제한 (속도가 너무 빨라지지 않도록)
            // 속도의 크기(스칼라) 계산: speed = √(vx² + vy²)
            float speed = sqrtf(p->velocity.x * p->velocity.x + p->velocity.y * p->velocity.y);
            if (speed > p->maxSpeed && speed > 0.0f)
            {
                // 속도의 방향은 유지하되 크기만 maxSpeed로 제한
                // 정규화: velocity / speed → 단위 벡터
                // 크기 조정: 단위 벡터 * maxSpeed
                p->velocity.x = (p->velocity.x / speed) * p->maxSpeed;
                p->velocity.y = (p->velocity.y / speed) * p->maxSpeed;
            }

            // (e) 속도를 위치에 더해 파티클 이동
            p->position.x += p->velocity.x;
            p->position.y += p->velocity.y;

            // (f) 화면을 벗어난 파티클은 재초기화
            // 화면 반대편에서 나타나는 것이 아니라 새로운 위치에서 다시 시작
            if (p->position.x < 0 || p->position.x > SCREEN_WIDTH ||
                p->position.y < 0 || p->position.y > SCREEN_HEIGHT)
                InitParticle(p);
        }

        // ============================================================
        // 단계 3: 렌더링 (그리기)
        // ============================================================
        BeginDrawing();

        // 화면을 검은색으로 지우기
        ClearBackground(BLACK);

        // 반투명 검은색 사각형으로 덮기 (잔상 효과)
        // trailAlpha 값이 작을수록 파티클이 더 선명하고 흔적이 빨리 사라짐
        // trailAlpha 값이 클수록 파티클 흔적이 더 오래 남음
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, (unsigned char)trailAlpha});

        // 모든 파티클을 원으로 그리기
        for (int i = 0; i < particleCount; i++)
            DrawCircle((int)particles[i].position.x, (int)particles[i].position.y,
                       particleRadius, particles[i].color);

        // ============================================================
        // 단계 4: ImGui 패널 렌더링
        // ============================================================
        // rlImGui 시작 (raylib 렌더링과 ImGui 렌더링을 분리)
        rlImGuiBegin();

        // ImGui 윈도우 위치 설정 (화면 우상단)
        // ImGuiCond_FirstUseEver: 처음 실행할 때만 적용, 이후 사용자가 움직이면 그 위치 유지
        ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH - 320, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        // ImGui 윈도우 시작
        if (ImGui::Begin("Flow Field Parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            // FPS 표시
            ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
            ImGui::Spacing();

            // === 플로우 필드 제어 슬라이더 ===
            // Time Speed: 플로우 필드가 얼마나 빠르게 변할지
            // 범위: 0.0 ~ 0.02, 형식: 소수점 5자리
            ImGui::SliderFloat("Time Speed", &timeSpeed, 0.0f, 0.02f, "%.5f");

            // Acceleration: 파티클이 받는 힘의 크기
            // 크면 파티클이 더 빠르게 가속
            ImGui::SliderFloat("Acceleration", &accelFactor, 0.1f, 1.5f);

            // Spatial Freq: 플로우 필드 패턴의 촘촘함
            // 작으면 큰 물결, 크면 작은 물결 많음
            ImGui::SliderFloat("Spatial Freq", &spatialFreq, 0.01f, 0.3f, "%.3f");
            ImGui::Spacing();

            // === 시각화 제어 슬라이더 ===
            // Particle Radius: 화면에 그려지는 파티클의 크기
            ImGui::SliderFloat("Particle Radius", &particleRadius, 0.5f, 5.0f);

            // Trail Alpha: 잔상 효과의 투명도
            // 0: 흔적이 빨리 사라져서 파티클만 보임
            // 50: 흔적이 오래 남아서 궤적이 선명함
            ImGui::SliderInt("Trail Alpha", (int *)&trailAlpha, 0, 50);

            // Particle Count: 실제로 시뮬레이션하고 그릴 파티클 개수
            // 성능 조정용 (개수가 많을수록 느려짐)
            ImGui::SliderInt("Particle Count", &particleCount, 100, MAX_PARTICLES, "%d");

            ImGui::Spacing();

            // === Reset 버튼 ===
            // 모든 파라미터를 초기값으로 되돌림
            // ImVec2(-1, 0): 버튼을 윈도우 너비에 맞춤 (가득 참)
            if (ImGui::Button("Reset", ImVec2(-1, 0)))
            {
                timeSpeed = 0.005f;
                accelFactor = 0.5f;
                spatialFreq = 0.1f;
                particleRadius = 2.0f;
                trailAlpha = 12;
                particleCount = 5000;
                timeStep = 0.0f;
            }
        }
        // ImGui 윈도우 종료 (반드시 필요!)
        ImGui::End();

        // rlImGui 종료
        rlImGuiEnd();

        // raylib 렌더링 종료 (화면 업데이트)
        EndDrawing();
    }

    // ============================================================
    // 정리 및 종료
    // ============================================================
    free(particles);         // 동적 할당한 파티클 배열 메모리 해제
    rlImGuiShutdown();       // ImGui 정리
    CloseWindow();           // raylib 윈도우 닫기

    return 0;
}
