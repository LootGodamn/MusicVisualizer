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

		CompiledSamples = Sound_Manager.read_samples(FilePath, 60);

		if (CompiledSamples != NULL) {
			cout << "Success loading data" << endl;
		}
	}

	
	return true;
}

int ASampleIndex = 0;

chrono::milliseconds MsPerFrame(15);

auto PastTime = chrono::system_clock::now();

float Size = 500.0f;
float Strength = 25.0f;
float FadeAmount = 2.0f;

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

			PastTime = chrono::system_clock::now();
		}
	}

	/*if (IsKeyPressed(81) && AudioInitiated) {
		cout << "Key detected\n";
		StopSound(LoadedSound);
		UnloadSound(LoadedSound);
		CloseAudioDevice();
		AudioInitiated = false;
	}*/

	auto DurationSincePast = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - PastTime);

	try {
		if (CompiledSamples[ASampleIndex] >= 0.001f) Size = 500 + (CompiledSamples[ASampleIndex] * Strength);
		if(Size > 500.0f)  Size -= FadeAmount;
		ASampleIndex = (floor(DurationSincePast.count() / 1000.0f) * 60) + ceil((DurationSincePast.count() % 1000) / 16.667f);
		GuiDrawRectangle(Rectangle_{ 0, 0, Size, Size}, 15, WHITE, DARKGRAY);
	}
	catch(int error){
		cout << "Done playing " << error << endl;
		ASampleIndex = 0;
		return false;
	}

	return true;
}