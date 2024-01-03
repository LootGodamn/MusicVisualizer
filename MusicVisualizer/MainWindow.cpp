#include <iostream>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "FileManager.h"

using namespace std;

FileManager File_Manager;

int main() {
	int ScreenH = GetMonitorHeight(0);
	int ScreenW = GetMonitorWidth(0);

	InitWindow(ScreenW, ScreenH, "Music Viz");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(BLACK);

		if (GuiButton(Rectangle_{600.0f, 600.0f, 160.0f, 28.0f }, "Open File")) {
			cout << "button clicked" << endl;
			File_Manager.OpenFileExplorer();
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}