#include <iostream>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <cyber/style_cyber.h>

#include "FileManager.h"

#include <sndfile.h>

using namespace std;

FileManager File_Manager;
int CurrentScreen = 0;

bool main_screen();
bool AudioInitiated = false;

int ScreenH;
int ScreenW;

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

		switch (CurrentScreen) {
			case 0:
				if (!main_screen()) return 1;
				break;

			case 1:
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

	Sound LoadedSound = Sound{};

	if (GuiButton(Rectangle_{ 14.0f, 34.0f, 160.0f, 28.0f }, "Open File")) {

		const char* FilePath = NULL;

		cout << "button clicked" << endl;
		FilePath = File_Manager.OpenFileExplorer();

		if (FilePath == NULL) { cerr << "File failed to load\n"; return false; }

		InitAudioDevice();

		if (!IsAudioDeviceReady()) { cerr << "Audio device unable to initialize" << endl; return false;}

		AudioInitiated = true; 
		cout << "Audio Initiated\n";

		LoadedSound = LoadSound(FilePath);
		SetSoundVolume(LoadedSound, 0.2f);

		if (IsSoundReady(LoadedSound) && !IsSoundPlaying(LoadedSound)) {
			PlaySound(LoadedSound);
		}

		//Reading Data from audio file

		// Open the audio file
		SF_INFO sfinfo;
		SNDFILE* sndfile = sf_open(FilePath, SFM_READ, &sfinfo);

		if (!sndfile) {
			std::cerr << "Error opening the file: " << sf_strerror(nullptr) << std::endl;
			return -1;
		}

		// Allocate buffer for audio samples
		const int bufferSize = 1024; // Adjust as needed
		float buffer[bufferSize];

		// Read audio samples into the buffer
		sf_count_t bytesRead = sf_readf_float(sndfile, buffer, bufferSize);

		if (bytesRead < 0) {
			std::cerr << "Error reading audio samples: " << sf_strerror(sndfile) << std::endl;
		}
		else {
			// Process the audio samples in the 'buffer' array
			for (sf_count_t i = 0; i < bytesRead; ++i) {
				// Your processing logic here
				cout << buffer[i];
				// 'buffer[i]' contains the audio sample at index 'i'
			}

			std::cout << "Successfully read and processed " << bytesRead << " audio samples." << std::endl;
		}

		// Close the audio file
		sf_close(sndfile);

	}

	if (IsKeyPressed(81) && AudioInitiated) {
		cout << "Key detected\n";
		StopSound(LoadedSound);
		UnloadSound(LoadedSound);
		CloseAudioDevice();
	}
	return true;
}

bool viz_screen() {
	return true;
}