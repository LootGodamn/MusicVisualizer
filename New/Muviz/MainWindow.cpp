#include <iostream>
using namespace std;

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "cyber/style_cyber.h"


Vector2 ScreenSize;
int CurrentScreen = 0;

int mainscreen();
int vizscreen();

int main() {
	//Setting window properties
	InitWindow(NULL, NULL, "Music Visualizer");
	SetTargetFPS(60);
	ScreenSize.y = GetScreenHeight();
	ScreenSize.x = GetScreenWidth();
	SetWindowSize(ScreenSize.x, ScreenSize.y);

	//Refocus Window to fit height
	SetWindowState(STATE_DISABLED);
	SetWindowState(STATE_FOCUSED);

	GuiLoadStyleCyber();

	cout << ScreenSize.y << "|" << ScreenSize.x << endl;

	//Main loop
	while (!WindowShouldClose()) {

		BeginDrawing();

		switch (CurrentScreen) {
		case 0:
			if (mainscreen() == 1) return 1;
			break;

		case 1:
			if (vizscreen() == 1) return 1;
			break;
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

int mainscreen(){
	//Categories of options
	GuiPanel(Rectangle_{ 0, 0, ScreenSize.x / 3.0f , static_cast<float>(ScreenSize.y) }, "Song Selection");
	GuiPanel(Rectangle_{ ScreenSize.x / 3.0f, 0, ScreenSize.x / 3.0f , ScreenSize.y * 1.0f }, "Properties");
	GuiPanel(Rectangle_{ (ScreenSize.x / 3.0f) * 2, 0, ScreenSize.x / 3.0f , ScreenSize.y * 1.0f }, "Element Selection");

	//File reading
	if (GuiButton(Rectangle_{ 14.0f, 34.0f, 160.0f, 28.0f }, "Open File")) {

		cout << "WAT" << endl;
	}

	//Button to switch screens
	if (GuiButton(Rectangle_{ ScreenSize.x / 2, 64.0f, 160.0f, 28.0f }, "Viz Screen")) {
		if (CurrentScreen == 0) CurrentScreen = 1;
		else CurrentScreen = 0;
	}

	return 0;
}

int vizscreen() {
	return 0;
}