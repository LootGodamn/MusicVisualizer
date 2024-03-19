#include <iostream>
#include <chrono>
#include <filesystem>
#include <cmath>

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

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

FileManager File_Manager;
SoundManager Sound_Manager;
char* FilePath;

char* BackgroundFilePath = NULL;
char* BgVideoFilePath = NULL;
char* CircleFilePath_ = NULL;
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
bool BarWidthEditMode = false;
bool BarGapEditMode = false;

int bufferSize;

float** CompiledSamples;

int fps = 120;

float largest_ = 0;
float* largest = &largest_;
float midpoint_ = 0;
float* midpoint = &midpoint_;

bool ElementSwitches[] = {true, true, true};
const char* ElementNames[] = {"Bars", "Color Circle", "Bass Circle"};

float UiMargin = 14.0f;
float SettingMargin;

bool SamplesReady = false;

Color VizColors[] = {BLACK, WHITE, WHITE, WHITE, WHITE};
char* FileNameDisplay = nullptr;

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
	float PosOffset = (strlen(DisplayText) / 2.0f) * 10;

	while (TimeSinceBeginning.count() < Duration) {
		TimeSinceBeginning = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - TextBeginTime);
		BeginDrawing();
		DrawRectangle(0, 0, ScreenSize.x, ScreenSize.y, Color{ 10, 10, 10, 255});
		DrawTextEx(GetFontDefault(), DisplayText, Vector2{ (ScreenSize.x / 2) - PosOffset, (ScreenSize.y / 2) - 10 }, 20, 1, RAYWHITE);
		EndDrawing();
	}
}

cv::VideoCapture CappedVideo;
double BgFpsDelay;

bool AudioInitiated = false;
Sound LoadedSound = Sound{};
int VizState = 1;
float VizLineHeights[75];

int VizLineGap = 5;
int VizLineWidth = 15;

float BaseCircleRadius;
float MaxCircleRadius;

float BassCircleRadius;
float MainCircleRadius;

int CircleSize = 1;
bool CircleSizeDropDownMode = false;

int mainscreen(){
	//Categories of options
	GuiPanel(Rectangle_{ 0, 0, SettingMargin , static_cast<float>(ScreenSize.y) }, "Song Selection");
	GuiPanel(Rectangle_{ SettingMargin, 0, ScreenSize.x * (2/3.0f) , ScreenSize.y * 1.0f}, "Visual Settings");

	/// Background Settings
		//Bg image picking
		if (GuiButton(Rectangle_{ SettingMargin + UiMargin + 185, 59.0f, 240.0f, 28.0f }, "Choose Bg Image")) {

			BackgroundFilePath = const_cast<char*>(File_Manager.OpenFileExplorer(1));
			if (BackgroundFilePath == nullptr) {
				timedtext(1, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
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

			BgVideoFilePath = const_cast<char*>(File_Manager.OpenFileExplorer(2));
			if (BgVideoFilePath == nullptr) {
				timedtext(1, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
				return 0;
			}

			cout << BgVideoFilePath << endl;

			/// > Load Video + details
			CappedVideo.open(BgVideoFilePath);

			// Check if the video file opened successfully
			if (!CappedVideo.isOpened()) {
				std::cerr << "Error: Unable to open video file." << std::endl;
				return -1;
			}

			BgFpsDelay = CappedVideo.get(cv::CAP_PROP_FPS);

			///BGScaleX = static_cast<float>(ScreenSize.x) / BackgroundImage.width;
			///BGScaleY = static_cast<float>(ScreenSize.y) / BackgroundImage.height;
			///UnloadImage(BackgroundImage);
		}

	//File reading

		if (FileNameDisplay != nullptr) GuiLabel(Rectangle_{ 160.0f + UiMargin + UiMargin, 34.0f, SettingMargin - ((UiMargin * 3)+160.0f), 28.0f }, FileNameDisplay);

	if (GuiButton(Rectangle_{ UiMargin, 34.0f, 160.0f, 28.0f }, "Open File")) {

		FilePath = const_cast<char*>(File_Manager.OpenFileExplorer(0));
		if (FilePath == nullptr) {
			timedtext(1, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
			return 0;
		}

		// Find the last occurrence of directory separator
		const char* LastSeparator = strrchr(FilePath, '/');
		if (LastSeparator == nullptr) {
			LastSeparator = strrchr(FilePath, '\\'); // For Windows paths
		}

		if (LastSeparator != nullptr) {
			// Copy everything after the last separator to the filename buffer
			FileNameDisplay = new char[strlen(LastSeparator)];
			std::strcpy(FileNameDisplay, LastSeparator + 1);
		}
		else {
			FileNameDisplay = nullptr;
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

		CompiledSamples = new float*[76];
		for (int i = 0; i < 76; ++i){
			CompiledSamples[i] = new float[bufferSize / Channels];
		}

		sf_close(audio_file);

		EndDrawing();
		if (Sound_Manager.readsamples(FilePath, CompiledSamples, ScreenSize.x, ScreenSize.y, fps, largest, midpoint) == 1) return 1;
		else SamplesReady = true;

		if (CompiledSamples != NULL) cout << "Success loading data" << endl;

		cout << *largest << " | " << *midpoint << endl;
	}

	

	// Circle Image picking
	if (GuiButton(Rectangle_{ SettingMargin + UiMargin + 185, 490.0f, 150.0f, 28.0f }, "Circle Image")) {

		CircleFilePath_ = const_cast<char*>(File_Manager.OpenFileExplorer(1));
		if (CircleFilePath_ == nullptr) {
			timedtext(1, "ERROR, NO FILE SELECTED OR INVALID SELECTION");
			return 0;
		}

		Image CircleImage = LoadImage(CircleFilePath_);
		CircleTexture = LoadTextureFromImage(CircleImage);

		CircleScaleX = CircleImage.width;
		CircleScaleY = CircleImage.height;
		UnloadImage(CircleImage);

		cout << CircleScaleX << "| " << CircleScaleY << endl;
	}

	if (GuiDropdownBox(Rectangle_{ SettingMargin + UiMargin + 185, 525.0f, 150.0f, 28.0f }, "Small;Medium;Large", &CircleSize, CircleSizeDropDownMode)) CircleSizeDropDownMode = !CircleSizeDropDownMode;

	GuiLabel(Rectangle_{ SettingMargin + UiMargin, 273.0f, 240.0f, 28.0f }, "Element Selection");

	//Button to switch screens
	if (GuiButton(Rectangle_{ UiMargin, 69.0f, 160.0f, 28.0f }, "Viz Screen")){
		switch (CircleSize) {
		case 0:
			BaseCircleRadius = 50.0f;
			break;
		case 1:
			BaseCircleRadius = 100.0f;
			break;
		case 2:
			BaseCircleRadius = 150.0f;
			break;
		}

		MaxCircleRadius = 75.0f + BaseCircleRadius;

		BassCircleRadius = MainCircleRadius = BaseCircleRadius;
		CurrentScreen = 1;
		VizState = 1;
	}

	GuiLabel(Rectangle_{ UiMargin, 104.0f, SettingMargin, 28.0f }, "NOTE : Only mono audio works properly as of now");

	//Toggles for visual elements + color pickers
	for (int i = 0; i < 3; i++) {
		GuiColorPicker(Rectangle_{ SettingMargin + UiMargin +(i * 185), 333.0f, 150, 150 }, "", &VizColors[i + 1]);
		GuiCheckBox(Rectangle_{ SettingMargin + (UiMargin*2)+(i * 185), 308.0f , 20.0f, 20.0f }, ElementNames[i], &ElementSwitches[i]);
	}

	//Bar Width n Gap
	if (GuiSpinner(Rectangle_{ SettingMargin + UiMargin + 50, 525.0f, 75.0f, 28.0f }, "Bar Width", &VizLineWidth, 5, 50, BarWidthEditMode)) BarWidthEditMode = !BarWidthEditMode;
	if (GuiSpinner(Rectangle_{ SettingMargin + UiMargin + 50, 560.0f, 75.0f, 28.0f }, "Bar Gap", &VizLineGap, 0, 30, BarGapEditMode)) BarGapEditMode = !BarGapEditMode;

	//Bar style dropdown
	if (GuiDropdownBox(Rectangle_{ SettingMargin + UiMargin, 490.0f, 150.0f, 28.0f }, "Top;Bottom;Left;Right", &BarMode, BarDropDownEditMode)) BarDropDownEditMode = !BarDropDownEditMode;

	return 0;
}

int VizLineAmount = 75;

float LineMaxHeight = 150;
int PreviousIndex = 0;

chrono::system_clock::time_point PastTime;
int CurrentSampleIndex = 0;

int MainCircleOffset = 0;

int CircleRange = 25;
chrono::system_clock::time_point BassStartTime;
chrono::system_clock::time_point BeatStartTime;

bool WindowInitiated = false;

int VidSpeed = 33;

int MouseDetectionSize = 128;

int countdigits(int number) {
	std::string strNumber = std::to_string(number);
	return strNumber.length();  // or strNumber.size()
}

float FindLineHeight(int FreqSkipRate, int i) {
	int ModuloIndex = round(CurrentSampleIndex % FreqSkipRate);
	float t = ModuloIndex / static_cast<float>(FreqSkipRate);
	int FreqIndex = CurrentSampleIndex / FreqSkipRate;

	float Height = std::lerp(CompiledSamples[i + 1][FreqIndex], CompiledSamples[i + 1][FreqIndex + 1], t);

	return std::min(((Height * 500 / *largest) * 2) + 5, LineMaxHeight);
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

	if (BgVideoFilePath != nullptr && CurrentSampleIndex != 0) {
		ClearBackground(BLANK);
		if (VidSpeed == 1) DrawRectangle(0, 0, ScreenSize.x, ScreenSize.y, Fade(WHITE, 0.1f));
		
		cv::Mat frame;

		// Read a frame from the video
		if (!CappedVideo.read(frame)) {
			//Loop Video
			std::cerr << "Error reading video frame." << std::endl;
		}

		if (frame.empty()) {
			// End of video, reset to the beginning
			CappedVideo.set(cv::CAP_PROP_POS_FRAMES, 0);
		}

		// Display the frame
		if (!WindowInitiated) {
			// Create a window with no decorations and set it to fullscreen
			cv::namedWindow("Bg Video Player", cv::WINDOW_NORMAL);
			cv::setWindowProperty("Bg Video Player", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
			cv::imshow("Bg Video Player", frame);
			SetWindowState(STATE_DISABLED);
			SetWindowSize(ScreenSize.x, ScreenSize.y);
			SetWindowState(STATE_FOCUSED);

			WindowInitiated = true;
		}
		else
		{
			try {
				cv::imshow("Bg Video Player", frame);
			}
			catch (const cv::Exception& e) {
				std::cerr << "OpenCV Exception: " << e.what() << std::endl;
			}
		}

		// Wait for a key press and adjust speed based on key input
		int key = cv::waitKey(VidSpeed);
	}
	else if(BackgroundFilePath != nullptr) {
		DrawTextureEx(BackgroundTexture, { 0, 0 }, 0.0f, Scale, VizColors[0]);
	}
	else
	{
		DrawRectangle(0, 0, ScreenSize.x, ScreenSize.y, VizColors[0]);
	}

	if (!SamplesReady) return 2;

	switch (VizState) {
	case 1: // StandBy stage (wait player to start play)

		//Button to switch screens
		if (GuiButton(Rectangle_{ (ScreenSize.x / 2.0f) - 80, (ScreenSize.y / 1.2f), 160.0f, 35.0f }, "Back")) {
			CurrentScreen = 0;
		}

		// Play Music and Visuals
		if (GuiButton(Rectangle_{ (ScreenSize.x / 2) - 12, (ScreenSize.y / 1.25f), 24, 24 }, GuiIconText(131, ""))) {

			if (!AudioInitiated) {
				if (!IsAudioDeviceReady()) {

					InitAudioDevice();

					if (!IsAudioDeviceReady()) { cout << "Audio device unable to initialize" << endl; return false; }
				}

				AudioInitiated = true;
				cout << "Audio Initiated\n";

				LoadedSound = LoadSound(FilePath);
				SetSoundVolume(LoadedSound, 0.2f);

				for (int i = 0; i < 50; i++) {
					VizLineHeights[i] = 0;
				}
			}
			
			if (IsSoundReady(LoadedSound) && !IsSoundPlaying(LoadedSound)) {
				PlaySound(LoadedSound);
				PastTime = chrono::system_clock::now();
				cout << *largest << " | " << *midpoint << endl;
				VizState = 2;
			}
		}

		break;
	case 2: // Playing in progress
		Vector2 MousePos = GetMousePosition();
		Vector2 PauseBtnPos;
		PauseBtnPos.x = ScreenSize.x / 2;
		PauseBtnPos.y = ScreenSize.y / 1.25f;

		//If Mouse in range
		if ((MousePos.x >= PauseBtnPos.x - MouseDetectionSize && MousePos.x < PauseBtnPos.x + MouseDetectionSize &&
			MousePos.y >= PauseBtnPos.y - MouseDetectionSize && MousePos.y < PauseBtnPos.y + MouseDetectionSize)) {

			if (GuiButton(Rectangle_{ PauseBtnPos.x - 12, PauseBtnPos.y, 24, 24 }, GuiIconText(133, ""))) {
				// Stop Music and Visuals
				StopSound(LoadedSound);
				UnloadSound(LoadedSound);
				MainCircleRadius = BaseCircleRadius;
				BassCircleRadius = BaseCircleRadius;
				VizState = 1;
				CurrentSampleIndex = 0;

				// Release the VideoCapture object
				CappedVideo.release();
				cv::destroyAllWindows();
				WindowInitiated = false;
				AudioInitiated = false;

				cout << "VIZ STATE " << VizState << endl;

				return 0;
			}
		}

		double DurationSincePast = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - PastTime).count() / 1000.0f;

		try {
			CurrentSampleIndex = round(DurationSincePast * fps);

			if (CurrentSampleIndex >= bufferSize / fps) {
				cout << "Finished Playing" << endl;
				return 0;
			}

			if (ElementSwitches[0]) {
				/// \todo The bars end faster and are unsynced on songs that are stereo. Works on mono :/

				for (int i = 0; i < VizLineAmount; i++) {

					float RemappedHeight = FindLineHeight(fps / 30, i);

					if (i == VizLineAmount - 1) MainCircleOffset = RemappedHeight;

					if (RemappedHeight >= VizLineHeights[i]) {
						VizLineHeights[i] = RemappedHeight;
					}
					else if(PreviousIndex < CurrentSampleIndex){
						VizLineHeights[i] = round(VizLineHeights[i] * 0.9f);
					}

					switch (BarMode) {
					case 0:
						DrawRectangle((ScreenSize.x / 2) + (((-VizLineAmount / 2.0f) + i) * (VizLineWidth + VizLineGap)), 24, VizLineWidth, VizLineHeights[i], VizColors[1]);
						break;
					case 1:
						DrawRectangle((ScreenSize.x / 2) + (((-VizLineAmount / 2.0f) + i) * (VizLineWidth + VizLineGap)), (ScreenSize.y / 1.1f) - (VizLineHeights[i] + 24), VizLineWidth, VizLineHeights[i], VizColors[1]);
						break;
					case 2:
						DrawRectangle(24, (ScreenSize.y / 2) + (((-VizLineAmount / 1.9f) + i) * (VizLineWidth + VizLineGap)), VizLineHeights[i], VizLineWidth, VizColors[1]);
						break;
					case 3:
						DrawRectangle(ScreenSize.x - (VizLineHeights[i] + 24), (ScreenSize.y / 1.9f) + (((-VizLineAmount / 2.0f) + i) * (VizLineWidth + VizLineGap)), VizLineHeights[i], VizLineWidth, VizColors[1]);
						break;
					}
				}
				PreviousIndex = CurrentSampleIndex;
			}

			// Main Circle
			if (ElementSwitches[1]) {

				if (!ElementSwitches[0]) {
					MainCircleOffset = FindLineHeight(fps / 30, VizLineAmount - 1);
				}
				
				if (MainCircleOffset + BaseCircleRadius > MainCircleRadius) {
					BeatStartTime = chrono::system_clock::now();
					MainCircleRadius = std::min(static_cast<float>(MainCircleOffset + BaseCircleRadius), MaxCircleRadius);
				}
				else if (MainCircleRadius > BaseCircleRadius) {
					float DurationSinceBeat = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - BeatStartTime).count() / 1000.0f;
					MainCircleRadius -= (DurationSinceBeat * 8.0f);
				}
				
				if (CircleFilePath_ == nullptr) DrawCircle(ScreenSize.x / 2, ScreenSize.y / 2, MainCircleRadius, VizColors[2]);
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

				DrawCircleLines(ScreenSize.x / 2, ScreenSize.y / 2, std::min(BassCircleRadius + (MainCircleRadius - BaseCircleRadius), 450.0f), VizColors[3]);
			}

			if (CompiledSamples[0][CurrentSampleIndex] * 1000.0f < -200.0f) VidSpeed = 1;
			else VidSpeed = 41;
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
			VizState = 1;
			return 1;
		}

		break;
	}
	
	//DrawTextEx(GetFontDefault(), "WORK IN PROGRESS", Vector2{ (ScreenSize.x / 2) - 80, (ScreenSize.y / 2) - 10 }, 20, 1, RAYWHITE);
	return 0;
}

