#include <iostream>
#include "SoundManager.h"
#include "FileManager.h"
#include <sndfile.h>
#include <fstream>

using namespace std;

float map(float value, float istart, float istop, float ostart, float ostop) {
	/// >Function to remap values
}

int* SoundManager::read_samples(const char* FilePath, int Hz) {

	/// >Reading Data from audio file

		/// Open the audio file

		/// Throw error if file not loaded

		/// Allocate buffer for audio samples

		/// Throw error if buffer fails to declare

		/// Read audio samples into the buffer
		/// Buffer size cant be big enough -> Throws access violation error 
		/// Either buffer[] size too small or audio_file got too little frames
		/// 
		/// Ensure the read bytes are not NULL
		/// Loop through each sample
			/// Find largest
			/// Find smallest
			/// Transfer valid samples to an array
		/// Find Pitch
		/// Find Loudness
		/// etc..
		/// 
		/// > Free Data
		/// Return Array
}