#pragma once
#include <sndfile.h>

class SoundManager {
public:
	int readsamples(const char* FilePath, float* TargetArray, int ScreenW, int ScreenH, int fps, float* largest, float* smallest);
	int* findpitch(const char* FilePath, int Hz);
	int* findloudness(const char* FilePath, int Hz);
	int* freedata();
};