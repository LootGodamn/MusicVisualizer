#include <iostream>
#include <chrono>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

#include "FileManager.h"
#include "SoundManager.h"
#include <sndfile.h>

#include "raylib.h"
#include "raygui.h"

#include "cyber/style_cyber.h"


Vector2 ScreenSize;
int CurrentScreen = 0;

int mainscreen();
int vizscreen();
void timedtext(int Duration, const char* DisplayText);
int fileexists(const fs::path& p, fs::file_status s = fs::file_status{});

FileManager File_Manager;
SoundManager Sound_Manager;
const char* FilePath;

int bufferSize;

float** CompiledSamples;
float* PitchSamples;
float* LoudnessSamples;

int fps = 120;

float largest_ = -10000;
float smallest_ = 10000;
float* largest = &largest_;
float* smallest = &smallest_;

bool SamplesReady = false;

int main() {
	//Setting window properties
	InitWindow(NULL, NULL, "Music Visualizer");
	SetTargetFPS(60);
	ScreenSize.y = GetScreenHeight();
	ScreenSize.x = GetScreenWidth();
	SetWindowSize(ScreenSize.x, ScreenSize.y);

	//Refocus Window to fit height
	SetWindowState(STATE_DISABLED);

	GuiLoadStyleCyber();

	cout << ScreenSize.y << "|" << ScreenSize.x << endl;

	SetWindowState(STATE_FOCUSED);

	//Screen selection
	while (!WindowShouldClose()) {

		BeginDrawing();

		switch (CurrentScreen) {
		case 0:
			switch (mainscreen()) {
			case 1:
				return 1;
				break;
			case 2:
				timedtext(1, "NO FILE SELECTED");
				break;
			}
			break;
			break;

		case 1:
			switch (vizscreen()) {
			case 1:
				return 1;
				break;
			case 2:
				CurrentScreen = 0;
				timedtext(1, "NO AUDIO FILE LOADED");
				break;
			}
			break;
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

void timedtext(int Duration, const char* DisplayText) {

	EndDrawing();

	auto TextBeginTime = chrono::system_clock::now();
	chrono::seconds TimeSinceBeginning = chrono::seconds(0);

	while (TimeSinceBeginning.count() < Duration) {
		TimeSinceBeginning = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - TextBeginTime);
		BeginDrawing();
		DrawRectangle(0, 0, ScreenSize.x, ScreenSize.y, Color{ 10, 10, 10, 255});
		DrawTextEx(GetFontDefault(), DisplayText, Vector2{ (ScreenSize.x / 2) - 120, (ScreenSize.y / 2) - 10 }, 20, 1, RAYWHITE);
		EndDrawing();
	}
}

int fileexists(const fs::path& p, fs::file_status s)
{
	std::cout << p;
	if (fs::status_known(s) ? fs::exists(s) : fs::exists(p))
		return 0;
	else
		return 1;
}

int mainscreen(){
	//Categories of options
	GuiPanel(Rectangle_{ 0, 0, ScreenSize.x / 3.0f , static_cast<float>(ScreenSize.y) }, "Song Selection");
	GuiPanel(Rectangle_{ ScreenSize.x / 3.0f, 0, ScreenSize.x / 3.0f , ScreenSize.y * 1.0f }, "Properties");
	GuiPanel(Rectangle_{ (ScreenSize.x / 3.0f) * 2, 0, ScreenSize.x / 3.0f , ScreenSize.y * 1.0f }, "Element Selection");

	//File reading
	/// \todo Only one file can be loaded per run. User should be able to open another file without errors while a file is already loaded
	if (GuiButton(Rectangle_{ 14.0f, 34.0f, 160.0f, 28.0f }, "Open File")) {

		FilePath = NULL;

		cout << "File window opened" << endl;

		FilePath = File_Manager.OpenFileExplorer();

		/// \todo IT KEEPS THROWING AN ERROR I CANT CATCH WHEN USER DOESNT CHOOSE ANY FILE
		try {
			if (fileexists(FilePath) == 1) {
				timedtext(2, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
				return 0;
			}
		}
		catch (int error) {
			timedtext(2, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
			return 0;
		}

		/// Open the audio file

		SNDFILE* audio_file;
		SF_INFO sfinfo;
		
		audio_file = sf_open(FilePath, SFM_READ, &sfinfo);

		/// Throw error if file not loaded
		if (!audio_file) {
			std::cerr << "Error opening the file: " << sf_strerror(nullptr) << std::endl;
			return 1;
		}
		
		const int Channels = sfinfo.channels;

		/// Allocate buffer for audio samples
		bufferSize = sfinfo.frames;
		cout << bufferSize / Channels << endl;

		CompiledSamples = new float*[11];
		for (int i = 0; i < 11; ++i){
			CompiledSamples[i] = new float[bufferSize / Channels];
		}

		sf_close(audio_file);

		EndDrawing();
		if (Sound_Manager.readsamples(FilePath, CompiledSamples, ScreenSize.x, ScreenSize.y, fps, largest, smallest) == 1) return 1;
		else SamplesReady = true;

		if (CompiledSamples != NULL) cout << "Success loading data" << endl;

		cout << *largest << " | " << *smallest << endl;
	}

	//Button to switch screens
	if (GuiButton(Rectangle_{ (ScreenSize.x / 2) - 80, (ScreenSize.y / 1.25f), 160.0f, 35.0f }, "Viz Screen")){
		CurrentScreen = 1;
	}

	return 0;
}

int VizLineAmount = 10;
int VizState = 0;

chrono::system_clock::time_point PastTime;
int CurrentSampleIndex = 0;

bool AudioInitiated = false;
Sound LoadedSound = Sound{};

float CircleRadius = 75.0f;
float BaseCircleRadius = 75.0f;
int CircleRange = 25;
chrono::system_clock::time_point BeatStartTime;

int vizscreen() {
	DrawRectangle(0, 0, ScreenSize.x, ScreenSize.y, Color{ 10, 10, 10, 255});

	if (!SamplesReady) return 2;

	switch (VizState) {
	
	case 0: // Initalizing stage
		if (!AudioInitiated) {
			InitAudioDevice();

			if (!IsAudioDeviceReady()) { cout << "Audio device unable to initialize" << endl; return false; }

			AudioInitiated = true;
			cout << "Audio Initiated\n";

			LoadedSound = LoadSound(FilePath);
			SetSoundVolume(LoadedSound, 0.2f);
		}

		/*for (int i = 0; i < VizLineAmount; i++) {
			VizLineHeights[i] = 5;
		}*/
		VizState = 1;
		break;
	case 1: // StandBy stage (wait player to start play)

		//Button to switch screens
		if (GuiButton(Rectangle_{ 24, 12, 160.0f, 35.0f }, "Back")) {
			CurrentScreen = 0;
			VizState = 0;
		}

		// Play Music and Visuals
		if (GuiButton(Rectangle_{ (ScreenSize.x / 2) - 12, (ScreenSize.y / 1.25f), 24, 24 }, GuiIconText(131, ""))) {
			
			VizState = 0;
			if (IsSoundReady(LoadedSound) && !IsSoundPlaying(LoadedSound)) {
				PlaySound(LoadedSound);
				PastTime = chrono::system_clock::now();
				cout << *largest << " | " << *smallest << endl;
				VizState = 2;
			}
		}

		break;
	case 2: // Playing in progress
		if (GuiButton(Rectangle_{(ScreenSize.x / 2) - 12, (ScreenSize.y / 1.25f), 24, 24}, GuiIconText(133, ""))) {
			// Stop Music and Visuals
			StopSound(LoadedSound);
			CircleRadius = BaseCircleRadius;
			VizState = 0;
			CurrentSampleIndex = 0;
		}

		auto DurationSincePast = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - PastTime);

		try {
			CurrentSampleIndex = round((DurationSincePast.count() / 1000.0f) * fps);

			if (CurrentSampleIndex >= bufferSize / fps) {
				cout << "Finished Playing" << endl;
				return 0;
			}

			int FreqSkipRate = fps / 8;

			for (int i = 0; i < VizLineAmount; i++) {

				/// \todo Remap amplitude values using largest and smallest to a reasonable range.
				/// > Normal remap functions dont work :[
				//int RemappedHeight = 0 + (CompiledSamples[CurrentSampleIndex + i] - *smallest) * (100 - 0) / (*largest - *smallest);
				//float RemappedHeight = 0 + (100 - 0) * ((CompiledSamples[CurrentSampleIndex + i] - *smallest) / (*largest - *smallest));
				
				//a* (1.0 - t) + b * t;

				int ModuloIndex = round(CurrentSampleIndex % FreqSkipRate);
				float t = ModuloIndex / static_cast<float>(FreqSkipRate);
				int FreqIndex = CurrentSampleIndex / FreqSkipRate;

				float Height = std::lerp(CompiledSamples[i + 1][FreqIndex], CompiledSamples[i + 1][FreqIndex + 1], t);

				if (i == 1) cout << FreqIndex << " [" << t << "] " << Height << endl;
				//float Height = CompiledSamples[i + 1][(CurrentSampleIndex / (fps / 2))] * 200;
				DrawRectangle((i * 20), 14, 15, Height, RAYWHITE);

				//Mirror Dupe
				DrawRectangle((ScreenSize.x - 20) - (i * 20), 14, 15, Height, RAYWHITE);
			}

			// Main Circle

			if (CompiledSamples[0][CurrentSampleIndex] * 1000.0f < -175.0f) {
				BeatStartTime = chrono::system_clock::now();
				CircleRadius -= (CompiledSamples[0][CurrentSampleIndex] * 100.0f);
			}
			else if (CircleRadius > BaseCircleRadius) {
				float DurationSinceBeat = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - BeatStartTime).count() / 1000.0f;
				CircleRadius = (BaseCircleRadius + CircleRange) - (CircleRange * (DurationSinceBeat * 16.0f));
			}

			//DrawCircle(ScreenSize.x / 2, ScreenSize.y / 2, CircleRadius, RAYWHITE);
			DrawCircle(ScreenSize.x / 2, ScreenSize.y / 2, 75, DARKBLUE);
			DrawCircleLines(ScreenSize.x / 2, ScreenSize.y / 2, CircleRadius, SKYBLUE);
		}
		catch (int error) {
			cout << "An Error occured while processing visuals : " << error << endl;

			/// All just to close playback and visuals
			StopSound(LoadedSound);
			UnloadSound(LoadedSound);
			CloseAudioDevice();
			AudioInitiated = false;
			CurrentSampleIndex = 0;
			delete[] CompiledSamples;
			delete[] PitchSamples;
			delete[] LoudnessSamples;

			return 1;
		}

		break;
	}
	
	//DrawTextEx(GetFontDefault(), "WORK IN PROGRESS", Vector2{ (ScreenSize.x / 2) - 80, (ScreenSize.y / 2) - 10 }, 20, 1, RAYWHITE);
	return 0;
}