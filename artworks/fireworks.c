#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

// ============================================
// 상수 정의
// ============================================

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 800

#define MAX_PARTICLES 6000
#define MAX_ROCKETS   16
#define STAR_COUNT    150

#define PARTICLE_MIN  40
#define PARTICLE_MAX  120

#define PARTICLE_SPEED_MIN 200.0f
#define PARTICLE_SPEED_MAX 600.0f

#define PARTICLE_LIFE 2.5f
#define GRAVITY       500.0f
#define DRAG          3.5f

#define SECONDARY_BURST_CHANCE 0.3f
#define SECONDARY_PARTICLE_MIN 15
#define SECONDARY_PARTICLE_MAX 40

#define ROCKET_SPEED       450.0f   // 로켓 상승 속도 (픽셀/초)
#define ROCKET_ARRIVAL_EPS 6.0f     // 목표 도달 판정 거리

// ============================================
// Particle 구조체
// ============================================

typedef struct
{
    Vector2 position;
    Vector2 velocity;
    Vector2 previousPosition;

    float life;
    float maxLife;
    float size;
    float maxSize;

    Color color;
    bool active;
    bool isSecondary;
} Particle;

// ============================================
// Rocket 구조체 (발사 후 목표 지점에서 폭발)
// ============================================

typedef struct
{
    Vector2 position;
    Vector2 target;
    Vector2 previousPosition;
    Vector2 velocity;

    Color trailColor;
    bool active;
} Rocket;

// ============================================
// Star 구조체 (배경 별, 위치 고정)
// ============================================

typedef struct
{
    Vector2 position;
    float phase;
    float speed;
} Star;

// ============================================
// 전역 데이터 (고정 크기, 동적 할당 없음)
// ============================================

static Particle particles[MAX_PARTICLES];
static int freeStack[MAX_PARTICLES];
static int freeTop = 0;
static int activeParticleCount = 0;

static Rocket rockets[MAX_ROCKETS];
static int rocketFreeStack[MAX_ROCKETS];
static int rocketFreeTop = 0;

static Star stars[STAR_COUNT];

// ============================================
// 풀 초기화
// ============================================

void InitParticlePool(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        freeStack[i] = i;
        particles[i].active = false;
    }

    freeTop = MAX_PARTICLES;
    activeParticleCount = 0;
}

void InitRocketPool(void)
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        rocketFreeStack[i] = i;
        rockets[i].active = false;
    }

    rocketFreeTop = MAX_ROCKETS;
}

// ============================================
// 유틸리티 함수
// ============================================

Color RandomHSVColor(void)
{
    float hue = GetRandomValue(0, 360);
    float saturation = 0.7f + GetRandomValue(0, 30) / 100.0f;
    float value = 0.9f + GetRandomValue(0, 10) / 100.0f;

    return ColorFromHSV(hue, saturation, value);
}

// ============================================
// 파티클 생성 / 소멸 (O(1))
// ============================================

void SpawnParticle(Vector2 center, float angle, float speed,
                    float life, float size, Color color, bool isSecondary)
{
    if (freeTop <= 0) return;

    int idx = freeStack[--freeTop];
    Particle *p = &particles[idx];

    p->position = center;
    p->previousPosition = center;

    p->velocity = (Vector2){ cosf(angle) * speed, sinf(angle) * speed };

    p->life = life;
    p->maxLife = life;

    p->size = size;
    p->maxSize = size;

    p->color = color;
    p->active = true;
    p->isSecondary = isSecondary;

    activeParticleCount++;
}

void KillParticle(int idx)
{
    particles[idx].active = false;
    freeStack[freeTop++] = idx;
    activeParticleCount--;
}

void CreateFirework(Vector2 center, int count)
{
    Color mainColor = RandomHSVColor();

    for (int i = 0; i < count; i++)
    {
        float angle = (float)i / count * 2.0f * PI;
        angle += (GetRandomValue(-20, 20) / 100.0f) * (PI / 180.0f);

        float speed = GetRandomValue(
            (int)(PARTICLE_SPEED_MIN * 100),
            (int)(PARTICLE_SPEED_MAX * 100)
        ) / 100.0f;

        float size = 4.0f + GetRandomValue(0, 30) / 10.0f;

        SpawnParticle(center, angle, speed, PARTICLE_LIFE, size, mainColor, false);
    }
}

void CreateSecondaryBurst(Vector2 center, int count)
{
    Color burstColor = RandomHSVColor();

    for (int i = 0; i < count; i++)
    {
        float angle = (float)i / count * 2.0f * PI;
        angle += (GetRandomValue(-30, 30) / 100.0f) * (PI / 180.0f);

        float speed = GetRandomValue(
            (int)(PARTICLE_SPEED_MIN * 50),
            (int)(PARTICLE_SPEED_MAX * 70)
        ) / 100.0f;

        float size = 2.5f + GetRandomValue(0, 20) / 10.0f;

        SpawnParticle(center, angle, speed, PARTICLE_LIFE * 0.6f, size, burstColor, true);
    }
}

// ============================================
// 로켓 생성 / 소멸 (O(1))
// ============================================

// 화면 하단에서 발사하여, 상단 절반 내의 무작위 지점을 목표로 함
void LaunchRocket(void)
{
    if (rocketFreeTop <= 0) return;

    int idx = rocketFreeStack[--rocketFreeTop];
    Rocket *r = &rockets[idx];

    float launchX = (float)GetRandomValue(100, WINDOW_WIDTH - 100);
    Vector2 startPos = (Vector2){ launchX, (float)WINDOW_HEIGHT };

    // 목표 지점: 세로는 상단 절반 내로 제한, 가로는 발사 위치 근처에서 약간 편차
    float targetX = launchX + (float)GetRandomValue(-150, 150);
    if (targetX < 80.0f) targetX = 80.0f;
    if (targetX > WINDOW_WIDTH - 80.0f) targetX = WINDOW_WIDTH - 80.0f;

    float targetY = (float)GetRandomValue(50, WINDOW_HEIGHT / 2);

    Vector2 target = (Vector2){ targetX, targetY };

    // 목표 방향으로의 단위 속도 벡터 계산
    Vector2 dir = (Vector2){ target.x - startPos.x, target.y - startPos.y };
    float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);

    if (dist < 1.0f) dist = 1.0f;

    Vector2 velocity = (Vector2){
        (dir.x / dist) * ROCKET_SPEED,
        (dir.y / dist) * ROCKET_SPEED
    };

    r->position = startPos;
    r->previousPosition = startPos;
    r->target = target;
    r->velocity = velocity;
    r->trailColor = RandomHSVColor();
    r->active = true;
}

void KillRocket(int idx)
{
    rockets[idx].active = false;
    rocketFreeStack[rocketFreeTop++] = idx;
}

// ============================================
// 업데이트 함수
// ============================================

void UpdateRockets(void)
{
    float dt = GetFrameTime();

    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        Rocket *r = &rockets[i];

        if (!r->active)
            continue;

        r->previousPosition = r->position;

        r->position.x += r->velocity.x * dt;
        r->position.y += r->velocity.y * dt;

        // 목표 지점 도달 판정
        float dx = r->target.x - r->position.x;
        float dy = r->target.y - r->position.y;
        float distToTarget = sqrtf(dx * dx + dy * dy);

        if (distToTarget <= ROCKET_ARRIVAL_EPS)
        {
            int count = GetRandomValue(PARTICLE_MIN, PARTICLE_MAX);
            CreateFirework(r->target, count);
            KillRocket(i);
        }
    }
}

void UpdateParticles(void)
{
    float dt = GetFrameTime();

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &particles[i];

        if (!p->active)
            continue;

        p->previousPosition = p->position;

        p->life -= dt;

        if (p->life <= 0.0f)
        {
            KillParticle(i);
            continue;
        }

        if (!p->isSecondary && p->life < 0.2f && p->life + dt >= 0.2f)
        {
            if (GetRandomValue(0, 100) / 100.0f < SECONDARY_BURST_CHANCE)
            {
                int burstCount = GetRandomValue(SECONDARY_PARTICLE_MIN, SECONDARY_PARTICLE_MAX);
                CreateSecondaryBurst(p->position, burstCount);
            }
        }

        float damping = expf(-DRAG * dt);
        p->velocity.x *= damping;
        p->velocity.y *= damping;

        p->velocity.y += GRAVITY * dt;

        p->position.x += p->velocity.x * dt;
        p->position.y += p->velocity.y * dt;

        float alpha = p->life / p->maxLife;
        p->size = p->maxSize * alpha;

        if (p->position.y > WINDOW_HEIGHT + 50.0f)
        {
            KillParticle(i);
        }
    }
}

// ============================================
// 렌더링 함수
// ============================================

void DrawRockets(void)
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        Rocket *r = &rockets[i];

        if (!r->active)
            continue;

        // 상승 궤적 잔상
        DrawLineEx(r->previousPosition, r->position, 2.5f, r->trailColor);

        // 로켓 머리 (밝은 점)
        DrawCircleV(r->position, 3.5f, WHITE);
        DrawCircleV(r->position, 2.0f, r->trailColor);
    }
}

void DrawParticles(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &particles[i];

        if (!p->active)
            continue;

        float alpha = p->life / p->maxLife;

        Color c = p->color;
        c.a = (unsigned char)(255 * alpha);

        Color trailColor = c;
        trailColor.a = (unsigned char)(c.a * 0.4f);

        if (p->previousPosition.x != p->position.x || p->previousPosition.y != p->position.y)
        {
            DrawLineEx(p->previousPosition, p->position, p->size * 0.5f, trailColor);
        }

        DrawCircleV(p->position, p->size, c);

        Color coreColor = c;
        coreColor.a = (unsigned char)(c.a * 0.7f);
        DrawCircleV(p->position, p->size * 0.4f, coreColor);
    }
}

// ============================================
// 별 배경
// ============================================

void InitStars(void)
{
    for (int i = 0; i < STAR_COUNT; i++)
    {
        stars[i].position = (Vector2)
        {
            (float)GetRandomValue(0, WINDOW_WIDTH),
            (float)GetRandomValue(0, WINDOW_HEIGHT)
        };

        stars[i].phase = (float)GetRandomValue(0, 628) / 100.0f;
        stars[i].speed = 1.0f + GetRandomValue(0, 200) / 100.0f;
    }
}

void DrawStars(float time)
{
    for (int i = 0; i < STAR_COUNT; i++)
    {
        float brightness = (sinf(time * stars[i].speed + stars[i].phase) + 1.0f) * 0.5f;
        unsigned char a = (unsigned char)(80 + brightness * 120);

        DrawCircleV(stars[i].position, 1.2f, (Color){ 200, 200, 255, a });
    }
}

// ============================================
// 메인 함수
// ============================================

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Fireworks - Raylib Artwork");

    SetTargetFPS(60);

    InitStars();
    InitParticlePool();
    InitRocketPool();

    while (!WindowShouldClose())
    {
        // 입력 처리: 스페이스바로 로켓 발사
        if (IsKeyPressed(KEY_SPACE))
        {
            LaunchRocket();
        }

        // 업데이트
        UpdateRockets();
        UpdateParticles();

        // 렌더링
        BeginDrawing();

        ClearBackground((Color){ 10, 10, 20, 255 });

        DrawStars((float)GetTime());
        DrawRockets();
        DrawParticles();

        // UI 텍스트
        DrawText("Press SPACE to launch a firework", 10, 10, 20, (Color){ 200, 200, 200, 255 });

        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", GetFPS());
        DrawText(fpsText, WINDOW_WIDTH - 150, 10, 20, (Color){ 200, 200, 200, 255 });

        char particleText[64];
        snprintf(particleText, sizeof(particleText), "Particles: %d", activeParticleCount);
        DrawText(particleText, WINDOW_WIDTH - 150, 40, 20, (Color){ 200, 200, 200, 255 });

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
