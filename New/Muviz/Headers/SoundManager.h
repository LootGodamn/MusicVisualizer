#pragma once
#include <sndfile.h>

class SoundManager {
public:
	int readsamples(const char* FilePath, float** TargetArray, int ScreenW, int ScreenH, int fps, float* largest, float* smallest);
};