#include <iostream>
#include "SoundManager.h"
#include "FileManager.h"
#include <sndfile.h>
#include <fstream>

using namespace std;

float *CompiledFrames;

float* SoundManager::read_samples(const char* FilePath, int Fps) {

	// Reading Data from audio file

	// Open the audio file
	SF_INFO sfinfo;
	SNDFILE* audio_file = sf_open(FilePath, SFM_READ, &sfinfo);

	// Throw error if file not loaded
	if (!audio_file) {
		std::cerr << "Error opening the file: " << sf_strerror(nullptr) << std::endl;
		return NULL;
	}

	const int Channels = sfinfo.channels;

	// Allocate buffer for audio samples
	const int bufferSize = sfinfo.frames;
	static float* buffer = new (std::nothrow) float[bufferSize * Channels] {0};
	CompiledFrames = new float[bufferSize / Fps] {0};

	// Throw error if buffer fails to declare
	if (!buffer) {
		std::cerr << "Error allocating memory for the buffer." << std::endl;
		sf_close(audio_file);
		return NULL;
	}

	// Read audio samples into the buffer
	// Buffer size cant be big enough -> Throws access violation error 
	// Either buffer[] size too small or audio_file got too little frames
	sf_count_t bytesRead = sf_readf_float(audio_file, buffer, bufferSize);

	if (bytesRead < 0) {
		std::cerr << "Error reading audio samples: " << sf_strerror(audio_file) << std::endl;
	}
	else {
		int SampleSkipRate = sfinfo.samplerate / Fps;
		// Process the audio samples in the 'buffer' array
		for (sf_count_t i = 0; i < bytesRead; i += SampleSkipRate) {
			if (i < bytesRead) {
				CompiledFrames[i / Fps] = buffer[i * Channels];
				cout << CompiledFrames[i / Fps] << endl;
			}
		}

		std::cout << "Successfully read and processed " << bytesRead << " audio samples." << std::endl;
	}

	// Clean up data
	delete[] buffer;
	sf_close(audio_file);

	return CompiledFrames;
}