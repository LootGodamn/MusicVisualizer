#include <iostream>
#include "SoundManager.h"
#include "FileManager.h"
#include <sndfile.h>
#include <fstream>

using namespace std;

float map(float value, float istart, float istop, float ostart, float ostop) {
	return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

int* SoundManager::read_samples(const char* FilePath, int Hz) {

	int* TargetArray;

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
	static float* buffer = new (std::nothrow) float[bufferSize * Channels];
	const int arraySize = (bufferSize / sfinfo.samplerate) * Hz;
	TargetArray = new int[arraySize];

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
		int SampleSkipRate = sfinfo.samplerate / Hz;
		int largest = -10000;
		int smallest = 10000;

		// Process the audio samples in the 'buffer' array
		for (sf_count_t i = 0; i < bytesRead; i += SampleSkipRate) {
			if (i < bytesRead) {

				float SampleSum = 0;
				for (int SubSample = 0; SubSample < 60; SubSample++) {
					SampleSum += buffer[(i + SubSample) * Channels];
				}

				float SampleAvg = ceil((SampleSum * 1000) / Hz);

				TargetArray[i / SampleSkipRate] = SampleAvg;
				if (largest < SampleAvg) largest = SampleAvg;
				if (smallest > SampleAvg) smallest = SampleAvg;
				
			}
		}

		cout << bufferSize / Hz << endl;

		//remap values 
		for (int i = 0; i < arraySize; i++) {
			TargetArray[i] = map(TargetArray[i], smallest, largest, -100, 100);
			cout << TargetArray[i] << endl;
		}

		std::cout << "Successfully read and processed " << bytesRead << " audio samples." << std::endl;
	}

	//smooth values

	int* SmoothedArray = new int[arraySize];
	for (int i = 0; i < arraySize; i++) {
		if (i == 0) SmoothedArray[0] = (TargetArray[0] + TargetArray[1]) / 2;
		else if (i == arraySize - 1) SmoothedArray[arraySize - 1] = (TargetArray[arraySize - 1] + TargetArray[arraySize - 2]) / 2;
		else {
			int avg = 0;
			int samplesize = 5;
			for (int a = -samplesize; a < samplesize + 1; a++) {
				avg += TargetArray[i + a];
			}
			avg /= (samplesize * 2) + 1;
			SmoothedArray[i] = avg;
		}
	}

	// Clean up data
	delete[] buffer;
	sf_close(audio_file);

	return SmoothedArray;
}