CC = clang
CXX = clang++
CFLAGS = -Wall -Wextra -std=c17
CXXFLAGS = -Wall -Wextra -std=c++17
LIBS = $(shell pkg-config --cflags --libs raylib)

IMGUI_DIR = third_party/imgui
RLIMGUI_DIR = third_party/rlImGui
IMGUI_SRCS = $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp \
             $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp \
             $(IMGUI_DIR)/imgui_demo.cpp $(RLIMGUI_DIR)/rlImGui.cpp
IMGUI_INCLUDES = -I$(IMGUI_DIR) -I$(RLIMGUI_DIR)

SRC = src/main.c
OUT = build/main

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

run: all
	./$(OUT)

# make art NAME=fireworks  ->  artworks/fireworks.c 빌드 및 실행
art:
	$(CC) $(CFLAGS) artworks/$(NAME).c -o build/$(NAME) $(LIBS)
	./build/$(NAME)

# make app NAME=example_app  ->  apps/example_app.cpp + rlImGui/imgui 빌드 및 실행
app:
	$(CXX) $(CXXFLAGS) $(IMGUI_INCLUDES) apps/$(NAME).cpp $(IMGUI_SRCS) -o build/$(NAME) $(LIBS)
	./build/$(NAME)

clean:
	rm -f build/*
