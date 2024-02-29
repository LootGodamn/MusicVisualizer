#include <iostream>
#include "SoundManager.h"
#include "FileManager.h"

#include <sndfile.h>
#include <fstream>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

using namespace std;

Color TranslucentOverlay = Color{ 10, 10, 10, 100 };

int SoundManager::readsamples(const char* FilePath, float* TargetArray, int ScreenW, int ScreenH, int fps, float* largest, float* smallest) {

	/// >Reading Data from audio file --------------------------------------------------------------
		/// Read audio samples into the buffer
		/// Buffer size cant be big enough -> Throws access violation error 
		/// Either buffer[] size too small or audio_file got too little frames
		/// 
		/// Ensure the read bytes are not NULL
		/// Loop through each sample
			/// Find largest
			/// Find smallest
			/// Transfer valid samples to an array
		/// Write data
	/// > -------------------------------------------------------------------------------------------
		
	// Read audio samples into the buffer
	// Buffer size cant be big enough -> Throws access violation error 
	// Either buffer[] size too small or audio_file got too little frames

	SF_INFO sfinfo;

	SNDFILE* File = sf_open(FilePath, SFM_READ, &sfinfo);

	const int Channels = sfinfo.channels;
	const int bufferSize = sfinfo.frames;
	const int ArraySize = bufferSize / Channels;

	static float* buffer = new (std::nothrow) float[bufferSize * Channels];

	/// Throw error if buffer fails to declare
	if (!buffer) {
		std::cerr << "Error allocating memory for the buffer." << std::endl;
		sf_close(File);
		return 1;
	}

	sf_count_t bytesRead = sf_readf_float(File, buffer, bufferSize);

	if (bytesRead < 0) {
		std::cerr << "Error reading audio samples: " << sf_strerror(File) << std::endl;
		return 1;
	}
	else {
		int SampleSkipRate = sfinfo.samplerate / fps;

		BeginDrawing();
				DisableCursor();
				GuiDrawRectangle(Rectangle_{ 0,0,static_cast<float>(ScreenW), static_cast<float>(ScreenH) }, 0, BLACK, TranslucentOverlay);
				//GuiDrawText("Reading File..", Rectangle_{ (ScreenW / 2.0f) - 130, (ScreenH / 2.0f) - 40, 260, 35 }, 1, RAYWHITE);
				DrawTextEx(GetFontDefault(), "Reading File..", Vector2{ (ScreenW / 2.0f) - 80, (ScreenH / 2.0f) - 10 }, 20, 1, RAYWHITE);
		EndDrawing();
		

		for (sf_count_t i = 0; i < bytesRead; i += SampleSkipRate) {
			if (i < bytesRead) {

				float SampleSum = 0;
				for (int SubSample = 0; SubSample < fps; SubSample++) {
					SampleSum += buffer[(i + SubSample) * Channels];
				}

				float SampleAvg = SampleSum / fps;

				TargetArray[i / SampleSkipRate] = SampleAvg;
				if (*largest < SampleAvg) *largest = SampleAvg;
				if (*smallest > SampleAvg) *smallest = SampleAvg;
			}
		}

		cout << bufferSize << endl;

		std::cout << "Successfully read and processed " << bytesRead << " audio samples." << std::endl;
		sf_close(File);
		EnableCursor();
		return 0;
	}
}

int* SoundManager::findpitch(const char* FilePath, int Hz) { return 0; }
int* SoundManager::findloudness(const char* FilePath, int Hz) { return 0; }
int* SoundManager::freedata() { return 0; }