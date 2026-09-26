# raylib-art: 제너레이티브 아트 실험 플랫폼

## 1. 프로젝트의 목적

이 프로젝트는 **raylib**을 사용하여 제너레이티브 아트(generative art)를 만들고 실험하는 환경을 제공합니다.

### 세 가지 워크플로우

1. **빠른 실험** (`src/main.c`) — 새로운 아이디어를 즉시 테스트
2. **결과 아카이빙** (`artworks/`) — 완성된 작품을 보관
3. **파라미터 튜닝** (`apps/`) — ImGui로 실시간 파라미터 조정

이 구조를 통해 개발부터 배포까지 체계적으로 진행할 수 있습니다.

---

## 2. 프로젝트 설정

### macOS 환경 설정

#### 2-1. 필수 도구 설치

```bash
# Xcode Command Line Tools 확인
xcode-select --install

# Homebrew 확인
brew --version

# raylib 설치
brew install raylib

# 설치 확인
pkg-config --cflags --libs raylib
```

#### 2-2. 저장소 클론 및 submodule 초기화

```bash
git clone <repository-url>
cd raylib-art

# Dear ImGui와 rlImGui submodule 초기화
git submodule update --init --recursive
```

#### 2-3. 빌드 확인

```bash
make run
```

---

### Windows 환경 설정

Windows에서는 다음 중 하나를 선택하세요.

#### 옵션 A: MinGW + MSYS2 (권장)

```bash
# MSYS2 설치 후 터미널에서
pacman -S mingw-w64-x86_64-raylib
pacman -S mingw-w64-x86_64-clang
pacman -S make
pacman -S git
```

저장소 클론:
```bash
git clone <repository-url>
cd raylib-art
git submodule update --init --recursive
make run
```

#### 옵션 B: Visual Studio + CMake

1. **Visual Studio 2022** 설치 (C++ 워크로드 포함)
2. **raylib 바이너리** 다운로드: https://github.com/raysan5/raylib/releases
3. 프로젝트 폴더에 `raylib/` 복사
4. CMakeLists.txt 생성 (또는 Visual Studio 프로젝트 수동 설정)
5. 빌드 및 실행

#### 옵션 C: WSL (Windows Subsystem for Linux)

WSL에서 macOS 설정을 동일하게 적용:
```bash
wsl
sudo apt-get update
sudo apt-get install libraylib-dev clang make git
# 이후 macOS와 동일한 단계 진행
```

---

## 3. 프로젝트 구조

```
raylib-art/
├── src/
│   └── main.c              # 실험 스크래치패드 (순수 C)
├── artworks/
│   ├── particle_noise_field.c
│   ├── fireworks.c
│   └── ...                 # 완성된 작품들 (순수 C)
├── apps/
│   ├── example_app.cpp     # ImGui 기본 템플릿 (C++)
│   └── particle_tuner.cpp  # 파라미터 튜닝 앱 (C++)
├── shaders/
│   ├── basic/
│   │   ├── vert.glsl
│   │   └── frag.glsl
│   └── ...
├── third_party/
│   ├── imgui/              # Dear ImGui (git submodule)
│   └── rlImGui/            # raylib용 ImGui 백엔드 (git submodule)
├── build/                  # 컴파일 결과물 (.gitignore)
└── Makefile
```

### 폴더별 역할

#### `src/` — 실험 스크래치패드

- **목적**: 새로운 아이디어를 빠르게 테스트
- **언어**: 순수 C (raylib만 사용)
- **특징**: GUI 없음, 결과 재생만
- **용도**: 알고리즘 개발, 원형 제작
- **빌드**: `make run`

#### `artworks/` — 결과물 아카이브

- **목적**: 완성된 작품을 버전 관리
- **언어**: 순수 C (raylib만 사용)
- **특징**: 변경 없음, 자동 생성, 배포 가능
- **용도**: 최종 결과물 보관, 포트폴리오
- **빌드**: `make art NAME=artwork_name`

#### `apps/` — 파라미터 튜닝 앱

- **목적**: ImGui 패널로 실시간 파라미터 조정
- **언어**: C++ (raylib + Dear ImGui 사용)
- **특징**: GUI 있음, 인터랙티브
- **용도**: 파라미터 최적화, 시뮬레이션 튜닝
- **빌드**: `make app NAME=app_name`

### 왜 이렇게 나눴나?

| 측면 | src/ | artworks/ | apps/ |
|------|------|-----------|-------|
| **목적** | 개발 | 배포 | 연구 |
| **언어** | C | C | C++ |
| **GUI** | ❌ | ❌ | ✅ |
| **수정** | 자주 | 거의 안 함 | 필요시 |
| **바이너리 크기** | 작음 | 작음 | 중간 |
| **의존성** | raylib | raylib | raylib + ImGui |

### 왜 apps는 C++인가?

Dear ImGui와 rlImGui는 **C++ 라이브러리**입니다.
- 모든 소스가 C++로 작성됨
- C 코드에서 C++ 라이브러리를 직접 호출 불가능
- 따라서 ImGui를 사용하려면 C++로 작성해야 함

---

## 4. Makefile과 빌드 방법

### Makefile 구조

```makefile
CC = clang                    # C 컴파일러
CXX = clang++                 # C++ 컴파일러
CFLAGS = -Wall -Wextra -std=c17
CXXFLAGS = -Wall -Wextra -std=c++17
LIBS = $(shell pkg-config --cflags --libs raylib)
```

### 빌드 타겟

#### 실험 (src/main.c)

```bash
make run       # 빌드 및 실행
make           # 빌드만
make clean     # 빌드 결과 삭제
```

#### 아트워크 (artworks/)

```bash
make art NAME=fireworks           # artworks/fireworks.c 빌드 및 실행
make art NAME=particle_noise_field
```

사용 가능한 아트워크 목록:
```bash
ls artworks/*.c | sed 's/artworks\///' | sed 's/.c//'
```

#### 튜닝 앱 (apps/)

```bash
make app NAME=particle_tuner    # apps/particle_tuner.cpp 빌드 및 실행
make app NAME=example_app
```

### 새로운 앱 만들기

1. `apps/my_app.cpp` 파일 생성
2. 기본 템플릿:

```cpp
#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

int main(void)
{
  InitWindow(1200, 800, "My App");
  SetTargetFPS(60);
  rlImGuiSetup(true);

  while (!WindowShouldClose())
  {
    // 시뮬레이션 코드
    
    BeginDrawing();
    ClearBackground(BLACK);

    // raylib 렌더링
    
    rlImGuiBegin();
    // ImGui 패널
    ImGui::Begin("Settings");
    ImGui::SliderFloat("Param", &param, 0.0f, 10.0f);
    ImGui::End();
    rlImGuiEnd();

    EndDrawing();
  }

  rlImGuiShutdown();
  CloseWindow();
  return 0;
}
```

3. 빌드:
```bash
make app NAME=my_app
```

---

## 5. 애플리케이션으로 만들어 배포

### macOS: .app 번들 생성

#### 5-1. 수동 .app 번들 생성

```bash
# 1. 앱 번들 디렉토리 구조 생성
mkdir -p MyArt.app/Contents/MacOS
mkdir -p MyArt.app/Contents/Resources

# 2. 바이너리 복사
cp build/particle_tuner MyArt.app/Contents/MacOS/particle_tuner

# 3. Info.plist 파일 생성
cat > MyArt.app/Contents/Info.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>particle_tuner</string>
  <key>CFBundleIdentifier</key>
  <string>com.example.raylib-art</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>Particle Tuner</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0</string>
  <key>CFBundleVersion</key>
  <string>1</string>
  <key>NSHighResolutionCapable</key>
  <true/>
</dict>
</plist>
EOF

# 4. 실행 가능 설정
chmod +x MyArt.app/Contents/MacOS/particle_tuner

# 5. 더블클릭으로 실행
open MyArt.app
```

#### 5-2. Finder에서 .app 배포

```bash
# 생성한 .app을 zip으로 압축
zip -r MyArt.zip MyArt.app

# 또는 DMG 이미지 생성 (배포용)
hdiutil create -volname MyArt -srcfolder . -ov -format UDZO MyArt.dmg
```

### Windows: .exe 실행파일 배포

#### 5-1. 필요한 DLL 복사

```bash
# MinGW 환경에서 빌드 후
mkdir MyArt
cp build/particle_tuner.exe MyArt/
cp /mingw64/bin/raylib.dll MyArt/
cp /mingw64/bin/libgcc_s_seh-1.dll MyArt/
cp /mingw64/bin/libwinpthread-1.dll MyArt/
```

#### 5-2. 인스톨러 생성 (NSIS)

1. NSIS 설치: https://nsis.sourceforge.io/
2. `installer.nsi` 파일 생성:

```nsis
!include "MUI2.nsh"

Name "MyArt"
OutFile "MyArt-installer.exe"
InstallDir "$PROGRAMFILES\MyArt"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

Section "Install"
  SetOutPath "$INSTDIR"
  File "MyArt\particle_tuner.exe"
  File "MyArt\*.dll"
  
  CreateDirectory "$SMPROGRAMS\MyArt"
  CreateShortcut "$SMPROGRAMS\MyArt\MyArt.lnk" "$INSTDIR\particle_tuner.exe"
  CreateShortcut "$DESKTOP\MyArt.lnk" "$INSTDIR\particle_tuner.exe"
SectionEnd
```

3. 빌드:
```bash
makensis installer.nsi
```

### Linux: AppImage 생성

```bash
# AppImage 빌드 도구 설치
wget https://github.com/AppImage/AppImageKit/releases/download/13/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage

# AppDir 구조 생성
mkdir -p MyArt.AppDir/usr/bin
cp build/particle_tuner MyArt.AppDir/usr/bin/

# AppImage 생성
./appimagetool-x86_64.AppImage MyArt.AppDir MyArt.AppImage
chmod +x MyArt.AppImage
```

---

## 6. 워크플로우 예시

### 새로운 제너레이티브 아트 만들기

```bash
# 1. src/main.c에서 알고리즘 개발
nano src/main.c

# 2. 빠르게 테스트
make run

# 3. 결과가 마음에 들면 artworks로 이동
cp src/main.c artworks/my_new_art.c

# 4. artworks에서 독립적으로 실행
make art NAME=my_new_art

# 5. src/main.c는 다음 실험 준비
# (내용을 지우거나 수정해서 새로운 아이디어 시작)
```

### 파라미터 튜닝 앱 개발

```bash
# 1. 기존 알고리즘을 apps로 옮김
cp artworks/particle_noise_field.c apps/flow_tuner.cpp
# (C → C++로 변환, ImGui 패널 추가)

# 2. 슬라이더로 파라미터 조정하며 테스트
make app NAME=flow_tuner

# 3. 최적 파라미터를 찾으면 배포용 .app/.exe 생성
# (위의 "5. 애플리케이션으로 만들어 배포" 참고)
```

---

## 7. 참고 링크

- **raylib**: https://www.raylib.com/
- **Dear ImGui**: https://github.com/ocornut/imgui
- **rlImGui**: https://github.com/raylib-extras/rlImGui
- **raylib GitHub**: https://github.com/raysan5/raylib
