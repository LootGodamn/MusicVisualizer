#include <iostream>
#include "SoundManager.h"
#include <sndfile.h>

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

	// Allocate buffer for audio samples
	const int bufferSize = 3837788;
	static float* buffer = new (std::nothrow) float[sfinfo.frames] {0};
	CompiledFrames = new float[3837788 / Fps] {0};
	cout << sfinfo.frames << endl;
	cout << bufferSize << endl;

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
		// Process the audio samples in the 'buffer' array
		for (sf_count_t i = 0; i < bytesRead; i += (sfinfo.samplerate / Fps)) {
			cout << buffer[i] << endl;
			CompiledFrames[i/60] = buffer[i];
		}

		std::cout << "Successfully read and processed " << bytesRead << " audio samples." << std::endl;
	}

	// Clean up data
	delete[] buffer;
	sf_close(audio_file);

	return CompiledFrames;
}