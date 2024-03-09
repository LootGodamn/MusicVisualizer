#include <iostream>
#include "SoundManager.h"
#include "FileManager.h"

#include <sndfile.h>
#include <fstream>
#include <fftw-3.3.5-dll64/fftw3.h>
#include <cmath>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

using namespace std;

Color TranslucentOverlay = Color{ 10, 10, 10, 100 };
const int numFrequencyBins = 50;

int SoundManager::readsamples(const char* FilePath, float** TargetArray, int ScreenW, int ScreenH, int fps, float* largest, float* smallest) {

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

	float* buffer = new (std::nothrow) float[bufferSize * Channels];

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

				//Reading Amplitude
				float SampleSum = 0;
				for (int SubSample = 0; SubSample < fps; SubSample++) {
					SampleSum += buffer[(i + SubSample) * Channels];
				}

				float SampleAvg = SampleSum / fps;

				TargetArray[0][i / SampleSkipRate] = SampleAvg;
			}
		}

		cout << bufferSize << endl;

	/// > FAST FOURIER TRANSFORM ------------------------------------------------------------------------------------------------------------------------------

		int FreqCalcSkipRate = sfinfo.samplerate / 30;

		// Initialize FFTW plan
		fftwf_complex* in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FreqCalcSkipRate);
		for (int i = 0; i < FreqCalcSkipRate; i++) {
			in[i][0] = 0;
			in[i][1] = 0; 
		}

		fftwf_complex* out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FreqCalcSkipRate);

		// Loop through the entire array

		fftwf_plan plan = fftwf_plan_dft_1d(FreqCalcSkipRate, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

		for (int i = 0; i < ArraySize; i += FreqCalcSkipRate) {
			// Ensure that we don't go beyond the end of the array
			int EndIndex = std::min(i + FreqCalcSkipRate, ArraySize);

			// Access and print the {fps} elements in between
			for (int j = i; j < EndIndex; ++j) {
				in[j - i][0] = buffer[j];
			}

			fftwf_execute(plan);

			// Output the values of different frequency bins
			for (int bin = 0; bin < numFrequencyBins; ++bin) {
				// Frequency calculation: bin * (sampling rate / array size)
				//double frequency = static_cast<double>(bin) * (sfinfo.samplerate / ArraySize);

				/// >IF Freq is inaccurate, change (fps / 2) to sfinfo.samplerate
				int binIndex = bin * FreqCalcSkipRate / numFrequencyBins;
				float Magnitude = sqrt(out[binIndex][0] * out[binIndex][0] + out[binIndex][1] * out[binIndex][1]);
				
				TargetArray[bin + 1][i / FreqCalcSkipRate] = Magnitude;
				if (*largest < Magnitude) *largest = Magnitude;

				///std::cout << "Bin " << bin << ": Magnitude = " << Magnitude << "\n";
			}
		}
		
		fftwf_destroy_plan(plan);
		
		fftwf_free(in);
		fftwf_free(out);

	/// > FAST FOURIER TRANSFORM ----------------------------------------------------------------------------------------------------------------------------------

		std::cout << "Successfully read and processed " << bytesRead << " audio samples." << std::endl;
		sf_close(File);
		EnableCursor();
		delete[] buffer;
		return 0;
	}
}