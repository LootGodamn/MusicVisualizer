#include <iostream>
#include <chrono>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <cyber/style_cyber.h>

#include "FileManager.h"
#include "SoundManager.h"

#include <sndfile.h>

using namespace std;

FileManager File_Manager;
SoundManager Sound_Manager;

int CurrentScreen = 0;

bool main_screen();
bool viz_screen();

bool AudioInitiated = false;

int ScreenH;
int ScreenW;

float* CompiledSamples;
int FileSize;

const char* FilePath;

int main() {

	//Setting window properties
	InitWindow(NULL, NULL, "Music Viz");
	SetTargetFPS(60);
	ScreenH = GetScreenHeight();
	ScreenW = GetScreenWidth();
	SetWindowSize(ScreenW, ScreenH);
	

	GuiLoadStyleCyber();

	// Specify the time in seconds
	double TargetTime = 0.0;

	cout << ScreenH << "|" << ScreenW << endl;

	while (!WindowShouldClose()) {

		BeginDrawing();

		if (IsKeyPressed(69)) {
			if (CurrentScreen == 0) CurrentScreen = 1;
			else CurrentScreen = 0;

			cout << "pressed " << endl;
		}

		switch (CurrentScreen) {
			case 0:
				if (!main_screen()) return 1;
				break;

			case 1:
				if (!viz_screen()) return 1;
				break;
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

bool main_screen() {
	GuiPanel(Rectangle_{ 0, 0, ScreenW / 3.0f , static_cast<float>(ScreenH)}, "Song Selection");
	GuiPanel(Rectangle_{ ScreenW / 3.0f, 0, ScreenW / 3.0f , ScreenH * 1.0f }, "Properties");
	GuiPanel(Rectangle_{ (ScreenW / 3.0f) * 2, 0, ScreenW / 3.0f , ScreenH * 1.0f }, "Element Selection");

	if (GuiButton(Rectangle_{ 14.0f, 34.0f, 160.0f, 28.0f }, "Open File")) {

		FilePath = NULL;

		cout << "button clicked" << endl;
		FilePath = File_Manager.OpenFileExplorer();

		if (FilePath == NULL) { cerr << "File failed to load\n"; return false; }

		CompiledSamples = Sound_Manager.read_samples(FilePath, 60, FileSize = File_Manager.file_size(FilePath));

		if (CompiledSamples != NULL) {
			cout << "Success loading data" << endl;
		}
	}

	
	return true;
}

int ASampleIndex = 0;
int ArraySize;

int frames_elapsed;
chrono::milliseconds MsPerFrame(15);

auto PastTime = chrono::system_clock::now();
auto GenesisTime = chrono::system_clock::now();


bool viz_screen() {
	Sound LoadedSound = Sound{};

	if (!AudioInitiated) {
		InitAudioDevice();

		if (!IsAudioDeviceReady()) { cerr << "Audio device unable to initialize" << endl; return false; }

		AudioInitiated = true;
		cout << "Audio Initiated\n";

		LoadedSound = LoadSound(FilePath);
		SetSoundVolume(LoadedSound, 0.2f);

		if (IsSoundReady(LoadedSound) && !IsSoundPlaying(LoadedSound)) {
			PlaySound(LoadedSound);
		}
	}

	if (IsKeyPressed(81) && AudioInitiated) {
		cout << "Key detected\n";
		StopSound(LoadedSound);
		UnloadSound(LoadedSound);
		CloseAudioDevice();
		AudioInitiated = false;
	}

	if (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - PastTime) >= MsPerFrame) {
		if (ASampleIndex >= (FileSize / 4)) {
			cout << "Done playing" << endl;
			ASampleIndex = 0;
			return false;
		}
		else
		{
			float size = (CompiledSamples[ASampleIndex] + 5.5) * 150;
			cout << size << endl;
			GuiDrawRectangle(Rectangle_{ 0, 0, size, size }, 15, WHITE, DARKGRAY);
			ASampleIndex++;
		}

		frames_elapsed++;
		PastTime = chrono::system_clock::now();
	}

	return true;
}