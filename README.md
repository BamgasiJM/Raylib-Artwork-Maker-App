# raylib C Generative Art on macOS

macOS에서 Homebrew로 raylib을 설치하고, Apple Clang을 사용하여 C 언어로 raylib 프로그램을 빌드하고 실행하는 가장 기본적인 환경을 구성합니다.

이 문서는 다음 환경을 기준으로 합니다.

- macOS Intel
- Visual Studio Code
- Apple Clang
- Homebrew
- raylib
- Make

목표는 IDE의 자동화 기능에 의존하기보다 먼저 `clang` 명령어로 직접 빌드 과정을 이해하고, 이후 간단한 `Makefile`로 반복 작업을 줄이는 것입니다.

---

## 1. 개발 환경 확인

### Clang 확인

터미널에서 다음을 실행합니다.

```bash
clang --version
```

macOS에서는 Apple Clang을 기본 C 컴파일러로 사용할 수 있습니다.

Clang이 설치되어 있지 않다면 다음 명령으로 Command Line Tools를 설치합니다.

```bash
xcode-select --install
```

설치 후 다시 확인합니다.

```bash
clang --version
```

### Homebrew 확인

```bash
brew --version
```

Homebrew가 설치되어 있지 않다면 공식 설치 안내를 참고합니다.

Intel Mac에서는 Homebrew가 일반적으로 `/usr/local` 아래에 설치됩니다.

설치 위치는 다음 명령으로 확인할 수 있습니다.

```bash
which brew
```

---

## 2. raylib 설치

Homebrew를 사용하여 raylib을 설치합니다.

```bash
brew install raylib
```

설치가 완료되면 다음 명령으로 확인합니다.

```bash
brew info raylib
```

그리고 `pkg-config`를 통해 raylib의 컴파일 및 링크 옵션이 정상적으로 확인되는지 테스트합니다.

```bash
pkg-config --cflags --libs raylib
```

출력에는 시스템에 설치된 raylib의 include 경로와 library 경로, 링크 옵션 등이 포함됩니다.

예를 들어 다음과 비슷한 형태입니다.

```text
-I/usr/local/include -L/usr/local/lib -lraylib ...
```

정확한 출력은 Homebrew 및 raylib 버전에 따라 달라질 수 있습니다.

---

## 3. 프로젝트 생성

연습 프로젝트를 다음과 같은 구조로 만듭니다.

```text
raylib-art/
├── src/
│   └── main.c
├── build/
└── Makefile
```

터미널에서:

```bash
mkdir -p ~/Documents/dev/raylib-art/src
cd ~/Documents/dev/raylib-art
mkdir build
```

VS Code로 프로젝트를 엽니다.

```bash
code .
```

`code` 명령어가 등록되어 있지 않다면 VS Code에서 직접 프로젝트 폴더를 열어도 됩니다.

---

## 4. 첫 번째 raylib 프로그램

`src/main.c` 파일을 만들고 다음 코드를 작성합니다.

```c
#include "raylib.h"

int main(void)
{
    InitWindow(800, 600, "Raylib Generative Art");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawCircle(400, 300, 100, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
```

이 프로그램의 기본 실행 구조는 다음과 같습니다.

```text
InitWindow()
    ↓
게임/그래픽 루프
    ↓
BeginDrawing()
    ↓
Draw...
    ↓
EndDrawing()
    ↓
CloseWindow()
```

`InitWindow()`로 창을 만들고, `while` 루프에서 매 프레임 그림을 그립니다.

`WindowShouldClose()`는 창 닫기 이벤트 등을 확인합니다.

---

## 5. clang으로 직접 빌드하기

처음에는 Makefile을 사용하지 않고 C 컴파일러를 직접 실행합니다.

프로젝트 루트에서 다음 명령을 실행합니다.

```bash
clang src/main.c -o build/main $(pkg-config --cflags --libs raylib)
```

여기서 중요한 부분은 다음과 같습니다.

```text
clang
  ↓
src/main.c
  ↓
raylib의 include / library 정보
  ↓
build/main
```

`pkg-config`는 현재 설치된 raylib에 필요한 컴파일 및 링크 옵션을 제공해 줍니다.

빌드가 성공하면 `build/main` 실행 파일이 생성됩니다.

실행합니다.

```bash
./build/main
```

800×600 크기의 창이 나타나고 가운데에 원이 표시되면 성공입니다.

---

## 6. clang 명령어의 의미

다음 명령을 다시 살펴보면:

```bash
clang src/main.c -o build/main $(pkg-config --cflags --libs raylib)
```

각 부분은 다음 의미를 갖습니다.

```text
clang
```

C 컴파일러입니다.

```text
src/main.c
```

컴파일할 C 소스 파일입니다.

```text
-o build/main
```

컴파일 결과의 실행 파일 이름과 위치를 지정합니다.

```text
$(pkg-config --cflags --libs raylib)
```

raylib을 컴파일하고 링크하는 데 필요한 옵션을 `pkg-config`로 가져옵니다.

즉, 이 명령을 이해하는 것이 중요합니다.

Makefile은 결국 이 긴 명령을 반복해서 입력하지 않도록 정리하는 도구일 뿐입니다.

---

## 7. Makefile 만들기

프로젝트 루트에 `Makefile`을 만듭니다.

```makefile
CC = clang
CFLAGS = -Wall -Wextra -std=c17
LIBS = $(shell pkg-config --cflags --libs raylib)

SRC = src/main.c
OUT = build/main

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
```

주의할 점은 Makefile의 명령어 앞에 들어가는 공백이 반드시 Tab 문자여야 한다는 것입니다.

예를 들어 다음 부분의 `$(CC)` 앞에는 Tab이 들어가야 합니다.

```makefile
all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)
```

---

## 8. Makefile로 빌드

이제 다음 명령만 실행하면 됩니다.

```bash
make
```

실제로는 다음과 같은 작업이 실행됩니다.

```text
src/main.c
    ↓
clang
    ↓
raylib library linking
    ↓
build/main
```

실행:

```bash
./build/main
```

또는 빌드와 실행을 한 번에:

```bash
make run
```

빌드 결과를 삭제하려면:

```bash
make clean
```

---

## 9. 현재 프로젝트 구조

```text
raylib-art/
├── src/
│   └── main.c          # 현재 실험 중인 코드 (순수 C)
├── artworks/            # 마음에 든 결과물을 이름 바꿔 아카이빙 (순수 C)
│   └── *.c
├── apps/                # imGui 컨트롤 패널이 있는 실험용 앱 (C++)
│   └── example_app.cpp
├── shaders/
├── third_party/
│   ├── imgui/            # Dear ImGui (git submodule)
│   └── rlImGui/          # raylib용 imGui 백엔드 (git submodule)
├── build/
└── Makefile
```

`build/`는 컴파일 시 자동 생성되는 실행 파일이므로 커밋하지 않습니다 (`.gitignore`에 등록됨).

이 저장소를 새로 클론했다면 submodule도 함께 받아야 합니다.

```bash
git submodule update --init --recursive
```

---

## 9-1. apps/: imGui 앱 만들기

`artworks/`가 정적인 결과물 아카이브라면, `apps/`는 imGui 패널로 파라미터를 실시간 조작하는 실험용 앱을 모아둡니다. imGui는 C++ 라이브러리이므로 `apps/*.cpp`는 `clang++`로 빌드됩니다.

```bash
make app NAME=example_app
```

기본 뼈대는 다음과 같습니다.

```cpp
#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

int main(void)
{
  InitWindow(1080, 720, "My App");
  rlImGuiSetup(true);

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    rlImGuiBegin();
    ImGui::ShowDemoWindow();   // 여기에 원하는 imGui 패널 구성
    rlImGuiEnd();

    EndDrawing();
  }

  rlImGuiShutdown();
  CloseWindow();
  return 0;
}
```

`artworks/`와 마찬가지로 `make art NAME=<파일이름확장자제외>` 으로 개별 아트워크를 빌드/실행할 수 있습니다.

```bash
make art NAME=fireworks
```

---

## 10. 다음 단계: C와 Generative Art

현재 프로그램은 단순히 원 하나를 그립니다.

다음 단계부터 raylib을 C 언어 학습과 제너레이티브 아트 실험 환경으로 사용할 수 있습니다.

학습 순서는 다음과 같이 진행하는 것을 권장합니다.

```text
C 기본 문법
    ↓
raylib 기본 Drawing API
    ↓
변수와 반복문
    ↓
배열
    ↓
struct
    ↓
포인터
    ↓
Vector2
    ↓
삼각함수
    ↓
랜덤
    ↓
Particle System
    ↓
Noise
    ↓
Flow Field
    ↓
Boids
    ↓
Cellular Automata
    ↓
Physarum
    ↓
GLSL Shader
```

예를 들어 다음과 같이 점의 위치를 변수로 관리하는 것부터 시작할 수 있습니다.

```c
float x = 400.0f;
float y = 300.0f;
float radius = 20.0f;
```

그다음 여러 개의 데이터를 배열로 관리합니다.

```c
Vector2 points[100];
```

그리고 더 발전시키면 하나의 입자를 구조체로 표현할 수 있습니다.

```c
typedef struct
{
    Vector2 position;
    Vector2 velocity;
    float radius;
} Particle;
```

이렇게 `struct`, 배열, 포인터, 함수, 메모리 관리 등의 C 언어 개념을 실제 그래픽 알고리즘에 적용하면서 학습할 수 있습니다.

---

## 11. 권장 작업 방식

처음에는 다음 과정을 반복하는 것이 좋습니다.

```text
main.c 수정
    ↓
make
    ↓
./build/main
    ↓
결과 확인
    ↓
알고리즘 수정
```

그리고 익숙해지면:

```bash
make run
```

하나로 빌드와 실행을 처리합니다.

처음부터 복잡한 IDE 프로젝트 설정이나 CMake를 사용하기보다 `clang` 명령어를 직접 실행해 보는 이유는 C 프로그램이 어떻게 컴파일되고 raylib 라이브러리가 어떻게 링크되는지를 이해하기 위해서입니다.

---

## 참고

- raylib: https://www.raylib.com/
- raylib GitHub: https://github.com/raysan5/raylib
- Homebrew: https://brew.sh/
- Homebrew raylib formula: https://formulae.brew.sh/formula/raylib
