#include <iostream>
#include <chrono>
#include <filesystem>
#include "opencv2/opencv.hpp"
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
void resizearray(float**& array, int oldRows, int oldCols, int newRows, int newCols);
int fileexists(const fs::path& p, fs::file_status s = fs::file_status{});

FileManager File_Manager;
SoundManager Sound_Manager;
char* FilePath;

char* BackgroundFilePath = NULL;
char* BgVideoFilePath = NULL;
char* CircleFilePath = NULL;
Texture2D BackgroundTexture;
Texture2D CircleTexture;
float CircleScaleX;
float CircleScaleY;
float BGScaleY = 0;
float BGScaleX = 0;
int ChosenImageScaleMode = 0;
bool ImageDropDownEditMode = false;

int BarMode = 0;
bool BarDropDownEditMode = false;

int bufferSize;

float** CompiledSamples;

int fps = 120;

float largest_ = 0;
float smallest_ = 10000;
float* largest = &largest_;
float* smallest = &smallest_;

bool ElementSwitches[] = {true, true, true, true};
const char* ElementNames[] = {"Bars", "Color Circle", "Bass Circle", "Clip Playback"};

float UiMargin = 14.0f;
float SettingMargin;

bool SamplesReady = false;

Color VizColors[] = {BLACK, WHITE, WHITE, WHITE, WHITE};

int main() {
	SetConfigFlags(FLAG_WINDOW_TRANSPARENT); // Configures window to be transparent
	//Setting window properties
	InitWindow(NULL, NULL, "Music Visualizer");
	SetTargetFPS(60);
	ScreenSize.y = GetScreenHeight();
	ScreenSize.x = GetScreenWidth();
	SetWindowSize(ScreenSize.x, ScreenSize.y);

	SettingMargin = (ScreenSize.x / 3.0f);

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

cv::VideoCapture CappedVideo;

int mainscreen(){
	//Categories of options
	GuiPanel(Rectangle_{ 0, 0, SettingMargin , static_cast<float>(ScreenSize.y) }, "Song Selection");
	GuiPanel(Rectangle_{ SettingMargin, 0, ScreenSize.x * (2/3.0f) , ScreenSize.y * 1.0f}, "Visual Settings");

	/// Background Settings
		//Bg image picking
		if (GuiButton(Rectangle_{ SettingMargin + UiMargin + 185, 59.0f, 240.0f, 28.0f }, "Choose Bg Image")) {

			BackgroundFilePath = NULL;

			cout << "BG File window opened" << endl;

			BackgroundFilePath = const_cast<char*>(File_Manager.OpenFileExplorer(1));

			/// \todo IT KEEPS THROWING AN ERROR I CANT CATCH WHEN USER DOESNT CHOOSE ANY FILE
			try {
				if (fileexists(BackgroundFilePath) == 1) {
					timedtext(2, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
					return 0;
				}
			}
			catch (int error) {
				timedtext(2, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
				return 0;
			}

			cout << BackgroundFilePath << endl;

			Image BackgroundImage = LoadImage(BackgroundFilePath);
			BackgroundTexture = LoadTextureFromImage(BackgroundImage);

			BGScaleX = static_cast<float>(ScreenSize.x) / BackgroundImage.width;
			BGScaleY = static_cast<float>(ScreenSize.y) / BackgroundImage.height;
			UnloadImage(BackgroundImage);
		}

		//Dropdown for image scale types
		if (GuiDropdownBox(Rectangle_{ SettingMargin + UiMargin + 185, 129.0f, 240.0f, 28.0f }, "Height;Width", &ChosenImageScaleMode, ImageDropDownEditMode)) ImageDropDownEditMode = !ImageDropDownEditMode;

		//Choosing bg color
		GuiLabel(Rectangle_{ SettingMargin + UiMargin, 24.0f, 150, 28}, "Background Settings");
		GuiColorPicker(Rectangle_{ SettingMargin + UiMargin, 59.0f, 150, 150 }, "", &VizColors[0]);

		//bg video picking
		if (GuiButton(Rectangle_{ SettingMargin + UiMargin + 185, 94.0f, 240.0f, 28.0f }, "Pick Background Video")) {

			BgVideoFilePath = NULL;

			cout << "BG Video File window opened" << endl;

			BgVideoFilePath = const_cast<char*>(File_Manager.OpenFileExplorer(2));

			cout << BgVideoFilePath << endl;

			/// > Load Video + details
			CappedVideo.open(BgVideoFilePath);

			// Check if the video file opened successfully
			if (!CappedVideo.isOpened()) {
				std::cerr << "Error: Unable to open video file." << std::endl;
				return -1;
			}

			///BGScaleX = static_cast<float>(ScreenSize.x) / BackgroundImage.width;
			///BGScaleY = static_cast<float>(ScreenSize.y) / BackgroundImage.height;
			///UnloadImage(BackgroundImage);
		}

	//File reading
	/// \todo Weird bug where new file isnt loaded after coming back from vizscreen
	if (GuiButton(Rectangle_{ UiMargin, 34.0f, 160.0f, 28.0f }, "Open File")) {

		FilePath = NULL;

		cout << "File window opened" << endl;

		FilePath = const_cast<char*>(File_Manager.OpenFileExplorer(0));

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

		delete[] CompiledSamples;
		
		const int Channels = sfinfo.channels;

		/// Allocate buffer for audio samples
		bufferSize = sfinfo.frames;
		cout << bufferSize / Channels << endl;

		CompiledSamples = new float*[51];
		for (int i = 0; i < 51; ++i){
			CompiledSamples[i] = new float[bufferSize / Channels];
		}

		sf_close(audio_file);

		EndDrawing();
		if (Sound_Manager.readsamples(FilePath, CompiledSamples, ScreenSize.x, ScreenSize.y, fps, largest, smallest) == 1) return 1;
		else SamplesReady = true;

		if (CompiledSamples != NULL) cout << "Success loading data" << endl;

		cout << *largest << " | " << *smallest << endl;
	}

	

	// Circle Image picking
	if (GuiButton(Rectangle_{ SettingMargin + UiMargin, 223.0f, 240.0f, 28.0f }, "Choose Circle Image")) {

		CircleFilePath = NULL;

		cout << "Circle File window opened" << endl;

		CircleFilePath = const_cast<char*>(File_Manager.OpenFileExplorer(1));

		/// \todo IT KEEPS THROWING AN ERROR I CANT CATCH WHEN USER DOESNT CHOOSE ANY FILE
		try {
			if (fileexists(CircleFilePath) == 1) {
				timedtext(2, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
				return 0;
			}
		}
		catch (int error) {
			timedtext(2, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
			return 0;
		}

		Image CircleImage = LoadImage(CircleFilePath);
		CircleTexture = LoadTextureFromImage(CircleImage);
		

		CircleScaleX = CircleImage.width;
		CircleScaleY = CircleImage.height;
		UnloadImage(CircleImage);

		cout << CircleScaleX << "| " << CircleScaleY << endl;
	}

	GuiLabel(Rectangle_{ SettingMargin + UiMargin, 273.0f, 240.0f, 28.0f }, "Element Selection");

	//Button to switch screens
	if (GuiButton(Rectangle_{ UiMargin, 69.0f, 160.0f, 28.0f }, "Viz Screen")){
		CurrentScreen = 1;
	}

	//Toggles for visual elements + color pickers
	for (int i = 0; i < 4; i++) {
		GuiColorPicker(Rectangle_{ SettingMargin + UiMargin +(i * 185), 333.0f, 150, 150 }, "", &VizColors[i + 1]);
		GuiCheckBox(Rectangle_{ SettingMargin + (UiMargin*2)+(i * 185), 308.0f , 20.0f, 20.0f }, ElementNames[i], &ElementSwitches[i]);
	}

	//Bar style dropdown
	if (GuiDropdownBox(Rectangle_{ SettingMargin + UiMargin, 490.0f, 150.0f, 28.0f }, "Top;Bottom;Left;Right", &BarMode, BarDropDownEditMode)) BarDropDownEditMode = !BarDropDownEditMode;

	return 0;
}

int VizLineAmount = 50;
int VizState = 0;

chrono::system_clock::time_point PastTime;
int CurrentSampleIndex = 0;

bool AudioInitiated = false;
Sound LoadedSound = Sound{};

float BassCircleRadius = 75.0f;
float MainCircleRadius = 75.0f;
int MainCircleOffset = 40;
float BaseCircleRadius = 75.0f;
int CircleRange = 25;
chrono::system_clock::time_point BassStartTime;
chrono::system_clock::time_point BeatStartTime;

Texture2D Frame;
float ElapsedTime = 0.0f;
int BgVideoIndex = 1;
float BgFpsFactor = 1 / 24;

int countdigits(int number) {
	std::string strNumber = std::to_string(number);
	return strNumber.length();  // or strNumber.size()
}

int vizscreen() {
	

	float Scale;
	switch (ChosenImageScaleMode) {
	case 0:
		if (BGScaleY != 0) Scale = BGScaleY;
		else BGScaleY = static_cast<float>(ScreenSize.y) / 1080; 
		break;
	case 1:
		if (BGScaleX != 0) Scale = BGScaleX;
		else BGScaleX = static_cast<float>(ScreenSize.x) / 1920;
		break;
	}

	if (BgVideoFilePath != NULL && CurrentSampleIndex != 0) {
		ClearBackground(BLANK);
		/*// Update
		float deltaTime = GetFrameTime();
		ElapsedTime += deltaTime;

		//BgVideoIndex = static_cast<int>(CurrentSampleIndex / BgFpsFactor) % 479;
		if (BgVideoIndex == 0) BgVideoIndex++;
		//cout << BgVideoIndex << endl;

		// Assuming your images are named from "0001.png" to "0479.png"
		const int minIndex = 1;  // Minimum index for your images
		const int maxIndex = 479;  // Maximum index for your images

		if (BgVideoIndex < minIndex || BgVideoIndex > maxIndex) {
			std::cerr << "Error: Invalid index " << BgVideoIndex << ". Out of range." << std::endl;
		}

		// Use std::ostringstream to format the image file name
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(4) << BgVideoIndex << ".png";

		char* result = new char[strlen(const_cast<const char*>(oss.str().c_str())) + strlen(BgVideoFilePath) + 2]; // +1 for the slash, +1 for the null terminator

		// Copy the path to the result
		strcpy(result, BgVideoFilePath);

		// Check if the path already ends with a slash
		if (result[strlen(result) - 1] != '\\') {
			// If not, add a slash
			strcat(result, "\\" );
		}

		// Concatenate the file name to the result
		strcat(result, oss.str().c_str());


		// Check if it's time to switch to the next image
		if (ElapsedTime >= BgFpsFactor) {
			BgVideoIndex = (BgVideoIndex + 1) % 479; // Cycle through images
			ElapsedTime = 0.0f; // Reset elapsed time
			Image FrameImage = LoadImage(result);
			Frame = LoadTextureFromImage(FrameImage);
			UnloadImage(FrameImage);
		}
		//cout << result << endl;
		
		DrawTextureEx(Frame, { 0 , 0 }, 0.0f, 1.0f, WHITE);
		
		delete[] result;*/
	}
	else if(BackgroundFilePath != NULL) {
		DrawTextureEx(BackgroundTexture, { 0, 0 }, 0.0f, Scale, VizColors[0]);
	}
	else
	{
		DrawRectangle(0, 0, ScreenSize.x, ScreenSize.y, VizColors[0]);
	}

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
			MainCircleRadius = BaseCircleRadius;
			BassCircleRadius = BaseCircleRadius;
			VizState = 0;
			CurrentSampleIndex = 0;
			return 0;
		}

		double DurationSincePast = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - PastTime).count() / 1000.0f;

		try {
			CurrentSampleIndex = round(DurationSincePast * fps);

			if (CurrentSampleIndex >= bufferSize / fps) {
				cout << "Finished Playing" << endl;
				return 0;
			}

			if (ElementSwitches[0]) {
				int FreqSkipRate = fps / 30;

				for (int i = 0; i < VizLineAmount; i++) {

					int ModuloIndex = round(CurrentSampleIndex % FreqSkipRate);
					float t = ModuloIndex / static_cast<float>(FreqSkipRate);
					int FreqIndex = CurrentSampleIndex / FreqSkipRate;

					float Height = std::lerp(CompiledSamples[i + 1][FreqIndex], CompiledSamples[i + 1][FreqIndex + 1], t);

					float RemappedHeight = ((Height * 100 / *largest) * 2) + 5;

					switch (BarMode) {
					case 0:
						DrawRectangle((ScreenSize.x / 2) + (((-VizLineAmount / 2.0f) + i) * 20), 24, 15, RemappedHeight, VizColors[1]);
						break;
					case 1:
						DrawRectangle((ScreenSize.x / 2) + (((-VizLineAmount / 2.0f) + i) * 20), (ScreenSize.y / 1.1f) - (RemappedHeight + 24), 15, RemappedHeight, VizColors[1]);
						break;
					case 2:
						DrawRectangle(24, (ScreenSize.y / 2) + (((-VizLineAmount / 1.9f) + i) * 20), RemappedHeight, 15, VizColors[1]);
						break;
					case 3:
						DrawRectangle(ScreenSize.x - (RemappedHeight + 24), (ScreenSize.y / 1.9f) + (((-VizLineAmount / 2.0f) + i) * 20), RemappedHeight, 15, VizColors[1]);
						break;
					}
				}
			}

			// Main Circle

			if (ElementSwitches[1]) {
				///cout << CompiledSamples[1][CurrentSampleIndex] + MainCircleOffset << endl;
				if ((CompiledSamples[1][CurrentSampleIndex] + MainCircleOffset) > 75 && (CompiledSamples[1][CurrentSampleIndex] + MainCircleOffset) >= MainCircleRadius) {
					BeatStartTime = chrono::system_clock::now();
					MainCircleRadius = CompiledSamples[1][CurrentSampleIndex] + MainCircleOffset;
				}
				else if (MainCircleRadius > BaseCircleRadius) {
					float DurationSinceBeat = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - BeatStartTime).count() / 1000.0f;
					MainCircleRadius -= (DurationSinceBeat * 8.0f);
				}
		
				
				if (CircleFilePath == NULL) DrawCircle(ScreenSize.x / 2, ScreenSize.y / 2, MainCircleRadius, VizColors[2]);
				else {
					DrawTextureEx(CircleTexture, { (ScreenSize.x / 2) - MainCircleRadius, (ScreenSize.y / 2) - MainCircleRadius }, 0.0f, (MainCircleRadius * 2) / std::max(CircleScaleX, CircleScaleY), VizColors[2]);
				}
			}

			// Bass Circle
			if (ElementSwitches[2]) {
				if (CompiledSamples[0][CurrentSampleIndex] * 1000.0f < -175.0f) {
					BassStartTime = chrono::system_clock::now();
					BassCircleRadius -= (CompiledSamples[0][CurrentSampleIndex] * 100.0f);
				}
				else if (BassCircleRadius > BaseCircleRadius) {
					float DurationSinceBeat = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - BassStartTime).count() / 1000.0f;
					BassCircleRadius = (BaseCircleRadius + CircleRange) - (CircleRange * (DurationSinceBeat * 16.0f));
				}

				DrawCircleLines(ScreenSize.x / 2, ScreenSize.y / 2, BassCircleRadius + (MainCircleRadius - BaseCircleRadius), VizColors[3]);
			}

			if (ElementSwitches[3]) {
				/// \todo Video playback logic
			}
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
			return 1;
		}

		break;
	}
	
	//DrawTextEx(GetFontDefault(), "WORK IN PROGRESS", Vector2{ (ScreenSize.x / 2) - 80, (ScreenSize.y / 2) - 10 }, 20, 1, RAYWHITE);
	return 0;
}

